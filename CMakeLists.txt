cmake_minimum_required(VERSION 3.19)

project(VocalIsolationModel)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# include SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# include Python2 for matplotlib wrapper
find_package(Python2 COMPONENTS Development NumPy)

# include OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(VIsoModel src/driver.cpp src/AudioTree.cpp src/../../Mu.S.I.C.A./PhaseI/audioView/src/AudioView.cpp)

target_link_libraries(VIsoModel ${SDL2_LIBRARIES})
target_link_libraries(VIsoModel ${OpenCV_LIBS} )
target_include_directories(VIsoModel PRIVATE ${Python2_INCLUDE_DIRS} ${Python2_NumPy_INCLUDE_DIRS})
target_link_libraries(VIsoModel Python2::Python Python2::NumPy)
