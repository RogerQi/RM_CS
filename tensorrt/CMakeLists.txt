cmake_minimum_required(VERSION 2.8)
project(jetson-inference)

# setup tensorRT flags
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")	# -std=gnu++11
set(BUILD_DEPS "YES" CACHE BOOL "If YES, will install dependencies into sandbox.  Automatically reset to NO after dependencies are installed.")

# setup CUDA
find_package(CUDA)

#setup OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS};
    -O3
	-gencode arch=compute_53,code=sm_53
	-gencode arch=compute_62,code=sm_62
)


# setup project output paths
set(PROJECT_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
set(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)

file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR})
file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)

#message("-- system arch:  ${CMAKE_SYSTEM_PROCESSOR}")
#message("-- output path:  ${PROJECT_OUTPUT_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)


# build C/C++ interface
include_directories(${PROJECT_INCLUDE_DIR} ${GIE_PATH}/include)
include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/)

file(GLOB inferenceSources *.cpp *.cu util/*.cpp util/cuda/*.cu)
file(GLOB inferenceIncludes *.h util/*.h util/cuda/*.h)

cuda_add_library(jetson-inference SHARED ${inferenceSources})
target_link_libraries(jetson-inference drm nvcaffe_parser nvinfer GL GLEW gstreamer-1.0 gstapp-1.0 ${OpenCV_LIBS})		# gstreamer-0.10 gstbase-0.10 gstapp-0.10


# transfer all headers to the include directory
foreach(include ${inferenceIncludes})
	message("-- Copying ${include}")
	configure_file(${include} ${PROJECT_INCLUDE_DIR} COPYONLY)
endforeach()


# create symbolic link for network data
execute_process( COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${PROJECT_SOURCE_DIR}/data/networks" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/networks" )


# copy image data
#file(GLOB imageData ${PROJECT_SOURCE_DIR}/data/images/*)

#foreach(image ${imageData})
#	message("-- Copying ${image}")
#	file(COPY ${image} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#endforeach()

# build samples & utilities

# install
foreach(include ${inferenceIncludes})
    install(FILES "${include}" DESTINATION include/jetson-inference)
endforeach()

# install the shared library
install(TARGETS jetson-inference DESTINATION lib/jetson-inference EXPORT jetson-inferenceConfig)

# install the cmake project, for importing
install(EXPORT jetson-inferenceConfig DESTINATION share/jetson-inference/cmake)
