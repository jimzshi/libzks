file(GLOB ZKS_SRC "./*.cpp")
file(GLOB ZKS_HEADER "./*.h")

add_library (zks STATIC ${ZKS_SRC})

install (TARGETS zks DESTINATION lib)
install (FILES ${ZKS_HEADER} DESTINATION include/libzks)