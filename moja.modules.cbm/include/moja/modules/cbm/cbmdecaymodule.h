#ifndef MOJA_MODULES_CBM_CBMDECAYMODULE_H_
#define MOJA_MODULES_CBM_CBMDECAYMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include <vector>

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

	/// <summary>
	/// Performs annual decay and turnover on a set of dead organic matter pools
	/// 
	/// Data requirements: 
	/// 
	/// 1: table named "decay_parameters" with 1 set of decay 
	///    parameters for each of the enumerated dom pools in the DomPool enum
	///    Columns: 
	///       SoilPoolId: the integer of the DomPool, which corresponds with the enumeration
	///       OrganicMatterDecayRate: the base decay rate
	///       Q10: the Q10
	///       Tref: the reference temperature (degrees Celcius)
	///       Max: the maximum decay rate for the dom pool
	/// 2: scalar "mean_annual_temperature" the mean annual temperature of the environment 
	/// 3: scalar "SlowMixingRate" the amount turned over from slow ag to slow bg annually
	/// </summary>
	class CBM_API CBMDecayModule : public flint::ModuleBase {
	public:
		CBMDecayModule() : ModuleBase() { }
		virtual ~CBMDecayModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void onLocalDomainInit() override;
		void onTimingInit() override;
		void onTimingStep() override;

	private:
		void getTransfer(flint::IOperation::Ptr operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 flint::IPool::ConstPtr poolSrc,
						 flint::IPool::ConstPtr poolDest);

		void getTransfer(flint::IOperation::Ptr operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 flint::IPool::ConstPtr pool);

        bool shouldRun();

		flint::IPool::ConstPtr _aboveGroundVeryFastSoil;
		flint::IPool::ConstPtr _belowGroundVeryFastSoil;
		flint::IPool::ConstPtr _aboveGroundFastSoil;
		flint::IPool::ConstPtr _belowGroundFastSoil;
		flint::IPool::ConstPtr _mediumSoil;
		flint::IPool::ConstPtr _aboveGroundSlowSoil;
		flint::IPool::ConstPtr _belowGroundSlowSoil;
		flint::IPool::ConstPtr _softwoodStemSnag;
		flint::IPool::ConstPtr _softwoodBranchSnag;
		flint::IPool::ConstPtr _hardwoodStemSnag;
		flint::IPool::ConstPtr _hardwoodBranchSnag;
		flint::IPool::ConstPtr _atmosphere;

        flint::IVariable* _spinupMossOnly;
        flint::IVariable* _isForest;

		double _T;
        double _slowMixingRate;
        bool _extraDecayRemovals = false;

		std::map<std::string, PoolDecayParameters> _decayParameters;
        std::map<std::string, std::map<std::string, double>> _decayRemovals;
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMDECAYMODULE_H_
