#include "moja/modules/cbm/peatlandgrowthcurvetransform.h"
#include "moja/modules/cbm/peatlands.h"

#include <moja/flint/ivariable.h>
#include <moja/datarepository/datarepository.h>
#include <moja/logging.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>


using moja::datarepository::IProviderRelationalInterface;

namespace moja {
	namespace modules {
		namespace cbm {

			/**
			 * Configuration function
			 * 
			 * Assign PeatlandGrowthcurve._landUnitController parameter &landUnitController, \n
			 * PeatlandGrowthcurve._dataRepository parameter &dataRepository, \n
			 * PeatlandGrowthcurve._gcbmGrowthCurveVar value of config["gcbm_growth_curve_var"] in PeatlandGrowthcurve._landUnitController, \n 
			 * PeatlandGrowthcurve._blackSpruceGrowthCurveVar value of config["black_spruce_growth_curve_var"] in PeatlandGrowthcurve._landUnitController
			 * 
			 * @param DynamicObject config
			 * @param flint::ILandUnitController& landUnitController
			 * @param datarepository::DataRepository& dataRepository
			 * @return void
			 * ***********************/
			void PeatlandGrowthcurve::configure(
				DynamicObject config,
				const flint::ILandUnitController& landUnitController,
				datarepository::DataRepository& dataRepository) {

				_landUnitController = &landUnitController;
				_dataRepository = &dataRepository;

				auto gcbmGrowthCurveVarName = config["gcbm_growth_curve_var"].convert<std::string>();
				_gcbmGrowthCurveVar = _landUnitController->getVariable(gcbmGrowthCurveVarName);

				auto blackSpruceGrowthCurveVarName = config["black_spruce_growth_curve_var"].convert<std::string>();
				_blackSpruceGrowthCurveVar = _landUnitController->getVariable(blackSpruceGrowthCurveVarName);
			}

			/**
			 * Assign PeatlandGrowthcurve._landUnitController as paramter controller
			 * 
			 * @param flint::ILandUnitController& controller
			 * @return void
			 * **************/
			void PeatlandGrowthCurveTransform::controllerChanged(const flint::ILandUnitController& controller) {
				_landUnitController = &controller;
			};

			/**
			 * If the value of variable "peatland_class" in PeatlandGrowthCurveTransform._landUnitController is empty, assign 
			 * PeatlandGrowthCurveTransform._value the value of  PeatlandGrowthCurveTransform._gcbmGrowthCurveVar and return PeatlandGrowthCurveTransform._value 
			 * 
			 * Else if the value of variable "peatland_class" in PeatlandGrowthCurveTransform._landUnitController has the value of either \n
			 * Peatlands::TREED_PEATLAND_BOG, TREED_PEATLAND_POORFEN, TREED_PEATLAND_RICHFEN or TREED_PEATLAND_SWAMP, indicating it is a treed peatland, \n 
			 * assign PeatlandGrowthCurveTransform._value the value of  PeatlandGrowthCurveTransform._blackSpruceGrowthCurveVar and return PeatlandGrowthCurveTransform._value as \n
			 * Treed peatland is assumed to always be black spruce.
			 * 
			 * Else if the value of variable "peatland_class" in PeatlandGrowthCurveTransform._landUnitController has the value of either \n
			 * Peatlands::FOREST_PEATLAND_BOG, FOREST_PEATLAND_POORFEN, FOREST_PEATLAND_RICHFEN or FOREST_PEATLAND_SWAMP, indicating it is a forest peatland, \n 
			 * and PeatlandGrowthCurveTransform._gcbmGrowthCurveVar is empty, assign PeatlandGrowthCurveTransform._value the value of  PeatlandGrowthCurveTransform._blackSpruceGrowthCurveVar and return PeatlandGrowthCurveTransform._value as \n
			 * Forested peatland uses the GCBM growth curve matching the classifier set, if available, otherwise fall back to a default growth curve for the
			 * peatland type.
			 * 
			 * Else, assign PeatlandGrowthCurveTransform._value the value of  PeatlandGrowthCurveTransform._gcbmGrowthCurveVar and return PeatlandGrowthCurveTransform._value 
			 * 
			 * @return DynamicVar& 
			 * ***********************/
			const DynamicVar& PeatlandGrowthCurveTransform::value() const {
				auto& peatland_class = _landUnitController->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				// Treed peatland is assumed to always be black spruce.
				bool treedPeatland = (
					peatlandId == (int)Peatlands::TREED_PEATLAND_BOG ||
					peatlandId == (int)Peatlands::TREED_PEATLAND_POORFEN ||
					peatlandId == (int)Peatlands::TREED_PEATLAND_RICHFEN ||
					peatlandId == (int)Peatlands::TREED_PEATLAND_SWAMP);

				if (treedPeatland) {
					const auto& blackSpruceGcIdValue = _blackSpruceGrowthCurveVar->value();
					Int64 blackSpruceGcId = blackSpruceGcIdValue.isEmpty() ? -1 : blackSpruceGcIdValue.convert<Int64>();
					_value = blackSpruceGcId;
					return _value;
				}

				// Forested peatland uses the GCBM growth curve matching the classifier set,
				// if available, otherwise fall back to a default growth curve for the
				// peatland type.
				bool forestedPeatland = (
					peatlandId == (int)Peatlands::FOREST_PEATLAND_BOG ||
					peatlandId == (int)Peatlands::FOREST_PEATLAND_POORFEN ||
					peatlandId == (int)Peatlands::FOREST_PEATLAND_RICHFEN ||
					peatlandId == (int)Peatlands::FOREST_PEATLAND_SWAMP);

				const auto& gcbmGrowthCurveIdValue = _gcbmGrowthCurveVar->value();
				Int64 gcbmGrowthCurveId = gcbmGrowthCurveIdValue.isEmpty() ? -1 : gcbmGrowthCurveIdValue.convert<Int64>();

				if (forestedPeatland && gcbmGrowthCurveId == -1) {
					// If it is forest peatland, and no valid growth curve, search default peatland growth curves.
					const auto& peatlandGcIdValue = _landUnitController->getVariable("forest_peatland_growth_curve_id")->value();
					Int64 peatlandGcId = peatlandGcIdValue.isEmpty() ? -1 : peatlandGcIdValue.convert<Int64>();
					_value = peatlandGcId;
					return _value;
				}

				_value = gcbmGrowthCurveId;
				return _value;
			}

		}
	}
} // namespace moja::modules::cbm

