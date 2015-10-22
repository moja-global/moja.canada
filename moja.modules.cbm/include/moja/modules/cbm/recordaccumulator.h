#ifndef MOJA_MODULES_CBM_KEYMANAGER_H_
#define MOJA_MODULES_CBM_KEYMANAGER_H_

#include <unordered_map>
#include <functional>

#include "moja/types.h"
#include "moja/modules/cbm/record.h"

namespace moja {
namespace modules {
namespace cbm {

    template<class TPersistable>
    class RecordAccumulator {
    public:
        Record<TPersistable>* accumulate(std::shared_ptr<Record<TPersistable>> record) {
            auto hash = record->hash();
            auto it = _records.find(hash);
            if (it != _records.end()) {
                // Found an existing ID for the key.
                auto existing = it->second;
                existing->merge(record.get());
                return existing.get();
            }

            // First time seeing this key - assign an ID.
            Int64 id = _nextId++;
            record->setId(id);
            _records.insert(std::make_pair(hash, record));
            return record.get();
        }

        std::vector<TPersistable> getPersistableCollection() {
            std::vector<TPersistable> persistables;
            for (auto record : _records) {
                persistables.push_back(record.second->asPersistable());
            }

            return persistables;
        }

        void clear() {
            _records.clear();
        }

    private:
        Int64 _nextId = 0;
        std::unordered_map<size_t, std::shared_ptr<Record<TPersistable>>> _records;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_KEYMANAGER_H_
