REM Crag build root folder
mkdir crag
cd crag
xcopy ..\crag.exe /D /Y
xcopy ..\..\..\..\font_bitmap.bmp /D /Y
xcopy ..\..\..\..\src\readme.html /D /Y
xcopy ..\..\..\..\dependencies\SDL\README-SDL.txt /D /Y

mkdir lib
xcopy ..\..\..\..\dependencies\python\LICENSE lib /D /Y

mkdir glsl
xcopy ..\..\..\..\glsl\* glsl /D /Y