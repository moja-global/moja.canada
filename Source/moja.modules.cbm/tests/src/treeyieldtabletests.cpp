#include <boost/test/unit_test.hpp>

#include "moja/logging.h"
#include "moja/dynamic.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"

namespace cbm = moja::modules::cbm;
using moja::DynamicObject;

// volume
std::vector<double> aVolumes{
    //   0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65
         0,   0,   5,  10,  20,  40,  75, 130, 200, 250, 275, 280, 282, 282
};

// interpolated aVolumes from current CBM
std::vector<double> aVolumeInterplotted{
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   2.0,   3.0,   4.0,
      5.0,   6.0,   7.0,   8.0,   9.0,  10.0,  12.0,  14.0,  16.0,  18.0,
     20.0,  24.0,  28.0,  32.0,  36.0,  40.0,  47.0,  54.0,  61.0,  68.0,
     75.0,  86.0,  97.0, 108.0, 119.0, 130.0, 144.0, 158.0, 172.0, 186.0,
    200.0, 210.0, 220.0, 230.0, 240.0, 250.0, 255.0, 260.0, 265.0, 270.0,
    275.0, 276.0, 277.0, 278.0, 279.0, 280.0, 280.4, 280.8, 281.2, 281.6,
    282.0, 282.0, 282.0, 282.0, 282.0, 282.0 };

// volume
std::vector<double> bVolumes{
    //   0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95
         0,   0,   5,  10,  20,  40,  75, 130, 200, 250, 275, 280, 282, 282,   0,   0,   0,   0,   0,   0
};

// volume
std::vector<double> cVolumes{
    //   0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100, 105, 110, 115, 120
         0,   0,   0,   0,   0,   0,  20,  52,  82, 105, 109, 121, 147, 160, 162, 183, 215, 222, 157, 134, 113,  90,  68,  44,  22
};

// interpolated cVolumes output from current CBM
std::vector<double> cVolumeInterplotted{
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   4.0,   8.0,  12.0,  16.0,
     20.0,  26.4,  32.8,  39.2,  45.6,  52.0,  58.0,  64.0,  70.0,  76.0,
     82.0,  86.6,  91.2,  95.8, 100.4, 105.0, 105.8, 106.6, 107.4, 108.2,
    109.0, 111.4, 113.8, 116.2, 118.6, 121.0, 126.2, 131.4, 136.6, 141.8,
    147.0, 149.6, 152.2, 154.8, 157.4, 160.0, 160.4, 160.8, 161.2, 161.6,
    162.0, 166.2, 170.4, 174.6, 178.8, 183.0, 189.4, 195.8, 202.2, 208.6,
    215.0, 216.4, 217.8, 219.2, 220.6, 222.0, 209.0, 196.0, 183.0, 170.0,
    157.0, 152.4, 147.8, 143.2, 138.6, 134.0, 129.8, 125.6, 121.4, 117.2,
    113.0, 108.4, 103.8,  99.2,  94.6,  90.0,  85.6,  81.2,  76.8,  72.4,
     68.0,  63.2,  58.4,  53.6,  48.8,  44.0,  39.6,  35.2,  30.8,  26.4,
     22.0 };

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

BOOST_AUTO_TEST_CASE(TreeYieldTableConstructor) {	
    // Based on mockTable, create a softwood yield table
    cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);
    
    // The age interval is 5, maximum age is 95.
    auto yieldTableMaxAge = yieldTable.maxAge();	
    BOOST_CHECK_EQUAL(yieldTableMaxAge, 65);

    // The underlying volume vector is size of 96.
    auto yieldTableSize = yieldTable.yieldsAtEachAge().size();
    BOOST_CHECK_EQUAL(yieldTableSize, 66);	
}

BOOST_AUTO_TEST_CASE(TreeYieldTableProperty) {
    // Based on mockTable, create a softwood yield table.
    cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);	

    // Check the indexer[], from age 60, volume should be 282.00 until the age 95.
    for (int age = 60; age <= 65; age++) {
        auto volume = yieldTable[age];
        BOOST_CHECK_EQUAL(volume, 282.00);
    }

    // Check the total volume of the yield table.
    double totalVolume = 0.0;
    for (int age = 0; age <= yieldTable.maxAge(); age++) {
        MOJA_LOG_DEBUG << age << "," << yieldTable[age];
        totalVolume += yieldTable[age];
    }
    
    MOJA_LOG_DEBUG << "Total Volume: " << totalVolume;
    BOOST_CHECK_EQUAL(yieldTable.totalVolume(), totalVolume);
}

BOOST_AUTO_TEST_CASE(InterpolateVolumeAtEachAge) {
    // Based on mockTable, create a softwood yield table.
    cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);	
    
    // Check the interpolation algorithm.
    for (int i = 0; i <= 65; i++) {
        MOJA_LOG_DEBUG << i << "," << yieldTable[i];
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

BOOST_AUTO_TEST_CASE(YieldTableAppendingLastVolumeData) {
    // Based on mockTable, create a softwood yield table.
    cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);

    // ZERO volumes at the end of the yield curve should be replaced by the last
    // non-zero value. After interploation, all of the value should be identical
    // as the last non-zero value.
    double lastValue = 282.00;
    for (int i = 65; i <= 95; i++){
        MOJA_LOG_DEBUG << i << "," << yieldTable[i];
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

BOOST_AUTO_TEST_CASE(YieldTableInterpolateLossCurve) {
    // Based on mockTable, create a softwood yield table.
    cbm::TreeYieldTable yieldTable(mockTable, cbm::SpeciesType::Softwood);
    
    // Maximum age is 120.
    BOOST_CHECK_EQUAL(yieldTable.maxAge(), 120);

    // Check the interpolation algorithm.
    for (int i = 0; i <= 120; i++) {
        MOJA_LOG_DEBUG << i << "," << yieldTable[i];
        BOOST_CHECK_EQUAL(yieldTable[i], cVolumeInterplotted[i]);
    }
}

BOOST_AUTO_TEST_SUITE_END();