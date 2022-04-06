#ifndef MOJA_MODULES_CBM_PLDECAY_H_
#define MOJA_MODULES_CBM_PLDECAY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/peatlanddecayparameters.h"
#include "moja/modules/cbm/peatlandturnoverparameters.h"
#include "moja/modules/cbm/peatlandwtdbasefch4parameters.h"

#include "moja/modules/cbm/timeseries.h"

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
				const flint::IPool* _woodyFoliageDead{ nullptr };
				const flint::IPool* _woodyFineDead{ nullptr };
				const flint::IPool* _woodyCoarseDead{ nullptr };
				const flint::IPool* _woodyRootsDead{ nullptr };
				const flint::IPool* _sedgeFoliageDead{ nullptr };
				const flint::IPool* _sedgeRootsDead{ nullptr };
				const flint::IPool* _feathermossDead{ nullptr };
				const flint::IPool* _acrotelm_o{ nullptr };
				const flint::IPool* _catotelm_a{ nullptr };
				const flint::IPool* _acrotelm_a{ nullptr };
				const flint::IPool* _catotelm_o{ nullptr };
				const flint::IPool* _co2{ nullptr };
				const flint::IPool* _ch4{ nullptr };
				const flint::IPool* _tempCarbon{ nullptr };

				flint::IVariable* _spinupMossOnly{ nullptr };

				int _peatlandId{ -1 };
				bool _runPeatland{ false };

				// decay parameters associated to this peatland unit
				std::shared_ptr<PeatlandDecayParameters> decayParas{ nullptr };

				// turnover parameters associated to this peatland unit
				std::shared_ptr<PeatlandTurnoverParameters> turnoverParas{ nullptr };

				//wtd-base and fch4 parameters
				std::shared_ptr<PeatlandWTDBaseFCH4Parameters> wtdFch4Paras{ nullptr };

				DynamicObject baseWTDParameters;

				void doDeadPoolTurnover(double turnoverRate);
				void doPeatlandDecay(double turnoverRate, double awtd);
				void doPeatlandNewCH4ModelDecay(double turnoverRate);
				void allocateCh4CO2(double awtd);

				double getCurrentYearWaterTable();
				double getToCO2Rate(double rate, double turnoverRate, double awtd);
				double getToCH4Rate(double rate, double turnoverRate, double awtd);
				double computeWaterTableDepth(double dc, int peatlandID);
			};
		}
	}
} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_PLDECAY_H_
