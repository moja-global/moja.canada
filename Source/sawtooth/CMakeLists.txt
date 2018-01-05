set(PACKAGE "sawtooth")
set(LIBNAME "sawtooth")
string(TOUPPER "${PACKAGE}" LIBNAME_EXPORT)

#include_directories(include)

set(SAWTOOTH_HEADERS
    sawtooth/constants.h
    sawtooth/dblayer.h
    sawtooth/disturbance_type.h
    sawtooth/equationset.h
    sawtooth/exports.h
    sawtooth/modelmeta.h
    sawtooth/mortalityprobability.h
    sawtooth/parameter_cbm.h
    sawtooth/parameter_core.h
    sawtooth/parameter_d1.h
    sawtooth/parameter_d2.h
    sawtooth/parameter_es1.h
    sawtooth/parameter_es2.h
    sawtooth/parameter_es3.h
    sawtooth/parameter_mlr35.h
    sawtooth/parameterset.h
    sawtooth/random.h
    sawtooth/results.h
    sawtooth/sawtootherror.h
    sawtooth/sawtoothexception.h
    sawtooth/sawtoothmatrix.h
    sawtooth/sawtoothmodel.h
    sawtooth/spatialvariable.h
    sawtooth/sqlite3.h
    sawtooth/stand.h
    sawtooth/standcbmextension.h
)

set(SAWTOOTH_SOURCES
    sawtooth/exports.cpp
    sawtooth/profiling_main.cpp
    sawtooth/results.cpp
    sawtooth/sawtoothmodel.cpp
    sawtooth/sqlite3.c
    sawtooth/stand.cpp
    sawtooth/standcbmextension.cpp
)

set(SRCS_SAWTOOTH ${SAWTOOTH_SOURCES} ${SAWTOOTH_HEADERS})

add_library(${LIBNAME} ${LIB_MODE} ${SRCS_SAWTOOTH})

set_target_properties(${LIBNAME} 
    PROPERTIES
    VERSION ${MOJA_VERSION} SOVERSION ${MOJA_VERSION_MAJOR}
    DEFINE_SYMBOL ${LIBNAME_EXPORT}_EXPORTS)
    
target_link_libraries(
    ${LIBNAME} 
)

install(DIRECTORY sawtooth
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

install(TARGETS ${LIBNAME}
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        RUNTIME DESTINATION bin)