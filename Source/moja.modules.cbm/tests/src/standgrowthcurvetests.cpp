#include <boost/test/unit_test.hpp>

#include "moja/logging.h"
#include "moja/dynamic.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/perdfactor.h"
#include "moja/types.h"

namespace cbm = moja::modules::cbm;
using moja::DynamicObject;

extern std::vector<double> aVolumes;
extern std::vector<double> bVolumes;
extern std::vector<double> cVolumes;
extern std::vector<double> swPerdFactors;
extern std::vector<double> hwPerdFactors;

extern std::vector<double> aVolumeInterplotted;
extern std::vector<double> cVolumeInterplotted;

// CBM output for total merchantable volume of a stand growth curve.
double cbmStandTotalVolume[121] = {
      0  ,   0  ,   0  ,   0  ,   0  ,   0  ,   1  ,   2  ,   3  ,   4  ,
      5  ,   6  ,   7  ,   8  ,   9  ,  10  ,  12  ,  14  ,  16  ,  18  ,
     20  ,  24  ,  28  ,  32  ,  36  ,  40  ,  51  ,  62  ,  73  ,  84  ,
     95  , 112.4, 129.8, 147.2, 164.6, 182  , 202  , 222  , 242  , 262  ,
    282  , 296.6, 311.2, 325.8, 340.4, 355  , 360.8, 366.6, 372.4, 378.2,
    384  , 387.4, 390.8, 394.2, 397.6, 401  , 406.6, 412.2, 417.8, 423.4,
    429  , 431.6, 434.2, 436.8, 439.4, 442  , 442.4, 442.8, 443.2, 443.6,
    444  , 448.2, 452.4, 456.6, 460.8, 465  , 471.4, 477.8, 484.2, 490.6,
    497  , 498.4, 499.8, 501.2, 502.6, 504  , 491  , 478  , 465  , 452  ,
    439  , 434.4, 429.8, 425.2, 420.6, 416  , 411.8, 407.6, 403.4, 399.2,
    395  , 390.4, 385.8, 381.2, 376.6, 372  , 367.6, 363.2, 358.8, 354.4,
    350  , 345.2, 340.4, 335.6, 330.8, 326  , 321.6, 317.2, 312.8, 308.4,
    304
};

// CBM output of softwood ratio.
double swRatio[121] = {
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 0.921569, 0.870968, 0.835616, 0.809524,
    0.789474, 0.765125, 0.747304, 0.733696, 0.722965, 0.714286, 0.712871, 0.711712, 0.710744, 0.709924,
    0.709220, 0.708024, 0.706941, 0.705955, 0.705053, 0.704225, 0.706763, 0.709220, 0.711600, 0.713908,
    0.716146, 0.712442, 0.708802, 0.705226, 0.701710, 0.698254, 0.689621, 0.681223, 0.673049, 0.665092,
    0.657343, 0.653383, 0.649470, 0.645604, 0.641784, 0.638009, 0.637432, 0.636856, 0.636282, 0.635708,
    0.635135, 0.629183, 0.623342, 0.617608, 0.611979, 0.606452, 0.598218, 0.590205, 0.582404, 0.574806,
    0.567404, 0.565811, 0.564226, 0.562650, 0.561082, 0.559524, 0.574338, 0.589958, 0.606452, 0.623894,
    0.642369, 0.649171, 0.656119, 0.663217, 0.670471, 0.677885, 0.684798, 0.691855, 0.699058, 0.706413,
    0.713924, 0.722336, 0.730949, 0.739769, 0.748805, 0.758065, 0.767138, 0.776432, 0.785953, 0.795711,
    0.805714, 0.816918, 0.828437, 0.840286, 0.852479, 0.865031, 0.876866, 0.889029, 0.901535, 0.914397,
    0.927632
};

struct StandGrowthCurveFixture {
    std::vector<DynamicObject> mockSWTable;
    std::vector<DynamicObject> mockHWTable;

    StandGrowthCurveFixture() {
        // Build softwood table.
        for (int i = 0; i < 14; i++) {
            DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", aVolumes[i] } });
            mockSWTable.push_back(row);
        }

        // Build hardwood table.
        for (int i = 0; i < 25; i++) {
            DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", cVolumes[i] } });
            mockHWTable.push_back(row);
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(StandGrowthCurveTests, StandGrowthCurveFixture);

BOOST_AUTO_TEST_CASE(StandGrowthCurveConstructorDefault) {
    // Create a stand growth curve with the growth curve ID as 101.
    cbm::StandGrowthCurve testCurve(101, 1);

    // Check the id, default age.
    auto gcId = testCurve.standGrowthCurveID();
    BOOST_CHECK_EQUAL(gcId, 101);

    // No yield tabled added, max stand age is 0, there are no softwood and hardwood components.
    BOOST_CHECK_EQUAL(testCurve.standMaxAge(), 0);
    BOOST_CHECK(!testCurve.hasYieldComponent(cbm::SpeciesType::Softwood));
    BOOST_CHECK(!testCurve.hasYieldComponent(cbm::SpeciesType::Hardwood));
}

// Test stand growth curve with softwood and hardwood components.
BOOST_AUTO_TEST_CASE(StandGrowthCurveConstructorSWHW) {
    // Create a stand growth curve with the growth curve ID as 101.
    cbm::StandGrowthCurve testCurve(101, 1);

    // Add softwood yield table.
    cbm::TreeYieldTable swTreeYieldTable(mockSWTable, cbm::SpeciesType::Softwood);
    testCurve.addYieldTable(swTreeYieldTable);

    // Add softwood PERD factor.
    auto swPerdFactor = std::make_unique<cbm::PERDFactor>();
    swPerdFactor->setDefaultValue(swPerdFactors);
    testCurve.setPERDFactor(std::move(swPerdFactor), cbm::SpeciesType::Softwood);

    // With softwood yield table and PERD factor, which means having softwood components.
    BOOST_CHECK(testCurve.hasYieldComponent(cbm::SpeciesType::Softwood));

    // Add hardwood PERD factor.
    auto hwPerdFactor = std::make_unique<cbm::PERDFactor>();
    hwPerdFactor->setDefaultValue(hwPerdFactors);	
    testCurve.setPERDFactor(std::move(hwPerdFactor), cbm::SpeciesType::Hardwood);

    // No hardwood yield table, even there is hardwood PERD factor, there is no hardwood component.
    BOOST_CHECK(!testCurve.hasYieldComponent(cbm::SpeciesType::Hardwood));

    // Add hardwood yield table now.
    cbm::TreeYieldTable hwTreeYieldTable(mockHWTable, cbm::SpeciesType::Hardwood);
    testCurve.addYieldTable(hwTreeYieldTable);

    // With hardwood yield table and hardwood PERD factor. There is hardwood component.
    BOOST_CHECK(testCurve.hasYieldComponent(cbm::SpeciesType::Hardwood));
}

// Test add two yield tables to a stand component (SW).
BOOST_AUTO_TEST_CASE(AddYieldTableSW) {
    // Create a stand growth curve with the growth curve ID as 101.
    cbm::StandGrowthCurve testCurve(102, 1);

    // Check the id, default age.
    auto gcId = testCurve.standGrowthCurveID();	

    // Add softwood yield table one.
    cbm::TreeYieldTable swTreeYieldTable(mockSWTable, cbm::SpeciesType::Softwood);
    testCurve.addYieldTable(swTreeYieldTable);

    // Add softwood PERD factor.
    auto swPerdFactor = std::make_unique<cbm::PERDFactor>();
    swPerdFactor->setDefaultValue(swPerdFactors);
    testCurve.setPERDFactor(std::move(swPerdFactor), cbm::SpeciesType::Softwood);

    // With softwood yield table and PERD factor, which means having softwood components.
    BOOST_CHECK(testCurve.hasYieldComponent(cbm::SpeciesType::Softwood));
    BOOST_CHECK(!testCurve.hasYieldComponent(cbm::SpeciesType::Hardwood));	
}

// Test one component (HW) only stand growth curve.
BOOST_AUTO_TEST_CASE(AddYieldTableHW) {
    // Create a stand growth curve with the growth curve ID as 101.
    cbm::StandGrowthCurve testCurve(103, 1);

    // Check the id, default age.
    auto gcId = testCurve.standGrowthCurveID();
    BOOST_CHECK_EQUAL(gcId, 103);

    // Add hardwood yield table - mock table two.
    cbm::TreeYieldTable treeYieldTable(mockHWTable, cbm::SpeciesType::Hardwood);
    testCurve.addYieldTable(treeYieldTable);

    // Add hardwood PERD factor.
    auto perdFactor = std::make_unique<cbm::PERDFactor>();
    perdFactor->setDefaultValue(swPerdFactors);
    testCurve.setPERDFactor(std::move(perdFactor), cbm::SpeciesType::Hardwood);

    // With hardwood yield table and PERD factor, which means having hardwood components.
    BOOST_CHECK(testCurve.hasYieldComponent(cbm::SpeciesType::Hardwood));

    // There is no softwood component.
    BOOST_CHECK(!testCurve.hasYieldComponent(cbm::SpeciesType::Softwood));
}

BOOST_AUTO_TEST_SUITE_END();

struct TestStandGrowthCurveFixture {
    std::vector<DynamicObject> mockSWTable;
    std::vector<DynamicObject> mockHWTable;

    // Create a stand growth curve with the growth curve ID as 101.
    cbm::StandGrowthCurve testCurve{ 101, 1 };

    TestStandGrowthCurveFixture() {
        // Build softwood table.
        for (int i = 0; i < 14; i++) {
            DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", aVolumes[i] } });
            mockSWTable.push_back(row);
        }

        // Build hardwood table.
        for (int i = 0; i < 25; i++) {
            DynamicObject row({ { "age", i * 5 }, { "merchantable_volume", cVolumes[i] } });
            mockHWTable.push_back(row);
        }

        // Add softwood yield table.
        cbm::TreeYieldTable swTreeYieldTable(mockSWTable, cbm::SpeciesType::Softwood);
        testCurve.addYieldTable(swTreeYieldTable);

        // Add softwood PERD factor.
        auto swPerdFactor = std::make_unique<cbm::PERDFactor>();
        swPerdFactor->setDefaultValue(swPerdFactors);
        testCurve.setPERDFactor(std::move(swPerdFactor), cbm::SpeciesType::Softwood);

        // Add hardwood PERD factor.
        auto hwPerdFactor = std::make_unique<cbm::PERDFactor>();
        hwPerdFactor->setDefaultValue(hwPerdFactors);
        testCurve.setPERDFactor(std::move(hwPerdFactor), cbm::SpeciesType::Hardwood);

        // Add hardwood yield table now.
        cbm::TreeYieldTable hwTreeYieldTable(mockHWTable, cbm::SpeciesType::Hardwood);
        testCurve.addYieldTable(hwTreeYieldTable);

        // Process the yield tables.
        testCurve.processStandYieldTables();
    }
};

BOOST_FIXTURE_TEST_SUITE(StandGrowthCurveFunctionTests, TestStandGrowthCurveFixture);

BOOST_AUTO_TEST_CASE(StandMaxAge) {
    auto standMaxAge = testCurve.standMaxAge();

    // From the softwood components, the maximum age is 65.
    // From the hardwood component, the maximum age is 120.
    // The combined stand max age should be 120.
    BOOST_CHECK_EQUAL(standMaxAge, 120);
}

BOOST_AUTO_TEST_CASE(GetStandTotalVolumeAtAge){	
    // Get the MOJA stand volume at each age, and compare to the CBM output.
    double tolerance = 0.000001;

    for (int i = 0; i <= 120; i++){		
        double cbmStandVolumeAtAge = cbmStandTotalVolume[i];
        double mojaStandVolumeAtAge = testCurve.getStandTotalVolumeAtAge(i);

        MOJA_LOG_DEBUG << "Age:\t" << i << "\tCBM:\t" << cbmStandVolumeAtAge
            << "\tMOJA:\t" << mojaStandVolumeAtAge;

        BOOST_CHECK_CLOSE(cbmStandVolumeAtAge, mojaStandVolumeAtAge, tolerance);
    }	
}

BOOST_AUTO_TEST_CASE(GetStandSoftwoodVolumeRatioAtAge) {
    double tolerance = 0.0001;

    for (int i = 0; i <= 120; i++) {
        double cbmOutputRatio = swRatio[i];
        double mojaRatio = testCurve.getStandSoftwoodVolumeRatioAtAge(i);

        MOJA_LOG_DEBUG << cbmOutputRatio - mojaRatio;
        BOOST_CHECK_CLOSE(cbmOutputRatio, mojaRatio, tolerance);
    }
}

BOOST_AUTO_TEST_CASE(GetStandAgeWithMaximumVolume) {
    // For this stand growth curve, the maximum merchantable volume is 504 at the age 85.
    int age = testCurve.getStandAgeWithMaximumVolume();
    BOOST_CHECK_EQUAL(age, 85);		
}

BOOST_AUTO_TEST_CASE(GetAnnualStandMaximumVolume) {
    // For this stand growth curve, the maximum merchantable volume is 504 at the age 85.
    auto maximumVolume = testCurve.getAnnualStandMaximumVolume();
    BOOST_CHECK_EQUAL(maximumVolume, 504.0);
}

BOOST_AUTO_TEST_SUITE_END();
