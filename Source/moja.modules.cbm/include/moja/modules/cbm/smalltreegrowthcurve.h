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

		void setRootBiomassEquation();

		void generateOrUpdateCarbonCurve();

	private:
		int maxAge{200};

		double a_bio{0.0};
		double b_bio{0.0};
		double a1{0.0};
		double a2{0.0};
		double a3{0.0};
		double b1{0.0};
		double b2{0.0};
		double b3{0.0};
		double c1{0.0};
		double c2{0.0};
		double c3{0.0};
		double a_vol{0.0};
		double b_vol{0.0};
		
		double vol_max{ 0.0 };
		double vol_min{ 0.0 };
		double p_sw_min{ 0.0 };
		double p_sw_max{ 0.0 };
		double p_fl_min{ 0.0 };
		double p_fl_max{ 0.0 };
		double p_br_min{ 0.0 };
		double p_br_max{ 0.0 };
		double p_sb_min{ 0.0 };
		double p_sb_max{ 0.0 };

		double sw_a{ 0.0 };
		double hw_a{ 0.0 };
		double hw_b{ 0.0 };
		double frp_a{ 0.0 };
		double frp_b{ 0.0 };
		double frp_c{ 0.0 };

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
