#ifndef MOJA_MODULES_CBM_MOSSDECAY_H_
#define MOJA_MODULES_CBM_MOSSDECAY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"

namespace moja {
	namespace modules {
		namespace cbm {
				
			class CBM_API MossDecayModule : public CBMModuleBase {
			public:
				MossDecayModule(std::shared_ptr<StandGrowthCurveFactory> gcFactory)
					: _gcFactory(gcFactory) {};

				virtual ~MossDecayModule() = default;

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

				void doLocalDomainInit() override;
				void doTimingInit() override;
				void doTimingStep() override;

			private:
				std::shared_ptr<StandGrowthCurveFactory> _gcFactory;

				flint::IVariable* _mossParameters;

				const flint::IPool* _featherMossFast = nullptr;;
				const flint::IPool* _sphagnumMossFast = nullptr;;
				const flint::IPool* _featherMossSlow = nullptr;;
				const flint::IPool* _sphagnumMossSlow = nullptr;;
				const flint::IPool* _CO2 = nullptr;;
				bool runMoss{ false };

				/// <summary>
				/// Base decay rate feather moss fast pool      
				/// </summary>
				double kff;	     

				/// <summary>
				/// Base decay rate sphagnum fast pool        
				/// </summary>
				double ksf;

				/// <summary>
				/// Base decay rate feather moss slow pool              
				/// </summary>       
				double kfs;	

				/// <summary>
				///Sphagnum slow pool base decay rate             
				/// </summary>	    
				double kss;	

				/// <summary>
				///Sphagnum slow pool base decay rate             
				/// </summary>		
				double q10;		//Q10 temperature coefficient

				/// <summary>
				///Sphagnum slow pool base decay rate             
				/// </summary>	
				double tref;	//reference temperature   

				/// <summary>
				///Sphagnum slow pool base decay rate             
				/// </summary>	  
				double akff;	//applied feather moss fast pool applied decay rate     

				/// <summary>
				///Applied feather moss slow pool applied decay rate           
				/// </summary>	
				double akfs;	 

				/// <summary>
				///Applied sphagnum fast pool applied decay rate           
				/// </summary>	   
				double aksf;	  

				/// <summary>
				///Applied sphagnum slow pool applied decay rate            
				/// </summary>	       
				double akss;	

				/// <summary>
				///Parameter for F6           
				/// </summary>	   
				double m;	

				/// <summary>
				///Parameter for F6            
				/// </summary>	                                
				double n;	  

				/// <summary>
				///Fast moss pool to slow moss pool turnover rate      
				/// </summary>	
				double fastToSlowTurnoverRate;	

				/// <summary>
				///Fast moss pool to CO2 air pool rate             
				/// </summary>	
				double fastToAirDecayRate;		

				double meanAnnualTemperature;
				Int64 currentStandGCId;

				//Sphagnum slow pool base decay rate, kss = m*ln(maxVolume) + n
				double F6(double m, double n, double maxVolume);

				//Applied decay rate to all moss pools: kff, kfs, ksf, kss
				//kff - feather fast decay rate
				//kfs - feather slow decay rate
				//ksf - sphagnum fast decay rate
				//kss - sphagnum slow decay rate
				double F7(double baseDecayRate, double meanAnnualTemperature, double q10);

				void updateMossAppliedDecayParameters(double standMaximumVolume, double meanAnnualTemperature);

				void doMossFastPoolDecay();

				void doMossSlowPoolDecay();
			};

		}
	}
}
#endif
