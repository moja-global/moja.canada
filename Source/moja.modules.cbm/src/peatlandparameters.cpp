#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * @brief Constructor
	 * 
	 * Initialise PeatlandParameters.spuId as parameter spuId, \n
	 * PeatlandParameters._peatlandType as parameter peatlandType, \n
	 * PeatlandParameters._landCoverType as parameter peatlandTreeClassifier
	 * 
	 * @param spuId int
	 * @param peatlandType PeatlandType
	 * @param peatlandTreeClassifier PeatlandLandCoverType
	 * **********************/

	PeatlandParameters::PeatlandParameters(int spuId, PeatlandType peatlandType, PeatlandLandCoverType peatlandTreeClassifier) {
		_spuId = spuId;
		_peatlandType = peatlandType;
		_landCoverType = peatlandTreeClassifier;
	}	

}}}