rm ../YoutubeDownloader.exe
cd build
rem rm -rf *
cmake -G "MSYS Makefiles" ..
make
cd ..