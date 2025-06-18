# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\UAVTelemViewer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\UAVTelemViewer_autogen.dir\\ParseCache.txt"
  "UAVTelemViewer_autogen"
  )
endif()
