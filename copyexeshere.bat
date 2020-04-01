cd %~dp0
del decryptparsed.exe
copy decryptparsed\release\decryptparsed.exe .
del asciihextobin.exe
copy asciihextobin\release\asciihextobin.exe .
del parserair.exe
copy parserair\release\parserair.exe .
del cryptfile.exe
copy cryptfile\release\cryptfile.exe .
del createmplate.exe
copy createmplate\release\createmplate.exe .
del fileadder.exe
copy fileadder\release\fileadder.exe .
del imgcreator.exe
copy imgcreator\release\imgcreator.exe .
del stage1.exe
copy stage1\release\stage1.exe .
upx stage1.exe
cryptfile.exe stage1.exe
del stage2.exe
copy stage2\release\stage2.exe .
upx stage2.exe
cryptfile.exe stage2.exe

pause