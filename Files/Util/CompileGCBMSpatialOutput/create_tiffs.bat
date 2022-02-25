@echo off

REM *************************** USER CONFIGURATION ***************************
REM Set simulation start year.
set SIMULATION_START_YEAR=2010

REM Set Python path - change this to your Python installation directory.
set GCBM_PYTHON=C:\Python37
REM **************************************************************************

REM Set GDAL library paths.
set GDAL_BIN=%GCBM_PYTHON%\lib\site-packages\osgeo
set GDAL_DATA=%GDAL_BIN%\data\gdal

set PYTHONPATH=%GCBM_PYTHON%\lib\site-packages;%GCBM_PYTHON%;%GDAL_BIN%

set PATH=%GCBM_PYTHON%\lib\site-packages;%GCBM_PYTHON%;%GDAL_BIN%;%GDAL_DATA%;%GCBM_PYTHON%\scripts

REM Clean up output directory.
if exist ..\..\processed_output\spatial rd /s /q ..\..\processed_output\spatial
md ..\..\processed_output\spatial

"%GCBM_PYTHON%\python.exe" create_tiffs.py --indicator_root ..\..\gcbm_project\output --start_year %SIMULATION_START_YEAR% --output_path ..\..\processed_output\spatial --log_path ..\..\logs --output_type tif
echo Done!
pause
