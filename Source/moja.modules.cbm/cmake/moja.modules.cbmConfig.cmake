include(CMakeFindDependencyMacro)
find_dependency(moja REQUIRED COMPONENTS moja.flint)
find_dependency(Poco REQUIRED COMPONENTS Foundation Data DataSQLite DataODBC JSON)
find_dependency(PostgreSQL REQUIRED)
find_dependency(PQXX REQUIRED)

if(NOT TARGET CBM::moja.modules.cbm)
    include("${CMAKE_CURRENT_LIST_DIR}/moja.modules.cbmTargets.cmake")
endif()