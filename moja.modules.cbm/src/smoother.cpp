#include <numeric>
#include <cmath>

#include "moja/modules/cbm/smoother.h"
#include <moja/modules/cbm/helper.h>
#include <moja/modules/cbm/lmeval.h>
#include <moja/modules/cbm/lmmin.h>

namespace moja {
namespace modules {
namespace cbm {	

    void Smoother::smooth(const StandGrowthCurve& standGrowthCurve,
                          ComponentBiomassCarbonCurve* carbonCurve,
                          SpeciesType speciesType) {

        int substitutionPoint = getComponentSmoothingSubstitutionRegionPoint(
            standGrowthCurve, speciesType);

        if (substitutionPoint < 1 || carbonCurve == nullptr) {
            return; // no need to smooth
        }		

        clearAndReserveDataSpace(smoothSampleSize);
        prepareSmoothingInputData(*carbonCurve,
                                  substitutionPoint,
                                  standGrowthCurve.standMaxAge());

        double merchC_wb2[2] = { 1, 0.1 };		// use any starting value, but not { 0, 0, 0 }
        double foliageC_wb2[2] = { 1, 0.1 };	// use any starting value, but not { 0, 0, 0 }		
        double totalAGBioC_wb2[2] = { 1, 0.1 };	// use any starting value, but not { 0, 0, 0 }	
        
        minimize(_smoothingMerchC.data(), merchC_wb2);		
        minimize(_smoothingFoliageC.data(), foliageC_wb2);
        minimize(_smoothingTotalAGBioC.data(), totalAGBioC_wb2);
        
        getFinalFittingRegionAndReplaceData(*carbonCurve, substitutionPoint,
                                            merchC_wb2, foliageC_wb2, totalAGBioC_wb2);
        clearAndReserveDataSpace(0);
    }

    int Smoother::getComponentSmoothingSubstitutionRegionPoint(
        const StandGrowthCurve& standGrowthCurve, SpeciesType speciesType) {

        int substitutionPoint = -1;

        // Get the stand max age.
        int standMaxAge = standGrowthCurve.standMaxAge();

        // Get annual maximum volume for a stand total merchantable.
        double maxAnnualTotaVolume = standGrowthCurve.getAnnualStandMaximumVolume();

        // Get the age at which the annual maximum volume reached.
        int minAgeForMaximumAnnualTotalMerchVol = standGrowthCurve.getStandAgeWithMaximumVolume();

        // Get the PERD factor for the component leading species.
        auto  pf = standGrowthCurve.getPERDFactor(speciesType);

        // Smooth only if annual maximum volume greater than the minimum PF volume.
        if (maxAnnualTotaVolume > pf->min_volume()) {
            bool doSmoothing = true;

            double volToCheckPrev = 0.0;
            double volToCheck = 0.0;
            double totalMerchVol = 0.0;
            double bioMerchStemwood = 0.0;
            double preNonMerchFactor = 0.0;
            double nonMerchFactor = 0.0;

            bool okToSmooth = false;
            bool nonMerchCapped = false;			

            for (int age = standMaxAge; age >= 0; age--) {
                preNonMerchFactor = nonMerchFactor;
                totalMerchVol = standGrowthCurve.getStandTotalVolumeAtAge(age);

                bioMerchStemwood = Helper::calculateMerchFactor(
                    totalMerchVol,
                    pf->a(), pf->b());

                nonMerchFactor = Helper::calculateNonMerchFactor(
                    bioMerchStemwood,
                    pf->a_nonmerch(), pf->b_nonmerch(), pf->k_nonmerch());

                if (nonMerchFactor < pf->cap_nonmerch()) {
                    okToSmooth = true;
                }

                if (nonMerchFactor < 1)	{
                    nonMerchFactor = 1.0;
                }

                if (nonMerchFactor > pf->cap_nonmerch()) {
                    nonMerchFactor = pf->cap_nonmerch();
                    nonMerchCapped = true;
                }

                volToCheckPrev = volToCheck;
                volToCheck = totalMerchVol;

                if (volToCheck == 0) {
                    if (maxAnnualTotaVolume < pf->min_volume()) {
                        doSmoothing = false;
                    }

                    if (!okToSmooth) {
                        doSmoothing = false;
                    }

                    if (doSmoothing) {
                        if (age < 2) {
                            doSmoothing = false;
                        }
                        else {
                            substitutionPoint = age + 1;
                            break;
                        }
                    }
                }

                if (doSmoothing && (age > 1 && age < minAgeForMaximumAnnualTotalMerchVol)) {
                    if (!nonMerchCapped){
                        if (volToCheck < pf->min_volume() && volToCheckPrev > pf->min_volume()) {
                            substitutionPoint = age + 1;
                            break;
                        }
                    }
                    else {
                        if (preNonMerchFactor < pf->cap_nonmerch()) {
                            if (volToCheck > pf->min_volume())	{
                                substitutionPoint = age + 1;
                                break;
                            }
                            else if (volToCheckPrev > pf->min_volume()){
                                substitutionPoint = age + 1;
                                break;
                            }
                        }
                    }
                }
            }
        }

        return substitutionPoint;
    }

    void Smoother::prepareSmoothingInputData(const ComponentBiomassCarbonCurve& carbonCurve,
                                             int substitutionPoint, int standMaxAge) {
        // The first value is set to 0.
        _smoothingMerchC[0] = 0;
        _smoothingFoliageC[0] = 0;
        _smoothingOtherC[0] = 0;
        _smoothingTotalAGBioC[0] = 0;
        _smoothingAageSerials[0] = 0;

        for (int i = 1; i < smoothSampleSize; i++) {
            int tempAgeIndex = substitutionPoint + i;
            if (tempAgeIndex <= standMaxAge) {
                _smoothingMerchC[i] = carbonCurve.getMerchCarbonAtAge(tempAgeIndex);
                _smoothingFoliageC[i] = carbonCurve.getFoliageCarbonAtAge(tempAgeIndex);
                _smoothingOtherC[i] = carbonCurve.getOtherCarbonAtAge(tempAgeIndex);
            }
            else {
                _smoothingMerchC[i] = carbonCurve.getMerchCarbonIncrement(standMaxAge);
                _smoothingFoliageC[i] = carbonCurve.getFoliageCarbonIncrement(standMaxAge);
                _smoothingOtherC[i] = carbonCurve.getOtherCarbonIncrement(standMaxAge);
            }

            _smoothingTotalAGBioC[i] = _smoothingMerchC[i] + _smoothingFoliageC[i] + _smoothingOtherC[i];
            _smoothingAageSerials[i] = substitutionPoint + i;
        }

        // Get the maximum value for the data in the smoothing region.
        _smoothingMaxMerchC		= (*std::max_element(_smoothingMerchC.begin(), _smoothingMerchC.end()));
        _smoothingMaxFoliageC	= (*std::max_element(_smoothingFoliageC.begin(), _smoothingFoliageC.end()));
        _smoothingMaxOtherC		= (*std::max_element(_smoothingOtherC.begin(), _smoothingOtherC.end()));
        _smoothingMaxTotalAGC	= (*std::max_element(_smoothingTotalAGBioC.begin(), _smoothingTotalAGBioC.end()));
        _smoothingMaxAge		= (*std::max_element(_smoothingAageSerials.begin(), _smoothingAageSerials.end()));

        // Average the original pool data by the maximum value.
        for (int i = 0; i < smoothSampleSize; i++) {
            _smoothingMerchC[i] = _smoothingMerchC[i] / _smoothingMaxMerchC;
            _smoothingOtherC[i] = _smoothingOtherC[i] / _smoothingMaxOtherC;
            _smoothingFoliageC[i] = _smoothingFoliageC[i] / _smoothingMaxFoliageC;
            _smoothingTotalAGBioC[i] = _smoothingTotalAGBioC[i] / _smoothingMaxTotalAGC;
            _smoothingAageSerials[i] = _smoothingAageSerials[i] / _smoothingMaxAge;
        }
    }

    void Smoother::clearAndReserveDataSpace(int workingFitingRange) {
        _smoothingMaxMerchC = 0;
        _smoothingMaxFoliageC = 0;
        _smoothingMaxOtherC = 0;
        _smoothingMaxTotalAGC = 0;
        _smoothingMaxAge = 0;

        _smoothingMerchC.clear();
        _smoothingFoliageC.clear();
        _smoothingOtherC.clear();
        _smoothingTotalAGBioC.clear();
        _smoothingAageSerials.clear();		

        _smoothingMerchC.resize(workingFitingRange+1);
        _smoothingFoliageC.resize(workingFitingRange+1);
        _smoothingOtherC.resize(workingFitingRange+1);
        _smoothingTotalAGBioC.resize(workingFitingRange+1);
        _smoothingAageSerials.resize(workingFitingRange+1);
    }

    /*
    * Find the minimum weibull paramters which are stored in the startingVals array
    */
    void Smoother::minimize(double yValues[], double startingVals[]) {	
        lm_data_type data;
        data.user_func = Smoother::weibull_2Parameter;
        data.user_t = _smoothingAageSerials.data();
        data.user_y = yValues;

        LmMin lmMin;
        lm_control_type control;		
        lmMin.lm_initialize_control(&control);

        // Perform the fit:		
        lmMin.lm_minimize(smoothSampleSize, numerOfParameters, startingVals,
                           LmEval::lm_evaluate_default, LmEval::lm_print_default,
                           &data, &control);
    
        if (startingVals[0] > 150) {
            startingVals[0] = 150;
            if (startingVals[1] > 5) {
                startingVals[1] = 5;
            }
        }
    }
    
    double Smoother::weibull_2Parameter(double t, double* p) {
        if (p[0] < 0) {
            p[0] = 0;
        }

        double a = p[0] * -1;

        if (p[1] < 0.7) {
            p[1] = 0.7;
        }

        double b = p[1];
        double c = pow(t, b);

        return 1 - exp(a*c);
    }


    int Smoother::getFinalFittingRegionAndReplaceData(
        ComponentBiomassCarbonCurve& carbonCurve, int substitutionPoint,
        double merchCWeibullParameters[], double foliageCWeibullParameters[],
        double totalAGBioCWeibullParameters[]) {

        int finalReplacementLength = 0;

        int totalReplacementRegion = substitutionPoint + extendedRegionSize;
        int fitAge5percent = totalReplacementRegion;
        int fitAge3percent = totalReplacementRegion;
        int fitAge1percent = totalReplacementRegion;	
        
        std::vector<double> merchReplacement(totalReplacementRegion + 1);
        std::vector<double> foliageReplacement(totalReplacementRegion + 1);
        std::vector<double> otherReplacement(totalReplacementRegion + 1);

        for (int i = 0; i <= totalReplacementRegion; i++) {
            double merch = carbonCurve.getMerchCarbonAtAge(i);
            double foliage = carbonCurve.getFoliageCarbonAtAge(i);
            double other = carbonCurve.getOtherCarbonAtAge(i);
            double total = merch + foliage + other;

            double fitMerch = weibull_2Parameter(i / _smoothingMaxAge, merchCWeibullParameters)
                * _smoothingMaxMerchC;

            double fitFoliage = weibull_2Parameter(i / _smoothingMaxAge, foliageCWeibullParameters)
                * _smoothingMaxFoliageC;

            double fitTotal = weibull_2Parameter(i / _smoothingMaxAge, totalAGBioCWeibullParameters)
                * _smoothingMaxTotalAGC;

            if (total == 0) {
                total = 0.00001;
            }

            if (std::isnan(fitMerch)) {
                fitMerch = 0;
            }

            merchReplacement[i] = fitMerch;

            if (fitTotal < fitMerch) {
                fitTotal = fitMerch;
            }

            if (fitTotal - fitMerch < fitFoliage) {
                fitFoliage = fitTotal - fitMerch;
            }

            if (std::isnan(fitFoliage)) {
                fitFoliage = 0;
            }

            foliageReplacement[i] = fitFoliage;
            otherReplacement[i] = fitTotal - fitFoliage - fitMerch;

            if ((abs(total - fitTotal) / total < 0.05) &&
                (i < fitAge3percent) &&
                (i >= substitutionPoint)) {

                fitAge5percent = i;
            }

            if ((abs(total - fitTotal) / total < 0.03) &&
                (i < fitAge3percent)
                && (i >= substitutionPoint)) {

                fitAge3percent = i;
            }

            if ((abs(total - fitTotal) / total < 0.01) &&
                (i < fitAge1percent) &&
                (i >= substitutionPoint)) {

                fitAge1percent = i;
            }
            
            // Case 1: the fit age was found for a 1% join.
            if (fitAge1percent < totalReplacementRegion) {
                finalReplacementLength = fitAge1percent;
            }
            // Case 2: the fit age was found for a 3% relaxed join, and not for a 1% join.
            else if (fitAge3percent < totalReplacementRegion) {
                finalReplacementLength = fitAge3percent;
            }
            // Case 3: the fit age was found for a 5% relaxed join, and not for a 3% join or a 1% join.
            else if (fitAge5percent < totalReplacementRegion) {
                finalReplacementLength = fitAge5percent;
            }
            // Case 4: neither a 1% nor 3% nor 5% join was found.
            else {
                finalReplacementLength = totalReplacementRegion;
            }
        }

        // Now, perform the final substitution.
        for (int i = 0; i <= finalReplacementLength; i++) {			
            carbonCurve.setMerchCarbonAtAge(i, merchReplacement[i]);
            carbonCurve.setFoliageCarbonAtAge(i, foliageReplacement[i]);
            carbonCurve.setOtherCarbonAtAge(i, otherReplacement[i]);
        }		

        return finalReplacementLength;
    }

}}}