#include "moja/modules/sawtooth/libraryfactory.h"
#include "moja/modules/sawtooth/sawtoothmodule.h"

#include <atomic>
#include <vector>
#include <Poco/Mutex.h>

namespace moja {
namespace modules {

    extern "C" {
        MOJA_LIB_API int getModuleRegistrations(moja::flint::ModuleRegistration* outModuleRegistrations) {
            int index = 0;
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "SawtoothModule", []() -> flint::IModule* { return new cbm::SawtoothModule(); } };
            return index;                                                                                  
        }

        MOJA_LIB_API int getTransformRegistrations(flint::TransformRegistration* outTransformRegistrations) {
            int index = 0;
            return index;
        }

        MOJA_LIB_API int getFlintDataRegistrations(moja::flint::FlintDataRegistration* outFlintDataRegistrations) {
            auto index = 0;
            return index;
        }
    }

}}