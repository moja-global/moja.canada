#include <memory.h>

#include <boost/test/unit_test.hpp>
#include <turtle/mock.hpp>

#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/dynamicstruct.h"
#include "moja/test/mocklandunitcontroller.h"
#include "moja/test/mockpoolcollection.h"
#include "moja/test/mockpool.h"
#include "moja/test/mockvariable.h"
#include "moja/test/mockoperation.h"
#include "moja/flint/imodule.h"

namespace mocks = moja::test;
using namespace moja;

struct CBMDecayModuleTestsFixture {
    Dynamic mat{ 25.0 };
    Dynamic snagSplit{ 0.25 };
    Dynamic mockTable;

	flint::ModuleMetaData metadata;

    CBMDecayModuleTestsFixture() {
        auto tableData = std::vector<DynamicObject>();
        for (int i = 0; i < 100; i++) {
            DynamicObject row;
            row.insert("pool", std::to_string(i));
            row.insert("organic_matter_decay_rate", 0.5);
            row.insert("q10", 0.5);
            row.insert("reference_temp", 0.5);
            row.insert("max_decay_rate_soft", 0.5);
            row.insert("prop_to_atmosphere", 0.5);
            tableData.push_back(row);
        }

        mockTable = Dynamic(tableData);

		metadata.setDefaults();
		metadata.moduleName = "LandUnitControllerOperationTestsFixture";
    }
};

BOOST_FIXTURE_TEST_SUITE(CBMDecayModuleTests, CBMDecayModuleTestsFixture);

void expectTransfer(std::shared_ptr<mocks::MockOperation> operation,
                    std::string sourceName,
                    std::string sinkName) {
    MOCK_EXPECT(operation->addTransferShared).exactly(1).with(
            mock::call([sourceName](flint::IPool::ConstPtr source) {
                return source->name() == sourceName;
            }),
            mock::call([sinkName](flint::IPool::ConstPtr sink) {
                return sink->name() == sinkName;
            }),
            mock::any).returns(operation.get());
};

#if 0

BOOST_AUTO_TEST_CASE(DecayModulePerformsExpectedTransfers) {
    auto mockLandUnitData = std::make_unique<mocks::MockLandUnitController>();
    
	auto mockPoolCollection = std::make_shared<mocks::MockPoolCollection>();
	
	mocks::MockVariable mockTableVariable;
	mocks::MockVariable mockVariable;
	mocks::MockVariable mockTemperatureVariable;
	mocks::MockVariable snagSplitVariable;

    auto mockOperation = std::make_shared<mocks::MockOperation>(metadata);
    std::vector<std::unique_ptr<mocks::MockPool>> mockPools;

    // Wire all of the mocks together.
	MOCK_EXPECT(mockLandUnitData->getCollection).returns(mockPoolCollection);
	MOCK_EXPECT(mockPoolCollection->getPoolByName).calls([&mockPools](const std::string name)->mocks::MockPool* {
		mockPools.push_back(std::make_unique<mocks::MockPool>(name, "", "", 1.0, 0));
		return mockPools.back().get();
	});

    MOCK_EXPECT(mockLandUnitData->getVariable).with("decay_parameters").returns(&mockTableVariable);
    MOCK_EXPECT(mockLandUnitData->getVariable).with("mean_annual_temperature").returns(&mockTemperatureVariable);
    MOCK_EXPECT(mockLandUnitData->getVariable).with("other_to_branch_snag_split").returns(&snagSplitVariable);

    MOCK_EXPECT(mockLandUnitData->getVariable).returns(&mockVariable);
    MOCK_EXPECT(mockLandUnitData->createProportionalOperation).returns(mockOperation);
    MOCK_EXPECT(mockVariable.value).returns(mockTable);
    MOCK_EXPECT(mockTableVariable.value).returns(mockTable);
    MOCK_EXPECT(mockTemperatureVariable.value).returns(mat);
    MOCK_EXPECT(snagSplitVariable.value).returns(snagSplit);


    // Actual expectations.
    expectTransfer(mockOperation, "AboveGroundVeryFastSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "AboveGroundVeryFastSoil", "Atmosphere");
    expectTransfer(mockOperation, "BelowGroundVeryFastSoil", "BelowGroundSlowSoil");
    expectTransfer(mockOperation, "BelowGroundVeryFastSoil", "Atmosphere");
    expectTransfer(mockOperation, "AboveGroundFastSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "AboveGroundFastSoil", "Atmosphere");
    expectTransfer(mockOperation, "BelowGroundFastSoil", "BelowGroundSlowSoil");
    expectTransfer(mockOperation, "BelowGroundFastSoil", "Atmosphere");
    expectTransfer(mockOperation, "MediumSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "MediumSoil", "Atmosphere");
    expectTransfer(mockOperation, "SoftwoodStemSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "SoftwoodStemSnag", "Atmosphere");
    expectTransfer(mockOperation, "SoftwoodBranchSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "SoftwoodBranchSnag", "Atmosphere");
    expectTransfer(mockOperation, "HardwoodStemSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "HardwoodStemSnag", "Atmosphere");
    expectTransfer(mockOperation, "HardwoodBranchSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "HardwoodBranchSnag", "Atmosphere");
    expectTransfer(mockOperation, "AboveGroundSlowSoil", "Atmosphere");
    expectTransfer(mockOperation, "BelowGroundSlowSoil", "Atmosphere");
    expectTransfer(mockOperation, "AboveGroundSlowSoil", "BelowGroundSlowSoil");
    MOCK_EXPECT(mockLandUnitData->submitOperation);

    moja::modules::cbm::CBMDecayModule module;
    flint::ModuleMetaData moduleMetadata { 1, 1, 1, 1, "test" };
    module.StartupModule(moduleMetadata);
    module.setLandUnitController(*mockLandUnitData.get());
    module.onLocalDomainInit(nullptr);
    module.onTimingInit(nullptr);
    module.onTimingStep(nullptr);
}
#endif

BOOST_AUTO_TEST_SUITE_END();
