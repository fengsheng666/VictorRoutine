#编译tests
OPTION(BUILD_test_normal "Build the test_normal of cmake of VictorRoutine" OFF)
IF(BUILD_test_normal)
	ADD_SUBDIRECTORY(test_normal)
ENDIF()

OPTION(BUILD_test_queue "Build the test_queue of cmake of VictorRoutine" OFF)
IF(BUILD_test_queue)
	ADD_SUBDIRECTORY(test_queue)
ENDIF()