#include <memory.h>

#include <boost/test/unit_test.hpp>
#include <turtle/mock.hpp>

#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/dynamicstruct.h"
#include "moja/test/mocklandunitcontroller.h"
#include "moja/test/mockpool.h"
#include "moja/test/mockvariable.h"
#include "moja/test/mockoperation.h"

namespace Mocks = moja::test;
using namespace moja;

typedef flint::ILandUnitControllerOperationResultIterator::Ptr OpResultPtr;

struct CBMDecayModuleTestsFixture {
	Dynamic mockTable;

	CBMDecayModuleTestsFixture() {
		auto tableData = std::vector<DynamicObject>();
		for (int i = 0; i < 100; i++) {
			DynamicObject row;
			row.insert("Pool", std::to_string(i));
			row.insert("OrganicMatterDecayRate", 0.5);
			row.insert("Q10", 0.5);
			row.insert("ReferenceTemp", 0.5);
			row.insert("MaxDecayRate_soft", 0.5);
			row.insert("PropToAtmosphere", 0.5);
			row.insert("MeanAnnualTemperature", 0.1);
			row.insert("SlowMixingRate", 0.25);
			tableData.push_back(row);
		}

		mockTable = Dynamic(tableData);
	}
};

BOOST_FIXTURE_TEST_SUITE(CBMDecayModuleTests, CBMDecayModuleTestsFixture);

void expectTransfer(std::shared_ptr<Mocks::MockOperation> operation,
                    std::string sourceName,
                    std::string sinkName) {
    MOCK_EXPECT(operation->addTransfer).exactly(1).with(
            mock::call([sourceName](const flint::IPool* source) {
                return source->name() == sourceName;
            }),
            mock::call([sinkName](const flint::IPool* sink) {
                return sink->name() == sinkName;
            }),
            mock::any).returns(operation.get());
};

BOOST_AUTO_TEST_CASE(DecayModulePerformsExpectedTransfers) {
    auto mockLandUnitData = std::make_unique<Mocks::MockLandUnitController>();
    
	Mocks::MockVariable mockTableVariable;
    Mocks::MockVariable mockVariable;

    auto mockOperation = std::make_shared<Mocks::MockOperation>();
    std::vector<std::unique_ptr<Mocks::MockPool>> mockPools;

    // Wire all of the mocks together.
    MOCK_EXPECT(mockLandUnitData->getPoolByName).calls(
            [&mockPools](const std::string name)->Mocks::MockPool* {

        mockPools.push_back(std::make_unique<Mocks::MockPool>(name, "", "", 1.0, 0));
        return mockPools.back().get();
    });
	MOCK_EXPECT(mockLandUnitData->getVariable).with("DecayParameters").returns(&mockTableVariable);
    MOCK_EXPECT(mockLandUnitData->getVariable).returns(&mockVariable);
	MOCK_EXPECT(mockLandUnitData->createProportionalOperation).returns(mockOperation);
	MOCK_EXPECT(mockVariable.value).returns(mockTable);
	MOCK_EXPECT(mockTableVariable.value).returns(mockTable);

    // Actual expectations.
    expectTransfer(mockOperation, "AboveGroundVeryFastSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "AboveGroundVeryFastSoil", "atmosphere");
    expectTransfer(mockOperation, "BelowGroundVeryFastSoil", "BelowGroundSlowSoil");
    expectTransfer(mockOperation, "BelowGroundVeryFastSoil", "atmosphere");
    expectTransfer(mockOperation, "AboveGroundFastSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "AboveGroundFastSoil", "atmosphere");
    expectTransfer(mockOperation, "BelowGroundFastSoil", "BelowGroundSlowSoil");
    expectTransfer(mockOperation, "BelowGroundFastSoil", "atmosphere");
    expectTransfer(mockOperation, "MediumSoil", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "MediumSoil", "atmosphere");
    expectTransfer(mockOperation, "SoftwoodStemSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "SoftwoodStemSnag", "atmosphere");
    expectTransfer(mockOperation, "SoftwoodBranchSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "SoftwoodBranchSnag", "atmosphere");
    expectTransfer(mockOperation, "HardwoodStemSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "HardwoodStemSnag", "atmosphere");
    expectTransfer(mockOperation, "HardwoodBranchSnag", "AboveGroundSlowSoil");
    expectTransfer(mockOperation, "HardwoodBranchSnag", "atmosphere");
    expectTransfer(mockOperation, "AboveGroundSlowSoil", "atmosphere");
    expectTransfer(mockOperation, "BelowGroundSlowSoil", "atmosphere");
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

BOOST_AUTO_TEST_SUITE_END();
