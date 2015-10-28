#include <boost/test/unit_test.hpp>
#include <Poco/Tuple.h>
#include <turtle/mock.hpp>

#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/recordaccumulator.h"
#include "moja/types.h"

using moja::modules::cbm::RecordAccumulator;
using moja::modules::cbm::FluxRecord;
using moja::modules::cbm::FluxRow;
using moja::Int64;

BOOST_AUTO_TEST_SUITE(RecordAccumulatorIntegrationTests);

BOOST_AUTO_TEST_CASE(HashesDoNotCollide) {
    RecordAccumulator<FluxRow> accumulator;

    // Check to make sure the order of items matters in hashes based on a
    // collection of IDs - these should not get merged by the accumulator.
    auto first = std::make_shared<FluxRecord>(1, 2, 3, 4, 5, 1.0);
    auto second = std::make_shared<FluxRecord>(5, 4, 3, 2, 1, 1.0);
    accumulator.accumulate(first);
    accumulator.accumulate(second);

    auto accumulatedItems = accumulator.getPersistableCollection();
    BOOST_CHECK_EQUAL(accumulatedItems.size(), 2);

    for (auto item : accumulatedItems) {
        BOOST_CHECK_EQUAL(item.get<6>(), 1);
    }
}

BOOST_AUTO_TEST_SUITE_END();
