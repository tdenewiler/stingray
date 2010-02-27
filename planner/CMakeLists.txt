# The name of our project is "PLANNER". CMakeLists files in this project can
# refer to the root source directory of the project as ${PLANNER_SOURCE_DIR} and
# to the root binary directory of the project as ${PLANNER_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (planner)

# Add compiler flags.
add_definitions (-Wall -Wno-write-strings -O2 -g)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)
include_directories (../kalman/include)
include_directories (../labjack/include)
include_directories (../microstrain/include)
include_directories (../parser/include)
include_directories (../pololu/include)
include_directories (../serial/include)
include_directories (../timing/include)
include_directories (${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# List the source files here.
set (SRCS ../common/src/messages)
set (SRCS ${SRCS} ../common/src/network)
set (SRCS ${SRCS} ../common/src/pid)
set (SRCS ${SRCS} src/planner)
set (SRCS ${SRCS} src/task)
set (SRCS ${SRCS} ../common/src/util)

# List the libraries here.
set (LIBS kalman)
set (LIBS ${LIBS} labjack)
set (LIBS ${LIBS} microstrain)
set (LIBS ${LIBS} parser)
set (LIBS ${LIBS} pololu)
set (LIBS ${LIBS} serial)
set (LIBS ${LIBS} timing)

# Put the executable in a common directory.
set (EXECUTABLE_OUTPUT_PATH ../bin)

# Build the executable.
add_executable (${PROJECT_NAME} ${SRCS})

# Link to libraries.
target_link_libraries (${PROJECT_NAME} ${LIBS})

# Link with OpenCV.
target_link_libraries (${PROJECT_NAME} ${OPENCV_LIBRARIES})

# Link to the math library.
IF (UNIX)
  target_link_libraries (${PROJECT_NAME} m)
ENDIF (UNIX)

