#ifndef MOJA_MODULES_CBM_CBMGROWTHMULTIPLIERMODULE_H_
#define MOJA_MODULES_CBM_CBMGROWTHMULTIPLIERMODULE_H_

#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/hash.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

	class GrowthMultiplierSeries {
	public:
		GrowthMultiplierSeries() {
			pos = multipliersByTimestep.begin();
		}

		GrowthMultiplierSeries(const GrowthMultiplierSeries& rhs) {
			multipliersByTimestep = rhs.multipliersByTimestep;
			pos = multipliersByTimestep.begin();
		}

		void add(int timestep, double multiplier) {
			multipliersByTimestep[timestep] = multiplier;
		}

		double next() {
			return end() ? 1.0 : pos++->second;
		}

		bool end() {
			return pos == multipliersByTimestep.end();
		}

	private:
		std::map<int, double> multipliersByTimestep;
		std::map<int, double>::iterator pos;
	};

	class GrowthMultiplierSet {
	public:
		void add(std::string forestType, int timestep, double multiplier) {
			auto it = seriesByForestType.find(forestType);
			if (it != seriesByForestType.end()) {
				it->second.add(timestep, multiplier);
			} else {
				seriesByForestType[forestType] = GrowthMultiplierSeries();
				seriesByForestType[forestType].add(timestep, multiplier);
			}
		}

		std::unordered_map<std::string, double> next() {
			std::unordered_map<std::string, double> currentMultipliers;
			for (auto& series : seriesByForestType) {
				currentMultipliers[series.first] = series.second.next();
			}

			return currentMultipliers;
		}

		bool end() {
			for (auto& series : seriesByForestType) {
				if (!series.second.end()) {
					// End of the set when the last forest type has run out of multipliers.
					return false;
				}
			}

			return true;
		}

	private:
		std::unordered_map<std::string, GrowthMultiplierSeries> seriesByForestType;
	};

    class GrowthMultiplierModule : public CBMModuleBase {
    public:
		GrowthMultiplierModule() : CBMModuleBase() {}
        virtual ~GrowthMultiplierModule() = default;

        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() { return flint::ModuleTypes::DisturbanceEvent; };

        virtual void doDisturbanceEvent(DynamicVar) override;
        virtual void doLocalDomainInit() override;
        virtual void doTimingInit() override;
        virtual void doTimingStep() override;
		virtual void doTimingShutdown() override;

    private:
		void advanceMultipliers();
		void clearMultipliers();

		bool _moduleEnabled;
        flint::IVariable* _currentGrowthMultipliers;
		std::unordered_map<std::string, GrowthMultiplierSet> _growthMultiplierSets;
		GrowthMultiplierSet _activeMultiplierSet;
    };

}}} // namespace moja::modules::cbm
#endif // MOJA_MODULES_CBM_CBMGROWTHMULTIPLIERMODULE_H_