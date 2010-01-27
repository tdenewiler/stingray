# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)

# Build the library.
add_library (sysid src/sysid)

