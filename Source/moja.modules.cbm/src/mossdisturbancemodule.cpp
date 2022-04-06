#include "moja/modules/cbm/mossdisturbancemodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/helper.h"

#include <moja/flint/variable.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/algorithm/string.hpp> 

namespace moja {
	namespace modules {
		namespace cbm {

			void MossDisturbanceModule::configure(const DynamicObject& config) {
			}

			void MossDisturbanceModule::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &MossDisturbanceModule::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &MossDisturbanceModule::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::TimingInit, &MossDisturbanceModule::onTimingInit, *this);
			}

			void MossDisturbanceModule::doLocalDomainInit() {

				// get the data by variable "moss_fire_parameters"
				const auto& mossFireParams = _landUnitData->getVariable("moss_fire_parameters")->value();

				recordMossTransfers(mossFireParams.extract<DynamicObject>());
			}

			void MossDisturbanceModule::doTimingInit() {
				auto gcID = _landUnitData->getVariable("growth_curve_id")->value();
				bool isGrowthCurveDefined = !gcID.isEmpty() && gcID != -1;

				auto mossLeadingSpecies = _landUnitData->getVariable("moss_leading_species")->value();
				auto speciesName = _landUnitData->getVariable("leading_species")->value();

				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();

				runMoss = peatlandId < 0 
					&& isGrowthCurveDefined 
					&& Helper::runMoss(gcID, mossLeadingSpecies, speciesName);
			}

			void MossDisturbanceModule::doDisturbanceEvent(DynamicVar n) {
				if (!runMoss) { return; } //skip if not run moss

				auto& data = n.extract<const DynamicObject>();

				// Get the disturbance type for either historical or last disturbance event.
				std::string disturbanceType = data["disturbance"];
				boost::algorithm::to_lower(disturbanceType);

				//check if it is fire disturbance
				bool isFire = boost::contains(disturbanceType, MossDisturbanceModule::fireEvent);

				if (runMoss && isFire) {
					auto distMatrix = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();

					std::string sourcePoolName;
					std::string sinkPoolName;

					int transferIndex = 0;
					double actualRate = 0.0;

					for (int sourceIndex = 0; sourceIndex < _sourcePools.size(); sourceIndex++) {
						for (int sinkIndex = 0; sinkIndex < _destPools.size(); sinkIndex++) {
							sourcePoolName = _sourcePools.at(sourceIndex);
							sinkPoolName = _destPools.at(sinkIndex);

							actualRate = _transferRates.at(transferIndex++);
							if (actualRate > 0.0) {
								auto transferCO2 = CBMDistEventTransfer(*_landUnitData, sourcePoolName, sinkPoolName, actualRate);
								distMatrix->push_back(transferCO2);
							}
						}
					}
				}
			}

			void MossDisturbanceModule::recordMossTransfers(const DynamicObject& data) {
				_transferRates.clear();

				_transferRates.push_back(data["FL2CO2"]);
				_transferRates.push_back(data["FL2CH4"]);
				_transferRates.push_back(data["FL2CO"]);
				_transferRates.push_back(data["FL2FS"]);
				_transferRates.push_back(data["FL2SS"]);
				_transferRates.push_back(data["SL2CO2"]);
				_transferRates.push_back(data["SL2CH4"]);
				_transferRates.push_back(data["SL2CO"]);
				_transferRates.push_back(data["SL2FS"]);
				_transferRates.push_back(data["SL2SS"]);
				_transferRates.push_back(data["FF2CO2"]);
				_transferRates.push_back(data["FF2CH4"]);
				_transferRates.push_back(data["FF2CO"]);
				_transferRates.push_back(data["FF2FS"]);
				_transferRates.push_back(data["FF2SS"]);
				_transferRates.push_back(data["SF2CO2"]);
				_transferRates.push_back(data["SF2CH4"]);
				_transferRates.push_back(data["SF2CO"]);
				_transferRates.push_back(data["SF2FS"]);
				_transferRates.push_back(data["SF2SS"]);
    }
}}}
