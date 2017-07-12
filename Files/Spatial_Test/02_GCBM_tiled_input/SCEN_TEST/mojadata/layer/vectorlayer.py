import os
from osgeo import ogr
from osgeo import gdal
from layer import Layer
from attribute import Attribute
from rasterlayer import RasterLayer
from gdalconst import *
from mojadata import cleanup
from exceptions import IOError

class VectorLayer(Layer):

    def __init__(self, name, path, attributes, raw=False, nodata_value=-1, data_type=None):
        self._data_type = data_type
        self._nodata_value = nodata_value
        self._name = name
        self._path = path
        self._raw = raw
        self._id_attribute = "value_id" if not raw else attributes.name
        self._attribute_table = {}
        self._attributes = [attributes] if isinstance(attributes, Attribute) \
                                        else attributes

        if not os.path.exists(path):
            raise IOError("File not found: {}".format(path))

    @property
    def name(self):
        return self._name

    @property
    def path(self):
        return self._path

    @path.setter
    def path(self, value):
        self._path = value

    @property
    def attributes(self):
        return [attr.db_name for attr in self._attributes]

    @property
    def attribute_table(self):
        return self._attribute_table

    def as_raster_layer(self, srs, min_pixel_size, block_extent,
                        requested_pixel_size=None, data_type=None,
                        bounds=None):
        tmp_dir = self._make_name()
        cleanup.register_temp_dir(tmp_dir)
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)

        reproj_path = os.path.join(tmp_dir, self._make_name(".shp"))
        gdal.VectorTranslate(reproj_path, self._path, dstSRS=srs, reproject=True)

        if not self._raw:
            self._build_attribute_table(reproj_path, self._nodata_value)

        tmp_raster_path = os.path.join(tmp_dir, self._make_name(".tmp.tiff"))
        gdal.Rasterize(tmp_raster_path, reproj_path,
                       xRes=min_pixel_size, yRes=min_pixel_size,
                       attribute=self._id_attribute,
                       noData=self._nodata_value,
                       creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"],
                       outputBounds=bounds)

        raster_path = os.path.join(tmp_dir, self._make_name(".tiff"))

        info = gdal.Info(tmp_raster_path, format="json")
        is_float = "Float" in info["bands"][0]["type"] if self._raw else False

        output_type = data_type if data_type is not None \
            else self._data_type if self._data_type is not None \
            else gdal.GDT_Float32 if is_float \
            else self.best_fit_data_type(self._get_min_max(tmp_raster_path))

        gdal.Translate(raster_path, tmp_raster_path, outputType=output_type,
                       creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"])

        return RasterLayer(raster_path, self.attributes, self._attribute_table)

    def _get_min_max(self, raster_path):
        info = gdal.Info(raster_path, format="json", computeMinMax=True)
        if not "computedMin" in info["bands"][0]:
            return (0, 0)

        return (info["bands"][0]["computedMin"], info["bands"][0]["computedMax"])

    def _make_name(self, ext=""):
        return "{}{}".format(self._name, ext)

    def _build_attribute_table(self, path, nodata_value):
        if not os.path.exists(path):
            raise IOError("File not found: {}".format(path))

        shapefile = ogr.Open(path, 1)
        layer = shapefile.GetLayer(0)
        layer.CreateField(ogr.FieldDefn(self._id_attribute, ogr.OFTInteger))
        next_value_id = 1
        for feature in self._get_features(layer):
            attribute_values = tuple(attr.sub(feature[attr.name])
                                     if attr.filter(feature[attr.name])
                                     else None
                                     for attr in self._attributes)

            value_id = nodata_value
            if None not in attribute_values:
                existing_key = [item[0] for item in self._attribute_table.items()
                                if item[1] == attribute_values]
                if existing_key:
                    value_id = existing_key[0]
                else:
                    value_id = next_value_id
                    self._attribute_table[value_id] = attribute_values
                    next_value_id += 1

            feature[self._id_attribute] = value_id
            layer.SetFeature(feature)

    def _get_features(self, layer):
        feature = layer.GetNextFeature()
        while feature:
            yield feature
            feature = layer.GetNextFeature()
