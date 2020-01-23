CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(xlsx_reader)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

set(CMAKE_DEBUG_POSTFIX "d")

OPTION(WITH_JSON_SUPPORT "use json to view data" ON)

FIND_PACKAGE(miniz CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${miniz_INCLUDE_DIRS})

FIND_PACKAGE(tinyxml2 CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${tinyxml2_include_dirs})


FIND_PACKAGE(typed_string CONFIG REQUIRED)

INCLUDE_DIRECTORIES(${typed_string_include_dirs})

SET(XLSX_READER_SRC_DIR ${CMAKE_SOURCE_DIR}/source)

SET(XLSX_READER_HEADER_DIR ${CMAKE_SOURCE_DIR}/include)
SET(XLSX_READER_TEST_DIR ${CMAKE_SOURCE_DIR}/test)

FILE(GLOB_RECURSE XLSX_READER_SRC_LIST ${XLSX_READER_SRC_DIR}/*.cpp)
FILE(GLOB_RECURSE XLSX_READER_TEST_LIST ${XLSX_READER_TEST_DIR}/*.cpp)

INCLUDE_DIRECTORIES(${XLSX_READER_HEADER_DIR})

ADD_LIBRARY(xlsx_reader  ${XLSX_READER_SRC_LIST})
target_link_libraries(xlsx_reader PUBLIC miniz::miniz tinyxml2::tinyxml2 typed_string)



ADD_EXECUTABLE(workbook_test ${XLSX_READER_TEST_DIR}/workbook_test.cpp)


target_link_libraries(workbook_test xlsx_reader)

IF(WITH_JSON_SUPPORT)
	find_package(nlohmann_json CONFIG REQUIRED)
	INCLUDE_DIRECTORIES(${nlohmann_json_include_dirs})
	ADD_EXECUTABLE(json_test ${XLSX_READER_TEST_DIR}/json_test.cpp)
	target_link_libraries(json_test xlsx_reader)
    target_link_libraries(json_test nlohmann_json::nlohmann_json)
ENDIF(WITH_JSON_SUPPORT)


foreach(p LIB BIN INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${p}")
	endif()
endforeach()

export(TARGETS xlsx_reader
      FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)

file(WRITE
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
    "include(\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)\n")

install(FILES
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    ARCHIVE  DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )

install(DIRECTORY ${XLSX_READER_HEADER_DIR}/ DESTINATION include/${PROJECT_NAME})

install(EXPORT ${CMAKE_PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})