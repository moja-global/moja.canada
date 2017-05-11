#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {

	PeatlandParameters::PeatlandParameters(int spuId, PeatlandType peatlandType, PeatlandForestType peatlandTreeClassifier) {
		_spuId = spuId;
		_peatlandType = peatlandType;
		_peatlandTreeClassifier = peatlandTreeClassifier;
	}	

}}}