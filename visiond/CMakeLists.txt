# NOTE: vision is the only library that visiond does NOT complain about when building.
# vision.h is also the only file out of messages.h, network.h, parser.h and timing.h
# that does NOT have a typedef struct{} statement. I don't know if there is a
# correlation there or not.

# NOTE: visiond is the only executable that is building using C++ (not C) source files.
# vision.cpp is the only other C++ source file used when building visiond. Maybe
# that is why the vision_*() functions are being found when trying to build visiond
# but none of the functions from C source files are being found.

# The name of our project is "VISIOND". CMakeLists files in this project can
# refer to the root source directory of the project as ${VISIOND_SOURCE_DIR} and
# to the root binary directory of the project as ${VISIOND_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (visiond)

# Add compiler flags.
add_definitions (-Wall -Wno-write-strings -O2 -g)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)
include_directories (../kalman/include)
include_directories (../labjack/include)
include_directories (../microstrain/include)
include_directories (../parser/include)
include_directories (../planner/include)
include_directories (../pololu/include)
include_directories (../serial/include)
include_directories (../timing/include)
include_directories (../vision/include)
include_directories (${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# List the source files here.
set (SRCS ../common/src/messages)
set (SRCS ${SRCS} ../common/src/network)
set (SRCS ${SRCS} src/visiond)

# List the libraries here.
set (LIBS parser)
set (LIBS ${LIBS} timing)
set (LIBS ${LIBS} vision)

# Put the executable in a common directory.
set (EXECUTABLE_OUTPUT_PATH ../bin)

# Build the executable.
add_executable (${PROJECT_NAME} ${SRCS})
#set_target_properties (${PROJECT_NAME} PROPERTIES HAS_CXX TRUE)
#set_target_properties (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# Link to libraries.
target_link_libraries (${PROJECT_NAME} ${LIBS})

# Link with OpenCV.
target_link_libraries (${PROJECT_NAME} ${OPENCV_LIBRARIES})

