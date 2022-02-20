#ifndef MOJA_MODULES_CBM_PLGROWTHPARAS_H_
#define MOJA_MODULES_CBM_PLGROWTHPARAS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/peatlandparameters.h"

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API PeatlandGrowthParameters : public PeatlandParameters {
			public:
				double FAr() const { return _FAr; }
				double NPPagls() const { return _NPPagls; }
				double Bagls() const { return _Bagls; }
				double a() const { return _a; }
				double b() const { return _b; }
				double AFfls() const { return _AFfls; }
				double Bags() const { return _Bags; }
				double GCs() const { return _GCs; }
				double AgBgS() const { return _AgBgS; }
				double GCsp() const { return _GCsp; }
				double NPPsp() const { return _NPPsp; }
				double Rsp() const { return _Rsp; }
				double GCfm() const { return _GCfm; }
				double NPPfm() const { return _NPPfm; }
				double Rfm() const { return _Rfm; }
				double Magls() const { return _Magls; }
				double SBags() const { return _SBags; }
				double aNPPs() const { return _aNPPs; }


				/// <summary>
				/// Default constructor
				/// </summary>
				PeatlandGrowthParameters() {};
				PeatlandGrowthParameters(int _spuId, PeatlandType _peatlandType, PeatlandLandCoverType _landCoverType);
				virtual ~PeatlandGrowthParameters() = default;

				void setValue(const DynamicObject& data) override;

			private:
				double _FAr{ 0 };		//Foliage:Aboveground ratio for low shrubs (applied to woody layer)
				double _NPPagls{ 0 };	//NPP for aboveground low shrubs
				double _Bagls{ 0 };		//Maximum Potential Biomass for aboveground low shrubs
				double _a{ 0 };			//Allometric function to convert low shrub Aboveground biomass into root production
				double _b{ 0 };			//Allometric function to convert low shrub Aboveground biomass into root production
				double _AFfls{ 0 };		//adjustment factor for Sedges
				double _Bags{ 0 };		//Maximum Biomass aboveground sedges 
				double _GCs{ 0 };		//Ground cover sedges
				double _AgBgS{ 0 };		//Aboveground:bellowground ratio sedges
				double _GCsp{ 0 };		//Ground Cover sphagnum
				double _NPPsp{ 0 };		//NPP sphagnum
				double _Rsp{ 0 };		//Recovery time(y) after stand replacing disturbance sphagnum
				double _GCfm{ 0 };		//Ground Cover feathermoss
				double _NPPfm{ 0 };		//NPP feathermoss
				double _Rfm{ 0 };		//Recovery time(y) after stand replacing disturbance feathermoss

				double _Magls{ 0 };		//Mortality rate for aboveground low Shrub
				double _SBags{ 0 };		//Scaled maximum stannding crop biomass
				double _aNPPs{ 0 };		//adjusted NPP for aboveground sedge

				void applyGrowthParameterFunctions();
			};

		}
	}
}
#endif