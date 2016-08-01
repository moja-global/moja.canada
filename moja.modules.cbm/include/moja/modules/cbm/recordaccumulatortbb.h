#ifndef MOJA_MODULES_CBM_RECORDACCUMULATORTBB_H_
#define MOJA_MODULES_CBM_RECORDACCUMULATORTBB_H_

#include "moja/types.h"
#include "moja/flint/record.h"
#include "moja/flint/recordutilstbb.h"

#include <tbb/concurrent_unordered_set.h>
#include <Poco/Mutex.h>

namespace moja {
namespace modules {
namespace cbm {

using namespace flint;

template<class TPersistable>
class RecordAccumulatorTBB {
public:
	typedef tbb::concurrent_unordered_set<std::shared_ptr<Record<TPersistable>>,
		RecordHasherTBB<TPersistable>,
		RecordComparerTBB<TPersistable>> rec_accu_set;

	typedef typename rec_accu_set::size_type rec_accu_set_size_type;

	Record<TPersistable>* insert(Int64 id, std::shared_ptr<Record<TPersistable>> record) {
		// ID has been assigned by user; assume that we can run with this.
		// Can't guarantee that this will be called in 'id increasing' order,
		// but a good guess perhaps.
		_nextId = id + 1;
		record->setId(id);
		_records.insert(record);
		return record.get();
	}

	Record<TPersistable>* accumulate(std::shared_ptr<Record<TPersistable>> record) {
		// Try without a lock first.
		auto it = _records.find(record);
		if (it != _records.end()) {
			// Found an existing ID for the key.
			auto existing = *it;
			existing->merge(record.get());
			return existing.get();
		}

		return addOrMerge(record);
	}

	Record<TPersistable>* accumulate(std::shared_ptr<Record<TPersistable>> record, Int64 requestedId) {
		// Try without a lock first.
		auto it = _records.find(record);
		if (it != _records.end()) {
			// Found an existing ID for the key.
			auto existing = *it;
			existing->merge(record.get());
			return existing.get();
		}

		return addOrMerge(record, requestedId);
	}

	Record<TPersistable>* addOrMerge(std::shared_ptr<Record<TPersistable>> record, Int64 requestedId = -1) {
		Poco::Mutex::ScopedLock lock(_mutex);
		auto it = _records.find(record);
		if (it == _records.end()) {
			// First time seeing this key - assign an ID.
			if (requestedId > -1) {
				// Can't guarantee that this will be called in 'id increasing' order,
				// but a good guess perhaps.
				_nextId = requestedId;
			}

			record->setId(_nextId++);
			_records.insert(record);
			return record.get();
		}
		else {
			auto existing = *it;
			existing->merge(record.get());
			return existing.get();
		}
	}

	Record<TPersistable>* search(std::shared_ptr<Record<TPersistable>> record) {
		auto it = _records.find(record);
		if (it != _records.end()) {
			// Found an existing ID for the key.
			auto existing = *it;
			return existing.get();
		}

		return nullptr;
	}

	std::vector<TPersistable> getPersistableCollection() {
		std::vector<TPersistable> persistables;
		for (auto record : _records) {
			persistables.push_back(record->asPersistable());
		}

		return persistables;
	}

	void clear() {
		_records.clear();
	}

	rec_accu_set_size_type size() {
		return _records.size();
	}

private:
	Poco::Mutex _mutex;
	Int64 _nextId = 0;
	rec_accu_set _records;
};

}}}

#endif // MOJA_MODULES_CBM_RECORDACCUMULATORTBB_H_