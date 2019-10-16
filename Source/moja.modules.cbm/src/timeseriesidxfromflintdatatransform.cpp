#include <moja/flint/ilandunitcontroller.h>
#include <moja/flint/spatiallocationinfo.h>
#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>
#include <moja/datarepository/iproviderspatialrasterinterface.h>

#include "moja/modules/cbm/timeseriesidxfromflintdatatransform.h"
#include "moja/modules/cbm/timeseries.h"

#include <boost/optional/optional.hpp>

namespace moja {
namespace modules {
namespace cbm {

void TimeSeriesIdxFromFlintDataTransform::configure(DynamicObject config, const flint::ILandUnitController& landUnitController, moja::datarepository::DataRepository& dataRepository) {
	_landUnitController = &landUnitController;
	auto provider = std::static_pointer_cast<moja::datarepository::IProviderSpatialRasterInterface>(dataRepository.getProvider(config["provider"].convert<std::string>()));
	_providerIndexer = &provider->indexer();
	_layer = provider->getLayer(config["data_id"].convert<std::string>());
	_subsame = config["sub_same"].convert<bool>();
	_startYear = config["start_year"].convert<int>();
	_dataPerYear = config["data_per_year"].convert<int>();
	_nYears = config["n_years"].convert<int>();
    _origin = config.contains("origin") ? config["origin"].convert<std::string>() : "start_sim";

	_spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(landUnitController.getVariable("spatialLocationInfo")->value().extract<std::shared_ptr<flint::IFlintData>>());
}

void TimeSeriesIdxFromFlintDataTransform::controllerChanged(const flint::ILandUnitController& landUnitController) {
	_landUnitController = &landUnitController;
	_spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(landUnitController.getVariable("spatialLocationInfo")->value().extract<std::shared_ptr<flint::IFlintData>>());
};

const DynamicVar& TimeSeriesIdxFromFlintDataTransform::value() const {
	datarepository::CellIdx cell(_spatialLocationInfo->_tileIdx, _spatialLocationInfo->_blockIdx, _spatialLocationInfo->_cellIdx, *_providerIndexer);
	auto layerIdx = _layer->indexer().convertIndex(cell);
	auto cellHash = layerIdx.hash();
	if (_lastCellHash != cellHash) {
		_lastCellHash = cellHash;
        bool atLeastOneValue = false;
		auto series = _layer->getValueByCellIndex(cell).extract<std::vector<boost::optional<float>>>();
		std::vector<boost::optional<double>> raw;
		for (const auto& f : series) {
            if (f.is_initialized()) {
                raw.push_back(boost::optional<double> { f.is_initialized(), f.get() });
                atLeastOneValue = true;
            }
            else {
                raw.push_back(boost::none);
            }
		}

        _cachedValue = DynamicVar();
        if (atLeastOneValue) {
            const auto& timing = _landUnitController->timing();
            TimeSeries ts(_startYear, _dataPerYear, _nYears, _subsame, raw, _origin == "calendar" ? DateOrigin::Calendar : DateOrigin::StartSim);
            ts.setTiming(&timing);
            _cachedValue = ts;
        }
	}

	return _cachedValue;
}

} // namespace cbm
} // namespace modules
} // namespace moja