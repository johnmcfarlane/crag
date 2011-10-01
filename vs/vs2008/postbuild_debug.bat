ECHO Copying files to the Debug/crag folder...
cd Debug

call ..\postbuild_common.bat

REM Copy configuration-specific files into Crag build root folder
xcopy ..\..\..\..\dependencies\SDL\VisualC\SDL\Debug\SDL.dll /D /Y
xcopy ..\..\..\..\dependencies\python\PCbuild\python31_d.dll /D /Y

REM Copy Python script files into crag\script folder
mkdir script
cd script
xcopy ..\..\..\..\..\script /D /Y
