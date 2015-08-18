#include <boost/test/unit_test.hpp>

#include "moja/dynamicstruct.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/volumetobiomassconverter.h"

namespace CBM = moja::modules::CBM;
using moja::DynamicObject;

extern std::vector<double> aVolumes;
extern std::vector<double> bVolumes;
extern std::vector<double> cVolumes;
extern std::vector<double> swPerdFactors;
extern std::vector<double> hwPerdFactors;


//following data are from CBM output
std::vector<double> swMerchC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5.28704, 5.95143, 6.60628, 7.25275, 8.52411, 9.77106, 10.9975, 12.2062, 13.3993, 15.3103, 17.1989, 19.0652, 20.9105, 22.7362, 25.5662, 28.3568, 31.1119, 33.8352, 36.5294, 39.9699, 43.37, 46.7336, 50.0638, 53.3632, 55.6953, 58.0136, 60.3189, 62.6117, 64.8926, 66.0616, 67.2279, 68.3914, 69.5522, 70.7104, 70.8921, 71.074, 71.2561, 71.4383, 71.6206, 71.6028, 71.593, 71.5919, 71.5919, 71.5932, 71.5471, 71.5012, 71.4557, 71.4105, 71.3655, 71.3587, 71.3518, 71.3449, 71.338, 71.3312, 71.2596, 71.1887, 71.1185, 71.0491, 70.9803, 70.8769, 70.775, 70.6746, 70.5757, 70.4781, 70.457, 70.4359, 70.4149, 70.3939, 70.373, 70.5695, 70.7718, 70.9803, 71.1954, 71.4174, 71.4977, 71.579, 71.6612, 71.7444, 71.8286, 71.9064, 71.99, 72.0802, 72.1714, 72.2637, 72.3659, 72.4694, 72.5743, 72.6805, 72.7882, 72.8925, 72.9982, 73.1052, 73.2137, 73.3237, 73.4453, 73.5688, 73.6942, 73.8215, 73.9508, 74.0712, 74.1934, 74.3174, 74.4433, 74.5712 };

std::vector<double> swFoliageC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3.25316, 3.39912, 3.44103, 3.48031, 3.55298, 3.61979, 3.68223, 3.74122, 3.79738, 3.63124, 3.54488, 3.50086, 3.48144, 3.47714, 3.48574, 3.50761, 3.53635, 3.56843, 3.60177, 3.67468, 3.7415, 3.80266, 3.85853, 3.90946, 3.93959, 3.96768, 4.01172, 4.08438, 4.15329, 4.19656, 4.23896, 4.28052, 4.32123, 4.36112, 4.35368, 4.34628, 4.33893, 4.33162, 4.32436, 4.29341, 4.27722, 4.27716, 4.27716, 4.27723, 4.27448, 4.27174, 4.26902, 4.26632, 4.26363, 4.26322, 4.26281, 4.2624, 4.26199, 4.26158, 4.2573, 4.25307, 4.24888, 4.24473, 4.24062, 4.23444, 4.22835, 4.22235, 4.21644, 4.21062, 4.20935, 4.20809, 4.20684, 4.20559, 4.20434, 4.21608, 4.22816, 4.24062, 4.25347, 4.26673, 4.27153, 4.27638, 4.2813, 4.28627, 4.2913, 4.29595, 4.3113, 4.33918, 4.36737, 4.39589, 4.42749, 4.4595, 4.49192, 4.52476, 4.55803, 4.59026, 4.62292, 4.65599, 4.6895, 4.72346, 4.76102, 4.79915, 4.83785, 4.87713, 4.91704, 4.95416, 5.0064, 5.06441, 5.1238, 5.18462 };

std::vector<double> swOtherC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23.834, 25.0017, 25.2251, 25.4141, 25.7128, 25.9324, 26.0936, 26.21, 26.2907, 24.3088, 22.929, 21.8768, 21.0209, 20.2914, 19.2849, 18.4098, 17.6165, 16.877, 16.1741, 15.5442, 14.9009, 14.246, 13.5811, 12.9074, 12.3976, 11.8862, 11.6944, 12.0709, 12.4422, 12.6393, 12.8351, 13.0298, 13.2233, 13.4157, 13.4339, 13.4523, 13.4706, 13.489, 13.5075, 13.478, 13.4624, 13.4622, 13.4622, 13.4625, 13.4538, 13.4452, 13.4366, 13.4281, 13.4197, 13.4184, 13.4171, 13.4158, 13.4145, 13.4132, 13.3997, 13.3864, 13.3732, 13.3602, 13.3472, 13.3278, 13.3086, 13.2897, 13.2711, 13.2528, 13.2488, 13.2449, 13.2409, 13.237, 13.233, 13.27, 13.308, 13.3472, 13.3877, 13.4294, 13.4445, 13.4598, 13.4753, 13.4909, 13.5068, 13.5214, 13.5462, 13.5828, 13.6199, 13.6573, 13.6987, 13.7406, 13.7831, 13.826, 13.8694, 13.9115, 13.9541, 13.9972, 14.0408, 14.085, 14.1338, 14.1833, 14.2336, 14.2845, 14.3362, 14.3843, 14.6916, 15.0889, 15.4969, 15.9163 };

std::vector<double> hwMerchC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.19312, 2.37005, 3.53063, 4.67485, 5.80288, 7.57577, 9.31005, 11.0078, 12.6711, 14.3024, 15.7721, 17.3411, 18.9158, 20.4863, 22.0529, 23.2533, 24.4517, 25.6483, 26.8431, 28.0361, 28.2355, 28.4348, 28.6341, 28.8335, 29.0329, 29.664, 30.2948, 30.9253, 31.5556, 32.1856, 33.5542, 34.9219, 36.2886, 37.6544, 39.0192, 39.7018, 40.3843, 41.0665, 41.7485, 42.4303, 42.5352, 42.64, 42.7449, 42.8497, 42.9546, 44.0553, 45.1554, 46.2551, 47.3542, 48.4528, 50.1259, 51.7979, 53.4688, 55.1385, 56.8072, 57.1721, 57.537, 57.9017, 58.2665, 58.6312, 55.2429, 51.8502, 48.4528, 45.0507, 41.6436, 40.4367, 39.2292, 38.0211, 36.8122, 35.6027, 34.4977, 33.3922, 32.286, 31.1792, 30.0718, 28.8582, 27.6438, 26.4286, 25.2127, 23.9959, 22.8312, 21.6658, 20.4996, 19.3326, 18.1648, 16.8898, 15.6139, 14.3369, 13.0589, 11.7798, 10.6063, 9.43188, 8.2565, 7.08013, 5.90276 };

std::vector<double> hwFoliageC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.10724, 0.179619, 0.231803, 0.270849, 0.300626, 0.335136, 0.35743, 0.37086, 0.377605, 0.380554, 0.380889, 0.415436, 0.45312, 0.490708, 0.528206, 0.556942, 0.585632, 0.614278, 0.642881, 0.671445, 0.676215, 0.680985, 0.685756, 0.690528, 0.6953, 0.710412, 0.725518, 0.740617, 0.755709, 0.770795, 0.803569, 0.83632, 0.869047, 0.901752, 0.934434, 0.950781, 0.967123, 0.98346, 0.999792, 1.01612, 1.01863, 1.02114, 1.02365, 1.02616, 1.02867, 1.05503, 1.08138, 1.10771, 1.13403, 1.16033, 1.2004, 1.24044, 1.28045, 1.32043, 1.36039, 1.36913, 1.37786, 1.3866, 1.39533, 1.40407, 1.32293, 1.24169, 1.16033, 1.07887, 0.997279, 0.96838, 0.939465, 0.910534, 0.881586, 0.852622, 0.826162, 0.799687, 0.773198, 0.746694, 0.720175, 0.691113, 0.662032, 0.632933, 0.603814, 0.574676, 0.546786, 0.518877, 0.490949, 0.463002, 0.435035, 0.404503, 0.373947, 0.343366, 0.312759, 0.282126, 0.254023, 0.225897, 0.197747, 0.169574, 0.141376 };

std::vector<double> hwOtherC{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.62303, 2.56816, 3.14182, 3.4887, 3.68613, 3.80773, 3.76229, 3.60659, 3.3743, 3.14937, 3.31558, 3.62926, 3.95675, 4.28362, 4.60985, 4.85996, 5.10972, 5.35912, 5.6082, 5.85696, 5.89842, 5.93989, 5.98137, 6.02286, 6.06436, 6.1961, 6.32778, 6.4594, 6.59096, 6.72246, 7.00819, 7.29371, 7.57903, 7.86415, 8.14906, 8.29157, 8.43404, 8.57646, 8.71884, 8.86117, 8.88305, 8.90495, 8.92685, 8.94873, 8.97062, 9.20039, 9.43006, 9.65961, 9.88905, 10.1184, 10.4677, 10.8167, 11.1655, 11.514, 11.8624, 11.9385, 12.0147, 12.0908, 12.167, 12.2431, 11.5358, 10.8276, 10.1184, 9.40819, 8.69693, 8.44499, 8.19291, 7.94069, 7.68833, 7.43581, 7.20513, 6.97431, 6.74337, 6.5123, 6.2811, 6.02772, 5.77417, 5.52045, 5.26656, 5.0125, 4.76932, 4.52596, 4.28244, 4.03874, 3.79487, 3.52861, 3.26214, 2.99544, 2.72851, 2.46134, 2.21622, 1.97089, 1.72535, 1.47958, 1.23359 };

struct V2BConverterFixture {
	CBM::PERDFactor swPf;
	CBM::PERDFactor hwPf;
	std::vector<DynamicObject> mockTableOne;
	std::vector<DynamicObject> mockTableTwo;
	std::shared_ptr<CBM::StandGrowthCurve> standGrowthCurve;

	V2BConverterFixture(){
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

		standGrowthCurve = std::make_shared<CBM::StandGrowthCurve>(101);

		//based on mockTable, create a softwood yield table
		auto swYieldTable = std::make_shared<CBM::TreeYieldTable>(mockTableOne, CBM::SpeciesType::Softwood);
		auto hwYieldTable = std::make_shared<CBM::TreeYieldTable>(mockTableTwo, CBM::SpeciesType::Hardwood);

		standGrowthCurve->addYieldTable(swYieldTable);
		standGrowthCurve->addYieldTable(hwYieldTable);

		//add softwood PERD factor
		auto swPerdFactor = std::make_unique<CBM::PERDFactor>();
		swPerdFactor->setDefaultValue(swPerdFactors);
		standGrowthCurve->setPERDFactor(std::move(swPerdFactor), CBM::SpeciesType::Softwood);

		//add hardwood PERD factor
		auto hwPerdFactor = std::make_unique<CBM::PERDFactor>();
		hwPerdFactor->setDefaultValue(hwPerdFactors);
		standGrowthCurve->setPERDFactor(std::move(hwPerdFactor), CBM::SpeciesType::Hardwood);

		standGrowthCurve->processStandYieldTables();
	}
};

BOOST_FIXTURE_TEST_SUITE(VolumeToBiomassConverterTests, V2BConverterFixture);

BOOST_AUTO_TEST_CASE(generateComponentBiomassCarbonCurve_sw){
	//check softwood component carbon at each age

	auto converter = std::make_unique<CBM::VolumeToBiomassConverter>();
	std::shared_ptr<CBM::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, CBM::SpeciesType::Softwood);
	
	double mojaValue = 0;
	double cbmOutput = 0;
	double TOLERANCE = 0.01;

	//std::cout << "SW Merchantable Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = swMerchC[i];
		mojaValue = carbonCurve->getMerchCarbonAtAge(i);

		//std::cout <<"Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}

	//check Foliage carbon serials
	//std::cout << "SW Foliage Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = swFoliageC[i];
		mojaValue = carbonCurve->getFoliageCarbonAtAge(i);

		//std::cout << "Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}

	//check Other carbon serals
	//std::cout << "SW Other Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = swOtherC[i];
		mojaValue = carbonCurve->getOtherCarbonAtAge(i);

		//std::cout << "Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}	
}

BOOST_AUTO_TEST_CASE(generateComponentBiomassCarbonCurve_hw){
	auto converter = std::make_unique<CBM::VolumeToBiomassConverter>();
	std::shared_ptr<CBM::ComponentBiomassCarbonCurve> carbonCurve = converter->generateComponentBiomassCarbonCurve(standGrowthCurve, CBM::SpeciesType::Hardwood);

	//now check the component carbon at each age slot
	//check Merchantable carbon serials
	double mojaValue = 0;
	double cbmOutput = 0;
	double TOLERANCE = 0.001;

	//std::cout << "HW Merchantable Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = hwMerchC[i];
		mojaValue = carbonCurve->getMerchCarbonAtAge(i);

		//std::cout << "Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}

	//check Foliage carbon serials
	//std::cout << "HW Foliage Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = hwFoliageC[i];
		mojaValue = carbonCurve->getFoliageCarbonAtAge(i);

		//std::cout << "Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}

	//check Other carbon serals
	//std::cout << "HW Other Carbon" << std::endl;
	for (int i = 0; i <= 120; i++){
		cbmOutput = hwOtherC[i];
		mojaValue = carbonCurve->getOtherCarbonAtAge(i);

		//std::cout << "Age: \t" << i << "\tCBM:\t" << cbmOutput << " MOJA:\t" << mojaValue << " Diff:\t" << cbmOutput - mojaValue << std::endl;
		BOOST_CHECK_CLOSE(cbmOutput, mojaValue, TOLERANCE);
	}
}

BOOST_AUTO_TEST_SUITE_END();