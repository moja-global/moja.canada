import os
import gdal
import osr
import math
from gdalconst import *
from mojadata import cleanup
import tempfile

class BoundingBox(object):

    def __init__(self, layer, epsg=4326, pixel_size=0.00025):
        self._layer = layer
        self._processBoundingBox(epsg, pixel_size)
        bbox = gdal.Open(self._layer.path)
        self._pixel_size = pixel_size
        self._x_size = bbox.RasterXSize
        self._y_size = bbox.RasterYSize
        self._srs = bbox.GetProjection()
        self._info = gdal.Info(bbox, format="json")

    @property
    def pixel_size(self):
        return self._pixel_size

    def normalize(self, layer, block_extent, requested_pixel_size=None, data_type=None):
        bounds=(self._info["cornerCoordinates"]["upperLeft"][0],
                self._info["cornerCoordinates"]["lowerRight"][1],
                self._info["cornerCoordinates"]["lowerRight"][0],
                self._info["cornerCoordinates"]["upperLeft"][1])
                                    
        layer = layer.as_raster_layer(self._srs,
                                      self._pixel_size,
                                      block_extent,
                                      requested_pixel_size,
                                      data_type,
                                      bounds)

        layer_path, ext = os.path.splitext(layer.path)
        tmp_path = "{}_tmp{}".format(layer_path, ext)
        cleanup.register_temp_file(tmp_path)
        self._warp(layer.path, tmp_path, layer.pixel_size)

        output_path = "{}_moja.tiff".format(os.path.basename(layer_path))
        self._pad(tmp_path, output_path, layer.pixel_size)
        layer.path = output_path

        return layer

    def _processBoundingBox(self, epsg, pixel_size):
        dest_srs = osr.SpatialReference()
        dest_srs.ImportFromEPSG(epsg)
        self._layer = self._layer.as_raster_layer(dest_srs, pixel_size, 0.1)

    def _pad(self, in_path, out_path, pixel_size):
        bounds = gdal.Info(in_path, format="json")["cornerCoordinates"]
        gdal.Warp(out_path, in_path,
                  multithread=True,
                  xRes=pixel_size, yRes=pixel_size,
                  outputBounds=(math.floor(bounds["upperLeft"][0]),
                                math.floor(bounds["lowerRight"][1]),
                                math.ceil(bounds["lowerRight"][0]),
                                math.ceil(bounds["upperLeft"][1])),
                  creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"])

    def _warp(self, in_path, out_path, pixel_size):
        gdal.Warp(out_path, in_path,
                  dstSRS=self._srs,
                  xRes=pixel_size, yRes=pixel_size,
                  multithread=True,
                  outputBounds=(self._info["cornerCoordinates"]["upperLeft"][0],
                                self._info["cornerCoordinates"]["lowerRight"][1],
                                self._info["cornerCoordinates"]["lowerRight"][0],
                                self._info["cornerCoordinates"]["upperLeft"][1]),
                  targetAlignedPixels=True,
                  creationOptions=["COMPRESS=DEFLATE", "BIGTIFF=YES"])
