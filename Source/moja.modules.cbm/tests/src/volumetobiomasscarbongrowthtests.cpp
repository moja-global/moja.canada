#include <boost/test/unit_test.hpp>

#include "moja/logging.h"
#include "moja/dynamic.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/volumetobiomassconverter.h"
#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"

namespace cbm = moja::modules::cbm;

using moja::DynamicVar;
using moja::DynamicObject;

extern std::vector<double> aVolumes;
extern std::vector<double> bVolumes;
extern std::vector<double> cVolumes;
extern std::vector<double> swPerdFactors;
extern std::vector<double> hwPerdFactors;

// Following is CBM growth output for stand from age 100 to 125.
double cbmagIncrements[25][6] = {
    { 0.102234, 0.0414343, 0.0316019, -1.21360, -0.253382, -0.029062  },
    { 0.103531, 0.0419331, 0.0320106, -1.21438, -0.253547, -0.029080  },
    { 0.104889, 0.0424175, 0.0324173, -1.21516, -0.253719, -0.029099  },
    { 0.106239, 0.0429382, 0.0328407, -1.21597, -0.253886, -0.029118  },
    { 0.107643, 0.0434589, 0.0332713, -1.21678, -0.254063, -0.029138  },
    { 0.104324, 0.0420761, 0.0322337, -1.16465, -0.243185, -0.027890  },
    { 0.105675, 0.0425797, 0.0326533, -1.16542, -0.243354, -0.027908  },
    { 0.107048, 0.0430822, 0.0330725, -1.16621, -0.243523, -0.027927  },
    { 0.108482, 0.0436373, 0.0335093, -1.16701, -0.243699, -0.027947  },
    { 0.109947, 0.0441704, 0.0339584, -1.16782, -0.243872, -0.027966  },
    { 0.121643, 0.0488319, 0.0375628, -1.27493, -0.266253, -0.030531  },
    { 0.123482, 0.0495186, 0.0381260, -1.27595, -0.266472, -0.030556  },
    { 0.125381, 0.0502205, 0.0387011, -1.27698, -0.266699, -0.030581  },
    { 0.127304, 0.0509510, 0.0392866, -1.27802, -0.266933, -0.030606  },
    { 0.129349, 0.0516872, 0.0399017, -1.27911, -0.267168, -0.030632  },
    { 0.120377, 0.0480728, 0.0371289, -1.17348, -0.245120, -0.0281035 },
    { 0.122177, 0.3072890, 0.0522332, -1.17442, -0.245331, -0.028126  },
    { 0.123993, 0.3972940, 0.0580153, -1.17539, -0.245545, -0.028149  },
    { 0.125923, 0.4080820, 0.0593882, -1.17636, -0.245765, -0.028173  },
    { 0.127869, 0.4193370, 0.0608206, -1.17737, -0.245993, -0.028197  },
    { 0.000000, 0.0000000, 0.0000000, 0.000000, 0.0000000, 0.0000000  },
    { 0.000000, 0.0000000, 0.0000000, 0.000000, 0.0000000, 0.0000000  },
    { 0.000000, 0.0000000, 0.0000000, 0.000000, 0.0000000, 0.0000000  },
    { 0.000000, 0.0000000, 0.0000000, 0.000000, 0.0000000, 0.0000000  },
    { 0.000000, 0.0000000, 0.0000000, 0.000000, 0.0000000, 0.0000000  },
};

// CBM stand initial value.
double cbmBiomassPools[2][7] = {
    // total,         merchantable,	 foliage,      subMerchantable, other,      coarse root,   fine root
    { 110.3671417236, 72.2636566162, 4.3958888054, 0.0000000000, 13.6572656631, 18.4406623840, 1.6096704006 },
    { 48.20652771000, 30.0717830658, 0.7201749682, 0.0000000000, 6.28109693530, 10.2396593094, 0.8938115239 }
};

// Following are CBM root carbon growth.
double cbmRootCarbon[25][4] = {
    //sw_coarse,sw_fine,hw_coarse,hw_fine	 
    { 18.4716, 1.61766, 9.98089, 0.874085 },
    { 18.5027, 1.62595, 9.71779, 0.85396  },
    { 18.5340, 1.63455, 9.45008, 0.83342  },
    { 18.5655, 1.64348, 9.17749, 0.81242  },
    { 18.5971, 1.65278, 8.89968, 0.79093  },
    { 18.6275, 1.66203, 8.62872, 0.76989  },
    { 18.6580, 1.67168, 8.35228, 0.74833  },
    { 18.6886, 1.68175, 8.06997, 0.7262   },
    { 18.7193, 1.69228, 7.78131, 0.70345  },
    { 18.7500, 1.70332, 7.48577, 0.680033 },
    { 18.7835, 1.71599, 7.15479, 0.653634 },
    { 18.8170, 1.72941, 6.81395, 0.626248 },
    { 18.8503, 1.74366, 6.46212, 0.59774  },
    { 18.8834, 1.75886, 6.09794, 0.56798  },
    { 18.9161, 1.77516, 5.71971, 0.53675  },
    { 18.9457, 1.79122, 5.35882, 0.50665  },
    { 19.0323, 1.81154, 4.98281, 0.47427  },
    { 19.1383, 1.83423, 4.58845, 0.439763 },
    { 19.2456, 1.85864, 4.17180, 0.402892 },
    { 19.3540, 1.88519, 3.72752, 0.363081 },
    { 19.3540, 1.88519, 3.72752, 0.363081 },
    { 19.3540, 1.88519, 3.72752, 0.363081 },
    { 19.3540, 1.88519, 3.72752, 0.363081 },
    { 19.3540, 1.88519, 3.72752, 0.363081 },
    { 19.3540, 1.88519, 3.72752, 0.363081 }
};
/*
struct V2BCarbonGrowthFixture {
    cbm::PERDFactor swPf;
    cbm::PERDFactor hwPf;
    std::vector<DynamicObject> mockSWTable;
    std::vector<DynamicObject> mockHWTable;
    std::shared_ptr<cbm::StandGrowthCurve> standGrowthCurve;
    std::shared_ptr<moja::test::MockVariable> mockAge;
    std::shared_ptr<moja::test::MockPool> 

    DynamicObject rootParameters{ {
        { "hw_a", 1.576 }, { "sw_a", 0.222 }, { "hw_b", 0.615 },
        { "frp_a", 0.072 }, { "frp_b", 0.354 }, { "frp_c", -0.0602119460500964 }
    } };

    V2BCarbonGrowthFixture() {
        swPf.setDefaultValue(swPerdFactors);
        hwPf.setDefaultValue(hwPerdFactors);

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

        standGrowthCurve = std::make_shared<cbm::StandGrowthCurve>(101);

        // Based on mockTable, create a softwood yield table.
        auto swYieldTable = std::make_shared<cbm::TreeYieldTable>(
            mockSWTable, cbm::SpeciesType::Softwood);

        auto hwYieldTable = std::make_shared<cbm::TreeYieldTable>(
            mockHWTable, cbm::SpeciesType::Hardwood);

        standGrowthCurve->addYieldTable(swYieldTable);
        standGrowthCurve->addYieldTable(hwYieldTable);

        // Add softwood PERD factor
        auto swPerdFactor = std::make_unique<cbm::PERDFactor>();
        swPerdFactor->setDefaultValue(swPerdFactors);
        standGrowthCurve->setPERDFactor(std::move(swPerdFactor), cbm::SpeciesType::Softwood);

        // Add hardwood PERD factor
        auto hwPerdFactor = std::make_unique<cbm::PERDFactor>();
        hwPerdFactor->setDefaultValue(hwPerdFactors);
        standGrowthCurve->setPERDFactor(std::move(hwPerdFactor), cbm::SpeciesType::Hardwood);

        standGrowthCurve->processStandYieldTables();
    }
};

BOOST_FIXTURE_TEST_SUITE(VolumeToBiomassConverterTests, V2BCarbonGrowthFixture);

BOOST_AUTO_TEST_CASE(VolumeToBiomassCarbonGrowthConstructor) {
    // Check softwood component carbon at each age.
    auto volumeToBioGrowth = std::make_shared<cbm::VolumeToBiomassCarbonGrowth>(rootParameters);

    // After the default construction, try to get the stand growth curve.
    bool findARandomCurve = volumeToBioGrowth->isBiomassCarbonCurveAvailable(101);

    BOOST_CHECK(findARandomCurve == false);	
}

BOOST_AUTO_TEST_CASE(GenerateBiomassCarbonCurve) {	
    auto volumeToBioGrowth = std::make_shared<cbm::VolumeToBiomassCarbonGrowth>(rootParameters);
    volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);

    // Try to get the newly generated the biomass carbon curve.
    bool findARandomCurve = volumeToBioGrowth->isBiomassCarbonCurveAvailable(101);

    BOOST_CHECK(findARandomCurve == true);
}

BOOST_AUTO_TEST_CASE(GetAGBiomassIncrements) {	
    auto volumeToBioGrowth = std::make_shared<cbm::VolumeToBiomassCarbonGrowth>(rootParameters);
    volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);
    int growthCurveID = 101;

    for (int age = 100; age < 125; age++) {
        auto agIncrement = volumeToBioGrowth->getBiomassCarbonIncrements(growthCurveID, age);
        double cbmSoftwoodMerch = cbmagIncrements[age - 100][0];
        double cbmSoftwoodOther = cbmagIncrements[age - 100][1];
        double cbmSoftwoodFoliage = cbmagIncrements[age - 100][2];
        double cbmHardwoodMerch = cbmagIncrements[age - 100][3];
        double cbmHardwoodOther = cbmagIncrements[age - 100][4];
        double cbmHardwoodFoliage = cbmagIncrements[age - 100][5];

        double mojaSoftwoodMerch = agIncrement->softwoodMerch();
        double mojaSoftwoodOther = agIncrement->softwoodOther();
        double mojaSoftwoodFoliage = agIncrement->softwoodFoliage();
        double mojaHardwoodMerch = agIncrement->hardwoodMerch();
        double mojaHardwoodOther = agIncrement->hardwoodOther();
        double mojaHardwoodFoliage = agIncrement->hardwoodFoliage();		

        MOJA_LOG_DEBUG << mojaSoftwoodMerch << ",\t" << cbmSoftwoodMerch << "\tDIff: " << (mojaSoftwoodMerch - cbmSoftwoodMerch);
        MOJA_LOG_DEBUG << mojaSoftwoodOther << ",\t" << cbmSoftwoodOther << "\tDIff: " << (mojaSoftwoodOther - cbmSoftwoodOther);
        MOJA_LOG_DEBUG << mojaSoftwoodFoliage << ",\t" << cbmSoftwoodFoliage << "\tDIff: " << (mojaSoftwoodFoliage - cbmSoftwoodFoliage);
        MOJA_LOG_DEBUG << mojaHardwoodMerch << ",\t" << cbmHardwoodMerch << "\tDIff: " << (mojaHardwoodMerch - cbmHardwoodMerch);
        MOJA_LOG_DEBUG << mojaHardwoodOther << ",\t" << cbmHardwoodOther << "\tDIff: " << (mojaHardwoodOther - cbmHardwoodOther);
        MOJA_LOG_DEBUG << mojaHardwoodFoliage << ",\t" << cbmHardwoodFoliage << "\tDIff: " << (mojaHardwoodFoliage - cbmHardwoodFoliage);

        BOOST_CHECK_EQUAL(std::round(mojaSoftwoodMerch - cbmSoftwoodMerch), 0);
        BOOST_CHECK_EQUAL(std::round(mojaSoftwoodOther - cbmSoftwoodOther), 0);
        BOOST_CHECK_EQUAL(std::round(mojaSoftwoodFoliage - cbmSoftwoodFoliage), 0);
        BOOST_CHECK_EQUAL(std::round(mojaHardwoodMerch - cbmHardwoodMerch), 0);
        BOOST_CHECK_EQUAL(std::round(mojaHardwoodOther - cbmHardwoodOther), 0);
        BOOST_CHECK_EQUAL(std::round(mojaHardwoodFoliage - cbmHardwoodFoliage), 0);
    }
}

BOOST_AUTO_TEST_CASE(GetBGBiomassIncrements) {	
    auto volumeToBioGrowth = std::make_shared<cbm::VolumeToBiomassCarbonGrowth>(rootParameters);
    volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);	

    // Assign the pool initial value same as CBM initial pool values, which is
    // the stand growth start point.
    double totalSWAgCarbon = cbmBiomassPools[0][1] + cbmBiomassPools[0][2] + cbmBiomassPools[0][3] + cbmBiomassPools[0][4];
    double standSWCoarseRootsCarbon = cbmBiomassPools[0][5];
    double standSWFineRootsCarbon = cbmBiomassPools[0][6];
    double totalHWAgCarbon = cbmBiomassPools[1][1] + cbmBiomassPools[1][2] + cbmBiomassPools[1][3] + cbmBiomassPools[1][4];
    double standHWCoarseRootsCarbon = cbmBiomassPools[1][5];
    double standHWFineRootsCarbon = cbmBiomassPools[1][6];

    std::shared_ptr<cbm::RootBiomassCarbonIncrement> bgIncrement = nullptr;
    std::shared_ptr<cbm::AboveGroundBiomassCarbonIncrement> agIncrement = nullptr;
    int growthCurveID = 101;

    for (int age = 100; age < 125; age++) {
        agIncrement = volumeToBioGrowth->getAGBiomassCarbonIncrements(growthCurveID, age);

        double mojaSoftwoodMerch = agIncrement->softwoodMerch();
        double mojaSoftwoodOther = agIncrement->softwoodOther();
        double mojaSoftwoodFoliage = agIncrement->softwoodFoliage();
        double mojaHardwoodMerch = agIncrement->hardwoodMerch();
        double mojaHardwoodOther = agIncrement->hardwoodOther();
        double mojaHardwoodFoliage = agIncrement->hardwoodFoliage();

        totalSWAgCarbon += (mojaSoftwoodMerch + mojaSoftwoodOther + mojaSoftwoodFoliage);
        totalHWAgCarbon += (mojaHardwoodMerch + mojaHardwoodOther + mojaHardwoodFoliage);

        bgIncrement = volumeToBioGrowth->getBGBiomassCarbonIncrements(
            totalSWAgCarbon, standSWCoarseRootsCarbon, standSWFineRootsCarbon,
            totalHWAgCarbon, standHWCoarseRootsCarbon, standHWFineRootsCarbon);

        double cbmSWCoarseRoot = cbmRootCarbon[age - 100][0];
        double cbmSWFineRoot = cbmRootCarbon[age - 100][1];
        double cbmHWCoarseRoot = cbmRootCarbon[age - 100][2];
        double cbmHWFineRoot = cbmRootCarbon[age - 100][3];

        // Get MOJA root increment/changes.
        standSWCoarseRootsCarbon += bgIncrement->softwoodCoarseRoots();
        standSWFineRootsCarbon += bgIncrement->softwoodFineRoots();
        standHWCoarseRootsCarbon += bgIncrement->hardwoodCoarseRoots();
        standHWFineRootsCarbon += bgIncrement->hardwoodFineRoots();

        MOJA_LOG_DEBUG << standSWCoarseRootsCarbon << ",\t" << cbmSWCoarseRoot << "\tDIff: " << (standSWCoarseRootsCarbon - cbmSWCoarseRoot);
        MOJA_LOG_DEBUG << standSWFineRootsCarbon << ",\t" << cbmSWFineRoot << "\tDIff: " << (standSWFineRootsCarbon - cbmSWFineRoot);
        MOJA_LOG_DEBUG << standHWCoarseRootsCarbon << ",\t" << cbmHWCoarseRoot << "\tDIff: " << (standHWCoarseRootsCarbon - cbmHWCoarseRoot);
        MOJA_LOG_DEBUG << standHWFineRootsCarbon << ",\t" << cbmHWFineRoot << "\tDIff: " << (standHWFineRootsCarbon - cbmHWFineRoot);

        BOOST_CHECK_EQUAL(std::round(cbmSWCoarseRoot - standSWCoarseRootsCarbon), 0);
        BOOST_CHECK_EQUAL(std::round(cbmSWFineRoot - standSWFineRootsCarbon), 0);
        BOOST_CHECK_EQUAL(std::round(cbmHWCoarseRoot - standHWCoarseRootsCarbon), 0);
        BOOST_CHECK_EQUAL(std::round(cbmHWFineRoot - standHWFineRootsCarbon), 0);
    }
}

BOOST_AUTO_TEST_SUITE_END();
*/
