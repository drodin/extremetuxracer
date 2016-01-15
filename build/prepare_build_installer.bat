echo This script removes several files from /data folder as a preparation to build the installer. Continue only if you are sure what you are doing!
pause

cd ..\data
del /s Makefile.am
del translations\xx_XX.lst
pause