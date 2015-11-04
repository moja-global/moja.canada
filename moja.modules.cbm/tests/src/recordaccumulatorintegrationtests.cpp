#include <boost/test/unit_test.hpp>
#include <Poco/Tuple.h>

#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/recordaccumulator.h"

using namespace moja::modules;

BOOST_AUTO_TEST_SUITE(RecordAccumulatorIntegrationTests);

BOOST_AUTO_TEST_CASE(HashesDoNotCollide) {
		cbm::RecordAccumulator<cbm::FluxRow> accumulator;

    // Check to make sure the order of items matters in hashes based on a
    // collection of IDs - these should not get merged by the accumulator.
    auto first = std::make_shared<cbm::FluxRecord>(1, 2, 3, 4, 5, 1.0);
    auto second = std::make_shared<cbm::FluxRecord>(5, 4, 3, 2, 1, 1.0);
    accumulator.accumulate(first);
    accumulator.accumulate(second);

    auto accumulatedItems = accumulator.getPersistableCollection();
    BOOST_CHECK_EQUAL(accumulatedItems.size(), 2);

    for (auto item : accumulatedItems) {
        BOOST_CHECK_EQUAL(item.get<6>(), 1);
    }
}

BOOST_AUTO_TEST_SUITE_END();
