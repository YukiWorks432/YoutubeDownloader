cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_CXX_COMPILER g++)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

project(YoutubeDownloader CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB_RECURSE UI_FILES src/*.ui)

include_directories(src)

set(CMAKE_CXX_FLAGS "-O3 -fopenmp -static")

add_executable(${PROJECT_NAME}
	${CPP_FILES}
	${UI_FILES}
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets stdc++fs)