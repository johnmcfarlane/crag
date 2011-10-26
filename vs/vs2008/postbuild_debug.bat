ECHO Copying files to the Debug/crag folder...
cd Debug

call ..\postbuild_common.bat

..\..\..\..\dependencies\python\PCBuild\python ..\..\..\..\build_py_deps.py ../../../../script/main.py ..\..\..\..\dependencies\python\lib lib

REM Copy configuration-specific files into Crag build root folder
xcopy ..\..\..\..\dependencies\SDL\VisualC\SDL\Debug\SDL.dll /D /Y
xcopy ..\..\..\..\dependencies\python\PCbuild\python31_d.dll /D /Y
