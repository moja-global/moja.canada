#include <boost/test/unit_test.hpp>
#include <Poco/Tuple.h>
#include <turtle/mock.hpp>

#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulator.h"

using namespace moja::modules;

typedef Poco::Tuple<std::string> MockRow;

template<class T>
MOCK_BASE_CLASS(MockRecord, moja::flint::Record<T>) {
    MOCK_METHOD(hash, 0, size_t())
    MOCK_METHOD_TPL(asPersistable, 0, T())
    MOCK_METHOD_TPL(merge, 1, void(moja::flint::Record<T>*))

    bool operator==(const moja::flint::Record<T>& other) const override {
        return true;
    }
};

struct RecordAccumulatorTestsFixture {
	moja::flint::RecordAccumulator<MockRow> accumulator;
};

BOOST_FIXTURE_TEST_SUITE(RecordAccumulatorTests, RecordAccumulatorTestsFixture);

BOOST_AUTO_TEST_CASE(KeysStartFromZero) {
    auto record = std::make_shared<MockRecord<MockRow>>();
    MOCK_EXPECT(record->merge);
    MOCK_EXPECT(record->hash).returns(1);
    auto stored = accumulator.accumulate(record);
    BOOST_CHECK_EQUAL(stored->getId(), 1);
}

BOOST_AUTO_TEST_CASE(CreatesNewIDIfKeyNotPresent) {
    for (int i = 0; i < 3; i++) {
        auto record = std::make_shared<MockRecord<MockRow>>();
        MOCK_EXPECT(record->merge);
        MOCK_EXPECT(record->hash).returns(i);
        auto stored = accumulator.accumulate(record);
        BOOST_CHECK_EQUAL(stored->getId(), i + 1);
    }
}

BOOST_AUTO_TEST_CASE(RetrievesExistingIDIfPresent) {
    size_t hash = 1;
    auto record = std::make_shared<MockRecord<MockRow>>();
    MOCK_EXPECT(record->merge);
    MOCK_EXPECT(record->hash).returns(hash);
    auto stored = accumulator.accumulate(record);
    auto id = stored->getId();

    auto similarRecord = std::make_shared<MockRecord<MockRow>>();
    MOCK_EXPECT(similarRecord->merge);
    MOCK_EXPECT(similarRecord->hash).returns(hash);
    stored = accumulator.accumulate(record);

    BOOST_CHECK_EQUAL(stored->getId(), id);
}

BOOST_AUTO_TEST_SUITE_END();
