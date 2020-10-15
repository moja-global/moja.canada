#ifndef MOJA_MODULES_CBM_PLDECAY_H_
#define MOJA_MODULES_CBM_PLDECAY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandwtdbasefch4parameters.h"

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
		const flint::IPool* _tempCarbon;
				
		double awtd{ 0.0 }; // annual water table depth
		double tic{ 0.0 }; // totoal initial carbon	
		bool _runPeatland{ false };

		// decay parameters associated to this peatland unit
		std::shared_ptr<PeatlandDecayParameters> decayParas;	

		// turnover parameters associated to this peatland unit
		std::shared_ptr<PeatlandTurnoverParameters> turnoverParas;		

		void doDeadPoolTurnover(double turnoverRate);

		void doPeatlandDecay(double turnoverRate);

		double getToCO2Rate(double rate, double turnoverRate);

		double getToCH4Rate(double rate, double turnoverRate);		
		//wtd-base and fch4 parameters
		std::shared_ptr<PeatlandWTDBaseFCH4Parameters> wtdFch4Paras;
		void doPeatlandNewCH4ModelDecay(double turnoverRate);
		void allocateCh4CO2();
	};

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_
