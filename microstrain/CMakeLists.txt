# Make sure the compiler can find the include files.
include_directories("include")
include_directories("../serial/include")

# Build the library.
add_library(microstrain src/microstrain)

# Link to the serial library.
target_link_libraries(microstrain serial)

