# The name of our project is "JOYDRIVE". CMakeLists files in this project can
# refer to the root source directory of the project as ${JOYDRIVE_SOURCE_DIR} and
# to the root binary directory of the project as ${JOYDRIVE_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (JOYDRIVE)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories(include)
include_directories(../common/include)
include_directories(../parser/include)
include_directories(../timing/include)
include_directories(../sysid/include)
include_directories(../planner/include)
include_directories(../microstrain/include)
include_directories(../labjack/include)
include_directories(../pololu/include)
include_directories(../joy/include)
include_directories(../serial/include)
include_directories(../kalman/include)
include_directories(${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${JOYDRIVE_BINARY_DIR}/parser)
link_directories (${JOYDRIVE_BINARY_DIR}/joy)

# Build the executable.
add_executable(joydrive ../common/src/messages ../common/src/util ../common/src/network src/joydrive)

# Link to libraries.
target_link_libraries(joydrive parser joy)

# Link to the math library.
IF (UNIX)
  target_link_libraries(joydrive m)
ENDIF (UNIX)

