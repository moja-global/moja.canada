set(PACKAGE "cbm")
set(LIBNAME "moja.modules.${PACKAGE}")
string(TOUPPER "${PACKAGE}" LIBNAME_EXPORT)

find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(TBB)
if(TBB_FOUND)
	include_directories(${TBB_INCLUDE_DIRS})
	if(CMAKE_SYSTEM MATCHES "Windows")
		set( TBB_TBB_LIBRARY debug     ${TBB_tbb_LIBRARY_DEBUG}
							 optimized ${TBB_tbb_LIBRARY_RELEASE}
						CACHE STRING "TBB library text")
	endif() 

	if (CMAKE_SYSTEM MATCHES "Linux" )
		set( TBB_TBB_LIBRARY ${TBB_tbb_LIBRARY_RELEASE})
	endif() 
endif()

# HEADERS AND SOURCE

include_directories( include ../../moja.core/include ../../moja.flint/include ../../moja.datarepository/include)

configure_file(../../templates/exports.h ${CMAKE_CURRENT_SOURCE_DIR}/include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h)

set(MOJA_MODULES_HEADERS
    include/moja/modules/${PACKAGE}/_modules.${PACKAGE}_exports.h
    include/moja/modules/${PACKAGE}/libraryfactory.h)

set(MOJA_MODULES_SOURCES
    src/libraryfactory.cpp)

set(MOJA_CBM_HEADERS
	include/moja/modules/${PACKAGE}/rootbiomassequation.h
	include/moja/modules/${PACKAGE}/standcomponent.h
	include/moja/modules/${PACKAGE}/abovegroundbiomasscarbonincrement.h
	include/moja/modules/${PACKAGE}/cbmaggregatorlandunitdata.h
    include/moja/modules/${PACKAGE}/cbmaggregatorsqlitewriter.h
	include/moja/modules/${PACKAGE}/cbmbuildlandunitmodule.h
	include/moja/modules/${PACKAGE}/cbmdecaymodule.h
	include/moja/modules/${PACKAGE}/cbmdisturbanceeventmodule.h
    include/moja/modules/${PACKAGE}/cbmtransitionrulesmodule.h
	include/moja/modules/${PACKAGE}/cbmlandunitdatatransform.h
	include/moja/modules/${PACKAGE}/cbmsequencer.h
	include/moja/modules/${PACKAGE}/cbmspinupdisturbancemodule.h
	include/moja/modules/${PACKAGE}/cbmspinupsequencer.h
	include/moja/modules/${PACKAGE}/componentbiomasscarboncurve.h
	include/moja/modules/${PACKAGE}/growthcurvetransform.h
	include/moja/modules/${PACKAGE}/helper.h
	include/moja/modules/${PACKAGE}/lmeval.h
	include/moja/modules/${PACKAGE}/lmmin.h
	include/moja/modules/${PACKAGE}/outputerstreamfluxpostnotify.h
	include/moja/modules/${PACKAGE}/outputerstreampostnotify.h
	include/moja/modules/${PACKAGE}/perdfactor.h
	include/moja/modules/${PACKAGE}/record.h
	include/moja/modules/${PACKAGE}/rootbiomasscarbonincrement.h
	include/moja/modules/${PACKAGE}/smoother.h
	include/moja/modules/${PACKAGE}/standbiomasscarboncurve.h
	include/moja/modules/${PACKAGE}/standgrowthcurve.h
	include/moja/modules/${PACKAGE}/treespecies.h
	include/moja/modules/${PACKAGE}/treeyieldtable.h
	include/moja/modules/${PACKAGE}/volumetobiomasscarbongrowth.h
	include/moja/modules/${PACKAGE}/volumetobiomassconverter.h
	include/moja/modules/${PACKAGE}/yieldtablegrowthmodule.h
	include/moja/modules/${PACKAGE}/cbmlandclasstransitionmodule.h
	include/moja/modules/${PACKAGE}/foresttypeconfiguration.h	
	include/moja/modules/${PACKAGE}/mossgrowthmodule.h
	include/moja/modules/${PACKAGE}/mossturnovermodule.h
	include/moja/modules/${PACKAGE}/mossdecaymodule.h	
	include/moja/modules/${PACKAGE}/mossdisturbancemodule.h
	include/moja/modules/${PACKAGE}/standgrowthcurvefactory.h
	include/moja/modules/${PACKAGE}/esgymmodule.h
	include/moja/modules/${PACKAGE}/peatlandgrowthmodule.h
	include/moja/modules/${PACKAGE}/peatlandparameters.h
	include/moja/modules/${PACKAGE}/peatlandgrowthparameters.h
	include/moja/modules/${PACKAGE}/peatlandturnoverparameters.h
	include/moja/modules/${PACKAGE}/peatlanddecayparameters.h	
	include/moja/modules/${PACKAGE}/peatlandgrowthcurve.h
	include/moja/modules/${PACKAGE}/peatlandturnovermodule.h
	include/moja/modules/${PACKAGE}/peatlanddecaymodule.h
	include/moja/modules/${PACKAGE}/peatlanddisturbancemodule.h
	include/moja/modules/${PACKAGE}/peatlandfireparameters.h
	include/moja/modules/${PACKAGE}/peatlandpreparemodule.h
	include/moja/modules/${PACKAGE}/printpools.h	
	include/moja/modules/${PACKAGE}/esgymspinupsequencer.h
)

set(MOJA_CBM_SOURCES
    src/cbmaggregatorlandunitdata.cpp
	src/cbmaggregatorsqlitewriter.cpp
	src/cbmbuildlandunitmodule.cpp
	src/cbmdecaymodule.cpp
	src/cbmdisturbanceeventmodule.cpp
	src/cbmlandunitdatatransform.cpp
	src/cbmsequencer.cpp
	src/cbmspinupdisturbancemodule.cpp
	src/cbmspinupsequencer.cpp
    src/cbmtransitionrulesmodule.cpp
	src/componentbiomasscarboncurve.cpp
	src/growthcurvetransform.cpp
	src/lmeval.cpp
	src/lmmin.cpp
	src/outputerstreamfluxpostnotify.cpp
	src/outputerstreampostnotify.cpp
	src/perdfactor.cpp
	src/record.cpp
	src/smoother.cpp
	src/standbiomasscarboncurve.cpp
	src/standgrowthcurve.cpp
	src/treespecies.cpp
	src/treeyieldtable.cpp
	src/volumetobiomasscarbongrowth.cpp
	src/volumetobiomassconverter.cpp
	src/yieldtablegrowthmodule.cpp
    src/cbmlandclasstransitionmodule.cpp
    src/standcomponent.cpp	
	src/mossgrowthmodule.cpp
	src/mossturnovermodule.cpp
	src/mossdecaymodule.cpp
	src/mossdisturbancemodule.cpp
	src/standgrowthcurvefactory.cpp	

	src/esgymmodule.cpp
	src/peatlandgrowthmodule.cpp	
	src/peatlandparameters.cpp	
	src/peatlandgrowthparameters.cpp	
	src/peatlandturnoverparameters.cpp
	src/peatlanddecayparameters.cpp		
	src/peatlandgrowthcurve.cpp
	src/peatlandturnovermodule.cpp
	src/peatlanddecaymodule.cpp
	src/peatlanddisturbancemodule.cpp
	src/peatlandfireparameters.cpp
	src/peatlandpreparemodule.cpp
	src/printpools.cpp
	src/esgymspinupsequencer.cpp
)

set(SRCS_CBM ${MOJA_CBM_SOURCES} ${MOJA_CBM_HEADERS})
set(SRCS ${MOJA_MODULES_SOURCES} ${MOJA_MODULES_HEADERS})

add_definitions( -DPOCO_NO_AUTOMATIC_LIBS )

add_library(${LIBNAME} ${LIB_MODE} ${SRCS} ${SRCS_CBM})

set_target_properties(${LIBNAME} 
    PROPERTIES
    VERSION ${MOJA_VERSION} SOVERSION ${MOJA_VERSION_MAJOR}
    DEFINE_SYMBOL ${LIBNAME_EXPORT}_EXPORTS)

target_link_libraries(
	${LIBNAME} 
	moja.core 
	moja.flint
	${Poco_FOUNDATION} 
	${Poco_DATA} 
	${Poco_DATA_SQLITE}
	${TBB_TBB_LIBRARY}
	)

# Set local include path
include_directories(${Poco_INCLUDE_DIRS})

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
