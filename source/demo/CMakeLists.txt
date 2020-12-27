#编译demo
OPTION(BUILD_demo_normal "Buile the demo_normal of cmake of VictorRoutine" OFF)
IF(BUILD_demo_normal)
	ADD_SUBDIRECTORY(demo_normal)
ENDIF()