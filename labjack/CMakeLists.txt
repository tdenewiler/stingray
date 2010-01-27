# Make sure the compiler can find the include files.
include_directories ("include")

# Build the library.
add_library (labjack src/u3.c src/battery_sensor.c src/depth_sensor.c src/labjack.c src/log_labjack.c src/water_sensor.c src/labjackusb.c)
