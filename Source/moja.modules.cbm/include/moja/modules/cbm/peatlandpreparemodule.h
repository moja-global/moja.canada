#ifndef MOJA_MODULES_CBM_PEATLAND_PREPARE_H_
#define MOJA_MODULES_CBM_PEATLAND_PREPARE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/*
			Prepare initial variables to simulate a peatland landunit (pixel)
			*/
			class CBM_API PeatlandPrepareModule : public CBMModuleBase {
			public:
				PeatlandPrepareModule() {};
				virtual ~PeatlandPrepareModule() {};

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;

			private:
				const flint::IPool* _atmosphere;
				const flint::IPool* _acrotelm_o;
				const flint::IPool* _catotelm_a;

				DynamicObject baseWTDParameters;
				int peatlandID{ -1 };

				bool _runPeatland{ false };
				bool _isInitialPoolLoaded{ false };
				bool _isForestPeatland{ false };
				bool _isTreedPeatland{ false };

		void resetWaterTableDepthValue();
		void checkTreedOrForestPeatland (int peatlandId);
		void loadPeatlandInitialPoolValues(const DynamicObject& data);
		double computeWaterTableDepth (double dc, int peatlandID);		
    };
}}}
#endif