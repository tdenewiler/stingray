# Make sure the compiler can find the include files.
include_directories ("include")

# Build the library.
add_library (labjack src/u3 src/battery_sensor src/depth_sensor src/labjack src/log_labjack src/water_sensor src/labjackusb)
