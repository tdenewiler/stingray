cmake_minimum_required (VERSION 2.6)

# Find OpenCV and Gtk.
# I downloaded the file FindOpenCV.cmake from http://opencv.willowgarage.com/wiki/Getting_started and copied it into /usr/share/cmake-2.6/Modules/.
# I downloaded the file FindGTK2.cmake from http://cmake-modules.googlecode.com/svn/trunk/Modules/GTK2/FindGTK2.cmake and copied it into /usr/share/cmake-2.6/Modules/.
find_package (OpenCV REQUIRED)
find_package (GTK2 COMPONENTS gtk)

# Recurse through subdirectories to build everything.
add_subdirectory (gui)
add_subdirectory (estimate)
add_subdirectory (joy)
add_subdirectory (joydrive)
add_subdirectory (kalman)
add_subdirectory (labjack)
add_subdirectory (labjackd)
add_subdirectory (microstrain)
add_subdirectory (nav)
add_subdirectory (parser)
add_subdirectory (planner)
add_subdirectory (pololu)
add_subdirectory (pololuCalibrate)
add_subdirectory (serial)
add_subdirectory (sysid)
add_subdirectory (timing)
add_subdirectory (vision)
add_subdirectory (visiond)

