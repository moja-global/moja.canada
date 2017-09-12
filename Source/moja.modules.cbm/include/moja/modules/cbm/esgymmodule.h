#ifndef MOJA_MODULES_CBM_ESGYMMODULE_H_
#define MOJA_MODULES_CBM_ESGYMMODULE_H_

#include <algorithm>

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/rootbiomassequation.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API ESGYMModule : public CBMModuleBase {
	public:
		ESGYMModule() {};

		virtual ~ESGYMModule() {};

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		flint::ModuleTypes moduleType() override { return flint::ModuleTypes::Model; };

        void doLocalDomainInit() override;
        void doTimingInit() override;
        void doTimingStep() override;	

    private:
        const flint::IPool* _softwoodMerch;
        const flint::IPool* _softwoodOther;
        const flint::IPool* _softwoodFoliage;
        const flint::IPool* _softwoodCoarseRoots;
        const flint::IPool* _softwoodFineRoots;

		const flint::IPool* _hardwoodMerch;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;

		const flint::IPool* _aboveGroundVeryFastSoil;
		const flint::IPool* _aboveGroundFastSoil;
		const flint::IPool* _belowGroundVeryFastSoil;
		const flint::IPool* _belowGroundFastSoil;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;
		const flint::IPool* _mediumSoil;
        const flint::IPool* _atmosphere;	

		flint::IVariable* _age;
        flint::IVariable* _turnoverRates;
		flint::IVariable* _regenDelay;
		flint::IVariable* _currentLandClass;

		flint::IVariable* _isForest;

		flint::IVariable* _cbm_species_id;
		flint::IVariable* _standSPUID;

		std::shared_ptr<SoftwoodRootBiomassEquation> SWRootBio;
		std::shared_ptr<HardwoodRootBiomassEquation> HWRootBio;

		DynamicObject growth_esgym_fixed_effects;
		std::map<moja::Int64, DynamicObject> growth_esgym_species_specific_effects;
		
		DynamicObject mortality_esgym_fixed_effects;
		std::map<moja::Int64, DynamicObject> mortality_esgym_species_specific_effects;
		
		DynamicObject growth_esgym_environmental_effects;
		DynamicObject mortality_esgym_environmental_effects;
		DynamicObject mean_esgym_environmental_effects;
		DynamicObject stddev_esgym_environmental_effects;
		
		DynamicObject foliageAllocationParameters;
		DynamicObject branchAllocationParameters;

		DynamicObject standBiomassModifierParameters;

		DynamicObject environmentalDescriptiveStatistics;

		DynamicObject topStumpParameters;

		std::map<int, double> co2Concentrations;

		void doTurnover(double M) const;
		void updateBiomassPools();
		bool shouldRun() const;

		// biomass and snag turnover rate/parameters
		double _softwoodFoliageFallRate;
		double _hardwoodFoliageFallRate;
		double _stemAnnualTurnOverRate;
		double _softwoodBranchTurnOverRate;
		double _hardwoodBranchTurnOverRate;
		double _otherToBranchSnagSplit;
		double _stemSnagTurnoverRate;
		double _branchSnagTurnoverRate;
		double _coarseRootSplit;
		double _coarseRootTurnProp;
		double _fineRootAGSplit;
		double _fineRootTurnProp;

        // record of the biomass carbon growth increment
        double swm;
        double swo;
        double swf;
        double hwm;
        double hwo;
        double hwf;
        double swcr;
        double swfr;
        double hwcr;
        double hwfr;

        // record of the current biomass and snag pool value
        double standSoftwoodMerch;
        double standSoftwoodOther;
        double standSoftwoodFoliage;
        double standSWCoarseRootsCarbon;
        double standSWFineRootsCarbon;
        double standHardwoodMerch;
        double standHardwoodOther;
        double standHardwoodFoliage;
        double standHWCoarseRootsCarbon;
        double standHWFineRootsCarbon;
        double softwoodStemSnag;
        double softwoodBranchSnag;
        double hardwoodStemSnag;
        double hardwoodBranchSnag;
		float ExtractRasterValue(const std::string name);
		double ComputeComponentGrowth(double predictor, double b0, double b1, double b2);
		double StandBiomassModifier(double standBio, double standBio_mu, double standBio_sig, double LamBs);
		/**
		* Predict normal growth and mortality using a growth and yield model
		* @param age the age in years
		* @param B1 fixed effects specific to growth or mortality
		* @param B2 fixed effects specific to growth or mortality
		* @param B3 fixed effects specific to growth or mortality
		* @param B4 fixed effects specific to growth or mortality
		* @param B5 fixed effects specific to growth or mortality
		* @param b1 random species specific effects specific to growth or mortality
		* @param b1 random species specific effects specific to growth or mortality
		* @param eeq_n long term mean equilibrium evaporation
		* @param eeq_mu equilibrium evaporation descriptive statistics mean
		* @param eeq_sig equilibrium evaporation descriptive statistics standard deviation
		* @param dwf_n long term mean days without frost
		* @param dwf_mu days without frost descriptive statistics mean
		* @param dwf_sig days without frost descriptive statistics standard deviation
		* @return the biomass growth or mortality change in Mg * ha^-1 * yr^-1
		*/
		double GrowthAndMortality(int age, double B1, double B2, double B3,
			double B4, double B5, double b1, double b2, double eeq_n, 
			double eeq_mu, double eeq_sig, double dwf_n, double dwf_mu, 
			double dwf_sig)
		{
			if (age < 0) {
				throw std::invalid_argument("age should be greater than or equal to 0");
			}
			double dwf = (dwf_n - dwf_mu) / dwf_sig;
			double eeq = (eeq_n - eeq_mu) / eeq_sig;
			double Y_n = (B1 + b1 + B3 * dwf + B4 * eeq + B5 * dwf * eeq)
				* (B2 + b2) * exp(-(B2 + b2)*age)* pow(1 - exp(-(B2 + b2)*age), 2.0);
			Y_n = std::max(0.0, Y_n);//clamp at 0
			return Y_n;
		}

		/**
		* accounts for transient environmental effects
		* @param B_dwf independent variable days without frost [d yr^-1]
		* @param B_dwf_mu independent variable mean days without frost [d yr^-1]
		* @param B_dwf_sig independent variable stddev days without frost [d yr^-1]
		* @param dwf_a climate variable anomaly days without frost [d yr^-1]

		* @param B_rswd independent variable Growing season mean downward solar radiation [W m^-2]
		* @param B_rswd_mu independent variable mean Growing season mean downward solar radiation [W m^-2]
		* @param B_rswd_sig independent variable stddev Growing season mean downward solar radiation [W m^-2]
		* @param rswd_a climate variable anomaly Growing season mean downward solar radiation [W m^-2]

		* @param B_tmean independent variable Warm-season mean air temperature [deg C]
		* @param B_tmean_mu independent variable mean Warm-season mean air temperature [deg C]
		* @param B_tmean_sig independent variable stddev Warm-season mean air temperature [deg C]
		* @param tmean_a climate variable anomaly Warm-season mean air temperature [deg C]

		* @param B_vpd independent variable Warm-season vapour pressure deficit [hPa]
		* @param B_vpd_mu independent variable mean Warm-season vapour pressure deficit [hPa]
		* @param B_vpd_sig independent variable stddev Warm-season vapour pressure deficit [hPa]
		* @param vpd_a climate variable anomaly Warm-season vapour pressure deficit [hPa]

		* @param B_eeq independent variable equilibrium evaporation [mm d^-1]
		* @param B_eeq_mu independent variable mean equilibrium evaporation [mm d^-1]
		* @param B_eeq_sig independent variable stddev stddev equilibrium evaporation [mm d^-1]
		* @param eeq_a climate variable anomaly equilibrium evaporation [mm d^-1]

		* @param B_ws independent variable Warm-season soil water content [mm]
		* @param B_ws_mu independent variable mean Warm-season soil water content [mm]
		* @param B_ws_sig independent variable stddev Warm-season soil water content [mm]
		* @param ws_a climate variable anomaly Warm-season soil water content [mm]

		* @param B_ndep independent variable annual nitrogen deposition [Kg N ha^-1 yr^-1]
		* @param B_ndep_mu independent variable mean annual nitrogen deposition [Kg N ha^-1 yr^-1]
		* @param B_ndep_sig independent variable stddev annual nitrogen deposition [Kg N ha^-1 yr^-1]
		* @param ndep absolute annual nitrogen deposition [Kg N ha^-1 yr^-1]

		* @param B_ca independent variable annual carbon dioxide concentration [ppm]
		* @param B_ca_mu independent variable mean annual carbon dioxide concentration [ppm]
		* @param B_ca_sig independent variable stddev annual carbon dioxide concentration [ppm]
		* @param ca absolute annual carbon dioxide concentration [ppm]

		* @return the environmental effect on a growth or mortality increment in 
		* Mg * ha^-1 * yr^-1
		*/
		double EnvironmentalModifier(
			double B_dwf, double B_dwf_mu, double B_dwf_sig, double dwf_a,
			double B_rswd, double B_rswd_mu, double B_rswd_sig, double rswd_a,
			double B_tmean, double B_tmean_mu, double B_tmean_sig, double tmean_a,
			double B_vpd, double B_vpd_mu, double B_vpd_sig, double vpd_a,
			double B_eeq, double B_eeq_mu, double B_eeq_sig, double eeq_a,
			double B_ws, double B_ws_mu, double B_ws_sig, double ws_a,
			double B_ndep, double B_ndep_mu, double B_ndep_sig, double ndep,
			double B_ca, double B_ca_mu, double B_ca_sig, double ca)
		{
			double result = (dwf_a - B_dwf_mu) / B_dwf_sig * B_dwf +
				(rswd_a - B_rswd_mu) / B_rswd_sig * B_rswd +
				(tmean_a - B_tmean_mu) / B_tmean_sig * B_tmean +
				(vpd_a - B_vpd_mu) / B_vpd_sig * B_vpd +
				(eeq_a - B_eeq_mu) / B_eeq_sig * B_eeq +
				(ws_a - B_ws_mu) / B_ws_sig * B_ws +
				(ndep - B_ndep_mu) / B_ndep_sig * B_ndep +
				(ca - B_ca_mu) / B_ca_sig * B_ca;
			return result;
		}
    };

}}}
#endif
