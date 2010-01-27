# Make sure the compiler can find the include files.
include_directories ("include")

# Build the library.
add_library (parser src/parser.c)
