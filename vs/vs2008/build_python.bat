cd ..\..\dependencies\python

cmd /c Tools\buildbot\external.bat
call "%VS90COMNTOOLS%vsvars32.bat"
cmd /c Tools\buildbot\clean.bat
vcbuild /useenv PCbuild\pcbuild.sln "Debug|Win32"
vcbuild /useenv PCbuild\pcbuild.sln "Releast|Win32"
