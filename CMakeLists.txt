cmake_minimum_required (VERSION 2.6)

# Find OpenCV.
# I downloaded the file FindOpenCV.cmake from http://opencv.willowgarage.com/wiki/Getting_started and copied it into /usr/share/cmake-2.6/Modules/.
FIND_PACKAGE(OpenCV REQUIRED)

# Recurse through subdirectories to build everything.
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

