if(CMAKE_SYSTEM MATCHES "Windows")

    find_path(PQXX_INCLUDE_DIR 
        NAMES pqxx
        PATH_SUFFIXES pqxx
    )
    
    find_library(PQXX_LIBRARY_DEBUG
        NAMES libpqxxd pqxxd
        PATH_SUFFIXES lib)

    find_library(PQXX_LIBRARY_RELEASE
        NAMES libpqxx pqxx
        PATH_SUFFIXES lib)

    include(SelectLibraryConfigurations)
    select_library_configurations(PQXX)

    set(PQXX_LIBRARY 
        debug ${PQXX_LIBRARY_DEBUG}
        optimized ${PQXX_LIBRARY_RELEASE}
                    CACHE STRING "PQXX library")
endif()

if (CMAKE_SYSTEM MATCHES "Linux" )

    file( TO_CMAKE_PATH "$ENV{PQXX_DIR}" _PQXX_DIR )

    find_library( PQXX_LIBRARY
      NAMES libpqxx pqxx
      PATHS
        ${_PQXX_DIR}/lib
        ${_PQXX_DIR}
        ${CMAKE_INSTALL_PREFIX}/lib
        ${CMAKE_INSTALL_PREFIX}/bin
        /usr/local/pgsql/lib
        /usr/local/lib
        /usr/lib
      DOC "Location of libpqxx library"
      NO_DEFAULT_PATH
    )

    find_path( PQXX_INCLUDE_DIR
      NAMES pqxx/pqxx
      PATHS
        ${_PQXX_DIR}/include
        ${_PQXX_DIR}
        ${CMAKE_INSTALL_PREFIX}/include
        /usr/local/pgsql/include
        /usr/local/include
        /usr/include
      DOC "Path to pqxx/pqxx header file. Do not include the 'pqxx' directory in this value."
      NO_DEFAULT_PATH
    )

endif(CMAKE_SYSTEM MATCHES "Linux")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PQXX
    REQUIRED_VARS PQXX_INCLUDE_DIR PQXX_LIBRARY
)

mark_as_advanced(PQXX_FOUND PQXX_INCLUDE_DIR PQXX_LIBRARY)

if(PQXX_FOUND)
    set(PQXX_INCLUDE_DIRS ${PQXX_INCLUDE_DIR})
    if(NOT PQXX_LIBRARIES)
        set(PQXX_LIBRARIES ${PQXX_LIBRARY})
    endif()

    if(NOT TARGET PQXX::PQXX)
        add_library(PQXX::PQXX UNKNOWN IMPORTED)
        set_target_properties(PQXX::PQXX PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            INTERFACE_INCLUDE_DIRECTORIES "${PQXX_INCLUDE_DIR}"
        )

        if (PQXX_LIBRARY_RELEASE)
            set_property(TARGET PQXX::PQXX APPEND PROPERTY 
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties( PQXX::PQXX PROPERTIES 
                IMPORTED_LOCATION_RELEASE ${PQXX_LIBRARY_RELEASE} )
        endif ()

        if (PQXX_LIBRARY_DEBUG)
            set_property(TARGET PQXX::PQXX APPEND PROPERTY 
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties( PQXX::PQXX PROPERTIES 
                IMPORTED_LOCATION_DEBUG ${PQXX_LIBRARY_DEBUG} )
        endif ()

        if(NOT PQXX_LIBRARY_DEBUG AND NOT PQXX_LIBRARY_RELEASE)
            set_target_properties( PQXX::PQXX PROPERTIES 
                IMPORTED_LOCATION ${PQXX_LIBRARY} )
        endif ()

    endif()
endif()
