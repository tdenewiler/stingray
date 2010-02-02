# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)
include_directories (${OPENCV_INCLUDE_DIR})

# Put the library in a common directory.
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Build the library.
add_library (vision src/vision)

# Link with OpenCV.
target_link_libraries (vision ${OPENCV_LIBRARIES})

