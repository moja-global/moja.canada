#include <boost/test/unit_test.hpp>

#include "moja/dynamicstruct.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"

namespace cbm = moja::modules::cbm;
using moja::DynamicObject;

// volume					  0, 5, 10, 15, 20, 25, 30, 35,  40,  45,  50,  55,  60,  65
std::vector<double> aVolumes{ 0, 0, 5, 10, 20, 40, 75, 130, 200, 250, 275, 280, 282, 282 };

//interplotted aVolumes from Current CBM
std::vector<double> aVolumeInterplotted{ 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 24, 28, 32, 36, 40, 47, 54, 61, 68, 75, 86, 97, 108, 119, 130, 144, 158, 172, 186, 200, 210, 220, 230, 240, 250, 255, 260, 265, 270, 275, 276, 277, 278, 279, 280, 280.4, 280.8, 281.2, 281.6, 282, 282, 282, 282, 282, 282 };

// volume					  0, 5, 10, 15, 20, 25, 30, 35,  40,  45,  50,  55,  60,  65, 70, 75, 80, 85, 90, 95
std::vector<double> bVolumes{ 0, 0, 5, 10, 20, 40, 75, 130, 200, 250, 275, 280, 282, 282, 0, 0, 0, 0, 0, 0 };

// volume					  0, 5, 10, 15, 20, 25, 30, 35, 40, 45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  100, 105, 110, 115, 120
std::vector<double> cVolumes{ 0, 0, 0, 0, 0, 0, 20, 52, 82, 105, 109, 121, 147, 160, 162, 183, 215, 222, 157, 134, 113, 90, 68, 44, 22 };

//interplotted cVolumes output from Current CBM
std::vector<double> cVolumeInterplotted{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8, 12, 16, 20, 26.4, 32.8, 39.2, 45.6, 52, 58, 64, 70, 76, 82, 86.6, 91.2, 95.8, 100.4, 105, 105.8, 106.6, 107.4, 108.2, 109, 111.4, 113.8, 116.2, 118.6, 121, 126.2, 131.4, 136.6, 141.8, 147, 149.6, 152.2, 154.8, 157.4, 160, 160.4, 160.8, 161.2, 161.6, 162, 166.2, 170.4, 174.6, 178.8, 183, 189.4, 195.8, 202.2, 208.6, 215, 216.4, 217.8, 219.2, 220.6, 222, 209, 196, 183, 170, 157, 152.4, 147.8, 143.2, 138.6, 134, 129.8, 125.6, 121.4, 117.2, 113, 108.4, 103.8, 99.2, 94.6, 90, 85.6, 81.2, 76.8, 72.4, 68, 63.2, 58.4, 53.6, 48.8, 44, 39.6, 35.2, 30.8, 26.4, 22 };

struct TreeYieldTableTest1Fixture {
	std::vector<DynamicObject> mockTable;		

	TreeYieldTableTest1Fixture() {
		for (int i = 0; i < 14; i++) {
			DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", aVolumes[i] } });
			mockTable.push_back(row);
		}
	}	
};

BOOST_FIXTURE_TEST_SUITE(TreeYieldTableTests, TreeYieldTableTest1Fixture);

BOOST_AUTO_TEST_CASE(TreeYieldTable_constructor)
{	
	//based on mockTable, create a softwood yield table
	cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);
	
	//the age interval is 5, maximum age is 95
	auto yieldTableMaxAge = yieldTable.maxAge();	
	BOOST_CHECK_EQUAL(yieldTableMaxAge, 65);

	//the underlying volume vector is size of 96
	auto yieldTableSize = yieldTable.yieldsAtEachAge().size();
	BOOST_CHECK_EQUAL(yieldTableSize, 66);	
}

BOOST_AUTO_TEST_CASE(TreeYieldTable_property)
{
	//based on mockTable, create a softwood yield table
	cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);	

	//check the indexer[], from age 60, volume should be 282.00 until the age 95, 
	for (int age = 60; age <= 65; age++){
		auto volume = yieldTable[age];
		BOOST_CHECK_EQUAL(volume, 282.00);
	}

	//check the total volume of the yield table
	double totalVolume = 0.0;
	for (int age = 0; age <= 65; age+=5){
		//std::cout << age << "," << yieldTable[age] << std::endl;
		totalVolume += yieldTable[age];
	}
	std::cout << "Total Volume: " << totalVolume << std::endl;
	//BOOST_CHECK_EQUAL(yieldTable.totalVolume(), totalVolume);
}

BOOST_AUTO_TEST_CASE(InterpolateVolumeAtEachAge)
{
	//based on mockTable, create a softwood yield table
	cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);	
	
	//Check the inperplot algorithem
	for (int i = 0; i <= 65; i++){
		//std::cout << i << "," << yieldTable[i] << std::endl;
		BOOST_CHECK_EQUAL(yieldTable[i], aVolumeInterplotted[i]);
	}		
}
BOOST_AUTO_TEST_SUITE_END();

struct TreeYieldTableTest2Fixture {
	std::vector<DynamicObject> mockTable;

	TreeYieldTableTest2Fixture() {
		for (int i = 0; i < 20; i++) {
			DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", bVolumes[i] } });
			mockTable.push_back(row);
		}
	}
};

BOOST_FIXTURE_TEST_SUITE(TreeYieldTableTest2, TreeYieldTableTest2Fixture);

BOOST_AUTO_TEST_CASE(YieldTable_AppendingLastVolumeData)
{
	//based on mockTable, create a softwood yield table
	cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);

	//ZERO volumes at the end of the yield curve should be replaced by the last non-zero value.
	//after interploation, all of the value should be identical as the last non-zero value.
	double lastValue = 282.00;
	for (int i = 65; i <= 95; i++){
		//std::cout << i << "," << yieldTable[i] << std::endl;
		BOOST_CHECK_EQUAL(yieldTable[i], lastValue);
	}
}

BOOST_AUTO_TEST_SUITE_END();

struct TreeYieldTableTest3Fixture {
	std::vector<DynamicObject> mockTable;

	TreeYieldTableTest3Fixture() {
		for (int i = 0; i < 25; i++) {
			DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", cVolumes[i] } });
			mockTable.push_back(row);
		}
	}
};

BOOST_FIXTURE_TEST_SUITE(TreeYieldTableTest3, TreeYieldTableTest3Fixture);

BOOST_AUTO_TEST_CASE(YieldTable_Interpolate_lossCurve)
{
	//based on mockTable, create a softwood yield table
	cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);
	
	//maximum age is 120
	BOOST_CHECK_EQUAL(yieldTable.maxAge(), 120);

	//Check the inperplot algorithem
	for (int i = 0; i <= 120; i++){
		//std::cout << i << "," << yieldTable[i] << std::endl;
		BOOST_CHECK_EQUAL(yieldTable[i], cVolumeInterplotted[i]);
	}
}

BOOST_AUTO_TEST_SUITE_END();