set(PACKAGE "sawtooth")
set(LIBNAME "moja.modules.${PACKAGE}")
string(TOUPPER "${PACKAGE}" LIBNAME_EXPORT)

# HEADERS AND SOURCE

include_directories(include ../moja.modules.cbm/include ${PROJECT_SOURCE_DIR}/sawtooth)

configure_file(../templates/exports.h
               ${CMAKE_CURRENT_SOURCE_DIR}/include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h)

set(MOJA_MODULES_HEADERS
    include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h
    include/moja/modules/${PACKAGE}/libraryfactory.h)

set(MOJA_MODULES_SOURCES
    src/libraryfactory.cpp)

set(MOJA_SAWTOOTH_HEADERS
    include/moja/modules/${PACKAGE}/sawtoothmodule.h
)

set(MOJA_SAWTOOTH_SOURCES
    src/sawtoothmodule.cpp
)

set(SRCS_SAWTOOTH ${MOJA_SAWTOOTH_SOURCES} ${MOJA_SAWTOOTH_HEADERS})
set(SRCS ${MOJA_MODULES_SOURCES} ${MOJA_MODULES_HEADERS} )

add_definitions(-DPOCO_NO_AUTOMATIC_LIBS)

add_library(${LIBNAME} ${LIB_MODE} ${SRCS} ${SRCS_SAWTOOTH})

set_target_properties(${LIBNAME} 
    PROPERTIES
    VERSION ${MOJA_VERSION} SOVERSION ${MOJA_VERSION_MAJOR}
    DEFINE_SYMBOL ${LIBNAME_EXPORT}_EXPORTS)

target_link_libraries(
	${LIBNAME} 
	${Moja_CORE}
	${Moja_FLINT}
    ${Moja_DATAREPOSITORY}
	${Poco_FOUNDATION}
	${Poco_DATA}
	${Poco_DATA_SQLITE}
	${Poco_DATA_ODBC}
    sawtooth
    moja.modules.cbm
)

add_dependencies(${LIBNAME} moja.modules.cbm sawtooth)

install(DIRECTORY include/moja
        DESTINATION include
        PATTERN ".svn" EXCLUDE)

install(TARGETS ${LIBNAME}
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        RUNTIME DESTINATION bin)

if(ENABLE_TESTS)
    add_subdirectory(tests)
endif()
