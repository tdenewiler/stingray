# The name of our project is "POLOLUCALIBRATE". CMakeLists files in this project can
# refer to the root source directory of the project as ${POLOLUCALIBRATE_SOURCE_DIR} and
# to the root binary directory of the project as ${POLOLUCALIBRATE_BINARY_DIR}.
cmake_minimum_required(VERSION 2.6)
project (POLOLUCALIBRATE)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories(include)
include_directories(../pololu/include)
include_directories(../serial/include)
include_directories(../labjack/include)

# Make sure the compiler can find the libraries.
link_directories (${POLOLUCALIBRATE_BINARY_DIR}/pololu)
link_directories (${POLOLUCALIBRATE_BINARY_DIR}/serial)
link_directories (${POLOLUCALIBRATE_BINARY_DIR}/labjack)

# Build the executable.
add_executable(pololuCalibrate src/pololuCalibrate)

# Link to libraries.
target_link_libraries(pololuCalibrate pololu serial labjack)

# Link to the math library.
IF (UNIX)
  target_link_libraries(pololuCalibrate m)
ENDIF (UNIX)

