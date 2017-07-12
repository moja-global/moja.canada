from mojadata.boundingbox import BoundingBox
from mojadata.tiler2d import Tiler2D
from mojadata.tiler3d import Tiler3D
from mojadata.layer.gdblayer import GDBLayer
from mojadata.layer.vectorlayer import VectorLayer
from mojadata.layer.rasterlayer import RasterLayer
from mojadata.cleanup import cleanup
from mojadata.layer.layerstack import LayerStack

import glob
import os
import gdal
import csv
import codecs

def scan_for_layers(path, filter):
    return sorted(glob.glob(os.path.join(path, filter)),
                  key=os.path.basename)

def unicode_csv_reader(utf8_data, dialect=csv.excel, **kwargs):
    csv_reader = csv.DictReader(utf8_data, dialect=dialect, **kwargs)
    for row in csv_reader:
        yield {k: unicode(v, 'utf-8') for k, v in row.iteritems()}

if __name__ == "__main__":
    with cleanup():
        bbox = BoundingBox(RasterLayer(r"D:\OS_PFC\rasters\forest_age.tif"))
        tiler = Tiler2D(bbox)

        layers = [
            RasterLayer(r"D:\OS_PFC\rasters\forest_age.tif"),
            RasterLayer(r"D:\OS_PFC\rasters\stratum.tif",
                        attributes=["Stratum"],
                        attribute_table={
                              0: ["Not stocked"],
                            123: ["Aw_comp"],
                              4: ["Aw_SO"],
                              5: ["Aw_SCS"],
                              6: ["Aw_SCN"],
                              7: ["Aw_Pj"],
                              8: ["AwS_S"],
                              9: ["AwS_N"],
                             10: ["MxPj"],
                             11: ["SAw-S"],
                             12: ["SAw-N"],
                             13: ["SwO"],
                             14: ["SwCFM"],
                             15: ["SwCG"],
                             16: ["SbO"],
                             17: ["SbCFM"],
                             18: ["SbCG"],
                            192: ["PjO-CFM"],
                             21: ["PjCG"],
                             22: ["AwUFM"],
                             23: ["AwUG"],
                             24: ["AwSU-s"],
                             25: ["AwSU-n"],
                              2: ["Comp-soft"],
                              1: ["Comp-hard"],
                        }),
            RasterLayer(r"D:\OS_PFC\rasters\new_baseline.tif"),
        ]
        
        for dist_layer_path in scan_for_layers(r"D:\OS_PFC\rasters\disturbances", "*.tif"):
            year = os.path.basename(dist_layer_path)[:4]
            attribute_table = open("disturbance_type.csv", "rb")
            reader = csv.DictReader(attribute_table)
            layer_attributes = {row["value"]: [row["disturbance_type"], row["year"].format(year=year)]
                                for row in reader}
            
            layers.append(RasterLayer(dist_layer_path,
                                      attributes=["disturbance_type", "year"],
                                      attribute_table=layer_attributes))

        tiler.tile(layers)
