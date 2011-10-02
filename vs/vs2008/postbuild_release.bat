ECHO Copying files to the Release/crag folder...
cd Release

call ..\postbuild_common.bat

REM Copy configuration-specific files into Crag build root folder
xcopy ..\..\..\..\dependencies\SDL\VisualC\SDL\Release\SDL.dll /D /Y
xcopy ..\..\..\..\dependencies\python\PCbuild\python31.dll /D /Y
