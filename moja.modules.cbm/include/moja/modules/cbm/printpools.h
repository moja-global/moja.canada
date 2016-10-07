#ifndef MOJA_MODULES_CBM_PRINTPOOLS_H_
#define MOJA_MODULES_CBM_PRINTPOOLS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API PrintPools {
	public:
		PrintPools(){}
		virtual ~PrintPools() = default;
		

		static void printMossPools(std::string message, flint::ILandUnitDataWrapper& landUnitData);
		static void printPeatlandPools(std::string message, flint::ILandUnitDataWrapper& landUnitData);
		static void printForestPools(std::string message, double mat, flint::ILandUnitDataWrapper& landUnitData);
	};
}}}
#endif
