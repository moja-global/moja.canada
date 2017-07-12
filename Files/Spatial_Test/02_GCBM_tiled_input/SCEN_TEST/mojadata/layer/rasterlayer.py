import os
import gdal
from gdalconst import *
from layer import Layer
from mojadata import cleanup
from exceptions import IOError

class RasterLayer(Layer):

    def __init__(self, path, attributes=None, attribute_table=None, nodata_value=-1, data_type=None):
        self._name = os.path.basename(path)
        self._path = path
        self._attributes = attributes or []
        self._attribute_table = attribute_table or {}
        self._nodata_value = nodata_value
        self._data_type = data_type

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
        tmp_dir, _ = os.path.splitext(self._name)
        cleanup.register_temp_dir(tmp_dir)
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)

        warp_path = os.path.join(tmp_dir, "warp_{}".format(self._name))
        gdal.Warp(warp_path, self._path, dstSRS=srs,
                  xRes=requested_pixel_size, yRes=requested_pixel_size,
                  creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"], outputBounds=bounds)

        output_path = os.path.join(tmp_dir, self._name)
        is_float = "Float" in self.data_type
        output_type = data_type if data_type is not None \
            else self._data_type if self._data_type is not None \
            else gdal.GDT_Float32 if is_float \
            else self.best_fit_data_type(self._get_min_max(warp_path))

        if self._nodata_value < 0:
            if output_type == gdal.GDT_Byte:
                self._nodata_value = Layer.byte_range[1]
            elif output_type == gdal.GDT_UInt16:
                self._nodata_value = Layer.uint16_range[1]
            elif output_type == gdal.GDT_UInt32:
                self._nodata_value = Layer.uint32_range[1]

        pixel_size = self._get_nearest_divisible_resolution(
            min_pixel_size, requested_pixel_size, block_extent) if requested_pixel_size \
            else self._get_nearest_divisible_resolution(
                min_pixel_size, self._get_native_pixel_size(warp_path), block_extent)

        gdal.Warp(output_path, warp_path,
                  multithread=True,
                  xRes=pixel_size, yRes=pixel_size,
                  outputType=output_type,
                  dstNodata=self._nodata_value,
                  creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"])

        return RasterLayer(output_path, self._attributes, self._attribute_table)

    def _get_native_pixel_size(self, raster_path):
        info = gdal.Info(raster_path, format="json")
        return abs(info["geoTransform"][1])

    def _get_nearest_divisible_resolution(self, min_pixel_size, requested_pixel_size, block_extent):
        nearest_block_divisible_size = \
            min_pixel_size * round(min_pixel_size / requested_pixel_size) \
            if requested_pixel_size > min_pixel_size \
            else min_pixel_size

        return nearest_block_divisible_size \
            if nearest_block_divisible_size < block_extent \
            else block_extent

    def _get_min_max(self, raster_path):
        info = gdal.Info(raster_path, format="json", computeMinMax=True)
        return (info["bands"][0]["computedMax"], info["bands"][0]["computedMax"])
