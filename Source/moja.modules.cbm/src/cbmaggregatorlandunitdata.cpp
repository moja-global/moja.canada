#include "moja/modules/cbm/cbmaggregatorlandunitdata.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ivariable.h>
#include <moja/flint/ioperationresult.h>
#include <moja/flint/ioperationresultflux.h>

#include <moja/itiming.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

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

    Int64 CBMAggregatorLandUnitData::getPoolId(const flint::IPool* pool) {
        PoolInfoRecord poolInfo(pool->name());
        return _poolInfoDimension->search(poolInfo)->getId();
    }

    void CBMAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto locationId = recordLocation(isSpinup);
        recordPoolsSet(locationId, isSpinup);
        recordFluxSet(locationId);
		recordAgeArea(locationId, isSpinup);
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
		auto testLocationID = _landUnitData->getVariable("LandUnitId");
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

	int CBMAggregatorLandUnitData::toAgeClass(int standAge) {		
		//age class ID start from 1
		// ID 1 [0, 0]
		// ID 2 [1, ageClassRange]

		if (standAge <= 0) {
			return 1;
		}

		int ageClassIdOffset = 2;
		int ageClassId = ageClassIdOffset + ((standAge - 1) / _ageClassRange);
		if (ageClassId > _numAgeClasses) {
			ageClassId = _numAgeClasses;
		}

		return ageClassId;
	}

	void CBMAggregatorLandUnitData::recordAgeArea(Int64 locationId, bool isSpinup) {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = toAgeClass(standAge);

		AgeAreaRecord ageAreaRecord(locationId, ageClass, _landUnitArea);
		_ageAreaDimension->accumulate(ageAreaRecord);		
	}

    void CBMAggregatorLandUnitData::recordFluxSet(Int64 locationId) {
        // If Flux set is empty, return immediately.
        if (_landUnitData->getOperationLastAppliedIterator().empty()) {
            return;
        }

        for (auto operationResult : _landUnitData->getOperationLastAppliedIterator()) {
            const auto& metaData = operationResult->metaData();

			std::string disturbanceTypeName = "Annual Process";
			int disturbanceTypeCode = 0;

			if (operationResult->hasDataPackage()) {
				auto& disturbanceData = operationResult->dataPackage().extract<const DynamicObject>();
				disturbanceTypeName = disturbanceData["disturbance"].convert<std::string>();
				disturbanceTypeCode = disturbanceData["disturbance_type_code"];
			}

			// Find the module info dimension record.
			ModuleInfoRecord moduleInfoRecord(
				metaData->libraryType, metaData->libraryInfoId,
				metaData->moduleType, metaData->moduleId, metaData->moduleName,
				disturbanceTypeName, disturbanceTypeCode);

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
		recordAgeClass();
    }

	void CBMAggregatorLandUnitData::recordAgeClass() {
		_ageClassRange = 20; // default age class range
		if (_landUnitData->hasVariable("age_class_range")) {
			_ageClassRange = _landUnitData->getVariable("age_class_range")->value();
		}

		int ageMaximum = 300; // default maximum age
		if (_landUnitData->hasVariable("age_maximum")) {
			ageMaximum = _landUnitData->getVariable("age_maximum")->value();
		}

		_numAgeClasses = 1 + ageMaximum / _ageClassRange;
		int start_age = 0;
		int end_age = 0;
		for (int ageClassNumber = 0; ageClassNumber < _numAgeClasses; ageClassNumber++) {
			if (ageClassNumber > 0) {
				start_age = (ageClassNumber - 1) * _ageClassRange + 1;
				end_age = ageClassNumber * _ageClassRange;
			}

			AgeClassRecord ageClassRecord(start_age, end_age);
			_ageClassDimension->accumulate(ageClassRecord);
		}
	}

    void CBMAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
