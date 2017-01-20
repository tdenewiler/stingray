# Make sure the compiler can find the include files.
include_directories (include)

# Put the library in a common directory.
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# List the source files here.
set (SRCS src/battery_sensor)
set (SRCS ${SRCS} src/depth_sensor)
set (SRCS ${SRCS} src/labjack)
set (SRCS ${SRCS} src/labjackusb)
set (SRCS ${SRCS} src/log_labjack)
set (SRCS ${SRCS} src/u3)
set (SRCS ${SRCS} src/water_sensor)

# Build the library.
add_library (labjack ${SRCS})
