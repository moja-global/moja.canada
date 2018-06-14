#ifndef MOJA_MODULES_CBM_SMALLTREEGROWTHCURVE_H_
#define MOJA_MODULES_CBM_SMALLTREEGROWTHCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/rootbiomassequation.h"
#include "moja/modules/cbm/treespecies.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Enumeration of tree species type, softwood or hardwood.
	/// </summary>
	
	enum class COMPONENT{BARK=1, BRANCH, FOLIAGE, STEMWOOD, OTHER};

	class CBM_API SmallTreeGrowthCurve {
	public:
		SmallTreeGrowthCurve() {}
		virtual ~SmallTreeGrowthCurve() {}

		SmallTreeGrowthCurve(SpeciesType speciesType);
		
		std::string getEcoBoundary() const;
		SpeciesType speciesType() const;

		void checkUpdateEcoParameters(std::string ecoBoundaryName, const DynamicObject& data);

		std::unordered_map<std::string, double> getSmallTreeBiomassCarbonIncrements(double stem, double other, double foliage, double coarseRoot, double fineRoot, int age);

		double getStemwoodVolumeAtAge(int age);

		double getStemwoodBiomass(double stemwoodVolume);

		double getBiomassPercentage(COMPONENT component, double totalStemVolume);

		void setRootBiomassEquation(std::shared_ptr<cbm::RootBiomassEquation> rtBiomassEquation);

		void generateOrUpdateCarbonCurve();

	private:
		int maxAge;

		double a_bio;
		double b_bio;
		double a1;
		double a2;
		double a3;
		double b1;
		double b2;
		double b3;
		double c1;
		double c2;
		double c3;
		double a_vol;
		double b_vol;

		std::string ecoBoundaryName;
		SpeciesType typeName;

		std::vector<double> stemCarbonIncrements;
		std::vector<double> foliageCarbonIncrements;
		std::vector<double> otherCarbonIncrements;

		std::shared_ptr<cbm::RootBiomassEquation> rootBiomassEquation;

		void setParametersValue(const DynamicObject& data);		
		void initilizeVectors();
		double commonDivider(double volume);
		std::unordered_map<std::string, double> getAGIncrements(double stem, double other, double foliage, int age);
	};

}}}
#endif
