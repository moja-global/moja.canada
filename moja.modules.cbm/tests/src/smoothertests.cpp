#include <boost/test/unit_test.hpp>

#include "moja/dynamicstruct.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/volumetobiomassconverter.h"

#include <math.h>

namespace cbm = moja::modules::cbm;
using moja::DynamicObject;

extern std::vector<double> aVolumes;
extern std::vector<double> bVolumes;
extern std::vector<double> cVolumes;
extern std::vector<double> swPerdFactors;
extern std::vector<double> hwPerdFactors;

//CBM output
int firstPERDAge = 18;
int standMaxAge = 120;
int sampleSize = 50;
int cbmSWFinalReplaceLength = 26;
int cbmHWFinalReplaceLength = 31;

std::vector<double> swWorkingMerchC{
	0, 0.0922399, 0.101266, 0.119018, 0.136428, 0.153552, 0.170428, 0.187088, 0.21377, 0.240139, 0.266197, 0.291961, 0.317454, 0.356967, 0.39593, 0.434399, 0.472422, 0.51004, 0.558078, 0.605552, 0.652516, 0.699013, 0.745081, 0.777643, 0.810013, 0.842201, 0.874214, 0.90606, 0.922383, 0.938667, 0.954912, 0.97112, 0.987291, 0.989828, 0.992368, 0.99491, 0.997454, 1, 0.999752, 0.999615, 0.999599, 0.9996, 0.999617, 0.998973, 0.998333, 0.997697, 0.997066, 0.996439, 0.996342, 0.996246 };
std::vector<double> swWorkingOtherC{
	0, 0.959469, 0.966657, 0.97802, 0.986372, 0.992505, 0.996929, 1, 0.924615, 0.872135, 0.83211, 0.799558, 0.771808, 0.733526, 0.70024, 0.670067, 0.641937, 0.615204, 0.591243, 0.566775, 0.541866, 0.516576, 0.490949, 0.471559, 0.452108, 0.444812, 0.459133, 0.473257, 0.48075, 0.4882, 0.495604, 0.502965, 0.510283, 0.510977, 0.511674, 0.512372, 0.513072, 0.513773, 0.512651, 0.512061, 0.512053, 0.512054, 0.512062, 0.511733, 0.511405, 0.511079, 0.510756, 0.510434, 0.510385, 0.510336 };
std::vector<double> swWorkingFoliageC{
	0, 0.789023, 0.798032, 0.814694, 0.830013, 0.844331, 0.857858, 0.870735, 0.83264, 0.812836, 0.802744, 0.798289, 0.797304, 0.799276, 0.804291, 0.810881, 0.818236, 0.825881, 0.8426, 0.857922, 0.871945, 0.884756, 0.896434, 0.903342, 0.909785, 0.919882, 0.936543, 0.952343, 0.962266, 0.971989, 0.981517, 0.990853, 1, 0.998292, 0.996596, 0.99491, 0.993236, 0.991571, 0.984473, 0.980762, 0.980746, 0.980747, 0.980764, 0.980132, 0.979504, 0.978881, 0.978261, 0.977646, 0.977551, 0.977457 };
std::vector<double> swWorkingTotalAGBioC{
	0, 0.394315, 0.404093, 0.422458, 0.4396, 0.45581, 0.471282, 0.486151, 0.483501, 0.488224, 0.496832, 0.507676, 0.519882, 0.540364, 0.562021, 0.584275, 0.606809, 0.629444, 0.661679, 0.693245, 0.72421, 0.754629, 0.784551, 0.80526, 0.825775, 0.849894, 0.880547, 0.910966, 0.926721, 0.942422, 0.95807, 0.973665, 0.98921, 0.991362, 0.993517, 0.995676, 0.997837, 1, 0.999125, 0.998661, 0.998645, 0.998646, 0.998664, 0.99802, 0.997381, 0.996746, 0.996115, 0.995488, 0.995392, 0.995296 };
std::vector<double> workingAgeSerials{
	0, 0.283582, 0.298507, 0.313433, 0.328358, 0.343284, 0.358209, 0.373134, 0.38806, 0.402985, 0.41791, 0.432836, 0.447761, 0.462687, 0.477612, 0.492537, 0.507463, 0.522388, 0.537313, 0.552239, 0.567164, 0.58209, 0.597015, 0.61194, 0.626866, 0.641791, 0.656716, 0.671642, 0.686567, 0.701493, 0.716418, 0.731343, 0.746269, 0.761194, 0.776119, 0.791045, 0.80597, 0.820896, 0.835821, 0.850746, 0.865672, 0.880597, 0.895522, 0.910448, 0.925373, 0.940299, 0.955224, 0.970149, 0.985075, 1 };

std::vector<double> hwWorkingMerchC{
	0, 0, 0, 0, 0, 0, 0, 0, 0.0279812, 0.0555827, 0.0828007, 0.109635, 0.13609, 0.177668, 0.218341, 0.258156, 0.297165, 0.335422, 0.369889, 0.406685, 0.443616, 0.480447, 0.517187, 0.54534, 0.573446, 0.601507, 0.629527, 0.657508, 0.662182, 0.666857, 0.671532, 0.676208, 0.680884, 0.695684, 0.710478, 0.725265, 0.740047, 0.754821, 0.786919, 0.818994, 0.851046, 0.883075, 0.915083, 0.931093, 0.947097, 0.963097, 0.979092, 0.995081, 0.997541, 1 };
std::vector<double> hwWorkingOtherC{
	0, 0, 0, 0, 0, 0, 0, 0, 0.182262, 0.288397, 0.352817, 0.391771, 0.413942, 0.427597, 0.422494, 0.405009, 0.378924, 0.353665, 0.372329, 0.407555, 0.444332, 0.481037, 0.517673, 0.545759, 0.573806, 0.601814, 0.629785, 0.65772, 0.662375, 0.667032, 0.67169, 0.67635, 0.68101, 0.695804, 0.710591, 0.725371, 0.740146, 0.754913, 0.787, 0.819063, 0.851103, 0.883121, 0.915115, 0.931119, 0.947118, 0.963111, 0.9791, 0.995083, 0.997541, 1 };
std::vector<double> hwWorkingFoliageC{
	0, 0, 0, 0, 0, 0, 0, 0, 0.10502, 0.1759, 0.227004, 0.265241, 0.294402, 0.328198, 0.35003, 0.363182, 0.369787, 0.372676, 0.373004, 0.406836, 0.443739, 0.480549, 0.517271, 0.545412, 0.573508, 0.60156, 0.629572, 0.657544, 0.662215, 0.666887, 0.671559, 0.676232, 0.680906, 0.695705, 0.710497, 0.725284, 0.740064, 0.754837, 0.786933, 0.819006, 0.851056, 0.883083, 0.915089, 0.931097, 0.947101, 0.9631, 0.979093, 0.995081, 0.997541, 1 };
std::vector<double> hwWorkingTotalAGBioC{
	0, 0, 0, 0, 0, 0, 0, 0, 0.0556136, 0.0973598, 0.131344, 0.160453, 0.186235, 0.222931, 0.255483, 0.285073, 0.312426, 0.339236, 0.370363, 0.406836, 0.443739, 0.480549, 0.517271, 0.545412, 0.573508, 0.60156, 0.629572, 0.657544, 0.662215, 0.666887, 0.671559, 0.676232, 0.680906, 0.695705, 0.710497, 0.725284, 0.740064, 0.754837, 0.786933, 0.819006, 0.851056, 0.883083, 0.915089, 0.931097, 0.947101, 0.963099, 0.979093, 0.995081, 0.997541, 1 };

std::vector<double>  swMerchC_wb2{ 11.4066, 4.15381 };
std::vector<double>  swFoliageC_wb2{ 3.6642, 0.771931 };
std::vector<double>  swTotalAGBioC_wb2{ 4.71944, 2.08535 };
std::vector<double>  hwMerchC_wb2{ 3.66958, 3.56988 };
std::vector<double>  hwFoliageC_wb2{ 3.23307, 3.09438 };
std::vector<double>  hwTotalAGBioC_wb2{ 3.51961, 3.41988 };

std::vector<double>  swCarbonCurveMerchC{
	0, 2.1234e-005, 0.000377965, 0.00203655, 0.00672748, 0.0169968, 0.036242, 0.068738, 0.119655, 0.195065, 0.30194, 0.448139, 0.642372, 0.894153, 1.21372, 1.61196, 2.10023, 2.69024, 3.39384, 4.22277, 5.18837, 6.3013, 7.57112, 9.00598, 10.6121, 12.3936, 14.3517 };
std::vector<double>  swCarbonCurveOtherC{
0, 0, 0, 0, 0, 0.146594, 0.785062, 1.58841, 2.54457, 3.64021, 4.86055, 6.18932, 7.6088, 9.09985, 10.6421, 12.214, 13.7933, 15.3568, 16.8812, 18.3431, 19.7195, 20.988, 22.1276, 23.119, 23.9451, 24.5912, 25.0463};
std::vector<double>  swCarbonCurveFoliageC{
	0, 0.0656408, 0.277946, 0.644904, 1.16847, 1.70067, 1.89214, 2.06341, 2.21785, 2.35798, 2.48577, 2.60279, 2.71032, 2.80944, 2.90103, 2.98586, 3.06459, 3.13779, 3.20597, 3.26955, 3.32894, 3.38448, 3.43648, 3.48522, 3.53094, 3.57388, 3.61423 };

std::vector<double>  hwCarbonCurveMerchC{
	0, 4.73777e-005, 0.000562615, 0.00239235, 0.00668066, 0.0148161, 0.0284009, 0.0492287, 0.0792664, 0.120639, 0.175612, 0.24658, 0.336047, 0.44661, 0.580943, 0.741768, 0.931844, 1.15393, 1.41077, 1.70504, 2.03934, 2.41617, 2.83783, 3.30646, 3.82396, 4.39193, 5.01169, 5.68418, 6.40994, 7.1891, 8.02131, 8.90573};
std::vector<double>  hwCarbonCurveOtherC{
	0, 5.04944e-005, 0.000500814, 0.00189391, 0.0048359, 0.00996432, 0.0179324, 0.0293987, 0.0450196, 0.0654426, 0.0912999, 0.123203, 0.161737, 0.207455, 0.26087, 0.322453, 0.392622, 0.471743, 0.560115, 0.657975, 0.765484, 0.882729, 1.00971, 1.14636, 1.29252, 1.44792, 1.61225, 1.78509, 1.96593, 2.15422, 2.34931, 2.5505};
std::vector<double>  hwCarbonCurveFoliageC{
	0, 7.38124e-006, 6.30403e-005, 0.000221044, 0.000538293, 0.00107345, 0.00188636, 0.00303766, 0.00458835, 0.00659952, 0.00913191, 0.0122456, 0.0159994, 0.0204508, 0.0256551, 0.0316652, 0.038531, 0.0462989, 0.0550112, 0.0647058, 0.0754151, 0.0871663, 0.0999802, 0.113871, 0.128846, 0.144904, 0.162038, 0.180231, 0.199458, 0.219687, 0.240876, 0.262974};


struct SmootherTestFixture {
	cbm::PERDFactor swPf;
	cbm::PERDFactor hwPf;
	std::vector<DynamicObject> mockTableOne;
	std::vector<DynamicObject> mockTableTwo;
	std::shared_ptr<cbm::StandGrowthCurve> standGrowthCurve;

	SmootherTestFixture(){
		swPf.setDefaultValue(swPerdFactors);
		hwPf.setDefaultValue(hwPerdFactors);

		// build softwood table
		for (int i = 0; i < 14; i++) {
			DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", aVolumes[i] } });
			mockTableOne.push_back(row);
		}

		//build hardwood table
		for (int i = 0; i < 25; i++) {
			DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", cVolumes[i] } });
			mockTableTwo.push_back(row);
		}

		standGrowthCurve = std::make_shared<cbm::StandGrowthCurve>(101);

		//based on mockTable, create a softwood yield table
		auto swYieldTable = std::make_shared<cbm::TreeYieldTable>(mockTableOne, cbm::SpeciesType::Softwood);
		auto hwYieldTable = std::make_shared<cbm::TreeYieldTable>(mockTableTwo, cbm::SpeciesType::Hardwood);

		standGrowthCurve->addYieldTable(swYieldTable);
		standGrowthCurve->addYieldTable(hwYieldTable);

		//add softwood PERD factor
		auto swPerdFactor = std::make_unique<cbm::PERDFactor>();
		swPerdFactor->setDefaultValue(swPerdFactors);
		standGrowthCurve->setPERDFactor(std::move(swPerdFactor), cbm::SpeciesType::Softwood);

		//add hardwood PERD factor
		auto hwPerdFactor = std::make_unique<cbm::PERDFactor>();
		hwPerdFactor->setDefaultValue(hwPerdFactors);
		standGrowthCurve->setPERDFactor(std::move(hwPerdFactor), cbm::SpeciesType::Hardwood);

		standGrowthCurve->processStandYieldTables();
	}
};

BOOST_FIXTURE_TEST_SUITE(VolumeToBiomassConverterTests, SmootherTestFixture);

BOOST_AUTO_TEST_CASE(getComponentSmoothingSubstitutionRegionPoint_sw){
	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();

	int swSubstitutionPoint = smoother->getComponentSmoothingSubstitutionRegionPoint(*standGrowthCurve, cbm::SpeciesType::Softwood);
	BOOST_CHECK(swSubstitutionPoint == firstPERDAge);
}

BOOST_AUTO_TEST_CASE(getComponentSmoothingSubstitutionRegionPoint_hw){
	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();

	int hwSubstitutionPoint = smoother->getComponentSmoothingSubstitutionRegionPoint(*standGrowthCurve, cbm::SpeciesType::Hardwood);
	BOOST_CHECK(hwSubstitutionPoint == firstPERDAge);
}

BOOST_AUTO_TEST_CASE(prepareSmoothingInputData_sw){	
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Softwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);

	double TOLERANCE = 0.001;
	std::cout << "SW Working Data:" << std::endl;
	for (int i = 0; i < sampleSize; i++){
		double mojaMerchC = smoother->smoothingMerchC()[i];
		double mojaFoliageC = smoother->smoothingFoliageC()[i];
		double mojaOtherC = smoother->smoothingOtherC()[i];
		double mojaTotalAGBioC = smoother->smoothingTotalAGBioC()[i];
		double mojaAgeSerials = smoother->smoothingAageSerials()[i];

		double cbmWorkingMerchC = swWorkingMerchC[i];
		double cbmWorkingFoliageC = swWorkingFoliageC[i];
		double cbmWorkingOtherC = swWorkingOtherC[i];
		double cbmWorkingTotalAGBioC = swWorkingTotalAGBioC[i];
		double cbmWorkingAgeSerials = workingAgeSerials[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaOtherC << ", " << mojaTotalAGBioC << ", " << mojaAgeSerials << std::endl;
		//std::cout << " CBM:\t" << cbmWorkingMerchC << ", " << cbmWorkingFoliageC << ", " << cbmWorkingOtherC << ", " << cbmWorkingTotalAGBioC << ", " << cbmWorkingAgeSerials << std::endl;

		BOOST_CHECK_CLOSE(mojaMerchC, cbmWorkingMerchC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaFoliageC, cbmWorkingFoliageC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaOtherC, cbmWorkingOtherC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaTotalAGBioC, cbmWorkingTotalAGBioC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaAgeSerials, cbmWorkingAgeSerials, TOLERANCE);
	}
}

BOOST_AUTO_TEST_CASE(prepareSmoothingInputData_hw){
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Hardwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);

	double TOLERANCE = 0.001;

	std::cout << "HW Working Data:" << std::endl;
	for (int i = 0; i < sampleSize; i++){
		double mojaMerchC = smoother->smoothingMerchC()[i];
		double mojaFoliageC = smoother->smoothingFoliageC()[i];
		double mojaOtherC = smoother->smoothingOtherC()[i];
		double mojaTotalAGBioC = smoother->smoothingTotalAGBioC()[i];
		double mojaAgeSerials = smoother->smoothingAageSerials()[i];

		double cbmWorkingMerchC = hwWorkingMerchC[i];
		double cbmWorkingFoliageC = hwWorkingFoliageC[i];
		double cbmWorkingOtherC = hwWorkingOtherC[i];
		double cbmWorkingTotalAGBioC = hwWorkingTotalAGBioC[i];
		double cbmWorkingAgeSerials = workingAgeSerials[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaOtherC << ", " << mojaTotalAGBioC << ", " << mojaAgeSerials << std::endl;
		//std::cout << " CBM:\t" << cbmWorkingMerchC << ", " << cbmWorkingFoliageC << ", " << cbmWorkingOtherC << ", " << cbmWorkingTotalAGBioC << ", " << cbmWorkingAgeSerials << std::endl;

		BOOST_CHECK_CLOSE(mojaMerchC, cbmWorkingMerchC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaFoliageC, cbmWorkingFoliageC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaOtherC, cbmWorkingOtherC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaTotalAGBioC, cbmWorkingTotalAGBioC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaAgeSerials, cbmWorkingAgeSerials, TOLERANCE);
	}
}

BOOST_AUTO_TEST_CASE(minimize_sw){
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Softwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);

	double TOLERANCE = 0.001;

	double merchC_wb2[2] = { 1, 0.1 };		// use any starting value, but not { 0, 0.0 }
	double foliageC_wb2[2] = { 1, 0.1 };	// use any starting value, but not { 0, 0.0 }		
	double totalAGBioC_wb2[2] = { 1, 0.1 };	// use any starting value, but not { 0, 0.0 }

	smoother->minimize(smoother->smoothingMerchC().data(), merchC_wb2);
	smoother->minimize(smoother->smoothingFoliageC().data(), foliageC_wb2);
	smoother->minimize(smoother->smoothingTotalAGBioC().data(), totalAGBioC_wb2);

	std::cout << "SW Weibu Parameter Tests:" << std::endl;
	for (int i = 0; i < 2; i++){
		double mojaMerchC = merchC_wb2[i];
		double mojaFoliageC = foliageC_wb2[i];
		double mojaTotalAGBioC = totalAGBioC_wb2[i];

		double cbmWorkingMerchC = swMerchC_wb2[i];
		double cbmWorkingFoliageC = swFoliageC_wb2[i];
		double cbmWorkingTotalAGBioC = swTotalAGBioC_wb2[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaTotalAGBioC << std::endl;
		//std::cout << " CBM:\t" << cbmWorkingMerchC << ", " << cbmWorkingFoliageC << ", " << cbmWorkingTotalAGBioC << std::endl;

		BOOST_CHECK_CLOSE(mojaMerchC, cbmWorkingMerchC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaFoliageC, cbmWorkingFoliageC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaTotalAGBioC, cbmWorkingTotalAGBioC, TOLERANCE);
	}
}

BOOST_AUTO_TEST_CASE(minimize_hw){
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Hardwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);

	double TOLERANCE = 0.001;

	double merchC_wb2[2] = { 1, 0.1 };		
	double foliageC_wb2[2] = { 1, 0.1 };		
	double totalAGBioC_wb2[2] = { 1, 0.1 };	

	smoother->minimize(smoother->smoothingMerchC().data(), merchC_wb2);
	smoother->minimize(smoother->smoothingFoliageC().data(), foliageC_wb2);
	smoother->minimize(smoother->smoothingTotalAGBioC().data(), totalAGBioC_wb2);

	std::cout << "HW Weibu Parameter Tests:" << std::endl;
	for (int i = 0; i < 2; i++){
		double mojaMerchC = merchC_wb2[i];
		double mojaFoliageC = foliageC_wb2[i];
		double mojaTotalAGBioC = totalAGBioC_wb2[i];

		double cbmWorkingMerchC = hwMerchC_wb2[i];
		double cbmWorkingFoliageC = hwFoliageC_wb2[i];
		double cbmWorkingTotalAGBioC = hwTotalAGBioC_wb2[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaTotalAGBioC << std::endl;
		//std::cout << " CBM:\t" << cbmWorkingMerchC << ", " << cbmWorkingFoliageC << ", " << cbmWorkingTotalAGBioC << std::endl;

		BOOST_CHECK_CLOSE(mojaMerchC, cbmWorkingMerchC, TOLERANCE);
		BOOST_CHECK_CLOSE(mojaFoliageC, cbmWorkingFoliageC, TOLERANCE);		
		BOOST_CHECK_CLOSE(mojaTotalAGBioC, cbmWorkingTotalAGBioC, TOLERANCE);		
	}
}

BOOST_AUTO_TEST_CASE(getFinalFittingRegionAndReplaceData_sw){
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Softwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);	

	double merchC_wb2[2] = { 1, 0.1 };		
	double foliageC_wb2[2] = { 1, 0.1 };		
	double totalAGBioC_wb2[2] = { 1, 0.1 };	

	smoother->minimize(smoother->smoothingMerchC().data(), merchC_wb2);
	smoother->minimize(smoother->smoothingFoliageC().data(), foliageC_wb2);
	smoother->minimize(smoother->smoothingTotalAGBioC().data(), totalAGBioC_wb2);	

	int swSubstitutionPoint = smoother->getComponentSmoothingSubstitutionRegionPoint(*standGrowthCurve, cbm::SpeciesType::Softwood);
	int finalReplaceLength = smoother->getFinalFittingRegionAndReplaceData(*carbonCurve, swSubstitutionPoint, merchC_wb2, foliageC_wb2, totalAGBioC_wb2);

	std::cout << "SW Final Replace Data Tests:" << std::endl;
	BOOST_CHECK(finalReplaceLength == cbmSWFinalReplaceLength);

	for (int i = 0; i <= finalReplaceLength; i++){
		double mojaMerchC = carbonCurve->getMerchCarbonAtAge(i);
		double mojaFoliageC = carbonCurve->getFoliageCarbonAtAge(i);
		double mojaOtherC = carbonCurve->getOtherCarbonAtAge(i);

		double cbmMerchC = swCarbonCurveMerchC[i];
		double cbmFoliageC = swCarbonCurveFoliageC[i];
		double cbmOtherC = swCarbonCurveOtherC[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaOtherC << std::endl;
		//std::cout << " CBM:\t" << cbmMerchC << ", " << cbmFoliageC << ", " << cbmOtherC << std::endl;

		BOOST_CHECK_EQUAL(std::round(mojaMerchC - cbmMerchC), 0);
		BOOST_CHECK_EQUAL(std::round(mojaFoliageC - cbmFoliageC), 0);
		BOOST_CHECK_EQUAL(std::round(mojaOtherC - cbmOtherC), 0);
	}
}

BOOST_AUTO_TEST_CASE(getFinalFittingRegionAndReplaceData_hw){
	auto converter = std::make_unique<cbm::VolumeToBiomassConverter>();
	std::shared_ptr<cbm::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, cbm::SpeciesType::Hardwood);

	std::unique_ptr<cbm::Smoother> smoother = std::make_unique<cbm::Smoother>();
	smoother->clearAndReserveDataSpace(sampleSize);
	smoother->prepareSmoothingInputData(*carbonCurve, firstPERDAge, standMaxAge);	

	double merchC_wb2[2] = { 1, 0.1 };		
	double foliageC_wb2[2] = { 1, 0.1 };	
	double totalAGBioC_wb2[2] = { 1, 0.1 };	

	smoother->minimize(smoother->smoothingMerchC().data(), merchC_wb2);
	smoother->minimize(smoother->smoothingFoliageC().data(), foliageC_wb2);
	smoother->minimize(smoother->smoothingTotalAGBioC().data(), totalAGBioC_wb2);

	int swSubstitutionPoint = smoother->getComponentSmoothingSubstitutionRegionPoint(*standGrowthCurve, cbm::SpeciesType::Hardwood);
	int finalReplaceLength = smoother->getFinalFittingRegionAndReplaceData(*carbonCurve, swSubstitutionPoint, merchC_wb2, foliageC_wb2, totalAGBioC_wb2);

	std::cout << "HW Final Replace Data Tests:" << std::endl;
	BOOST_CHECK(finalReplaceLength == cbmHWFinalReplaceLength);

	for (int i = 0; i <= finalReplaceLength; i++){
		double mojaMerchC = carbonCurve->getMerchCarbonAtAge(i);
		double mojaFoliageC = carbonCurve->getFoliageCarbonAtAge(i);
		double mojaOtherC = carbonCurve->getOtherCarbonAtAge(i);

		double cbmMerchC = hwCarbonCurveMerchC[i];
		double cbmFoliageC = hwCarbonCurveFoliageC[i];
		double cbmOtherC = hwCarbonCurveOtherC[i];

		//std::cout << "MOJA:\t" << mojaMerchC << ", " << mojaFoliageC << ", " << mojaOtherC << std::endl;
		//std::cout << " CBM:\t" << cbmMerchC << ", " << cbmFoliageC << ", " << cbmOtherC << std::endl;

		BOOST_CHECK_EQUAL(std::round(mojaMerchC - cbmMerchC), 0);
		BOOST_CHECK_EQUAL(std::round(mojaFoliageC - cbmFoliageC), 0);
		BOOST_CHECK_EQUAL(std::round(mojaOtherC - cbmOtherC), 0);
	}
}
BOOST_AUTO_TEST_SUITE_END();
