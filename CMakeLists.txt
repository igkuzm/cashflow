cmake_minimum_required(VERSION 3.15)

set(TARGET cashflow)

project(
	${TARGET} VERSION 1.0 
	DESCRIPTION "C library to calculate targets in cashflow game"
	HOMEPAGE_URL ""
	LANGUAGES C 
)

add_subdirectory(SQLiteConnect)

add_library(${TARGET} STATIC 
	${TARGET}.c
	uuid4/uuid4.c
	)
target_link_libraries(${TARGET} 
	SQLiteConnect
)

if(${${TARGET}_BUILD_TEST})
	set(RESOURCE_FILES Readme.md)
	add_executable(${TARGET}_test 
		test.c
		${RESOURCE_FILES}
	)
	
	target_link_libraries(${TARGET}_test ${TARGET})

	set_target_properties(${TARGET}_test PROPERTIES
		MACOSX_BUNDLE TRUE
		RESOURCE "${RESOURCE_FILES}" 
	)
endif()
