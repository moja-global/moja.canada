#include "moja/flint/variable.h"

#include "moja/modules/cbm/smalltreegrowthcurve.h"
#include <cmath>

namespace moja {
namespace modules {
namespace cbm {
	SmallTreeGrowthCurve::SmallTreeGrowthCurve(SpeciesType speciesTypeName) {
        typeName = speciesTypeName;			
    }
    
    SpeciesType SmallTreeGrowthCurve::speciesType() const {
        return typeName;
    }

	std::string SmallTreeGrowthCurve::getEcoBoundary() const {
		return ecoBoundaryName;
	}

	void SmallTreeGrowthCurve::setParametersValue(const DynamicObject& data) {				
		a1 = data["a1"];
		a2 = data["a2"];
		a3 = data["a3"];
		b1 = data["b1"];
		b2 = data["b2"];
		b3 = data["b3"];
		c1 = data["c1"];
		c2 = data["c2"];
		c3 = data["c3"];
		a_bio = data["a_bio"];
		b_bio = data["b_bio"];
		a_vol = data["a_vol"];
		b_vol = data["b_vol"];

		maxAge = data["maxAge"];
	}

	void SmallTreeGrowthCurve::checkUpdateEcoParameters(std::string ecoZoneName, const DynamicObject& data) {
		if (ecoBoundaryName.empty() || ecoBoundaryName.compare(ecoBoundaryName) != 0) {
			//Eco boundary is changed, update the ecoBoundaryName first
			ecoBoundaryName = ecoZoneName;

			//get the eco-zone based parameters
			setParametersValue(data);

			//initialize the vectors
			initilizeVectors();

			//generate carbon curve for this small tree
			generateOrUpdateCarbonCurve();
		}
	}

	void SmallTreeGrowthCurve::setRootBiomassEquation(std::shared_ptr < cbm::RootBiomassEquation > rtBiomassEquation) {
		rootBiomassEquation = rtBiomassEquation;
	}

	void SmallTreeGrowthCurve::generateOrUpdateCarbonCurve() {
		for (int ageIndex = 0; ageIndex <= maxAge; ageIndex++) {
			double totalStemVolAtAge = getStemwoodVolumeAtAge(ageIndex);			
			double foliageBioRatioAtAge = getBiomassPercentage(COMPONENT::FOLIAGE, totalStemVolAtAge);
			double stemBioRatioAtAge = getBiomassPercentage(COMPONENT::STEMWOOD, totalStemVolAtAge);
			double otherBioRatioAtAge = 1 - foliageBioRatioAtAge - stemBioRatioAtAge;
			double stemwoodBioAtAge = getStemwoodBiomass(totalStemVolAtAge);

			double totalStemVolAtAgePlus = getStemwoodVolumeAtAge(ageIndex+1);		

			double foliageBioRatioAtAgePlus = getBiomassPercentage(COMPONENT::FOLIAGE, totalStemVolAtAgePlus);
			double stemBioRatioAtAgePlus = getBiomassPercentage(COMPONENT::STEMWOOD, totalStemVolAtAgePlus);
			double otherBioRatioAtAgePlus = 1 - foliageBioRatioAtAgePlus - stemBioRatioAtAgePlus;	
			double stemwoodBioAtAgePlus = getStemwoodBiomass(totalStemVolAtAgePlus);

			//store component's carbon increment
			stemCarbonIncrements[ageIndex] = 0.5 * (stemwoodBioAtAgePlus * stemBioRatioAtAgePlus - stemwoodBioAtAge * stemBioRatioAtAge);
			foliageCarbonIncrements[ageIndex] = 0.5 *( stemwoodBioAtAgePlus * foliageBioRatioAtAgePlus - stemwoodBioAtAge * foliageBioRatioAtAge);
			otherCarbonIncrements[ageIndex] = 0.5 * (stemwoodBioAtAgePlus * otherBioRatioAtAgePlus - stemwoodBioAtAge * otherBioRatioAtAge);
		}
	}

	std::unordered_map<std::string, double> SmallTreeGrowthCurve::getSmallTreeBiomassCarbonIncrements(double stem, double other, double foliage, double coarseRoot, double fineRoot, int age) {
		auto agIncrements = getAGIncrements( stem,  other,  foliage,  age);

		//get the total above ground biomass carbon
		double totalAGCarbon = stem + other + foliage + agIncrements["stemwood"] + agIncrements["other"] + agIncrements["foliage"];

		//get the total below ground biomass
		double totalBGBiomass = rootBiomassEquation->calculateRootBiomass(totalAGCarbon);

		auto rootProps = rootBiomassEquation->calculateRootProportions(totalBGBiomass);

		//get the total root biomassCarbon
		double rootCarbon = rootBiomassEquation->biomassToCarbon(totalBGBiomass);
		
		return std::unordered_map<std::string, double>{
			{"stemwood", agIncrements["stemwood"] },
			{"other", agIncrements["other"] },
			{"foliage", agIncrements["foliage"] },
			{"coarseRoot", rootCarbon * rootProps.coarse - coarseRoot },
			{"fineRoot", rootCarbon * rootProps.fine - fineRoot }			
		};
	}

	std::unordered_map<std::string, double> SmallTreeGrowthCurve::getAGIncrements(double stem, double other, double foliage, int age) {
		if (age > maxAge) { age = maxAge; } //TBD

		// Return either the increment or the remainder of the pool value, if
		// the increment would result in a negative pool value.
		return std::unordered_map<std::string, double> {
			{ "stemwood", std::max(stemCarbonIncrements[age], -stem) },
			{ "other", std::max(otherCarbonIncrements[age], -other) },
			{ "foliage", std::max(foliageCarbonIncrements[age], -foliage) }
		};
	}

	double SmallTreeGrowthCurve::getStemwoodVolumeAtAge(int age) {
		double retVal = a_vol * pow(age, b_vol) * (exp(-1 * a_vol * age));
		return retVal;
	}

	double SmallTreeGrowthCurve::getStemwoodBiomass(double stemwoodVolume) {
		double retVal = a_bio * pow(stemwoodVolume, b_bio);
		return retVal;
	}

	double SmallTreeGrowthCurve::getBiomassPercentage(COMPONENT component, double stemVolume) {
		double biomassPercentage = 0.0;
		double vol_max = 35.143543946;
		double vol_min = 7.5556895285;
		double p_sw_min = 0.347893;
		double p_sw_max = 0.3840097;

		if (stemVolume < vol_min) {
			return p_sw_min;
		}
		if (stemVolume > vol_max) {
			return p_sw_max;
		}

		switch (component) {		
		case COMPONENT::BARK:
			biomassPercentage = exp(a1 + a2 * stemVolume + a3 * log(stemVolume + 5)) / commonDivider(stemVolume);
			break;
		case COMPONENT::BRANCH:
			biomassPercentage = exp(b1 + b2 * stemVolume + b3 * log(stemVolume + 5)) / commonDivider(stemVolume);
			break;
		case COMPONENT::FOLIAGE:
			biomassPercentage = exp(c1 + c2 * stemVolume + c3 * log(stemVolume + 5)) / commonDivider(stemVolume);
			break;
		case COMPONENT::STEMWOOD:
			biomassPercentage = 1 / commonDivider(stemVolume);		
			break;
		}

		return biomassPercentage;
	}	

	double SmallTreeGrowthCurve::commonDivider(double volume) {
		double retVal = 1
						+ exp(a1 + a2 * volume + a3 * log(volume + 5))
						+ exp(b1 + b2 * volume + b3 * log(volume + 5))
						+ exp(c1 + c2 * volume + c3 * log(volume + 5));
		return retVal;
	}

	void SmallTreeGrowthCurve::initilizeVectors() {
		if (maxAge == 0) maxAge = 200; //by default

		stemCarbonIncrements.resize(maxAge + 1);
		foliageCarbonIncrements.resize(maxAge + 1);
		otherCarbonIncrements.resize(maxAge + 1);
	}

}}}