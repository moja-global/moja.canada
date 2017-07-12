import logging
import os, gdal, osr, ogr
from gdalconst import *
import numpy as np

__version__ = "0.1.0"

log = logging.getLogger('mojadata')

# register all of the drivers 
gdal.AllRegister()

class NullHandler(logging.Handler):
    def emit(self, record):
        pass
log.addHandler(NullHandler())

class Tile(object):

    def __init__(self, X, Y): 
        self.Xmin = X
        self.Xmax = X + 1
        self.Ymin = Y
        self.Ymax = Y + 1
        self.Name = "{0}{1:03d}_{2}{3:03d}".format('-' if self.Xmin<0  else '', abs(self.Xmin), '-' if self.Ymax<0  else '', abs(self.Ymax))


def warp_progress(pct, message, user_data):
  print ("{0} {1}".format(user_data, pct*100))
  return 1

def Resample(inFn, datatype, xMin, yMin, xMax, yMax, pixelSize = 0.00025, epsg = 4326):
    # https://jgomezdans.github.io/gdal_notes/reprojection.html
    # input WKT
    srcDataSource = gdal.Open(inFn)

    dstSrs = osr.SpatialReference()
    dstSrs.ImportFromEPSG(epsg)
    
    # Get the Geotransform vector
    geo_t = srcDataSource.GetGeoTransform()
    x_size = srcDataSource.RasterXSize
    y_size = srcDataSource.RasterYSize
    srcWkt = srcDataSource.GetProjection()
    if srcWkt:
        srcSrs = osr.SpatialReference()
        srcSrs.ImportFromWkt(srcWkt)

        tx = osr.CoordinateTransformation(srcSrs, dstSrs)

        # Work out the boundaries of the new dataset in the target projection
        (ulx, uly, ulz ) = tx.TransformPoint( geo_t[0], geo_t[3])
        (lrx, lry, lrz ) = tx.TransformPoint( geo_t[0] + geo_t[1]*x_size, geo_t[3] + geo_t[5]*y_size )

        # pixel alignment and bounds
        ulx = int(ulx/pixelSize)*pixelSize 
        uly = int(uly/pixelSize)*pixelSize 
        lrx = int(lrx/pixelSize)*pixelSize 
        lry = int(lry/pixelSize)*pixelSize 
    else:
        srcSrs = dstSrs

    #if ulx < xMin: ulx = xMin
    #if uly > yMax: uly = yMax
    #if lrx > xMax: lrx = xMax
    #if lry < yMin: lry = yMin
    ulx = xMin
    uly = yMax
    lrx = xMax
    lry = yMin

    # Calculate the new geotransform
    new_geo = ( ulx, pixelSize, geo_t[2], uly, geo_t[4], -pixelSize )

    mem_drv = gdal.GetDriverByName( 'MEM' )
    # The size of the raster is given the new projection and pixel spacing
    # Using the values we calculated above. Also, setting it to store one band
    # and to use Byte data type.
    dstDataSource = mem_drv.Create('', int((lrx - ulx)/pixelSize), int((uly - lry)/pixelSize), 1, datatype)
    # Set the geotransform
    dstDataSource.SetGeoTransform( new_geo )
    dstDataSource.SetProjection ( dstSrs.ExportToWkt() )
    # Perform the projection/resampling 
    returnVal = gdal.ReprojectImage( srcDataSource, dstDataSource,  srcSrs.ExportToWkt(), dstSrs.ExportToWkt(),  gdal.GRA_Bilinear, 9000.0,0.0, warp_progress, inFn)#, options = ['NUM_THREADS = ALL_CPUS'])

    srcDataSource = None
    return dstDataSource

def Reproject(inFn, epsg = 4326):
    srcDataSource = ogr.Open(inFn, 0)
    srcLayer = srcDataSource.GetLayerByIndex(0)
    srcSrs = srcLayer.GetSpatialRef()
    dstSrs = osr.SpatialReference()
    dstSrs.ImportFromEPSG(epsg)

    if (srcSrs == dstSrs):
        return ogr.GetDriverByName("Memory").CopyDataSource(srcDataSource, "", {})

    # create the CoordinateTransformation
    coordTrans = osr.CoordinateTransformation(srcSrs, dstSrs)

    dstDataSource = ogr.GetDriverByName("MEMORY").CreateDataSource('memData');
    dstLayer = dstDataSource.CreateLayer("reprojected", dstSrs, geom_type=ogr.wkbMultiPolygon)

    # add fields
    srcLayerDefn = srcLayer.GetLayerDefn()
    for i in range(0, srcLayerDefn.GetFieldCount()):
        fieldDefn = srcLayerDefn.GetFieldDefn(i)
        dstLayer.CreateField(fieldDefn)

    # get the output layer's feature definition
    dstLayerDefn = dstLayer.GetLayerDefn()

    # loop through the input features
    srcFeature = srcLayer.GetNextFeature()
    while srcFeature:
        # get the input geometry
        geom = srcFeature.GetGeometryRef()
        # reproject the geometry
        geom.Transform(coordTrans)
        # create a new feature
        dstFeature = ogr.Feature(dstLayerDefn)
        # set the geometry and attribute
        dstFeature.SetGeometry(geom)
        for i in range(0, dstLayerDefn.GetFieldCount()):
            dstFeature.SetField(dstLayerDefn.GetFieldDefn(i).GetNameRef(), srcFeature.GetField(i))
        # add the feature
        dstLayer.CreateFeature(dstFeature)
        # destroy the features and get the next input feature
        dstFeature.Destroy()
        srcFeature.Destroy()
        srcFeature = srcLayer.GetNextFeature()
    srcLayer = None
    srcDataSource = None
    return dstDataSource

def CreateMask(inFn, pixelSize, xMin, yMin, xMax, yMax, epsg = 4326):
    srcDataSource = Reproject(inFn)
    srcLayer = srcDataSource.GetLayerByIndex(0)
    xRes = int((xMax - xMin)/pixelSize)
    yRes = int((yMax - yMin)/pixelSize)

    mem_drv = gdal.GetDriverByName( 'MEM' )
    dstDataSource = mem_drv.Create('', xRes, yRes, 1, gdal.GDT_Byte)

    dstSrs = osr.SpatialReference()
    dstSrs.ImportFromEPSG(epsg)

    dstDataSource.SetProjection(dstSrs.ExportToWkt())

    dstDataSource.SetGeoTransform((xMin, pixelSize, 0, yMax, 0, -pixelSize))
    band = dstDataSource.GetRasterBand(1)
    band.SetNoDataValue(0)

    # Rasterize
    gdal.RasterizeLayer(dstDataSource, [1], srcLayer, 
                        burn_values=[1], 
                        options = ['ALL_TOUCHED=TRUE'])

    return dstDataSource

def RasterizeLayer(outFn, srcLayer, attribute, datatype, pixelSize, nodata, xMin, yMin, xMax, yMax, epsg = 4326):
    xRes = int((xMax - xMin)/pixelSize)
    yRes = int((yMax - yMin)/pixelSize)
    driver = gdal.GetDriverByName ( "GTiff" )
    dstDataSource = driver.Create(outFn, xRes, yRes, 1, datatype, [ 'TILED=YES', 'COMPRESS=DEFLATE' ])
    dstSrs = osr.SpatialReference()
    dstSrs.ImportFromEPSG(epsg)
    dstDataSource.SetProjection(dstSrs.ExportToWkt())
    dstDataSource.SetGeoTransform((xMin, pixelSize, 0.0, yMax, 0.0, -pixelSize))
    dstDataSource.GetRasterBand(1).SetNoDataValue(nodata);

    # Rasterize
    err = gdal.RasterizeLayer(dstDataSource, [1], srcLayer, options = [ 'ATTRIBUTE=%s' % attribute ])

    if (err != 0):
        raise RuntimeError("error rasterizing layer: %s" % err)
    #flush rasters cache
    dstDataSource.FlushCache()

def CreateRasterTiles(raster_file, tiles):
    raster_name = os.path.splitext(os.path.basename(raster_file))[0]
    output_folder = os.path.join(os.path.dirname(raster_file),raster_name)
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    ds = gdal.Open(raster_file, GA_ReadOnly)
    # get image size 
    rows = ds.RasterYSize 
    cols = ds.RasterXSize 

    transform = ds.GetGeoTransform() 
    xOrigin = transform[0] 
    yOrigin = transform[3] 
    pixelWidth = transform[1] 
    pixelHeight = transform[5]
    xTileSize = int(1.0/pixelWidth)
    yTileSize = int(1.0/abs(pixelHeight))
    xBlockSize = int(0.1/pixelWidth)
    yBlockSize = int(0.1/abs(pixelHeight))

    for tile in tiles:
        # Tile offset from top left
        xOffset = int((tile.Xmin -xOrigin) / pixelWidth) 
        yOffset = int((tile.Ymax -yOrigin) / pixelHeight)

        band = ds.GetRasterBand(1) 
        blockedFile = open (os.path.join(output_folder, '{0}_{1}.blk'.format(raster_name, tile.Name)), "wb")

        for row in range(yOffset, yOffset + yTileSize, yBlockSize):
            for col in range(xOffset, xOffset + xTileSize, xBlockSize): 
                data = band.ReadAsArray(col, row, xBlockSize, yBlockSize)

                b = bytes(data)
                blockedFile.write(b)

def CreateRasterStack(output_folder, rasters, raster_name, tiles):
    transform = rasters[0].GetGeoTransform() 

    xOrigin = transform[0] 
    yOrigin = transform[3] 
    pixelWidth = transform[1] 
    pixelHeight = transform[5]
    xTileSize = int(1.0/pixelWidth)
    yTileSize = int(1.0/abs(pixelHeight))
    xBlockSize = int(0.1/pixelWidth)
    yBlockSize = int(0.1/abs(pixelHeight))

    for tile in tiles:
        # Tile offset from top left
        xOffset = int((tile.Xmin -xOrigin) / pixelWidth) 
        yOffset = int((tile.Ymax -yOrigin) / pixelHeight)

        blockedFileName = '{0}_{1}.blk'.format(raster_name, tile.Name)
        blockedFile = open(os.path.join(output_folder, blockedFileName), "wb")
        print(blockedFileName)
        for row in range(yOffset, yOffset + yTileSize, yBlockSize):
            for col in range(xOffset, xOffset + xTileSize, xBlockSize): 
                blocklayers = list()
                for raster in rasters:
                    band = raster.GetRasterBand(1) 
                    blocklayer = band.ReadAsArray(col, row, xBlockSize, yBlockSize)
                    #print(blocklayer)
                    blocklayers.append(blocklayer)
                block = np.stack(blocklayers,-1)
                b = bytes(block)
                blockedFile.write(b)

