#ifndef MOJA_MODULES_CBM_PLDECAY_H_
#define MOJA_MODULES_CBM_PLDECAY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"

namespace moja {
namespace modules {
namespace cbm {
		
	class CBM_API PeatlandDecayModule : public flint::ModuleBase {
	public:
		PeatlandDecayModule() : ModuleBase() { }
		virtual ~PeatlandDecayModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;	

		void onLocalDomainInit() override;
		void onTimingInit() override;
		void onTimingStep() override;

		double computeWTD(double dc, int functionCode);

	private:	
		flint::IPool::ConstPtr _woodyFoliageDead;
		flint::IPool::ConstPtr _woodyStemsBranchesDead;	
		flint::IPool::ConstPtr _woodyRootsDead;
		flint::IPool::ConstPtr _sedgeFoliageDead;
		flint::IPool::ConstPtr _sedgeRootsDead;
		flint::IPool::ConstPtr _feathermossDead;
		flint::IPool::ConstPtr _acrotelm;
		flint::IPool::ConstPtr _catotelm;
		flint::IPool::ConstPtr _co2;
		flint::IPool::ConstPtr _ch4;
				
		double wtd;
				
		//peatland age variable, peatland age may be very old
		flint::IVariable* _peatlandAge;

		// decay parameters associated to this peatland unit
		std::shared_ptr<PeatlandDecayParameters> decayParas;	

		// turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;

		void doDeadPoolTurnover(double turnoverRate);

		void doPeatlandDecay(double turnoverRate);

		double getToCO2Rate(double rate, double turnoverRate);

		double getToCH4Rate(double rate, double turnoverRate);

		void updatePeatlandLivePoolValue();
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_
