# The name of our project is "PLANNER". CMakeLists files in this project can
# refer to the root source directory of the project as ${PLANNER_SOURCE_DIR} and
# to the root binary directory of the project as ${PLANNER_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (PLANNER)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories(include)
include_directories(../serial/include)
include_directories(../common/include)
include_directories(../labjack/include)
include_directories(../kalman/include)
include_directories(../parser/include)
include_directories(../microstrain/include)
include_directories(../pololu/include)
include_directories(../timing/include)
include_directories(${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${PLANNER_BINARY_DIR}/serial)
link_directories (${PLANNER_BINARY_DIR}/parser)
link_directories (${PLANNER_BINARY_DIR}/microstrain)
link_directories (${PLANNER_BINARY_DIR}/pololu)
link_directories (${PLANNER_BINARY_DIR}/timing)
link_directories (${PLANNER_BINARY_DIR}/labjack)
link_directories (${PLANNER_BINARY_DIR}/kalman)

# Build the executable.
add_executable(planner ../common/src/messages ../common/src/util ../common/src/pid ../common/src/network src/task src/planner)

# Link to libraries.
target_link_libraries(planner serial parser microstrain pololu timing labjack kalman)

# Link with OpenCV.
TARGET_LINK_LIBRARIES (planner ${OPENCV_LIBRARIES})

# Link to the math library.
IF (UNIX)
  target_link_libraries(planner m)
ENDIF (UNIX)

