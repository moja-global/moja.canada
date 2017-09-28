set(PACKAGE "sawtooth")
set(LIBNAME "sawtooth")
string(TOUPPER "${PACKAGE}" LIBNAME_EXPORT)

#include_directories(include)

set(SAWTOOTH_HEADERS
    sawtooth/cbmparameter.h
    sawtooth/climatevariable.h
    sawtooth/constants.h
    sawtooth/dblayer.h
    sawtooth/defaultparameter.h
    sawtooth/disturbance_type.h
    sawtooth/equationset.h
    sawtooth/es1parameter.h
    sawtooth/es2parameter.h
    sawtooth/es3parameter.h
    sawtooth/exports.h
    sawtooth/mlr35parameter.h
    sawtooth/modelmeta.h
    sawtooth/mortalityprobability.h
    sawtooth/parameterset.h
    sawtooth/random.h
    sawtooth/results.h
    sawtooth/sawtootherror.h
    sawtooth/sawtoothexception.h
    sawtooth/sawtoothmodel.h
    sawtooth/speciesparameter.h
    sawtooth/sqlite3.h
    sawtooth/stand.h
    sawtooth/standcbmextension.h
    sawtooth/sawtoothmatrix.h
)

set(SAWTOOTH_SOURCES
    sawtooth/exports.cpp
    sawtooth/profiling_main.cpp
    sawtooth/results.cpp
    sawtooth/sawtoothmodel.cpp
    sawtooth/stand.cpp
    sawtooth/standcbmextension.cpp
    sawtooth/sqlite3.c
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

install(DIRECTORY include/moja
        DESTINATION include
        PATTERN ".svn" EXCLUDE)

install(TARGETS ${LIBNAME}
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX}
        RUNTIME DESTINATION bin)