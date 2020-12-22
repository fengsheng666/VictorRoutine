#编译VRoutine
OPTION(BUILD_VRoutine "Build the VRoutine of cmake of VictorRoutine" ON)
IF(BUILD_VRoutine)
	ADD_SUBDIRECTORY(VRoutine)
ENDIF()
