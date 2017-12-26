cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Set project information                                                                                                   
project(panoram_image CXX)
set(PROJECT_BRIEF "A cross-platform, for panoram image display demo")

find_package(PkgConfig)
pkg_check_modules(Libdrm REQUIRED libdrm)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PKG_Libdrm_CXXFLAGS}")

file(GLOB_RECURSE Client_SRC "src/*.cpp")
add_executable(panoram_image ${Client_SRC})

target_include_directories(panoram_image PRIVATE 
	"${Libdrm_INCLUDE_DIRS}")
target_link_libraries(panoram_image 
	"${Libdrm_LIBRARIES}" gbm EGL GLESv2)
