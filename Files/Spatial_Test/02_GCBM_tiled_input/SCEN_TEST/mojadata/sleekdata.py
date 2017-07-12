import os, gdal, json, glob
from gdalconst import *
from sleek import Resample, Reproject, CreateMask, RasterizeLayer, CreateRasterTiles, Tile, CreateRasterStack
import datetime

KenyaTiles = [
                                        Tile( 34,  5 ), Tile( 35,  5 ),
                        Tile( 33,  4 ), Tile( 34,  4 ), Tile( 35,  4 ), Tile( 36,  4 ), Tile( 37,  4 ),                                 Tile( 40,  4 ), Tile( 41,  4 ),
                                        Tile( 34,  3 ), Tile( 35,  3 ), Tile( 36,  3 ), Tile( 37,  3 ), Tile( 38,  3 ), Tile( 39,  3 ), Tile( 40,  3 ), Tile( 41,  3 ),
                                        Tile( 34,  2 ), Tile( 35,  2 ), Tile( 36,  2 ), Tile( 37,  2 ), Tile( 38,  2 ), Tile( 39,  2 ), Tile( 40,  2 ), Tile( 41,  2 ),
                                        Tile( 34,  1 ), Tile( 35,  1 ), Tile( 36,  1 ), Tile( 37,  1 ), Tile( 38,  1 ), Tile( 39,  1 ), Tile( 40,  1 ),
                        Tile( 33,  0 ), Tile( 34,  0 ), Tile( 35,  0 ), Tile( 36,  0 ), Tile( 37,  0 ), Tile( 38,  0 ), Tile( 39,  0 ), Tile( 40,  0 ),
                        Tile( 33, -1 ), Tile( 34, -1 ), Tile( 35, -1 ), Tile( 36, -1 ), Tile( 37, -1 ), Tile( 38, -1 ), Tile( 39, -1 ), Tile( 40, -1 ), Tile( 41, -1 ),
                        Tile( 33, -2 ), Tile( 34, -2 ), Tile( 35, -2 ), Tile( 36, -2 ), Tile( 37, -2 ), Tile( 38, -2 ), Tile( 39, -2 ), Tile( 40, -2 ), Tile( 41, -2 ),
                                                        Tile( 35, -3 ), Tile( 36, -3 ), Tile( 37, -3 ), Tile( 38, -3 ), Tile( 39, -3 ), Tile( 40, -3 ), Tile( 41, -3 ),
                                                                                        Tile( 37, -4 ), Tile( 38, -4 ), Tile( 39, -4 ), Tile( 40, -4 ),
                                                                                                        Tile( 38, -5 ), Tile( 39, -5 )        ]


def CreateCountryMask(inFn, outFn, pixelSize):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    driver = gdal.GetDriverByName ( "GTiff" )
    mask = CreateMask(inFn, pixelSize, 33.0, -5.0, 42.0, 6.0)
    mask_ds = driver.CreateCopy( outFn, mask, 0, ['TILED=YES', 'COMPRESS=DEFLATE'] )
    mask_ds = None 

def CreateLandCoverLayer(input_folder, layers, output_folder, layer_name):

    output_folder = os.path.join(output_folder, layer_name)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    rasters = list()
    for layer in layers:
        inFn = os.path.join(input_folder, layer['file'])
        basename = os.path.splitext(layer['file'])[0] 
        raster = Resample(inFn, gdal.GDT_Byte, 33.0, -5.0, 42.0, 6.0) 
        rasters.append(raster)
        driver = gdal.GetDriverByName ( "GTiff" )
        dst_ds = driver.CreateCopy(os.path.join(output_folder,basename+".tiff"), raster, 0, ['TILED=YES', 'COMPRESS=DEFLATE'] )
        dst_ds = None

    CreateRasterStack(output_folder, rasters, layer_name, KenyaTiles)

def CreateCountyLayer(inFn, outFn):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    sourceDs = Reproject(inFn)
    srcLayer = sourceDs.GetLayerByIndex(0)

    srcFeature = srcLayer.GetNextFeature()
    id = 1;
    while srcFeature:
        srcFeature.SetField("const_no", id);
        srcLayer.SetFeature(srcFeature);
        id=id+1
        srcFeature = srcLayer.GetNextFeature()

    RasterizeLayer(outFn, srcLayer, 'const_no', gdal.GDT_Byte, 0.00025, 255, 33.0, -5.0, 42.0, 6.0)
    CreateRasterTiles(outFn, KenyaTiles)

def CreateAEZLayer(inFn, outFn):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    sourceDs = Reproject(inFn)
    srcLayer = sourceDs.GetLayerByIndex(0)

    RasterizeLayer(outFn, srcLayer, 'MZONE', gdal.GDT_Byte, 0.00025, 255, 33.0, -5.0, 42.0, 6.0)
    CreateRasterTiles(outFn, KenyaTiles)

def CreateSoilLayer(inFn, outFn):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    sourceDs = Reproject(inFn)
    srcLayer = sourceDs.GetLayerByIndex(0)

    RasterizeLayer(outFn, srcLayer, 'SUID', gdal.GDT_Int16, 0.00025, 32767, 33.0, -5.0, 42.0, 6.0)
    CreateRasterTiles(outFn, KenyaTiles)
    
def CreatePlantationLayer(inFn, outFn):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    sourceDs = Reproject(inFn)
    srcLayer = sourceDs.GetLayerByIndex(0)

    RasterizeLayer(outFn, srcLayer, 'OBJECTID', gdal.GDT_Int16, 0.00025, 32767, 33.0, -5.0, 42.0, 6.0)
    CreateRasterTiles(outFn, KenyaTiles)


def CreateTempLayer(raster_name, inFolder, fileNames, output_folder):

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    raster_files = sorted(glob.glob(os.path.join(inFolder,fileNames)), key=os.path.basename)
    rasters = list()
    for raster_file in raster_files:
        print('Resample {0}'.format(raster_file))
        raster = Resample(raster_file, gdal.GDT_Float32, 33.0, -5.0, 42.0, 6.0, 0.05) 
        rasters.append(raster)

    CreateRasterStack(output_folder, rasters, raster_name, KenyaTiles)

def WriteGridLayerInfo(output_folder, layer_name, cellSize, nodata):
    layerInfo = {
        'layer_type': 'GridLayer',
        'layer_prefix': layer_name,
        'tileLatSize': 1.0,
        'tileLonSize': 1.0,
        'blockLatSize': 0.1,
        'blockLonSize': 0.1,
        'cellLatSize': cellSize,
        'cellLonSize': cellSize,
        'nodata': nodata
    }
    with open(os.path.join(output_folder,layer_name+'.json'), 'w') as f:
        json.dump(layerInfo, f, ensure_ascii=False)

def CreateDEMLayer(inFn, outFn):
    output_folder = os.path.dirname(outFn)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    driver = gdal.GetDriverByName ( "GTiff" )
    raster = Resample(inFn, gdal.GDT_Int16, 33.0, -5.0, 42.0, 6.0)
    
    dst_ds = driver.CreateCopy( outFn, raster, 0, ['TILED=YES', 'COMPRESS=DEFLATE'])
    dst_ds = None
    CreateRasterTiles(outFn, KenyaTiles)
    raster_name = os.path.splitext(os.path.basename(outFn))[0]
    output_folder = os.path.join(os.path.dirname(outFn),raster_name)
    WriteGridLayerInfo(output_folder, raster_name, 0.00025, -32768)

if __name__ == "__main__":

    srcRoot = 'G:/Base Data'
    dstRoot = 'S:/Flint_Run/Spatial'
    
    CreateTempLayer('evap', os.path.join(srcRoot,'Local/Raw Data/Climate/Dailyclimategrids_KMD_V2/evap'), "evap20*.grd", os.path.join(dstRoot,'evap'))
    CreateTempLayer('maxt', os.path.join(srcRoot,'Local/Raw Data/Climate/Dailyclimategrids_KMD_V2/maxt'), "maxt20*.grd", os.path.join(dstRoot,'maxt'))
    CreateTempLayer('mint', os.path.join(srcRoot,'Local/Raw Data/Climate/Dailyclimategrids_KMD_V2/mint'), "mint20*.grd", os.path.join(dstRoot,'mint'))
    CreateTempLayer('rain', os.path.join(srcRoot,'Local/Raw Data/Climate/Dailyclimategrids_KMD_V2/rain'), "rain20*.grd", os.path.join(dstRoot,'rain'))
    CreateTempLayer('srad', os.path.join(srcRoot,'Local/Raw Data/Climate/Dailyclimategrids_KMD_V2/srad'), "srad20*.grd", os.path.join(dstRoot,'srad'))

    CreateDEMLayer(os.path.join(srcRoot,"Local\Raw Data\DEM's\KenyaDEM30m\KenyaDEM30m_DRSRS_V1.tif"), os.path.join(dstRoot,'KenyaDEM30m_DRSRS_V1.tiff'))

    #CreateTempLayer('Tmin', os.path.join(srcRoot,'Local/Raw Data/Climate/Tmin'), os.path.join(dstRoot,'Tmin'))
    #CreateTempLayer('Tmax', os.path.join(srcRoot,'Local/Raw Data/Climate/Tmax'), os.path.join(dstRoot,'Tmax'))

    CreateCountryMask(os.path.join(srcRoot,'Local/Raw Data/Admin Boundaries/Counties/KenyaCounties_SOK_V1.shp'), os.path.join(dstRoot,'KenyaCounties_SOK_V1_mask5k.tiff'), 0.05)
    CreateCountryMask(os.path.join(srcRoot,'Local/Raw Data/Admin Boundaries/Counties/KenyaCounties_SOK_V1.shp'), os.path.join(dstRoot,'KenyaCounties_SOK_V1_mask1k.tiff'), 0.01)

    CreateCountyLayer(os.path.join(srcRoot,'Local/Raw Data/Admin Boundaries/Counties/KenyaCounties_SOK_V1.shp'), os.path.join(dstRoot,'KenyaCounties_SOK_V1.tiff'))

    CreateAEZLayer(os.path.join(srcRoot,'Local/Derived Data/Other/Agro-Ecological Zones/Kenya_aez_dd_drsrs_V2_WGS.shp'), os.path.join(dstRoot,'Kenya_aez_dd_drsrs_V2_WGS.tiff'))

    CreateSoilLayer(os.path.join(srcRoot,'Local/Raw Data/Soil/Soil Organic Carbon/Soilcarbon_KALRO_V2/Carbon stock 2002.shp'), os.path.join(dstRoot,'Soilcarbon_KALRO_V2.tiff'))
    CreatePlantationLayer(os.path.join(srcRoot,'Local/Raw Data/Forest/Plantationsdata_KFS_V4/Sleek_Plantation_Submitted_1_CCI_Corrected_WGS.shp'), os.path.join(dstRoot,'Plantationsdata_KFS_V4.tiff'))

    #landcoverlayers = [
    #                   {'file': 'KLC2006_DRSRS_V1.img', 'date': datetime.date(2006,12,31)},
    #                   {'file': 'KLC2008_DRSRS_V1.img', 'date': datetime.date(2008,12,31)},
    #                   {'file': 'KLC2010_DRSRS_V2.img', 'date': datetime.date(2010,12,31)},
    #                   {'file': 'KLC2012_DRSRS_V1.img', 'date': datetime.date(2012,12,31)},
    #                   {'file': 'klc2014_drsrs_v2.img', 'date': datetime.date(2014,12,31)}]
    #CreateLandCoverLayer(os.path.join(srcRoot,'Local/Raw Data/Land Cover/Kenya Land Cover'),
    #                     landcoverlayers,
    #                     dstRoot,
    #                     'KLC_DRSRS')
    landcoverlayers = [
                       {'file': 'CLASS2006v01_WGS84.tif', 'date': datetime.date(2006,12,31)},
                       {'file': 'CLASS2008v01_WGS84.tif', 'date': datetime.date(2008,12,31)},
                       {'file': 'CLASS2010v01_WGS84.tif', 'date': datetime.date(2010,12,31)},
                       {'file': 'CLASS2012v01_WGS84.tif', 'date': datetime.date(2012,12,31)},
                       {'file': 'CLASS2014v01_WGS84.tif', 'date': datetime.date(2014,12,31)}]
    CreateLandCoverLayer('S:/output2SLEEK',
                         landcoverlayers,
                         dstRoot,
                         'CLASSv01_WGS84')
