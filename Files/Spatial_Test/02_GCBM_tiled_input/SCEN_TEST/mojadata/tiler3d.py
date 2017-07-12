import gdal
import os
import simplejson as json
import numpy as np
from gdalconst import *

class Tiler3D(object):

    def __init__(self, bounding_box, tile_extent=1.0, block_extent=0.1):
        self._bounding_box = bounding_box
        self._tile_extent = tile_extent
        self._block_extent = block_extent

    def tile(self, stacks):
        working_dir = os.path.abspath(os.curdir)
        for stack in stacks:
            print "Processing stack: {}".format(stack.name)
            output_folder = os.path.join(stack.name)
            if not os.path.exists(output_folder):
                os.makedirs(output_folder)

            os.chdir(output_folder)

            rasters = []
            first_stack_layer = None
            for i, layer in enumerate(stack.layers):
                print "  Processing layer: {}".format(layer.name)
                layer = self._bounding_box.normalize(
                    layer,
                    self._block_extent,
                    stack.requested_pixel_size,
                    stack.data_type)

                rasters.append(gdal.Open(layer.path, GA_ReadOnly))

                if i == 0:
                    first_stack_layer = layer

            for tile in first_stack_layer.tiles(self._tile_extent, self._block_extent):
                print "  Processing tile: {}".format(tile.name)
                out_path = "{}_{}.blk".format(stack.name, tile.name)

                with open(out_path, "wb") as blocked_file:
                    for block in tile.blocks:
                        block_data = []
                        for raster in rasters:
                            band = raster.GetRasterBand(1)
                            block_data.append(band.ReadAsArray(
                                block.x_offset, block.y_offset,
                                block.x_size, block.y_size))

                        block = np.stack(block_data, -1)
                        b = str(bytearray(block))
                        blocked_file.write(b)

            metadata_path = "{}.json".format(stack.name)
            self._write_metadata(stack, first_stack_layer, metadata_path)
            os.chdir(working_dir)

    def _write_metadata(self, stack, layer, path):
        info = gdal.Info(layer.path, format="json")
        pixel_size = abs(info["geoTransform"][1])

        metadata = {
            "layer_type"   : "StackLayer",
            "layer_data"   : layer.data_type,
            "nLayers"      : stack.years,
            "nStepsPerYear": stack.steps_per_year,
            "nodata"       : layer.nodata_value,
            "tileLatSize"  : self._tile_extent,
            "tileLonSize"  : self._tile_extent,
            "blockLatSize" : self._block_extent,
            "blockLonSize" : self._block_extent,
            "cellLatSize"  : pixel_size,
            "cellLonSize"  : pixel_size
        }

        if layer.attribute_table:
            attributes = {}
            for attr_values, pixel_value in layer.attribute_table.iteritems():
                if len(attr_values) == 1:
                    attributes[pixel_value] = attr_values[0]
                else:
                    attributes[pixel_value] = dict(zip(layer.attributes, attr_values))

            metadata["attributes"] = attributes

        with io.open(path, "w", encoding="utf8") as out_file:
            out_file.write(json.dumps(metadata, out_file, indent=4, ensure_ascii=False))
