#include "moja/flint/variable.h"

#include "moja/modules/cbm/smalltreegrowthcurve.h"
#include <cmath>

#include <moja/logging.h>

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
		vol_max = data["vol_max"];
		vol_min = data["vol_min"];
		p_sw_min = data["p_sw_min"];
		p_sw_max = data["p_sw_max"];
		p_fl_min = data["p_fl_min"];
		p_fl_max = data["p_fl_max"];
		p_sb_min = data["p_sb_min"];
		p_sb_max = data["p_sb_max"];
		p_br_min = data["p_br_min"];
		p_br_max = data["p_br_max"];

		sw_a = data["sw_a"];
		hw_a = data["hw_a"];
		hw_b = data["hw_b"];
		frp_a = data["frp_a"];
		frp_b = data["frp_b"];
		frp_c = data["frp_c"];
	}

	void SmallTreeGrowthCurve::checkUpdateEcoParameters(std::string ecoZoneName, const DynamicObject& data) {
		if (ecoBoundaryName.empty() || ecoBoundaryName.compare(ecoZoneName) != 0) {
			//Eco boundary is changed, update the ecoBoundaryName first
			ecoBoundaryName = ecoZoneName;

			//get the eco-zone based parameters
			setParametersValue(data);

			//set root biomass equation
			setRootBiomassEquation();

			//initialize the vectors
			initilizeVectors();

			//generate carbon curve for this small tree
			generateOrUpdateCarbonCurve();
		}
	}

	void SmallTreeGrowthCurve::setRootBiomassEquation() {
		if (typeName == SpeciesType::Softwood){		
			rootBiomassEquation = std::make_shared<SoftwoodRootBiomassEquation>(sw_a, frp_a, frp_b, frp_c);
		} else{
			rootBiomassEquation = std::make_shared<HardwoodRootBiomassEquation>(hw_a, hw_b, frp_a, frp_b, frp_c);
		}
	}

	void SmallTreeGrowthCurve::generateOrUpdateCarbonCurve() {
		for (int ageIndex = 0; ageIndex <= maxAge; ageIndex++) {
			double totalStemVolAtAge = getStemwoodVolumeAtAge(ageIndex);			
			double barkBioRatioAtAge = getBiomassPercentage(COMPONENT::BARK, totalStemVolAtAge);
			double foliageBioRatioAtAge = getBiomassPercentage(COMPONENT::FOLIAGE, totalStemVolAtAge);
			double stemBioRatioAtAge = getBiomassPercentage(COMPONENT::STEMWOOD, totalStemVolAtAge);
			double otherBioRatioAtAge = 1 - foliageBioRatioAtAge - stemBioRatioAtAge - barkBioRatioAtAge;
			double stemwoodBioAtAge = getStemwoodBiomass(totalStemVolAtAge);
						
			//Calculate total tree biomass by stemwood biomass
			double treeBioAtAge = (stemBioRatioAtAge == 0) ? 0 : (stemwoodBioAtAge / stemBioRatioAtAge);

			/*
			MOJA_LOG_INFO << ageIndex << ", " << totalStemVolAtAge << ", " << stemwoodBioAtAge << ", "<< foliageBioRatioAtAge << ", " << stemBioRatioAtAge << ", "
				<< otherBioRatioAtAge << ", " << treeBioAtAge;
			*/

			double totalStemVolAtAgePlus = getStemwoodVolumeAtAge(ageIndex+1);
			double barkBioRatioAtAgePlus = getBiomassPercentage(COMPONENT::BARK, totalStemVolAtAge);
			double foliageBioRatioAtAgePlus = getBiomassPercentage(COMPONENT::FOLIAGE, totalStemVolAtAgePlus);
			double stemBioRatioAtAgePlus = getBiomassPercentage(COMPONENT::STEMWOOD, totalStemVolAtAgePlus);		
			double otherBioRatioAtAgePlus = 1 - foliageBioRatioAtAgePlus - stemBioRatioAtAgePlus - barkBioRatioAtAgePlus;
			double stemwoodBioAtAgePlus = getStemwoodBiomass(totalStemVolAtAgePlus);
			double treeBioAtAgePlus = (stemBioRatioAtAgePlus == 0) ? 0 : (stemwoodBioAtAgePlus / stemBioRatioAtAgePlus);

			//store component's carbon increment
			stemCarbonIncrements[ageIndex] = 0.5 * (treeBioAtAgePlus * stemBioRatioAtAgePlus - treeBioAtAge * stemBioRatioAtAge);
			foliageCarbonIncrements[ageIndex] = 0.5 *(treeBioAtAgePlus * foliageBioRatioAtAgePlus - treeBioAtAge * foliageBioRatioAtAge);
			otherCarbonIncrements[ageIndex] = 0.5 * (treeBioAtAgePlus * otherBioRatioAtAgePlus - treeBioAtAge * otherBioRatioAtAge);
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
		if (age > maxAge) {
			age = maxAge; 
			//special solution for small tree over age 200
			return std::unordered_map<std::string, double> {
				{ "stemwood", 0.0 },
				{ "other", 0.0 },
				{ "foliage", 0.0 }
			};
		}		
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

		if (stemVolume < vol_min) {
			switch (component) {
				case COMPONENT::BARK:
					biomassPercentage = p_sb_min;
					break;
				case COMPONENT::BRANCH:
					biomassPercentage = p_br_min;
					break;
				case COMPONENT::FOLIAGE:
					biomassPercentage = p_fl_min;
					break;
				case COMPONENT::STEMWOOD:
					biomassPercentage = p_sw_min;
					break;
			}			
		}
		else if (stemVolume > vol_max) {
			switch (component) {		
				case COMPONENT::BARK:
					biomassPercentage = p_sb_max;
					break;
				case COMPONENT::BRANCH:
					biomassPercentage = p_br_max;
					break;
				case COMPONENT::FOLIAGE:
					biomassPercentage = p_fl_max;
					break;
				case COMPONENT::STEMWOOD:
					biomassPercentage = p_sw_max;
					break;
			}			
		} else {
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