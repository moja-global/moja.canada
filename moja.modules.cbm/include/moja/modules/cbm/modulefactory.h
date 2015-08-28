#ifndef MOJA_Modules_CBM_ModuleFactory_H_
#define MOJA_Modules_CBM_ModuleFactory_H_

#include "moja/flint/librarymanager.h"

namespace moja {
namespace modules {

    extern "C" MOJA_LIB_API int getModuleRegistrations(
        moja::flint::ModuleRegistration* outModuleRegistrations);

    extern "C" MOJA_LIB_API int getTransformRegistrations(
        moja::flint::TransformRegistration* outTransformRegistrations);

}}

#endif
