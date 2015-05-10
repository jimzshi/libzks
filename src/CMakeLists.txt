file(GLOB ZKS_SRC "./*.cpp")
file(GLOB ZKS_HEADER "./*.h")

SET_SOURCE_FILES_PROPERTIES(
  simlog.ini
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
  )

add_library (zks STATIC ${ZKS_SRC} ${ZKS_HEADER} simlog.ini)

install (TARGETS zks DESTINATION lib)
install (FILES ${ZKS_HEADER} DESTINATION include/libzks)
install (FILES simlog.ini DESTINATION etc)