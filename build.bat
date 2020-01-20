set app=YoutubeDownloader
cd build
::rm -rf *
cmake -G "MSYS Makefiles" ..
make
call :ZIP

:RET
cd ..
exit /b

:ZIP
cd ../../
rm -f ./%app%.zip
cd %app%
rm -rf *
rsync -a ../ ./ --exclude "/%app%/"
Python ./copydll.py ./%app%.exe
rm ./copydll.py ./ldd.py
cd ../
PowerShell Compress-Archive -Path ./%app% -DestinationPath ./%app%.zip -update
rm -f %app%.exe
cd ./%app%-src
exit /b