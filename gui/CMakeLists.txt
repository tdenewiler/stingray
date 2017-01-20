# The name of our project is "GUI". CMakeLists files in this project can
# refer to the root source directory of the project as ${GUI_SOURCE_DIR} and
# to the root binary directory of the project as ${GUI_BINARY_DIR}.
cmake_minimum_required (VERSION 2.6)
project (gui)

# Add compiler flags.
add_definitions (-Wall -Wno-write-strings -O2 -g)
#add_definitions (-DUSE_GTK_GUI)

# Make sure the compiler can find the include files.
include_directories (include)
include_directories (../common/include)
include_directories (../kalman/include)
include_directories (../labjack/include)
include_directories (../microstrain/include)
include_directories (../parser/include)
include_directories (../planner/include)
include_directories (../pololu/include)
include_directories (../serial/include)
include_directories (../timing/include)
include_directories (../vision/include)
include_directories (../visiond/include)
#include_directories (/usr/include/gtk-2.0)
include_directories (${OPENCV_INCLUDE_DIR})
include_directories (${GTK2_INCLUDE_DIRS})

# Make sure the compiler can find the libraries.
link_directories (${PROJECT_BINARY_DIR})

# List the source files here.
set (SRCS src/buttons)
set (SRCS ${SRCS} src/client)
set (SRCS ${SRCS} src/events)
set (SRCS ${SRCS} src/gui)
set (SRCS ${SRCS} ../common/src/messages)
set (SRCS ${SRCS} ../common/src/network)

# List the libraries here.
set (LIBS parser)

# Put the executable in a common directory.
set (EXECUTABLE_OUTPUT_PATH ../bin)

# Build the executable.
add_executable (${PROJECT_NAME} ${SRCS})

# Link to libraries.
target_link_libraries (${PROJECT_NAME} ${LIBS})

# Link with OpenCV and Gtk.
target_link_libraries (${PROJECT_NAME} ${OPENCV_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ${GTK2_LIBRARIES})
