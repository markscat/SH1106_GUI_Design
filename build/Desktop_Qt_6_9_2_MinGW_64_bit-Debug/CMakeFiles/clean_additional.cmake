# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\SH1106_GUI_Design_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SH1106_GUI_Design_autogen.dir\\ParseCache.txt"
  "SH1106_GUI_Design_autogen"
  )
endif()
