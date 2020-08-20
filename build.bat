@echo off
set app=YoutubeDownloader
set Reset=1
set type=Release
set install=1
cd build
if %Reset%==1 (
    rm -rf *
)
cmake -G "MSYS Makefiles" .. > result.txt 2>&1
make --no-print-directory >> result.txt 2>&1
findstr /i Error result.txt >> result.txt 2>&1
if %errorlevel%==0 (
    echo Make Error
    type result.txt
    goto :RET
) else (
    goto :ZIP
)

:ZIP
cd ../../
cd %app%
if %Reset%==1 (
    rm -rf *
)
rsync -a ../ ./ --exclude "/%app%/"
Python ./copydll.py %app%.exe
rm -rf ./copydll.py ./ldd.py ./%app%.zip ./%app%_install.exe ./styles/icons/icons.ai ./styles/icons/scifi/ais

if %type%==Debug (
    lldb -command=.gdbinit %app%.exe
)

rm -f ./.gdbinit
cd ../
rm -f %app%.exe
cd ./%app%-src
exit /b

:RET
cd ..
exit /b