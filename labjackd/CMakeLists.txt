# The name of our project is "LABJACKD". CMakeLists files in this project can
# refer to the root source directory of the project as ${LABJACKD_SOURCE_DIR} and
# to the root binary directory of the project as ${LABJACKD_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (LABJACKD)

# Add compiler flags.
add_definitions ("-Wall -O2 -g")

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../serial/include)
include_directories (../common/include)
include_directories (../labjack/include)
include_directories (../parser/include)
include_directories (../pololu/include)
include_directories (../timing/include)

# Make sure the compiler can find the libraries.
link_directories (${LABJACKD_BINARY_DIR}/labjack)
link_directories (${LABJACKD_BINARY_DIR}/parser)

# Build the executable.
add_executable (labjackd ../common/src/util ../common/src/messages ../common/src/network src/labjackd)

# Link to libraries.
target_link_libraries (labjackd labjack parser)

# Link to the math library.
IF (UNIX)
  target_link_libraries(labjackd m)
ENDIF (UNIX)

