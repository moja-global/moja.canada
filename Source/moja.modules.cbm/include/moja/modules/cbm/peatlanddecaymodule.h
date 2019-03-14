#ifndef MOJA_MODULES_CBM_PLDECAY_H_
#define MOJA_MODULES_CBM_PLDECAY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"

namespace moja {
namespace modules {
namespace cbm {
		
	class CBM_API PeatlandDecayModule : public CBMModuleBase {
	public:
		PeatlandDecayModule() : CBMModuleBase() { }
		virtual ~PeatlandDecayModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;	

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;

	private:	
		const flint::IPool* _woodyFoliageDead;
		const flint::IPool* _woodyFineDead;	
		const flint::IPool* _woodyCoarseDead;
		const flint::IPool* _woodyRootsDead;
		const flint::IPool* _sedgeFoliageDead;
		const flint::IPool* _sedgeRootsDead;
		const flint::IPool* _feathermossDead;
		const flint::IPool* _acrotelm_o;
		const flint::IPool* _catotelm_a;
		const flint::IPool* _acrotelm_a;
		const flint::IPool* _catotelm_o;	
		const flint::IPool* _co2;
		const flint::IPool* _ch4;
				
		double awtd; // annual water table depth
		double tic; // totoal initial carbon	

		// decay parameters associated to this peatland unit
		std::shared_ptr<PeatlandDecayParameters> decayParas;	

		// turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;		

		void doDeadPoolTurnover(double turnoverRate);

		void doPeatlandDecay(double turnoverRate);

		double getToCO2Rate(double rate, double turnoverRate);

		double getToCH4Rate(double rate, double turnoverRate);		
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_
