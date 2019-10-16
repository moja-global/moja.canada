include(CMakeFindDependencyMacro)
find_dependency(moja REQUIRED COMPONENTS moja.flint)
find_dependency(Poco REQUIRED COMPONENTS Foundation Data DataSQLite DataODBC JSON)
find_dependency(PQXX REQUIRED)

if(NOT TARGET moja::moja.modules.cbm)
    include("${MojaModulesCBM_CMAKE_DIR}/moja.modules.cbmTargets.cmake")
endif()