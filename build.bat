@echo off
set app=YoutubeDownloader
set Reset=1
set type=Release
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
rm -f ./copydll.py ./ldd.py ./%app%.zip
if %type%==Debug (
    gdb %app%.exe
    cd ../
) else (
    cd ../
    PowerShell Compress-Archive -CompressionLevel Fastest -Update -Path ./%app% -DestinationPath ./%app%.zip > nul
)
rm -f %app%.exe
cd ./%app%-src
exit /b

:RET
cd ..
exit /b