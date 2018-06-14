set(PACKAGE "cbm")
set(LIBNAME "moja.modules.${PACKAGE}")
string(TOUPPER "${PACKAGE}" LIBNAME_EXPORT)

# HEADERS AND SOURCE

include_directories(include
    ${PROJECT_SOURCE_DIR}/sawtooth)

configure_file(../templates/exports.h
               ${CMAKE_CURRENT_SOURCE_DIR}/include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h)

set(MOJA_MODULES_HEADERS
    include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h
    include/moja/modules/${PACKAGE}/libraryfactory.h)

set(MOJA_MODULES_SOURCES
    src/libraryfactory.cpp)

set(MOJA_CBM_HEADERS
	include/moja/modules/${PACKAGE}/abovegroundbiomasscarbonincrement.h
	include/moja/modules/${PACKAGE}/cbmageindicators.h
	include/moja/modules/${PACKAGE}/cbmaggregatorlandunitdata.h
    include/moja/modules/${PACKAGE}/cbmaggregatorpostgresqlwriter.h
    include/moja/modules/${PACKAGE}/cbmaggregatorsqlitewriter.h
	include/moja/modules/${PACKAGE}/cbmbuildlandunitmodule.h
	include/moja/modules/${PACKAGE}/cbmdecaymodule.h
	include/moja/modules/${PACKAGE}/cbmdisturbanceeventmodule.h
    include/moja/modules/${PACKAGE}/cbmdisturbancelistener.h
    include/moja/modules/${PACKAGE}/cbmtransitionrulesmodule.h
	include/moja/modules/${PACKAGE}/cbmlandclasstransitionmodule.h
	include/moja/modules/${PACKAGE}/cbmlandunitdatatransform.h
	include/moja/modules/${PACKAGE}/cbmmodulebase.h
	include/moja/modules/${PACKAGE}/cbmsequencer.h
	include/moja/modules/${PACKAGE}/cbmspinupdisturbancemodule.h
	include/moja/modules/${PACKAGE}/cbmspinupsequencer.h
	include/moja/modules/${PACKAGE}/componentbiomasscarboncurve.h
	include/moja/modules/${PACKAGE}/esgymmodule.h
	include/moja/modules/${PACKAGE}/esgymspinupsequencer.h
	include/moja/modules/${PACKAGE}/foresttypeconfiguration.h	
	include/moja/modules/${PACKAGE}/growthcurvetransform.h
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
	include/moja/modules/${PACKAGE}/peatlandpreparemodule.h
	include/moja/modules/${PACKAGE}/peatlandturnoverparameters.h
	include/moja/modules/${PACKAGE}/peatlandturnovermodule.h
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
	include/moja/modules/${PACKAGE}/treespecies.h
	include/moja/modules/${PACKAGE}/treeyieldtable.h
	include/moja/modules/${PACKAGE}/volumetobiomasscarbongrowth.h
	include/moja/modules/${PACKAGE}/volumetobiomassconverter.h
	include/moja/modules/${PACKAGE}/yieldtablegrowthmodule.h
    include/moja/modules/${PACKAGE}/sawtoothmodule.h
	include/moja/modules/${PACKAGE}/smalltreegrowthmodule.h
	include/moja/modules/${PACKAGE}/smalltreegrowthcurve.h
	include/moja/modules/${PACKAGE}/peatlandspinupnext.h
)

set(MOJA_CBM_SOURCES
	src/cbmageindicators.cpp
    src/cbmaggregatorlandunitdata.cpp
	src/cbmaggregatorpostgresqlwriter.cpp
	src/cbmaggregatorsqlitewriter.cpp
	src/cbmbuildlandunitmodule.cpp
	src/cbmdecaymodule.cpp
	src/cbmdisturbanceeventmodule.cpp
    src/cbmdisturbancelistener.cpp
    src/cbmlandclasstransitionmodule.cpp
	src/cbmlandunitdatatransform.cpp
	src/cbmsequencer.cpp
	src/cbmspinupdisturbancemodule.cpp
	src/cbmspinupsequencer.cpp
    src/cbmtransitionrulesmodule.cpp
	src/componentbiomasscarboncurve.cpp
	src/esgymmodule.cpp
	src/esgymspinupsequencer.cpp
	src/growthcurvetransform.cpp
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
	src/peatlandpreparemodule.cpp
	src/peatlandturnovermodule.cpp
	src/peatlandturnoverparameters.cpp
	src/perdfactor.cpp
	src/printpools.cpp
	src/record.cpp
	src/smoother.cpp
	src/standbiomasscarboncurve.cpp
    src/standcomponent.cpp	
	src/standgrowthcurve.cpp
	src/standgrowthcurvefactory.cpp	
	src/treespecies.cpp
	src/treeyieldtable.cpp
	src/volumetobiomasscarbongrowth.cpp
	src/volumetobiomassconverter.cpp
	src/yieldtablegrowthmodule.cpp
    src/sawtoothmodule.cpp
	src/smalltreegrowthmodule.cpp
	src/smalltreegrowthcurve.cpp
	src/peatlandspinupnext.cpp
)

set(SRCS_CBM ${MOJA_CBM_SOURCES} ${MOJA_CBM_HEADERS})
set(SRCS_SAWTOOTH ${SRCS_SAWTOOTH})
set(SRCS ${MOJA_MODULES_SOURCES} ${MOJA_MODULES_HEADERS} )

add_library(${LIBNAME} ${LIB_MODE} ${SRCS} ${SRCS_CBM} ${SRCS_SAWTOOTH})

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
)
add_dependencies(${LIBNAME} sawtooth)

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
