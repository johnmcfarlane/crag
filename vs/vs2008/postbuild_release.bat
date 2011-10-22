ECHO Copying files to the Release/crag folder...
cd Release

call ..\postbuild_common.bat

..\..\..\..\dependencies\python\PCBuild\python ..\..\..\..\build_py_deps.py ../../../../script/main.py ..\..\..\..\dependencies\python\lib lib

REM Copy configuration-specific files into Crag build root folder
xcopy ..\..\..\..\dependencies\SDL\VisualC\SDL\Release\SDL.dll /D /Y
xcopy ..\..\..\..\dependencies\python\PCbuild\python31.dll /D /Y
xcopy ..\..\..\..\src\crag\readme.html /D /Y
