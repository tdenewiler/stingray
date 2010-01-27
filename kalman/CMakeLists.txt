# Make sure the compiler can find the include files.
include_directories (include)
include_directories (${OPENCV_INCLUDE_DIR})

# Build the library.
add_library (kalman src/kalman.c)

# Link with OpenCV.
#TARGET_LINK_LIBRARIES (kalman ${OPENCV_LIBRARIES})

