# Make sure the compiler can find the include files.
include_directories (include)

# Put the library in a common directory.
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Build the library.
add_library (joy src/joy)
