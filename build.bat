set app=YoutubeDownloader
set ver=0.0.1
cd build
::rm -rf *
::cmake -G "MSYS Makefiles" ..
::make
call :ZIP

:RET
cd ..
exit /b

:ZIP
cd ../../
mkdir %app%
cd ./%app%
rsync -a ../ ./ --exclude "/%app%/"
Python ./copydll.py ./%app%.exe
rm ./copydll.py
cd ../
PowerShell Compress-Archive -Path ./%app% -DestinationPath ./%app%_v%ver%.zip -Force
rm -r -f %app%
cd ./%app%-src
exit /b