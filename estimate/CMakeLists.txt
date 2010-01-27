# The name of our project is "ESTIMATE". CMakeLists files in this project can
# refer to the root source directory of the project as ${ESTIMATE_SOURCE_DIR} and
# to the root binary directory of the project as ${ESTIMATE_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (ESTIMATE)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories(include)
include_directories(../common/include)
include_directories(../parser/include)
include_directories(../timing/include)
include_directories(../sysid/include)

# Make sure the compiler can find the libraries.
link_directories (${ESTIMATE_BINARY_DIR}/parser)
link_directories (${ESTIMATE_BINARY_DIR}/timing)
link_directories (${ESTIMATE_BINARY_DIR}/sysid)

# Build the executable.
add_executable(estimate ../common/src/messages ../common/src/network src/estimate)

# Link to libraries.
target_link_libraries(estimate parser timing sysid)

# Link to the math library.
IF (UNIX)
  target_link_libraries(estimate m)
ENDIF (UNIX)

