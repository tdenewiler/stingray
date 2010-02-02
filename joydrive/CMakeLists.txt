# The name of our project is "JOYDRIVE". CMakeLists files in this project can
# refer to the root source directory of the project as ${JOYDRIVE_SOURCE_DIR} and
# to the root binary directory of the project as ${JOYDRIVE_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (joydrive)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)
include_directories (../joy/include)
include_directories (../kalman/include)
include_directories (../labjack/include)
include_directories (../microstrain/include)
include_directories (../parser/include)
include_directories (../planner/include)
include_directories (../pololu/include)
include_directories (../serial/include)
include_directories (../sysid/include)
include_directories (../timing/include)
include_directories (${OPENCV_INCLUDE_DIR})

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# List the source files here.
set (SRCS src/joydrive)
set (SRCS ${SRCS} ../common/src/messages)
set (SRCS ${SRCS} ../common/src/network)
set (SRCS ${SRCS} ../common/src/util)

# List the libraries here.
set (LIBS joy)
set (LIBS ${LIBS} parser)

# Put the executable in a common directory.
set (EXECUTABLE_OUTPUT_PATH ../bin)

# Build the executable.
add_executable (${PROJECT_NAME} ${SRCS})

# Link to libraries.
target_link_libraries (${PROJECT_NAME} ${LIBS})

# Link to the math library.
IF (UNIX)
  target_link_libraries (${PROJECT_NAME} m)
ENDIF (UNIX)

