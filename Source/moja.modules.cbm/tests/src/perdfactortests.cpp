#include <boost/test/unit_test.hpp>

#include "moja/dynamic.h"
#include "moja/modules/cbm/perdfactor.h"

namespace cbm = moja::modules::cbm;
using moja::DynamicObject;

std::vector<std::string> columnNames{
    "forest_type", "a", "b", "a_non_merch", "b_non_merch", "k_non_merch", "cap_non_merch", "a_sap", "b_sap",
    "k_sap", "cap_sap", "a1", "a2", "a3", "b1", "b2", "b3", "c1", "c2", "c3", "min_volume", "max_volume",
    "low_stemwood_prop", "high_stemwood_prop", "low_stembark_prop", "high_stembark_prop", "low_branches_prop",
    "high_branches_prop", "low_foliage_prop", "high_foliage_prop", "softwood_top_prop", "softwood_stump_prop",
    "hardwood_top_prop", "hardwood_stump_prop"
};

// Softwood perd factors from CBM input.
std::vector<double> swPerdFactors{
     1              ,   0.940757947    ,  0.893308863     , 26.74911954      , -0.842754964,
     0.634304739    ,   4.359026813    ,  7.876122189     , -1.768994003     ,  0.999038508,
     1.031175277    ,  -1.719625       , -0.0001865       , -0.0678935       , -1.338323   ,
    -0.0003408      ,  -0.115071       , -1.365578        , -0.0008351       , -0.1792023  ,
     0.789301498    , 409.5496953      ,  0.641263962     ,  0.777724244     ,  0.1019464  ,
     0.085726225    ,   0.137384444    ,  0.088669801     ,  0.119405194     ,  0.04787973 ,
     1.7940000295639,   5.3899998664856,  3.00200009346008,  5.51999998092651
};

// Hardwood perd factors from CBM input.
std::vector<double> hwPerdFactors{
     2              ,   0.605526666    ,   0.968769544     , 43.36460139      , -1.094296331,
     0.694702256    ,   4.864578887    , 204.6063969       , -2.758123355     ,  0.999990336,
     1.012284879    ,  -2.226267       ,  -0.004037        ,  0.2254254       , -2.987039   ,
    -0.0065417      ,   0.4155504      ,  -3.009557        , -0.0057198       ,  0.0936287  ,
    14.10121815     , 203.6218461      ,   0.706766368     ,  0.765715018     ,  0.140113889,
     0.12107513     ,   0.110740086    ,   0.093784032     ,  0.042379657     ,  0.019425819,
     1.7940000295639,   5.3899998664856,   3.00200009346008,  5.51999998092651
};

double tolerance = 0.000001;

struct PERDFactorFixture {
    std::vector<DynamicObject> mockTable;

    PERDFactorFixture() {
        DynamicObject swRow;
        for (int i = 0; i < columnNames.size(); i++) {
            if (i == 0) {
                swRow.insert(columnNames[i], "SoftWood");
            } else {
                swRow.insert(columnNames[i], swPerdFactors[i]);
            }
        }
        mockTable.push_back(swRow);
    
        DynamicObject hwRow;
        for (int i = 0; i < columnNames.size(); i++) {
            if (i == 0) {
                hwRow.insert(columnNames[i], "HardWood");
            } else {
                hwRow.insert(columnNames[i], hwPerdFactors[i]);
            }
        }
        mockTable.push_back(hwRow);
    }
};

BOOST_FIXTURE_TEST_SUITE(PERDFactorTests, PERDFactorFixture);

BOOST_AUTO_TEST_CASE(PERDFactorConstructorSW) {
    cbm::PERDFactor pf;

    // Get the softwood PERD row.
    auto pfSWRow = mockTable[0];
    pf.setValue(pfSWRow);
    pf.setTreeSpeciesID(1);

    BOOST_CHECK_EQUAL(pf.merchEquationNumber(), 1);
    BOOST_CHECK_EQUAL(pf.nonmerchEquationNumber(), 2);
    BOOST_CHECK_EQUAL(pf.saplingEquationNumber(), 4);
    BOOST_CHECK_EQUAL(pf.otherEquationNumber(), 7);
    
    BOOST_CHECK_CLOSE(pf.a(), swPerdFactors[1], tolerance);
    BOOST_CHECK_CLOSE(pf.b(), swPerdFactors[2], tolerance);
    BOOST_CHECK_CLOSE(pf.a_nonmerch(), swPerdFactors[3], tolerance);
    BOOST_CHECK_CLOSE(pf.b_nonmerch(), swPerdFactors[4], tolerance);
    BOOST_CHECK_CLOSE(pf.k_nonmerch(), swPerdFactors[5], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_nonmerch(), swPerdFactors[6], tolerance);
    BOOST_CHECK_CLOSE(pf.a_sap(), swPerdFactors[7], tolerance);
    BOOST_CHECK_CLOSE(pf.b_sap(), swPerdFactors[8], tolerance);
    BOOST_CHECK_CLOSE(pf.k_sap(), swPerdFactors[9], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_sap(), swPerdFactors[10], tolerance);
    BOOST_CHECK_CLOSE(pf.a1(), swPerdFactors[11], tolerance);
    BOOST_CHECK_CLOSE(pf.a2(), swPerdFactors[12], tolerance);
    BOOST_CHECK_CLOSE(pf.a3(), swPerdFactors[13], tolerance);
    BOOST_CHECK_CLOSE(pf.b1(), swPerdFactors[14], tolerance);
    BOOST_CHECK_CLOSE(pf.b2(), swPerdFactors[15], tolerance);
    BOOST_CHECK_CLOSE(pf.b3(), swPerdFactors[16], tolerance);
    BOOST_CHECK_CLOSE(pf.c1(), swPerdFactors[17], tolerance);
    BOOST_CHECK_CLOSE(pf.c2(), swPerdFactors[18], tolerance);
    BOOST_CHECK_CLOSE(pf.c3(), swPerdFactors[19], tolerance);
    BOOST_CHECK_CLOSE(pf.min_volume(), swPerdFactors[20], tolerance);
    BOOST_CHECK_CLOSE(pf.max_volume(), swPerdFactors[21], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stemwood_prop(), swPerdFactors[22], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stemwood_prop(), swPerdFactors[23], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stembark_prop(), swPerdFactors[24], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stembark_prop(), swPerdFactors[25], tolerance);
    BOOST_CHECK_CLOSE(pf.low_branches_prop(), swPerdFactors[26], tolerance);
    BOOST_CHECK_CLOSE(pf.high_branches_prop(), swPerdFactors[27], tolerance);
    BOOST_CHECK_CLOSE(pf.low_foliage_prop(), swPerdFactors[28], tolerance);
    BOOST_CHECK_CLOSE(pf.high_foliage_prop(), swPerdFactors[29], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_top_prop(), swPerdFactors[30], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_stump_prop(), swPerdFactors[31], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_top_prop(), swPerdFactors[32], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_stump_prop(), swPerdFactors[33], tolerance);
}

BOOST_AUTO_TEST_CASE(PERDFactorConstructorHW) {
    cbm::PERDFactor pf;
    // Get the hardwood PERD row.
    auto pfHWRow = mockTable[1];
    pf.setValue(pfHWRow);
    pf.setTreeSpeciesID(2);

    BOOST_CHECK_EQUAL(pf.merchEquationNumber(), 1);
    BOOST_CHECK_EQUAL(pf.nonmerchEquationNumber(), 2);
    BOOST_CHECK_EQUAL(pf.saplingEquationNumber(), 4);
    BOOST_CHECK_EQUAL(pf.otherEquationNumber(), 7);
    BOOST_CHECK_CLOSE(pf.a(), hwPerdFactors[1], tolerance);
    BOOST_CHECK_CLOSE(pf.b(), hwPerdFactors[2], tolerance);
    BOOST_CHECK_CLOSE(pf.a_nonmerch(), hwPerdFactors[3], tolerance);
    BOOST_CHECK_CLOSE(pf.b_nonmerch(), hwPerdFactors[4], tolerance);
    BOOST_CHECK_CLOSE(pf.k_nonmerch(), hwPerdFactors[5], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_nonmerch(), hwPerdFactors[6], tolerance);
    BOOST_CHECK_CLOSE(pf.a_sap(), hwPerdFactors[7], tolerance);
    BOOST_CHECK_CLOSE(pf.b_sap(), hwPerdFactors[8], tolerance);
    BOOST_CHECK_CLOSE(pf.k_sap(), hwPerdFactors[9], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_sap(), hwPerdFactors[10], tolerance);
    BOOST_CHECK_CLOSE(pf.a1(), hwPerdFactors[11], tolerance);
    BOOST_CHECK_CLOSE(pf.a2(), hwPerdFactors[12], tolerance);
    BOOST_CHECK_CLOSE(pf.a3(), hwPerdFactors[13], tolerance);
    BOOST_CHECK_CLOSE(pf.b1(), hwPerdFactors[14], tolerance);
    BOOST_CHECK_CLOSE(pf.b2(), hwPerdFactors[15], tolerance);
    BOOST_CHECK_CLOSE(pf.b3(), hwPerdFactors[16], tolerance);
    BOOST_CHECK_CLOSE(pf.c1(), hwPerdFactors[17], tolerance);
    BOOST_CHECK_CLOSE(pf.c2(), hwPerdFactors[18], tolerance);
    BOOST_CHECK_CLOSE(pf.c3(), hwPerdFactors[19], tolerance);
    BOOST_CHECK_CLOSE(pf.min_volume(), hwPerdFactors[20], tolerance);
    BOOST_CHECK_CLOSE(pf.max_volume(), hwPerdFactors[21], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stemwood_prop(), hwPerdFactors[22], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stemwood_prop(), hwPerdFactors[23], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stembark_prop(), hwPerdFactors[24], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stembark_prop(), hwPerdFactors[25], tolerance);
    BOOST_CHECK_CLOSE(pf.low_branches_prop(), hwPerdFactors[26], tolerance);
    BOOST_CHECK_CLOSE(pf.high_branches_prop(), hwPerdFactors[27], tolerance);
    BOOST_CHECK_CLOSE(pf.low_foliage_prop(), hwPerdFactors[28], tolerance);
    BOOST_CHECK_CLOSE(pf.high_foliage_prop(), hwPerdFactors[29], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_top_prop(), hwPerdFactors[30], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_stump_prop(), hwPerdFactors[31], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_top_prop(), hwPerdFactors[32], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_stump_prop(), hwPerdFactors[33], tolerance);
}

BOOST_AUTO_TEST_CASE(PERDFactorSetDefault) {
    cbm::PERDFactor pf;

    // Set the pf from a data array.
    pf.setTreeSpeciesID(1);
    pf.setDefaultValue(swPerdFactors);

    BOOST_CHECK_CLOSE(pf.a(), swPerdFactors[1], tolerance);
    BOOST_CHECK_CLOSE(pf.b(), swPerdFactors[2], tolerance);
    BOOST_CHECK_CLOSE(pf.a_nonmerch(), swPerdFactors[3], tolerance);
    BOOST_CHECK_CLOSE(pf.b_nonmerch(), swPerdFactors[4], tolerance);
    BOOST_CHECK_CLOSE(pf.k_nonmerch(), swPerdFactors[5], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_nonmerch(), swPerdFactors[6], tolerance);
    BOOST_CHECK_CLOSE(pf.a_sap(), swPerdFactors[7], tolerance);
    BOOST_CHECK_CLOSE(pf.b_sap(), swPerdFactors[8], tolerance);
    BOOST_CHECK_CLOSE(pf.k_sap(), swPerdFactors[9], tolerance);
    BOOST_CHECK_CLOSE(pf.cap_sap(), swPerdFactors[10], tolerance);
    BOOST_CHECK_CLOSE(pf.a1(), swPerdFactors[11], tolerance);
    BOOST_CHECK_CLOSE(pf.a2(), swPerdFactors[12], tolerance);
    BOOST_CHECK_CLOSE(pf.a3(), swPerdFactors[13], tolerance);
    BOOST_CHECK_CLOSE(pf.b1(), swPerdFactors[14], tolerance);
    BOOST_CHECK_CLOSE(pf.b2(), swPerdFactors[15], tolerance);
    BOOST_CHECK_CLOSE(pf.b3(), swPerdFactors[16], tolerance);
    BOOST_CHECK_CLOSE(pf.c1(), swPerdFactors[17], tolerance);
    BOOST_CHECK_CLOSE(pf.c2(), swPerdFactors[18], tolerance);
    BOOST_CHECK_CLOSE(pf.c3(), swPerdFactors[19], tolerance);
    BOOST_CHECK_CLOSE(pf.min_volume(), swPerdFactors[20], tolerance);
    BOOST_CHECK_CLOSE(pf.max_volume(), swPerdFactors[21], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stemwood_prop(), swPerdFactors[22], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stemwood_prop(), swPerdFactors[23], tolerance);
    BOOST_CHECK_CLOSE(pf.low_stembark_prop(), swPerdFactors[24], tolerance);
    BOOST_CHECK_CLOSE(pf.high_stembark_prop(), swPerdFactors[25], tolerance);
    BOOST_CHECK_CLOSE(pf.low_branches_prop(), swPerdFactors[26], tolerance);
    BOOST_CHECK_CLOSE(pf.high_branches_prop(), swPerdFactors[27], tolerance);
    BOOST_CHECK_CLOSE(pf.low_foliage_prop(), swPerdFactors[28], tolerance);
    BOOST_CHECK_CLOSE(pf.high_foliage_prop(), swPerdFactors[29], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_top_prop(), swPerdFactors[30], tolerance);
    BOOST_CHECK_CLOSE(pf.softwood_stump_prop(), swPerdFactors[31], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_top_prop(), swPerdFactors[32], tolerance);
    BOOST_CHECK_CLOSE(pf.hardwood_stump_prop(), swPerdFactors[33], tolerance);
}

BOOST_AUTO_TEST_SUITE_END();