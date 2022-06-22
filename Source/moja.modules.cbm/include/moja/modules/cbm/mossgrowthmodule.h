#ifndef MOJA_MODULES_CBM_MOSSGROWTH_H_
#define MOJA_MODULES_CBM_MOSSGROWTH_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
	namespace modules {
		namespace cbm {

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

				flint::IVariable* _mossParameters = nullptr;;
				const flint::IPool* _atmosphere = nullptr;;
				const flint::IPool* _featherMossLive = nullptr;;
				const flint::IPool* _sphagnumMossLive = nullptr;;
				flint::IVariable* _regenDelay = nullptr;
				flint::IVariable* _spinupMossOnly = nullptr;
				flint::IVariable* _age = nullptr;

				bool runMoss{ false };
				Int64 currentStandGCId{ -1 };

				//moss growth related parameters
				
				/// <summary>
				/// Parameter for F1  
				/// </summary>	
				double a{ 0.0 };	//parameter for F1   

				/// <summary>
				/// Parameter for F1  
				/// </summary>                                       
				double b{ 0.0 };	//parameter for F1   

				/// <summary>
				/// Parameter for F2  
				/// </summary>                                       
				double c{ 0.0 };	//parameter for F2     

				/// <summary>
				/// Parameter for F2
				/// </summary>                                     
				double d{ 0.0 };	//parameter for F2   

				/// <summary>
				/// Parameter for F3
				/// </summary>                                       
				double e{ 0.0 };	//parameter for F3   

				/// <summary>
				/// Parameter for F3  
				/// </summary>                                       
				double f{ 0.0 };	//parameter for F3  

				/// <summary>
				/// Parameter for F4  
				/// </summary>                                        
				double g{ 0.0 };	//parameter for F4    

				/// <summary>
				/// Parameter for F4  
				/// </summary>                                      
				double h{ 0.0 };	//parameter for F4    

				/// <summary>
				/// Parameter for F5  
				/// </summary>                                      
				double i{ 0.0 };	//parameter for F5  

				/// <summary>
				/// Parameter for F5
				/// </summary>                                        
				double j{ 0.0 };	//parameter for F5      

				/// <summary>
				/// Parameter for F5  
				/// </summary>                                    
				double l{ 0.0 };	//parameter for F5  

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
		}
	}
}
#endif
