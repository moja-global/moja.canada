#ifndef MOJA_MODULES_CBM_MOSSGROWTH_H_
#define MOJA_MODULES_CBM_MOSSGROWTH_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Parameters for moss related computing.
	/// </summary>	
	class CBM_API MossGrowthModule : public CBMModuleBase {
	public:    
		MossGrowthModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory)
			: _gcFactory(gcFactory) {};

		virtual ~MossGrowthModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;

	private:
		std::shared_ptr<StandGrowthCurveFactory> _gcFactory;

		flint::IVariable* _mossParameters;
		const flint::IPool* _atmosphere;
		const flint::IPool* _featherMossLive;
		const flint::IPool* _sphagnumMossLive;

		bool runMoss;
		bool _skipForPeatland;
		Int64 currentStandGCId;

		//moss growth related parameters
		double a;	//parameter for F1                                          
		double b;	//parameter for F1                                          
		double c;	//parameter for F2                                          
		double d;	//parameter for F2                                          
		double e;	//parameter for F3                                          
		double f;	//parameter for F3                                          
		double g;	//parameter for F4                                          
		double h;	//parameter for F4                                          
		double i;	//parameter for F5                                          
		double j;	//parameter for F5                                          
		double l;	//parameter for F5  

		//Canopy openess, O(t) as a function of	merchant volume : 10^(((a)*(Log(V(t))) + b)
		double F1(double a, double b, double volume);

		//Feather moss ground cover, GCFm(t) = c*O(t) + d
		double F2(double c, double d, int age, double openNess);

		//Sphagnum ground cover, GCSp(t) = e*O(t) + f
		double F3(double e, double f, int age, double openNess);

		//Feather moss NPP, NPPFm = (g*O(t))^h
		double F4(double g, double h, double openNess);

		//Sphagnum NPP, NPPSp = i*(O(t)^2) + j*O(t) + l
		double F5(double i, double j, double l, double openNess);

		void doMossGrowth(int mossAge, double standMerchVolume);		                
	};

}}}
#endif
