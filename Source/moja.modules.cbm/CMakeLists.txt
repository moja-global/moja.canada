set(PACKAGE "cbm")
set(LIBNAME "moja.modules.${PACKAGE}")
string(REPLACE "." "_" NEW_PACKAGE "${PACKAGE}")
string(TOUPPER "${NEW_PACKAGE}" LIBNAME_EXPORT)

find_package(Boost COMPONENTS system thread filesystem date_time chrono program_options log log_setup REQUIRED)
find_package(Poco COMPONENTS Foundation Data DataSQLite DataODBC JSON REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(PQXX REQUIRED)
find_package(ODBC REQUIRED)
find_package(moja COMPONENTS moja.flint REQUIRED)

include_directories(include)
configure_file(../templates/exports.h
               ${CMAKE_CURRENT_SOURCE_DIR}/include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h)
configure_file(../templates/version.h
               ${CMAKE_CURRENT_SOURCE_DIR}/include/moja/modules/${PACKAGE}/version.h)

set(PROJECT_HEADERS
    include/moja/modules/${PACKAGE}/libraryfactory.h
    include/moja/modules/${PACKAGE}/version.h
    include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h
)

set(PROJECT_SOURCES
    src/libraryfactory.cpp
)

set(PROJECT_MODULE_HEADERS
    include/moja/modules/${PACKAGE}/abovegroundbiomasscarbonincrement.h
    include/moja/modules/${PACKAGE}/ageclasshelper.h
    include/moja/modules/${PACKAGE}/cbmageindicators.h
    include/moja/modules/${PACKAGE}/cbmaggregatorcsvwriter.h
    include/moja/modules/${PACKAGE}/cbmaggregatorlandunitdata.h
    include/moja/modules/${PACKAGE}/cbmaggregatorlibpqxxwriter.h
    include/moja/modules/${PACKAGE}/cbmaggregatorpostgresqlwriter.h
    include/moja/modules/${PACKAGE}/cbmaggregatorsqlitewriter.h
    include/moja/modules/${PACKAGE}/cbmbuildlandunitmodule.h
    include/moja/modules/${PACKAGE}/cbmdecaymodule.h
    include/moja/modules/${PACKAGE}/cbmdisturbanceeventmodule.h
    include/moja/modules/${PACKAGE}/cbmdisturbancelistener.h
    include/moja/modules/${PACKAGE}/cbmflataggregatorlandunitdata.h
    include/moja/modules/${PACKAGE}/cbmtransitionrulesmodule.h
    include/moja/modules/${PACKAGE}/cbmlandclasstransitionmodule.h
    include/moja/modules/${PACKAGE}/cbmmodulebase.h
    include/moja/modules/${PACKAGE}/cbmpartitioningmodule.h
    include/moja/modules/${PACKAGE}/cbmsequencer.h
    include/moja/modules/${PACKAGE}/cbmspinupdisturbancemodule.h
    include/moja/modules/${PACKAGE}/cbmspinupsequencer.h
    include/moja/modules/${PACKAGE}/cbmpeatlandspinupoutput.h
    include/moja/modules/${PACKAGE}/componentbiomasscarboncurve.h
    include/moja/modules/${PACKAGE}/disturbancemonitormodule.h
    include/moja/modules/${PACKAGE}/esgymmodule.h
    include/moja/modules/${PACKAGE}/esgymspinupsequencer.h
    include/moja/modules/${PACKAGE}/flatrecord.h
    include/moja/modules/${PACKAGE}/foresttypeconfiguration.h
    include/moja/modules/${PACKAGE}/growthmultipliermodule.h
    include/moja/modules/${PACKAGE}/helper.h
    include/moja/modules/${PACKAGE}/lmeval.h
    include/moja/modules/${PACKAGE}/lmmin.h
    include/moja/modules/${PACKAGE}/mossdecaymodule.h
    include/moja/modules/${PACKAGE}/mossdisturbancemodule.h
    include/moja/modules/${PACKAGE}/mossgrowthmodule.h
    include/moja/modules/${PACKAGE}/mossturnovermodule.h
    include/moja/modules/${PACKAGE}/outputerstreamfluxpostnotify.h
    include/moja/modules/${PACKAGE}/outputerstreampostnotify.h
    include/moja/modules/${PACKAGE}/peatlanddecaymodule.h
    include/moja/modules/${PACKAGE}/peatlanddecayparameters.h
    include/moja/modules/${PACKAGE}/peatlanddisturbancemodule.h    
    include/moja/modules/${PACKAGE}/peatlandfireparameters.h
    include/moja/modules/${PACKAGE}/peatlandgrowthcurve.h
    include/moja/modules/${PACKAGE}/peatlandgrowthmodule.h
    include/moja/modules/${PACKAGE}/peatlandgrowthparameters.h
    include/moja/modules/${PACKAGE}/peatlandparameters.h 
    include/moja/modules/${PACKAGE}/peatlandturnoverparameters.h
    include/moja/modules/${PACKAGE}/peatlandturnovermodule.h
    include/moja/modules/${PACKAGE}/peatlandspinupturnovermodule.h
    include/moja/modules/${PACKAGE}/peatlandturnovermodulebase.h
	include/moja/modules/${PACKAGE}/peatlandwtdbasefch4parameters.h
    include/moja/modules/${PACKAGE}/perdfactor.h
    include/moja/modules/${PACKAGE}/printpools.h
    include/moja/modules/${PACKAGE}/record.h
    include/moja/modules/${PACKAGE}/rootbiomasscarbonincrement.h
    include/moja/modules/${PACKAGE}/rootbiomassequation.h
    include/moja/modules/${PACKAGE}/smoother.h
    include/moja/modules/${PACKAGE}/standbiomasscarboncurve.h
    include/moja/modules/${PACKAGE}/standcomponent.h
    include/moja/modules/${PACKAGE}/standgrowthcurve.h
    include/moja/modules/${PACKAGE}/standgrowthcurvefactory.h
    include/moja/modules/${PACKAGE}/standmaturitymodule.h
    include/moja/modules/${PACKAGE}/timeseries.h
    include/moja/modules/${PACKAGE}/timeseriesutils.h
    include/moja/modules/${PACKAGE}/treespecies.h
    include/moja/modules/${PACKAGE}/treeyieldtable.h
    include/moja/modules/${PACKAGE}/turnoverrates.h
    include/moja/modules/${PACKAGE}/volumetobiomasscarbongrowth.h
    include/moja/modules/${PACKAGE}/volumetobiomassconverter.h
    include/moja/modules/${PACKAGE}/yieldtablegrowthmodule.h
    include/moja/modules/${PACKAGE}/smalltreegrowthmodule.h
    include/moja/modules/${PACKAGE}/smalltreegrowthcurve.h
    include/moja/modules/${PACKAGE}/peatlandspinupnext.h
	include/moja/modules/${PACKAGE}/peatlands.h
)

set(PROJECT_MODULE_SOURCES
    src/ageclasshelper.cpp
    src/cbmageindicators.cpp
    src/cbmaggregatorcsvwriter.cpp
    src/cbmaggregatorlandunitdata.cpp
    src/cbmaggregatorlibpqxxwriter.cpp
    src/cbmaggregatorpostgresqlwriter.cpp
    src/cbmaggregatorsqlitewriter.cpp
    src/cbmbuildlandunitmodule.cpp
    src/cbmdecaymodule.cpp
    src/cbmdisturbanceeventmodule.cpp
    src/cbmdisturbancelistener.cpp
    src/cbmflataggregatorlandunitdata.cpp
    src/cbmlandclasstransitionmodule.cpp
    src/cbmpartitioningmodule.cpp
    src/cbmsequencer.cpp
    src/cbmspinupdisturbancemodule.cpp
    src/cbmpeatlandspinupoutput.cpp
    src/cbmspinupsequencer.cpp
    src/cbmtransitionrulesmodule.cpp
    src/componentbiomasscarboncurve.cpp
    src/disturbancemonitormodule.cpp
    src/esgymmodule.cpp
    src/esgymspinupsequencer.cpp
    src/flatrecord.cpp
    src/growthmultipliermodule.cpp
    src/lmeval.cpp
    src/lmmin.cpp
    src/mossdecaymodule.cpp
    src/mossdisturbancemodule.cpp
    src/mossgrowthmodule.cpp
    src/mossturnovermodule.cpp
    src/outputerstreamfluxpostnotify.cpp
    src/outputerstreampostnotify.cpp
    src/peatlanddecaymodule.cpp
    src/peatlanddecayparameters.cpp
    src/peatlanddisturbancemodule.cpp   
    src/peatlandfireparameters.cpp
    src/peatlandgrowthcurve.cpp
    src/peatlandgrowthmodule.cpp
    src/peatlandgrowthparameters.cpp
    src/peatlandparameters.cpp  
    src/peatlandspinupturnovermodule.cpp
    src/peatlandturnovermodule.cpp
    src/peatlandturnovermodulebase.cpp
    src/peatlandturnoverparameters.cpp
	src/peatlandwtdbasefch4parameters.cpp
    src/perdfactor.cpp
    src/printpools.cpp
    src/record.cpp
    src/smoother.cpp
    src/standbiomasscarboncurve.cpp
    src/standcomponent.cpp
    src/standgrowthcurve.cpp
    src/standgrowthcurvefactory.cpp
    src/standmaturitymodule.cpp
    src/timeseries.cpp
    src/treespecies.cpp
    src/treeyieldtable.cpp
    src/volumetobiomasscarbongrowth.cpp
    src/volumetobiomassconverter.cpp
    src/yieldtablegrowthmodule.cpp
    src/smalltreegrowthmodule.cpp
    src/smalltreegrowthcurve.cpp
    src/peatlandspinupnext.cpp
)

set(PROJECT_TRANSFORM_HEADERS
    include/moja/modules/${PACKAGE}/cbmlandunitdatatransform.h
    include/moja/modules/${PACKAGE}/dynamicgrowthcurvetransform.h
    include/moja/modules/${PACKAGE}/dynamicgrowthcurvelookuptransform.h
    include/moja/modules/${PACKAGE}/growthcurvetransform.h
	include/moja/modules/${PACKAGE}/peatlandgrowthcurvetransform.h
    include/moja/modules/${PACKAGE}/timeseriesidxfromflintdatatransform.h
    include/moja/modules/${PACKAGE}/transitionruletransform.h
)

set(PROJECT_TRANSFORM_SOURCES
    src/cbmlandunitdatatransform.cpp
    src/dynamicgrowthcurvetransform.cpp
    src/dynamicgrowthcurvelookuptransform.cpp
    src/growthcurvetransform.cpp
	src/peatlandgrowthcurvetransform.cpp
    src/timeseriesidxfromflintdatatransform.cpp
    src/transitionruletransform.cpp
)

set(PROJECT_FLINTDATA_HEADERS
)

set(PROJECT_FLINTDATA_SOURCES
)

set(PROJECT_PROVIDER_HEADERS
)

set(PROJECT_PROVIDER_SOURCES
)

source_group("header files\\other"					FILES ${PROJECT_HEADERS})
source_group("source files\\other"					FILES ${PROJECT_SOURCES})
source_group("header files\\modules"				FILES ${PROJECT_MODULE_HEADERS})
source_group("source files\\modules"				FILES ${PROJECT_MODULE_SOURCES})
source_group("header files\\transforms"				FILES ${PROJECT_TRANSFORM_HEADERS})
source_group("source files\\transforms"				FILES ${PROJECT_TRANSFORM_SOURCES})
source_group("header files\\flintdata"				FILES ${PROJECT_FLINTDATA_HEADERS})
source_group("source files\\flintdata"				FILES ${PROJECT_FLINTDATA_SOURCES})
source_group("header files\\providers"				FILES ${PROJECT_PROVIDER_HEADERS})
source_group("source files\\providers"				FILES ${PROJECT_PROVIDER_SOURCES})

set(SRCS
	${PROJECT_SOURCES} ${PROJECT_HEADERS}
	${PROJECT_MODULE_SOURCES} ${PROJECT_MODULE_HEADERS}
	${PROJECT_TRANSFORM_SOURCES} ${PROJECT_TRANSFORM_HEADERS}
	${PROJECT_FLINTDATA_SOURCES} ${PROJECT_FLINTDATA_HEADERS}
    ${PROJECT_PROVIDER_HEADERS} ${PROJECT_PROVIDER_SOURCES}
)

add_library(${LIBNAME} ${LIB_MODE} ${SRCS})
add_library(${PROJECT_NAME}::${LIBNAME} ALIAS ${LIBNAME})

set_target_properties(${LIBNAME}
    PROPERTIES
    VERSION ${MOJA_VERSION} SOVERSION ${MOJA_VERSION_MAJOR}
    DEFINE_SYMBOL ${LIBNAME_EXPORT}_EXPORTS
)

target_include_directories(${LIBNAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${LIBNAME}
    PUBLIC
        moja::moja.flint
        Poco::Foundation
        Poco::Data
        Poco::DataSQLite
        Poco::DataODBC
        Poco::JSON
        PQXX::PQXX
        ODBC::ODBC
		PostgreSQL::PostgreSQL
)

##############################################
# Installation instructions

include(GNUInstallDirs)

MOJA_INSTALL(${LIBNAME})
MOJA_GENERATE_PACKAGE(${LIBNAME})

if(ENABLE_TESTS)
    add_subdirectory(tests)

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xvf ${CMAKE_CURRENT_SOURCE_DIR}/../../Examples/data.zip
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
		OUTPUT_QUIET)
endif()
