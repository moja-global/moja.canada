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
		PeatlandGrowthParameters(){};
		PeatlandGrowthParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier);
		virtual ~PeatlandGrowthParameters() = default;

		void setValue(const DynamicObject& data) override;
		void setDefaultValue(const std::vector<double>& data) override;

	private:
		double _FAr;		//Foliage:Aboveground ratio for low shrubs (applied to woody layer)
		double _NPPagls;	//NPP for aboveground low shrubs
		double _Bagls;		//Maximum Potential Biomass for aboveground low shrubs
		double _a;			//Allometric function to convert low shrub Aboveground biomass into root production
		double _b;			//Allometric function to convert low shrub Aboveground biomass into root production
		double _AFfls;		//adjustment factor for Sedges
		double _Bags;		//Maximum Biomass aboveground sedges 
		double _GCs;		//Ground cover sedges
		double _AgBgS;		//Aboveground:bellowground ratio sedges
		double _GCsp;		//Ground Cover sphagnum
		double _NPPsp;		//NPP sphagnum
		double _Rsp;		//Recovery time(y) after stand replacing disturbance sphagnum
		double _GCfm;		//Ground Cover feathermoss
		double _NPPfm;		//NPP feathermoss
		double _Rfm;		//Recovery time(y) after stand replacing disturbance feathermoss

		double _Magls;		//Mortality rate for aboveground low Shrub
		double _SBags;		//Scaled maximum stannding crop biomass
		double _aNPPs;		//adjusted NPP for aboveground sedge

		void applyGrowthParameterFunctions();

	};
	
}}}
#endif