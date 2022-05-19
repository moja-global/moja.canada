#ifndef MOJA_MODULES_CBM_CBMDECAYMODULE_H_
#define MOJA_MODULES_CBM_CBMDECAYMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/peatlands.h"

namespace moja {
namespace modules {
namespace cbm {

	struct PoolDecayParameters {
		std::string pool;
		double baseDecayRate;
		double maxDecayRate;
		double q10;
		double tRef;
		double pAtm;

		PoolDecayParameters() {}

		PoolDecayParameters(const DynamicObject& data) {
			pool = data["pool"].convert<std::string>();
			baseDecayRate = data["organic_matter_decay_rate"];
			q10 = data["q10"];
			tRef = data["reference_temp"];
			maxDecayRate = data["max_decay_rate_soft"];
			pAtm = data["prop_to_atmosphere"];
		}

		double getDecayRate(double mat) {
			return std::min(
				baseDecayRate * std::exp((mat - tRef) * std::log(q10) * 0.1),
				maxDecayRate);
		}
	};

	class CBM_API CBMDecayModule : public CBMModuleBase {
	public:
		CBMDecayModule() : CBMModuleBase() { }
		virtual ~CBMDecayModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;

	private:
		void getTransfer(std::shared_ptr<flint::IOperation> operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 const flint::IPool* poolSrc,
						 const flint::IPool* poolDest);

		void getTransfer(std::shared_ptr<flint::IOperation> operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 const flint::IPool* pool);

        bool shouldRun();
		void initPeatland();

		const flint::IPool* _aboveGroundVeryFastSoil;
		const flint::IPool* _belowGroundVeryFastSoil;
		const flint::IPool* _aboveGroundFastSoil;
		const flint::IPool* _belowGroundFastSoil;
		const flint::IPool* _mediumSoil;
		const flint::IPool* _aboveGroundSlowSoil;
		const flint::IPool* _belowGroundSlowSoil;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;
		const flint::IPool* _atmosphere;

        flint::IVariable* _spinupMossOnly;
        flint::IVariable* _isDecaying;

        double _slowMixingRate;
		bool _extraDecayRemovals { false };
		bool _skipForPeatland { false };

		std::map<std::string, PoolDecayParameters> _decayParameters;
        std::map<std::string, std::map<std::string, double>> _decayRemovals;
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMDECAYMODULE_H_
