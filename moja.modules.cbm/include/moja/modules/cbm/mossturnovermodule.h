#ifndef MOJA_MODULES_CBM_MOSSTURNOVER_H_
#define MOJA_MODULES_CBM_MOSSTURNOVER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Parameters for moss related computing.
	/// </summary>	
	class CBM_API MossTurnoverModule : public moja::flint::ModuleBase{
	public:    
		MossTurnoverModule();
		virtual ~MossTurnoverModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes ModuleType() override { return flint::ModuleTypes::Model; };

		void onLocalDomainInit() override;
		void onTimingInit() override;
		void onTimingStep() override;


	private:	
		flint::IVariable* _mossParameters;

		flint::IPool::ConstPtr _featherMossLive;
		flint::IPool::ConstPtr _sphagnumMossLive;
		flint::IPool::ConstPtr _featherMossFast;
		flint::IPool::ConstPtr _sphagnumMossFast;

		bool runMoss;
		double fmlTurnoverRate; //Feather moss turnover rate                   
		double smlTurnoverRate; //Sphagnum moss turnover rate    

		void doLiveMossTurnover();	
	};

}}}
#endif
