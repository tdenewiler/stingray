# Make sure the compiler can find the include files.
include_directories (include)
include_directories (${OPENCV_INCLUDE_DIR})

# Put the library in a common directory.
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Add compiler flags.
add_definitions (-Wno-write-strings)

# Build the library.
add_library (kalman src/kalman)

