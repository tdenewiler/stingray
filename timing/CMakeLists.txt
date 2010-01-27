# Make sure the compiler can find the include files.
include_directories ("include")

# Build the library.
add_library (timing src/timing.c)
