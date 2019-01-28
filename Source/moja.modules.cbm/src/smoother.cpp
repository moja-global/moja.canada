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

        double merchC_wb2[]      = { 1, 0.1 };	// use any starting value, but not { 0, 0, 0 }
        double foliageC_wb2[]    = { 1, 0.1 };	// use any starting value, but not { 0, 0, 0 }		
        double totalAGBioC_wb2[] = { 1, 0.1 };	// use any starting value, but not { 0, 0, 0 }	
        
        minimize(_smoothingMerchC.data(), merchC_wb2);		
        minimize(_smoothingFoliageC.data(), foliageC_wb2);
        minimize(_smoothingTotalAGBioC.data(), totalAGBioC_wb2);
        
        getFinalFittingRegionAndReplaceData(*carbonCurve, substitutionPoint,
                                            merchC_wb2, foliageC_wb2, totalAGBioC_wb2);
        clearAndReserveDataSpace(1);
    }

    int Smoother::getComponentSmoothingSubstitutionRegionPoint(
        const StandGrowthCurve& standGrowthCurve, SpeciesType speciesType) {

        // Get the stand max age.
        int standMaxAge = standGrowthCurve.standMaxAge();

        // Get annual maximum volume for a stand total merchantable.
        double maxAnnualTotaVolume = standGrowthCurve.getAnnualStandMaximumVolume();

        // Get the age at which the annual maximum volume reached.
        int minAgeForMaximumAnnualTotalMerchVol = standGrowthCurve.getStandAgeWithMaximumVolume();

        double ration = standGrowthCurve.getStandSoftwoodVolumeRatioAtAge(minAgeForMaximumAnnualTotalMerchVol);

        // Get the PERD factor for the component leading species.
        auto  pf = standGrowthCurve.getPERDFactor(speciesType);


        // Smooth only if annual maximum volume greater than the minimum PF volume.
        if (maxAnnualTotaVolume > pf->min_volume()) {

            double prevNonMerchFactor = 0.0;
            bool OkToSmooth = false;
            double nonMerchFactor = 0.0;
            for (int age = standMaxAge - 2; age >= 0; age--)
            {
                double merchVolume = standGrowthCurve.getStandTotalVolumeAtAge(age);
                double merchVolume1YearOlder = standGrowthCurve.getStandTotalVolumeAtAge(age + 1);
                double bioMerchStemwood = Helper::calculateMerchFactor(merchVolume, pf->a(), pf->b());

                //get the nonmerch factor (eq 2)
                prevNonMerchFactor = nonMerchFactor;
                nonMerchFactor = Helper::calculateNonMerchFactor(bioMerchStemwood, pf->a_nonmerch(), pf->b_nonmerch(), pf->k_nonmerch());;
                if (nonMerchFactor < pf->cap_nonmerch())
                {
                    OkToSmooth = true;
                }
                if (nonMerchFactor < 1.0)
                {
                    nonMerchFactor = 1.0;
                }
                bool NonMerchCapped = false;
                if (nonMerchFactor >= pf->cap_nonmerch())
                {
                    nonMerchFactor = pf->cap_nonmerch();
                    NonMerchCapped = true;
                }
                if (OkToSmooth && merchVolume == 0 && age > 1)
                {
                    return age + 1;
                }
                else if (age <= 1)
                {
                    return -1;
                }
                if ((age < minAgeForMaximumAnnualTotalMerchVol))
                {
                    if (!NonMerchCapped)
                    {
                        if ((merchVolume < pf->min_volume()) && (merchVolume1YearOlder > pf->min_volume()))
                        {// if we're uncapped and vol has just crossed below min vol
                            if (OkToSmooth)
                                return age + 1;
                            else
                                return -1;
                        }
                    }
                    else if (NonMerchCapped)
                    {
                        if ((prevNonMerchFactor < pf->cap_nonmerch()) && (merchVolume >= pf->min_volume()))
                        {// if we've just crossed into uncapped territory and are above min vol
                            if (OkToSmooth)
                                return age + 1;
                            else
                                return -1;
                        }
                        else if ((prevNonMerchFactor < pf->cap_nonmerch()) && (merchVolume < pf->min_volume()) && (merchVolume1YearOlder > pf->min_volume()))
                        {// if we've just crossed into uncapped territory and crossed below  min vol
                            if (OkToSmooth)
                                return age + 1;
                            else
                                return -1;
                        }
                    }
                }

            }
        }
        return -1;
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
                _smoothingMerchC[i] = carbonCurve.getMerchCarbonAtAge(standMaxAge);
                _smoothingFoliageC[i] = carbonCurve.getFoliageCarbonAtAge(standMaxAge);
                _smoothingOtherC[i] = carbonCurve.getOtherCarbonAtAge(standMaxAge);
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

        _smoothingMerchC.resize(workingFitingRange);
        _smoothingFoliageC.resize(workingFitingRange);
        _smoothingOtherC.resize(workingFitingRange);
        _smoothingTotalAGBioC.resize(workingFitingRange);
        _smoothingAageSerials.resize(workingFitingRange);
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

            merchReplacement.at(i) = fitMerch;

            if (fitTotal < fitMerch) {
                fitTotal = fitMerch;
            }

            if (fitTotal - fitMerch < fitFoliage) {
                fitFoliage = fitTotal - fitMerch;
            }

            if (std::isnan(fitFoliage)) {
                fitFoliage = 0;
            }

            foliageReplacement.at(i) = fitFoliage;
            otherReplacement.at(i) = fitTotal - fitFoliage - fitMerch;

            double fitMatchRatio = fabs(total - fitTotal) / total;

            if ((fitMatchRatio < 0.05) &&
                (i < fitAge5percent) &&
                (i >= substitutionPoint)) {

                fitAge5percent = i;
            }

            if ((fitMatchRatio < 0.03) &&
                (i < fitAge3percent)
                && (i >= substitutionPoint)) {

                fitAge3percent = i;
            }

            if ((fitMatchRatio < 0.01) &&
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
            carbonCurve.setMerchCarbonAtAge(i, merchReplacement.at(i));
            carbonCurve.setFoliageCarbonAtAge(i, foliageReplacement.at(i));
            carbonCurve.setOtherCarbonAtAge(i, otherReplacement.at(i));
        }		

        return finalReplacementLength;
    }

}}}