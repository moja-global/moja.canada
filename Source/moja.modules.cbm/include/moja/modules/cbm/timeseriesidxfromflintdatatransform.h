#ifndef MOJA_MODULES_CBM_TIMESERIESIDXFROMFLINTDATATRANSFORM_H_
#define MOJA_MODULES_CBM_TIMESERIESIDXFROMFLINTDATATRANSFORM_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

#include <moja/flint/itransform.h>

namespace moja {

namespace datarepository {
	class IProviderLayer;
	class TileBlockCellIndexer;
}

namespace flint {
    class SpatialLocationInfo;
}

namespace modules {
namespace cbm {

class CBM_API TimeSeriesIdxFromFlintDataTransform : public flint::ITransform {
public:
	TimeSeriesIdxFromFlintDataTransform() {
		_lastCellHash = std::numeric_limits<size_t>::max();
	};

	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;

	const DynamicVar& value() const override;

private:
	const flint::ILandUnitController* _landUnitController;

	mutable std::shared_ptr<const flint::SpatialLocationInfo> _spatialLocationInfo;
	const datarepository::IProviderLayer* _layer;
	const datarepository::TileBlockCellIndexer* _providerIndexer;

	bool _subsame;
	int _startYear;
	int _dataPerYear;
	int _nYears;
    std::string _origin;

	mutable DynamicVar _cachedValue;
	mutable size_t _lastCellHash;
};

} // namespace cbm
} // namespace modules
} // namespace moja

#endif // MOJA_MODULES_CBM_TIMESERIESIDXFROMFLINTDATATRANSFORM_H_