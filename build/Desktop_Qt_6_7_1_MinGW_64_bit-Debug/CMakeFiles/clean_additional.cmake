# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appqmlqtdesign_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appqmlqtdesign_autogen.dir\\ParseCache.txt"
  "appqmlqtdesign_autogen"
  )
endif()
