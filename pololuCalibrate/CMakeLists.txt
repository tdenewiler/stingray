# The name of our project is "POLOLUCALIBRATE". CMakeLists files in this project can
# refer to the root source directory of the project as ${POLOLUCALIBRATE_SOURCE_DIR} and
# to the root binary directory of the project as ${POLOLUCALIBRATE_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (pololuCalibrate)

# Add compiler flags.
add_definitions (-Wall -O2 -g)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../labjack/include)
include_directories (../pololu/include)
include_directories (../serial/include)

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# List the source files here.
set (SRCS src/pololuCalibrate)

# List the libraries here.
set (LIBS labjack)
set (LIBS ${LIBS} pololu)
set (LIBS ${LIBS} serial)

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

