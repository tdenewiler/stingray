# Make sure the compiler can find the include files.
include_directories("include")
include_directories("../serial/include")

# Build the library.
add_library(pololu src/pololu.c)

# Link to the serial library.
target_link_libraries(pololu serial)

