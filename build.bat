@echo off
set app=YoutubeDownloader
set Reset=0
set type=Release
set comp=0
cd build
if %Reset%==1 (
    rm -rf *
)
cmake -G "MSYS Makefiles" .. > result.txt 2>&1
make >> result.txt 2>&1
findstr /i Error result.txt >> result.txt 2>&1
if %errorlevel%==0 (
    echo make Error
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
rm -rf ./copydll.py ./ldd.py ./%app%.zip ./styles/icons/icons.ai ./styles/icons/scifi/ais

if %type%==Debug (
    gdb %app%.exe
    cd ../
) else (
    cd ../
    if %comp%==1 (
    PowerShell Compress-Archive -CompressionLevel Fastest -Update -Path ./%app% -DestinationPath ./%app%.zip > nul
    )
)
rm -f %app%.exe
cd ./%app%-src
exit /b

:RET
cd ..
exit /b