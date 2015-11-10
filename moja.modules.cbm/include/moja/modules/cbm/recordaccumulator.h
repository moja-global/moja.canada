#ifndef MOJA_MODULES_CBM_KEYMANAGER_H_
#define MOJA_MODULES_CBM_KEYMANAGER_H_

#include <unordered_set>

#include "moja/types.h"
#include "moja/modules/cbm/record.h"

namespace moja {
namespace modules {
namespace cbm {

    template<class TPersistable>
    struct RecordHasher {
        std::size_t operator()(std::shared_ptr<Record<TPersistable>> record) const {
            return record->hash();
        }
    };

    template<class TPersistable>
    struct RecordComparer {
        bool operator() (std::shared_ptr<Record<TPersistable>> lhs,
                         std::shared_ptr<Record<TPersistable>> rhs) const {
            return *lhs == *rhs;
        }
    };

    template<class TPersistable>
    class RecordAccumulator {
    public:
        Record<TPersistable>* accumulate(std::shared_ptr<Record<TPersistable>> record) {
            auto it = _records.find(record);
            if (it != _records.end()) {
                // Found an existing ID for the key.
                auto existing = *it;
                existing->merge(record.get());
                return existing.get();
            }

            // First time seeing this key - assign an ID.
            record->setId(_nextId++);
            _records.insert(record);
            return record.get();
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

    private:
        Int64 _nextId = 0;
        std::unordered_set<std::shared_ptr<Record<TPersistable>>,
                           RecordHasher<TPersistable>,
                           RecordComparer<TPersistable>> _records;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_KEYMANAGER_H_
