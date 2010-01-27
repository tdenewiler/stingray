# The name of our project is "NAV". CMakeLists files in this project can
# refer to the root source directory of the project as ${NAV_SOURCE_DIR} and
# to the root binary directory of the project as ${NAV_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (NAV)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories(include)
include_directories(../serial/include)
include_directories(../common/include)
include_directories(../planner/include)
include_directories(../labjack/include)
include_directories(../labjackd/include)
include_directories(../kalman/include)
include_directories(../parser/include)
include_directories(../microstrain/include)
include_directories(../pololu/include)
include_directories(../timing/include)
include_directories(${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${NAV_BINARY_DIR}/serial)
link_directories (${NAV_BINARY_DIR}/parser)
link_directories (${NAV_BINARY_DIR}/microstrain)
link_directories (${NAV_BINARY_DIR}/pololu)
link_directories (${NAV_BINARY_DIR}/timing)

# Build the executable.
add_executable(nav ../common/src/messages ../common/src/util ../common/src/pid ../common/src/network src/nav)

# Link to libraries.
target_link_libraries(nav serial parser microstrain pololu timing)

# Link to the math library.
IF (UNIX)
  target_link_libraries(nav m)
ENDIF (UNIX)

