#include "moja/modules/cbm/volumetobiomassconverter.h"
#include "moja/modules/cbm/helper.h"

namespace moja {
namespace modules {
namespace cbm {

    std::shared_ptr<ComponentBiomassCarbonCurve> VolumeToBiomassConverter::generateComponentBiomassCarbonCurve(
        StandGrowthCurve& standGrowthCurve, SpeciesType speciesType) {

        int standMaxAge = standGrowthCurve.standMaxAge();
        auto pf = standGrowthCurve.getPERDFactor(speciesType);

        std::shared_ptr<ComponentBiomassCarbonCurve> compomentCarbonCurve =
            std::make_shared<ComponentBiomassCarbonCurve>(standMaxAge);
        
        for (int age = standMaxAge; age >= 0; age--) {
            double totalMerchVol = standGrowthCurve.getStandTotalVolumeAtAge(age);
            double bioMerchStemwood = Helper::calculateMerchFactor(totalMerchVol, pf->a(), pf->b());
            double nonMerchFactor = Helper::calculateNonMerchFactor(
                bioMerchStemwood, pf->a_nonmerch(), pf->b_nonmerch(), pf->k_nonmerch());	

            if (nonMerchFactor < 1.0 || std::isnan(nonMerchFactor) || std::isinf(nonMerchFactor)) {
                nonMerchFactor = 1.0;
            }

            if (nonMerchFactor > pf->cap_nonmerch()) {
                nonMerchFactor = pf->cap_nonmerch();
            }

            double bioNonmerchStemwood = (nonMerchFactor - 1) * bioMerchStemwood;
            double saplingFactor = Helper::calculateSaplingFactor(
                bioMerchStemwood + bioNonmerchStemwood, pf->k_sap(), pf->a_sap(), pf->b_sap());

            if (saplingFactor < 1.0 || std::isnan(saplingFactor) || std::isinf(saplingFactor)) {
                saplingFactor = 1.0;
            }

            if (saplingFactor > pf->cap_sap()) {
                saplingFactor = pf->cap_sap();
            }

            double bioSapStemwood = (saplingFactor - 1) * (bioMerchStemwood + bioNonmerchStemwood);
            double bioTotalStemwood = bioMerchStemwood + bioNonmerchStemwood + bioSapStemwood;

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

            double bioTotalTree = bioTotalStemwood / pStemwood;
            double bioStemBark = bioTotalTree * pStembark;
            double bioBranches = bioTotalTree * pBranches;
            double bioFoliage = bioTotalTree * pFoliage;

            double softwoodVolumeRatio = standGrowthCurve.getStandSoftwoodVolumeRatioAtAge(age);

            if (speciesType == SpeciesType::Softwood) {
                bioTotalTree *= softwoodVolumeRatio;
                bioTotalStemwood *= softwoodVolumeRatio;
                bioStemBark *= softwoodVolumeRatio;
                bioBranches *= softwoodVolumeRatio;
                bioFoliage *= softwoodVolumeRatio;
                bioMerchStemwood *= softwoodVolumeRatio;
                bioNonmerchStemwood *= softwoodVolumeRatio;
                bioSapStemwood *= softwoodVolumeRatio;
            } else {
                bioTotalTree *= (1 - softwoodVolumeRatio);
                bioTotalStemwood *= (1 - softwoodVolumeRatio);
                bioStemBark *= (1 - softwoodVolumeRatio);
                bioBranches *= (1 - softwoodVolumeRatio);
                bioFoliage *= (1 - softwoodVolumeRatio);
                bioMerchStemwood *= (1 - softwoodVolumeRatio);
                bioNonmerchStemwood *= (1 - softwoodVolumeRatio);
                bioSapStemwood *= (1 - softwoodVolumeRatio);
            }

            double bioMerchBark = 0.0;
            double bioMerchC = 0.0;
            double bioOtherC = 0.0;

            if (bioMerchStemwood > 0.0) {
                double bioTop = 0.0;
                double bioStump = 0.0;
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

                bioMerchStemwood -= (bioTop + bioStump);
                bioMerchC = std::max(0.0, (bioMerchStemwood + bioMerchBark) * 0.5);
                bioOtherC = (bioTotalTree - bioFoliage - bioMerchStemwood - bioMerchBark) * 0.5;
            } else {
                bioOtherC = (bioTotalTree - bioFoliage) * 0.5;
                bioSapStemwood = bioTotalTree - bioFoliage - bioBranches;
            }

            bioOtherC < 0.0 ? 0.0 : bioOtherC;
            double bioFoliageC = std::max(0.0, bioFoliage * 0.5);

            compomentCarbonCurve->setMerchCarbonAtAge(age, bioMerchC);
            compomentCarbonCurve->setOtherCarbonAtAge(age, bioOtherC);
            compomentCarbonCurve->setFoliageCarbonAtAge(age, bioFoliageC);			
        }

        return compomentCarbonCurve;
    }

    void VolumeToBiomassConverter::doSmoothing(
        const StandGrowthCurve& standGrowthCurve,
        ComponentBiomassCarbonCurve* carbonCurve,
        SpeciesType speciesType) {

        if (_smootherEnabled) {
            _smoother.smooth(standGrowthCurve, carbonCurve, speciesType);
        }
    }

}}}
