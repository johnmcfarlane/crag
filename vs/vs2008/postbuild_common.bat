REM Crag build root folder
mkdir crag
cd crag
xcopy ..\crag.exe /D /Y
xcopy ..\..\..\..\font_bitmap.bmp /D /Y

xcopy ..\..\..\..\dependencies\python\LICENSE lib /D /Y
