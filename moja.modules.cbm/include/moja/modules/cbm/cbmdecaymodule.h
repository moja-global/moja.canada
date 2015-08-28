#ifndef CBMDecayModule_H_
#define CBMDecayModule_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include <vector>

namespace moja {
namespace modules {
namespace CBM {

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

		void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
		void onTimingInit(const flint::TimingInitNotification::Ptr&) override;
		void onTimingStep(const flint::TimingStepNotification::Ptr& n) override;

	private:
		void getTransfer(flint::IOperation* operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 const flint::IPool* poolSrc,
						 const flint::IPool* poolDest);

		void getTransfer(flint::IOperation* operation,
						 double meanAnnualTemperature,
						 const std::string& domPool,
						 const flint::IPool* pool);

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
		double _T;
		double _slowMixingRate;

		std::map<std::string, PoolDecayParameters> _decayParameters;
	};

}}} // namespace moja::Modules::CBM

#endif // CBMDecayModule_H_