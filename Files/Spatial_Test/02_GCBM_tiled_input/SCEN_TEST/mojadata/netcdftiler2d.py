import csv
import gdal
import os
from gdalconst import *

class NetCDFTiler2D(object):

    def __init__(self, bounding_box):
        self._bounding_box = bounding_box

    def tile(self, layers):
        for layer in layers:
            print "Processing layer: {}".format(layer.path)
            self._bounding_box.normalize(layer)
            raster_name, _ = os.path.splitext(os.path.basename(layer.path))
            output_folder = os.path.join(os.path.dirname(layer.path), raster_name)
            if not os.path.exists(output_folder):
                os.makedirs(output_folder)

            attribute_table_path = os.path.join(output_folder, "{}.csv".format(raster_name))
            self._write_attribute_table(layer, attribute_table_path)

            ds = gdal.Open(layer.path, GA_ReadOnly)
            for tile in layer.tiles:
                print "Processing tile: {}".format(tile.name)
                band = ds.GetRasterBand(1)
                out_path = os.path.join(
                    output_folder,
                    "{}_{}.nc".format(raster_name, tile.name))

                with open(out_path, "wb") as blocked_file:
                    for block in tile.blocks:
                        data = band.ReadAsArray(block.x_offset, block.y_offset,
                                                block.x_size, block.y_size)
                        b = str(bytearray(data))
                        blocked_file.write(b)

    def _write_attribute_table(self, layer, path):
        with open(path, "wb") as attr_fh:
            writer = csv.writer(attr_fh)
            writer.writerows(((pixel_value, attr_value)
                              for attr_value, pixel_value
                              in layer.attribute_table.iteritems()))
