cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_CXX_COMPILER g++)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(YoutubeDownloader CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenMP REQUIRED)
find_package(HarfBuzz REQUIRED)

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB_RECURSE UI_FILES src/*.ui)

include_directories(src)

if(CMAKE_BUILD_TYPE EQUAL Debug) 
	set(CMAKE_CXX_FLAGS "-O0 -g3 -fopenmp -pthread -DDEBUG")
else()
	set(CMAKE_CXX_FLAGS "-O3 -fopenmp -pthread -mwindows")
endif()

add_executable(${PROJECT_NAME}
	${CPP_FILES}
	${UI_FILES}
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets HarfBuzz stdc++fs)