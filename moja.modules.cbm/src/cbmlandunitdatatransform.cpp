#include "moja/modules/cbm/cbmlandunitdatatransform.h"
#include "moja/datarepository/iproviderinterface.h"

#include <limits>

namespace moja {
	namespace modules {
		namespace cbm {

			void CBMLandUnitDataTransform::configure(DynamicObject config, const flint::ILandUnitController& landUnitController, datarepository::DataRepository& dataRepository) {
				_landUnitController = &landUnitController;
				_dataRepository = &dataRepository;
				//_landUnitStaticData = _landUnitController->getVariable("LandUnitStaticDataQuery");

				std::string providerName = config["provider"];
				_provider = std::static_pointer_cast<datarepository::IProviderRelationalInterface>(_dataRepository->getProvider(providerName));
				std::string varName = config["variable"];
				_varName = varName;
 				_varToUse = _landUnitController->getVariable(_varName);
			}

			void CBMLandUnitDataTransform::controllerChanged(const flint::ILandUnitController& controller) {
				_landUnitController = &controller;
				_varToUse = _landUnitController->getVariable(_varName);
			};

			const Dynamic& CBMLandUnitDataTransform::value() const {
				const auto& table = _varToUse->value();
				for (const auto& row : table) {
					_resultsObject["SpatialUnitId"] = row["SpatialUnitId"];
					_resultsObject["Area"] = row["Area"];
					_resultsObject["Age"] = row["Age"];
					_resultsObject["GrowthCurveId"] = row["GrowthCurveId"];
					_resultsObject["AdminBoundryId"] = row["AdminBoundryId"];
					_resultsObject["EcoBoundryId"] = row["EcoBoundryId"];
					_resultsObject["ClimateTimeSeriesId"] = row["ClimateTimeSeriesId"];
				}
				_results = _resultsObject;
				return _results;
			}

		}
	}
} // namespace moja::modules::SLEEK

