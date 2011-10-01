ECHO Copying files to the Release/crag folder...
cd Release
mkdir crag
cd crag

REM Copy files into crag root directory
xcopy ..\..\..\..\dependencies\SDL\VisualC\SDL\Release\SDL.dll /D /Y
xcopy ..\..\..\..\dependencies\python\PCbuild\python31.dll /D /Y
xcopy ..\crag.exe /D /Y

REM Copy script files
mkdir script
cd script
xcopy ..\..\..\..\..\script /D /Y /E /S
cd ..

REM Copy python libs
..\..\pylibs_copy.bat
