# Copyright (c) 2020 Wind River Systems, Inc. All Rights Reserved.
#
cmake_minimum_required(VERSION 3.10.2)
project(motion_detect)
set(OPENCV_INSTALL_DIR "/home/dan/projects/cto/opencv/vxworks/src/install/")
include_directories(${OPENCV_INSTALL_DIR}/usr/include/opencv4)
link_directories(${OPENCV_INSTALL_DIR}/usr/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_EXECUTABLE_SUFFIX ".vxe")
aux_source_directory("src" DIR_SRCS)
add_executable (motion_detect ${DIR_SRCS} )

target_link_libraries(motion_detect opencv_stitching opencv_highgui opencv_ml opencv_videoio opencv_video opencv_dnn  opencv_objdetect opencv_calib3d opencv_imgcodecs opencv_features2d opencv_flann  opencv_photo opencv_imgproc opencv_core z)
