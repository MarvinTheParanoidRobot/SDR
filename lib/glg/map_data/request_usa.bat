:: This is the Windows version of the request

echo  Edit location of the MAP_SERVER executable before running the script.

SETLOCAL

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: For the stand-alone map server, uncomment and edit location of the GLG 
:: installation directory - GLG_DIR, which is set by the GLG installation
:: otherwise.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::SET GLG_DIR_3_8=C:\Glg

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Select location of the map server executable
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::SET MAP_SERVER=%GLG_DIR_3_8%\eval\bin\GlmMap.exe
::SET MAP_SERVER=%GLG_DIR_3_8%\map_server\bin\GlmMap.exe
SET MAP_SERVER=%GLG_DIR_3_8%\bin\GlmMap.exe

SET DATASET_DIR=%GLG_DIR_3_8%\map_data

SET OUT_FILE=output.jpg

echo USA map example, orthographic projection, verbose output

"%MAP_SERVER%" -generate -verbosity 1 -dataset "%DATASET_DIR%/sample.sdf" -output "%OUT_FILE%" -oGISreq "VERSION=1.3.0&REQUEST=GetMap&SRS=AUTO2:42003,1.,-97,38&WIDTH=700&HEIGHT=700&BBOX=-2500000,-2500000,2500000,2500000&BGCOLOR=0x0&STYLES=default&FORMAT=image/jpeg&LAYERS=earth,shadow,grid,states&STRETCH=0"

echo To view the output, open output.jpg in any image viewing tool
echo

    start %OUT_FILE%
