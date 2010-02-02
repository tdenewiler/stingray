# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../serial/include)

# Put the library in a common directory.
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# Build the library.
add_library(pololu src/pololu)

# Link to the serial library.
target_link_libraries (pololu serial)

