#编译demo
OPTION(BUILD_normal_scene "Buile the normal_scene of cmake of VictorRoutine" OFF)
IF(BUILD_normal_scene)
	ADD_SUBDIRECTORY(normal_scene)
ENDIF()