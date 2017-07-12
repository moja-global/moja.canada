import gdal
import math
from math import pow
from mojadata.tile import Tile
from gdalconst import *

class Layer(object):

    byte_range   = (0, pow(2, 8) - 1)
    int16_range  = (-pow(2, 16) / 2, pow(2, 16) / 2 - 1)
    uint16_range = (0, pow(2, 16) - 1)
    int32_range  = (-pow(2, 32) / 2, pow(2, 32) / 2 - 1)
    uint32_range = (0, pow(2, 32) - 1)

    @property
    def name(self):
        raise NotImplementedError()
    
    @property
    def path(self):
        raise NotImplementedError()

    @path.setter
    def path(self, value):
        raise NotImplementedError()

    @property
    def attributes(self):
        raise NotImplementedError()

    @property
    def attribute_table(self):
        raise NotImplementedError()

    @property
    def pixel_size(self):
        info = gdal.Info(self._path, format="json")
        return abs(info["geoTransform"][1])

    @property
    def data_type(self):
        info = gdal.Info(self._path, format="json")
        return info["bands"][0]["type"]

    @property
    def nodata_value(self):
        info = gdal.Info(self._path, format="json")
        value = info["bands"][0]["noDataValue"]
        dt = str(self.data_type).lower()
        if dt == "float32" or dt == "float" or dt == str(gdal.GDT_Float32):
            return float(value)
        else:
            return int(value)

    '''
    Rasterizes a layer with specified settings.

    :param srs: The destination projection
    :type srs: :class:`.osr.SpatialReference`
    :param min_pixel_size: The minimum pixel size, in units specified by :param srs:
    :param requested_pixel_size: [optional] The requested pixel size; the size
        actually used will be the next closest pixel size divisible by
        :param min_pixel_size:
    :param data_type: [optional] The data type to use; auto-detected if unspecified
    :param nodata_value: [optional] The nodata value to use; -1 if unspecified
    '''
    def as_raster_layer(self, srs, min_pixel_size, block_extent,
                        requested_pixel_size=None, data_type=None,
                        bounds=None):
        raise NotImplementedError()

    def tiles(self, tile_extent, block_extent):
        ds = gdal.Open(self.path, GA_ReadOnly)
        info = gdal.Info(ds, format="json")
        transform = ds.GetGeoTransform()
        pixel_size = abs(transform[1])
        origin = (transform[0], transform[3])
        bounds = info["cornerCoordinates"]
        y_min = int(math.floor(bounds["lowerRight"][1]))
        y_max = int(math.ceil(bounds["upperLeft"][1]))
        x_min = int(math.floor(bounds["upperLeft"][0]))
        x_max = int(math.ceil(bounds["lowerRight"][0]))
        for x in xrange(x_min, x_max):
            for y in xrange(y_min, y_max):
                yield Tile(x, y, origin, pixel_size, tile_extent, block_extent)

    def best_fit_data_type(self, range):
        return gdal.GDT_Float32 if math.floor(range[0]) != math.ceil(range[0]) or math.floor(range[1]) != math.ceil(range[1]) \
            else gdal.GDT_Int16   if range[0] >= Layer.int16_range[0]  and range[1] <= Layer.int16_range[1]  \
            else gdal.GDT_UInt16  if range[0] >= Layer.uint16_range[0] and range[1] <= Layer.uint16_range[1] \
            else gdal.GDT_Int32   if range[0] >= Layer.int32_range[0]  and range[1] <= Layer.int32_range[1]  \
            else gdal.GDT_UInt32
