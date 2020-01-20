@echo off
set app=YoutubeDownloader
cd build
::rm -rf *
cmake -G "MSYS Makefiles" .. > nul
make > result.txt 2>&1
findstr /i Error result.txt > nul
if %errorlevel%==0 (
    echo make Error
    goto :RET
) else (
    goto :ZIP
)

:ZIP
cd ../../
cd %app%
rm -rf *
rsync -a ../ ./ --exclude "/%app%/"
Python ./copydll.py ./%app%.exe
rm -f ./copydll.py ./ldd.py ./%app%.zip
cd ../
PowerShell Compress-Archive -CompressionLevel Fastest -Update -Path ./%app% -DestinationPath ./%app%.zip
rm -f %app%.exe
cd ./%app%-src
exit /b

:RET
cd ..
exit /b