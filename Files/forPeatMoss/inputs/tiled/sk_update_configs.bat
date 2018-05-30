@echo off

echo Updating GCBM configuration...
python ..\..\..\..\tools\tiler\update_gcbm_config.py --layer_root . --gcbm_config ..\..\..\..\tools\peatland_config.json --provider_config ..\..\..\..\tools\provider_peatland.json

echo Done!
pause
