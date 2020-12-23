#编译tests
OPTION(BUILD_normal_test "Buile the normal_test of cmake of VictorRoutine" OFF)
IF(BUILD_normal_test)
	ADD_SUBDIRECTORY(normal_test)
ENDIF()