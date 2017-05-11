#ifndef MOJA_MODULES_CBM_LIBRARYFACTORY_H_
#define MOJA_MODULES_CBM_LIBRARYFACTORY_H_

#include "moja/flint/librarymanager.h"

namespace moja {
namespace modules {

extern "C" MOJA_LIB_API int getModuleRegistrations(moja::flint::ModuleRegistration* outModuleRegistrations);
extern "C" MOJA_LIB_API int getTransformRegistrations(moja::flint::TransformRegistration* outTransformRegistrations);
extern "C" MOJA_LIB_API int getFlintDataRegistrations(moja::flint::FlintDataRegistration* outFlintDataRegistrations);

}
}

#endif
