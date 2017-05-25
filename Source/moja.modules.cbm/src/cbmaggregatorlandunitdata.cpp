#include "moja/modules/cbm/cbmaggregatorlandunitdata.h"
#include "moja/flint/recordaccumulatorwithmutex.h"

namespace moja {
namespace modules {
namespace cbm {

	void CBMAggregatorLandUnitData::configure(const DynamicObject& config) {
		if (config.contains("reporting_classifier_set")) {
			_classifierSetVar = config["reporting_classifier_set"].extract<std::string>();
		} else {
			_classifierSetVar = "classifier_set";
		}
	}

	void CBMAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMAggregatorLandUnitData::onOutputStep		, *this);
		notificationCenter.subscribe(signals::Error			 , &CBMAggregatorLandUnitData::onError			, *this);
    }

    Int64 CBMAggregatorLandUnitData::getPoolId(flint::IPool::ConstPtr pool) {
        PoolInfoRecord poolInfo(pool->name());
        return _poolInfoDimension->search(poolInfo)->getId();
    }

    void CBMAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto locationId = recordLocation(isSpinup);
        recordPoolsSet(locationId, isSpinup);
        recordFluxSet(locationId);
    }

	void CBMAggregatorLandUnitData::recordClassifierNames(const DynamicObject& classifierSet) {
		Poco::Mutex::ScopedLock lock(*_classifierNamesLock);
		if (!_classifierNames->empty()) {
			return;
		}

		for (const auto& classifier : classifierSet) {
			std::string name = classifier.first;
			std::replace(name.begin(), name.end(), '.', '_');
			std::replace(name.begin(), name.end(), ' ', '_');
			_classifierNames->push_back(name);
		}
	}

    Int64 CBMAggregatorLandUnitData::recordLocation(bool isSpinup) {
        Int64 dateRecordId = -1;
        if (!isSpinup) {
            // Find the date dimension record.
            const auto timing = _landUnitData->timing();
            DateRecord dateRecord(
                timing->step(), timing->curStartDate().year(),
                timing->curStartDate().month(), timing->curStartDate().day(),
                timing->fractionOfStep(), timing->stepLengthInYears());

            auto storedDateRecord = _dateDimension->accumulate(dateRecord);
            dateRecordId = storedDateRecord->getId();
        }

        // Classifier set information.
        const auto& landUnitClassifierSet = _classifierSet->value().extract<DynamicObject>();
        std::vector<Poco::Nullable<std::string>> classifierSet;
        bool firstPass = _classifierNames->empty();
		if (firstPass) {
			recordClassifierNames(landUnitClassifierSet);
		}

        for (const auto& classifier : landUnitClassifierSet) {
			Poco::Nullable<std::string> classifierValue;
			if (!classifier.second.isEmpty()) {
				classifierValue = classifier.second.convert<std::string>();
			}

            classifierSet.push_back(classifierValue);
        }

        ClassifierSetRecord cSetRecord(classifierSet);
        auto storedCSetRecord = _classifierSetDimension->accumulate(cSetRecord);
        auto classifierSetRecordId = storedCSetRecord->getId();

        std::string landClass = _landClass->value().extract<std::string>();
		LandClassRecord landClassRecord(landClass);
        auto storedLandClassRecord = _landClassDimension->accumulate(landClassRecord);
        auto landClassRecordId = storedLandClassRecord->getId();

		TemporalLocationRecord locationRecord(
            classifierSetRecordId, dateRecordId, landClassRecordId, _landUnitArea);

        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        return storedLocationRecord->getId();
    }

    void CBMAggregatorLandUnitData::recordPoolsSet(Int64 locationId, bool isSpinup) {
        auto pools = _landUnitData->poolCollection();
        for (auto& pool : _landUnitData->poolCollection()) {
			PoolInfoRecord poolInfo(pool->name());
            auto poolId = _poolInfoDimension->search(poolInfo)->getId();
            double poolValue = pool->value() * _landUnitArea;
			PoolRecord poolRecord(locationId, poolId, poolValue);
            _poolDimension->accumulate(poolRecord);
        }
    }

    void CBMAggregatorLandUnitData::recordFluxSet(Int64 locationId) {
        // If Flux set is empty, return immediately.
        if (_landUnitData->getOperationLastAppliedIterator().empty()) {
            return;
        }

        for (auto operationResult : _landUnitData->getOperationLastAppliedIterator()) {
            const auto& metaData = operationResult->metaData();
			
			// Find the module info dimension record.
			auto disturbanceTypeName = metaData->disturbanceTypeName == ""
        		? "Annual Process"
        		: metaData->disturbanceTypeName;

			auto disturbanceType = disturbanceTypeName == "Annual Process"
        		? 0
        		: metaData->disturbanceType;

			ModuleInfoRecord moduleInfoRecord(
				metaData->libraryType, metaData->libraryInfoId,
				metaData->moduleType, metaData->moduleId, metaData->moduleName,
				disturbanceTypeName, disturbanceType);

			auto storedModuleInfoRecord = _moduleInfoDimension->accumulate(moduleInfoRecord);
			auto moduleInfoRecordId = storedModuleInfoRecord->getId();

			DisturbanceRecord disturbanceRecord(locationId, moduleInfoRecordId, _landUnitArea);
			_disturbanceDimension->accumulate(disturbanceRecord);

			for (auto it : operationResult->operationResultFluxCollection()) {
                auto srcIx = it->source();
                auto dstIx = it->sink();
                if (srcIx == dstIx) {
                    continue; // don't process diagonal - flux to & from same pool is ignored
                }

                auto fluxValue = it->value() * _landUnitArea;
                auto srcPool = _landUnitData->getPool(srcIx);
                auto dstPool = _landUnitData->getPool(dstIx);

                // Now have the required dimensions - look for the flux record.
				FluxRecord fluxRecord(
                    locationId, moduleInfoRecordId, getPoolId(srcPool),
                    getPoolId(dstPool), fluxValue);

                _fluxDimension->accumulate(fluxRecord);
            }
        }

        _landUnitData->clearLastAppliedOperationResults();
    }

	void CBMAggregatorLandUnitData::doError(std::string msg) {
		bool detailsAvailable = _spatialLocationInfo != nullptr;

		auto module = detailsAvailable ? _spatialLocationInfo->getProperty("module").convert<std::string>() : "unknown";
		ErrorRecord errorRec(module, msg);
		const auto storedError = _errorDimension->accumulate(errorRec);

		auto locationId = detailsAvailable ? recordLocation(true) : -1;
		LocationErrorRecord locErrRec(locationId, storedError->getId());
		_locationErrorDimension->accumulate(locErrRec);
	}

    void CBMAggregatorLandUnitData::doTimingInit() {
        _landUnitArea = _spatialLocationInfo->getProperty("landUnitArea");

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

    void CBMAggregatorLandUnitData::doLocalDomainInit() {
		for (auto& pool : _landUnitData->poolCollection()) {
			PoolInfoRecord poolInfoRecord(pool->name());
			_poolInfoDimension->accumulate(poolInfoRecord);
		}

        _spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(
            _landUnitData->getVariable("spatialLocationInfo")->value()
            .extract<std::shared_ptr<flint::IFlintData>>());

        _classifierSet = _landUnitData->getVariable(_classifierSetVar);
        _landClass = _landUnitData->getVariable("unfccc_land_class");
    }

    void CBMAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
