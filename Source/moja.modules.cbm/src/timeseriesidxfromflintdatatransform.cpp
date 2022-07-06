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

	/**
	 * Configuration function
	 * 
	 * Assign TimeSeriesIdxFromFlintDataTransform._landUnitController as parameter landUnitController&, 
	 * TimeSeriesIdxFromFlintDataTransform._subsame, TimeSeriesIdxFromFlintDataTransform._startYear, TimeSeriesIdxFromFlintDataTransform._dataPerYear, 
	 * TimeSeriesIdxFromFlintDataTransform._nYears value of "sub_same", "start_year", "data_per_year" in 
	 * parameter config, _origin, value of "origin" if it exists in parameter config, else the string "start_sim",
	 * TimeSeriesIdxFromFlintDataTransform._spatialLocationInfo value of variable "spatialLocationInfo" in parameter landUnitController \n
	 * Assign the value of the getProvider() on parameter dataRepository with argument config["provider"] to a variable provider,
	 * set the result of the indexer() method on variable provider to TimeSeriesIdxFromFlintDataTransform._providerIndex, and result of getLayer() on
	 * variable provider with argument config["data_id"] to TimeSeriesIdxFromFlintDataTransform._layer
	 * 
	 * @param config DynamicObject 
	 * @param controller flint::ILandUnitController&
	 * @param dataRepository moja::datarepository::DataRepository& 
	 * @return void
	 * ***************************/
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

	/**
	 * Assign TimeSeriesIdxFromFlintDataTransform._landUnitController as parameter &landUnitController, 
	 * TimeSeriesIdxFromFlintDataTransform._spatialLocationInfo as statically
	 * casted pointer to variable "spatialLocationInfo" in parameter landUnitController
	 * 
	 * @param landUnitController flint::ILandUnitController&
	 * @return void
	 */
	void TimeSeriesIdxFromFlintDataTransform::controllerChanged(const flint::ILandUnitController& landUnitController) {
		_landUnitController = &landUnitController;
		_spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(landUnitController.getVariable("spatialLocationInfo")->value().extract<std::shared_ptr<flint::IFlintData>>());
	};

	/**
	 * Instantiate an object of datarepository::CellIdx, with the tile index, block index, cell index 
	 * and provider index corresponding to the current spatial location \n
	 * If the value of TimeSeriesIdxFromFlintDataTransform._lastCellHash is not the same as the current cell hash, then assign the value of the current cell hash to 
	 * TimeSeriesIdxFromFlintDataTransform._lastCellHash \n
	 * For each value in the result of getValueByCellIndex() on TimeSeriesIdxFromFlintDataTransform._layer, if
	 * value.is_initialized() is true, add the value to a vector raw. \n
	 * If atleast one value returned a true value for value.is_initialized(), instantiate an object of TimeSeries
	 * and assgin it to TimeSeriesIdxFromFlintDataTransform._cachedValue \n
	 * If none of the values return true for value.is_initialized(), assign TimeSeriesIdxFromFlintDataTransform._cachedValue to DynamicVar() \n
	 * Return TimeSeriesIdxFromFlintDataTransform._cachedValue
	 * 
	 * @return DynamicVar&
	 */
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