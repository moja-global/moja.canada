#include "moja/modules/cbm/volumetobiomassconverter.h"
#include "moja/modules/cbm/helper.h"

namespace moja {
namespace modules {
namespace cbm {

    VolumeToBiomassConverter::VolumeToBiomassConverter() {
        _smootherEnabled = true;
        _smoother = std::make_unique<Smoother>();
    }

    std::shared_ptr<ComponentBiomassCarbonCurve> VolumeToBiomassConverter::generateComponentBiomassCarbonCurve(
        std::shared_ptr<StandGrowthCurve> standGrowthCurve, SpeciesType speciesType) {

        int standMaxAge = standGrowthCurve->standMaxAge();
        auto pf = standGrowthCurve->getPERDFactor(speciesType);

        std::shared_ptr<ComponentBiomassCarbonCurve> compomentCarbonCurve =
            std::make_shared<ComponentBiomassCarbonCurve>(standMaxAge);
        
        double preTotalMerchVol = 0;
        double totalMerchVol = 0;
        double preNonMerchFactor = 0;
        double nonMerchFactor = 0;

        double bioTotalTree = 0;
        double bioTotalStemwood = 0;
        double bioMerchStemwood = 0;
        double bioNonmerchStemwood = 0;
        double bioStemBark = 0;
        double bioBranches = 0;
        double bioFoliage = 0;
        double bioSapStemwood = 0;
        double softwoodVolumeRatio = 0;

        bool nonMerchCapped = false;

        // Get the age at which the annual maximum volume reached.
        int minAgeForMaximumAnnualTotalMerchVol = standGrowthCurve->getStandAgeWithMaximumVolume();

        for (int age = standMaxAge; age >= 0; age--) {
            preTotalMerchVol = totalMerchVol;
            totalMerchVol = standGrowthCurve->getStandTotalVolumeAtAge(age);

            bioMerchStemwood = Helper::calculateMerchFactor(totalMerchVol, pf->a(), pf->b());

            preNonMerchFactor = nonMerchFactor;
            nonMerchFactor = Helper::calculateNonMerchFactor(
                bioMerchStemwood, pf->a_nonmerch(), pf->b_nonmerch(), pf->k_nonmerch());	

            if (nonMerchFactor < 1) {
                nonMerchFactor = 1.0;
            }

            if (nonMerchFactor > pf->cap_nonmerch()) {
                nonMerchFactor = pf->cap_nonmerch();
                nonMerchCapped = true;
            }

            bioNonmerchStemwood = (nonMerchFactor - 1) * bioMerchStemwood;

            double saplingFactor = Helper::calculateSaplingFactor(
                bioMerchStemwood + bioNonmerchStemwood, pf->k_sap(), pf->a_sap(), pf->b_sap());

            if (saplingFactor < 1) {
                saplingFactor = 1.0;
            }

            if (saplingFactor > pf->cap_sap()) {
                saplingFactor = pf->cap_sap();
            }

            bioSapStemwood = (saplingFactor - 1) * (bioMerchStemwood + bioNonmerchStemwood);
            bioTotalStemwood = bioMerchStemwood + bioNonmerchStemwood + bioSapStemwood;

            double pStemwood = 0.0;
            double pStembark = 0.0;
            double pBranches = 0.0;
            double pFoliage = 0.0;

            if (totalMerchVol < pf->min_volume()) {
                pStemwood = pf->low_stemwood_prop();
                pStembark = pf->low_stembark_prop();
                pBranches = pf->low_branches_prop();
                pFoliage = 1 - pStemwood - pStembark - pBranches;
            } else if (totalMerchVol > pf->max_volume()) {
                pStemwood = pf->high_stemwood_prop();
                pStembark = pf->high_stembark_prop();
                pBranches = pf->high_branches_prop();
                pFoliage = 1 - pStemwood - pStembark - pBranches;
            } else {
                double Aterm = Helper::modelTerm(totalMerchVol, pf->a1(), pf->a2(), pf->a3());
                double Bterm = Helper::modelTerm(totalMerchVol, pf->b1(), pf->b2(), pf->b3());
                double Cterm = Helper::modelTerm(totalMerchVol, pf->c1(), pf->c2(), pf->c3());

                pStemwood = 1 / (1 + Aterm + Bterm + Cterm);
                pStembark = Aterm / (1 + Aterm + Bterm + Cterm);
                pBranches = Bterm / (1 + Aterm + Bterm + Cterm);
                pFoliage = 1 - pStemwood - pStembark - pBranches;
            }

            bioTotalTree = bioTotalStemwood / pStemwood;
            bioStemBark = bioTotalTree * pStembark;
            bioBranches = bioTotalTree * pBranches;
            bioFoliage = bioTotalTree * pFoliage;

            softwoodVolumeRatio = standGrowthCurve->getStandSoftwoodVolumeRationAtAge(age);

            if (speciesType == SpeciesType::Softwood)
            {
                bioTotalTree *= softwoodVolumeRatio;
                bioTotalStemwood *= softwoodVolumeRatio;
                bioStemBark *= softwoodVolumeRatio;
                bioBranches *= softwoodVolumeRatio;
                bioFoliage *= softwoodVolumeRatio;
                bioMerchStemwood *= softwoodVolumeRatio;
                bioNonmerchStemwood *= softwoodVolumeRatio;
                bioSapStemwood *= softwoodVolumeRatio;
            }
            else
            {
                bioTotalTree *= (1 - softwoodVolumeRatio);
                bioTotalStemwood *= (1 - softwoodVolumeRatio);
                bioStemBark *= (1 - softwoodVolumeRatio);
                bioBranches *= (1 - softwoodVolumeRatio);
                bioFoliage *= (1 - softwoodVolumeRatio);
                bioMerchStemwood *= (1 - softwoodVolumeRatio);
                bioNonmerchStemwood *= (1 - softwoodVolumeRatio);
                bioSapStemwood *= (1 - softwoodVolumeRatio);
            }

            double bioTop = 0;
            double bioStump = 0;

            double bioMerchBark = 0;
            double bioOtherBark = 0;
            double bioMerchC = 0;
            double bioOtherC = 0;
            double bioFoliageC = 0;

            if (bioMerchStemwood > 0.0) {
                if (speciesType == SpeciesType::Softwood) {
                    bioTop = bioMerchStemwood * pf->softwood_top_prop() / 100.0;
                    bioStump = bioMerchStemwood * pf->softwood_stump_prop() / 100.0;
                    bioMerchBark = bioStemBark * (bioMerchStemwood / bioTotalStemwood) *
                        (1 - (pf->softwood_top_prop() + pf->softwood_stump_prop()) / 100.0);
                } else if (speciesType == SpeciesType::Hardwood) {
                    bioTop = bioMerchStemwood * pf->hardwood_top_prop() / 100.0;
                    bioStump = bioMerchStemwood * pf->hardwood_stump_prop() / 100.0;
                    bioMerchBark = bioStemBark * (bioMerchStemwood / bioTotalStemwood) *
                        (1 - (pf->hardwood_top_prop() + pf->hardwood_stump_prop()) / 100.0);
                }

                bioOtherBark = bioStemBark - bioMerchBark;
                bioMerchStemwood -= (bioTop + bioStump);

                bioMerchC = (bioMerchStemwood + bioMerchBark) * 0.5;
                bioFoliageC = bioFoliage * 0.5;
                bioOtherC = (bioTotalTree - bioFoliage - bioMerchStemwood - bioMerchBark) * 0.5;
            } else {
                bioMerchC = 0;
                bioOtherC = (bioTotalTree - bioFoliage) * 0.5;
                bioFoliageC = bioFoliage * 0.5;
                bioMerchBark = 0;
                bioOtherBark = 0;
                bioSapStemwood = bioTotalTree - bioFoliage - bioBranches - bioOtherBark;
            }

            if (bioMerchC < 0) {
                bioMerchC = 0;
            }

            if (bioFoliageC < 0) {
                bioFoliageC = 0;
            }

            if (bioOtherC < 0) {
                bioOtherC = 0;
            }

            compomentCarbonCurve->setMerchCarbonAtAge(age, bioMerchC);
            compomentCarbonCurve->setOtherCarbonAtAge(age, bioOtherC);
            compomentCarbonCurve->setFoliageCarbonAtAge(age, bioFoliageC);			
        }

        return compomentCarbonCurve;
    }

    void VolumeToBiomassConverter::DoSmoothing(
        const StandGrowthCurve& standGrowthCurve,
        ComponentBiomassCarbonCurve* carbonCurve,
        SpeciesType sepciesType) {

        if (_smootherEnabled) {
            _smoother->smooth(standGrowthCurve, carbonCurve, sepciesType);
        }
    }

}}}
