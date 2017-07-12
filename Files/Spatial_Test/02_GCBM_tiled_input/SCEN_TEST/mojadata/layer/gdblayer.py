import gdal
import ogr
import os
from layer import Layer
from rasterlayer import RasterLayer
from gdalconst import *
from mojadata import cleanup
from exceptions import IOError

class GDBLayer(Layer):

    def __init__(self, name, path, layer, attributes, raw=False, nodata_value=-1):
        self._nodata_value = nodata_value
        self._name = name
        self._path = path
        self._layer = layer
        self._raw = raw
        self._id_attribute = "value_id" if not raw else attributes
        self._attribute_table = {}
        self._attributes = [attributes] if isinstance(attributes, basestring) \
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
        return self._attributes

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
        gdal.VectorTranslate(reproj_path, self._path,
                             dstSRS=srs,
                             reproject=True,
                             layers=[self._layer])

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
        output_type = data_type or self.best_fit_data_type(self._get_min_max(tmp_raster_path))
        gdal.Translate(raster_path, tmp_raster_path, outputType=output_type,
                       creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"])

        return RasterLayer(raster_path, self._attributes, self._attribute_table)

    def _get_min_max(self, raster_path):
        info = gdal.Info(raster_path, format="json", computeMinMax=True)
        return (info["bands"][0]["computedMax"], info["bands"][0]["computedMax"])

    def _make_name(self, ext=""):
        return "{}{}".format(self._name, ext)

    def _build_attribute_table(self, path, nodata_value):
        shapefile = ogr.Open(path, 1)
        layer = shapefile.GetLayer(0)
        layer.CreateField(ogr.FieldDefn(self._id_attribute, ogr.OFTInteger))
        next_value_id = 1
        for feature in self._get_features(layer):
            attribute_values = tuple(feature[attribute] for attribute in self._attributes)
            value_id = nodata_value
            if None not in attribute_values:
                existing_key = [item[0] for item in self._attribute_table.items()
                                if item[1] == attribute_values]
                if existing_key:
                    value_id = existing_key[0]
                else:
                    value_id = next_value_id
                    self._attribute_table[attribute_values] = value_id
                    next_value_id += 1

            feature[self._id_attribute] = value_id
            layer.SetFeature(feature)

    def _get_features(self, layer):
        feature = layer.GetNextFeature()
        while feature:
            yield feature
            feature = layer.GetNextFeature()
