REM Crag build root folder
mkdir crag
cd crag
xcopy ..\crag.exe /D /Y
xcopy ..\..\..\..\font_bitmap.bmp /D /Y

mkdir Lib
cd Lib

xcopy ..\..\..\..\..\dependencies\python\Lib\copyreg.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\codecs.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\io.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\os.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\ntpath.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\stat.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\genericpath.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\_abcoll.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\abc.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\_weakrefset.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\site.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\locale.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\re.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\sre_compile.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\sre_parse.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\sre_constants.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\collections.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\keyword.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\heapq.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\bisect.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\weakref.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\functools.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\random.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\__future__.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\warnings.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\linecache.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\tokenize.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\string.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\token.py /D /Y
xcopy ..\..\..\..\..\dependencies\python\Lib\types.py /D /Y

mkdir encodings
cd encodings
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\__init__.pyc /D /Y
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\aliases.py /D /Y
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\cp1252.py /D /Y
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\latin_1.py /D /Y
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\utf_8.py /D /Y
xcopy ..\..\..\..\..\..\dependencies\python\Lib\encodings\utf_32_be.pyc /D /Y
cd ..

xcopy ..\..\..\..\..\dependencies\python\LICENSE /D /Y
cd ..
