# The name of our project is "VISIOND". CMakeLists files in this project can
# refer to the root source directory of the project as ${VISIOND_SOURCE_DIR} and
# to the root binary directory of the project as ${VISIOND_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (VISIOND)

# Add compiler flags.
add_definitions (-Wall -Wno-write-strings -O2 -g)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../serial/include)
include_directories (../common/include)
include_directories (../labjack/include)
include_directories (../kalman/include)
include_directories (../parser/include)
include_directories (../microstrain/include)
include_directories (../pololu/include)
include_directories (../timing/include)
include_directories (../vision/include)
include_directories (../planner/include)
include_directories (${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${VISIOND_BINARY_DIR}/parser)
link_directories (${VISIOND_BINARY_DIR}/timing)
link_directories (${VISIOND_BINARY_DIR}/vision)

# Build the executable.
add_executable (visiond ../common/src/messages ../common/src/util ../common/src/network src/visiond)

# Link to libraries.
target_link_libraries (visiond serial parser microstrain pololu timing labjack kalman vision)

# Link with OpenCV.
target_link_libraries (visiond ${OPENCV_LIBRARIES})

