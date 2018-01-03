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
        recordPoolsSet(locationId);
        recordFluxSet(locationId);
		recordAgeArea(locationId);
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
        if (isSpinup) {
            const auto timing = _landUnitData->timing();
            DateRecord dateRecord(0, 0, 0, 0, 0, timing->stepLengthInYears());
            auto storedDateRecord = _dateDimension->accumulate(dateRecord);
            dateRecordId = storedDateRecord->getId();
        } else {
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

        Poco::Nullable<int> ageClassId;
        if (_landUnitData->hasVariable("age_class")) {
            ageClassId = _landUnitData->getVariable("age_class")->value().extract<int>();
        }

		TemporalLocationRecord locationRecord(
            classifierSetRecordId, dateRecordId, landClassRecordId, ageClassId, _landUnitArea);

        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        return storedLocationRecord->getId();
    }

    void CBMAggregatorLandUnitData::recordPoolsSet(Int64 locationId) {
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
        // Reserve 1 for non-forest stand with age < 0
        if (standAge < 0) {
            return 1;
        }

		// Calculate the age class as an integer starting from 2. In GCBM must use 2.0 for ageClassId offset:
        
        // The endpoint age of the first age class.
        int firstEndPoint = _ageClassRange - 1;
        
        // An offset of the age to ensure that the first age class will have the endpoint FIRSTENDPOINT.
        double offset = firstEndPoint - (_ageClassRange / 2.0) + 0.5;

        // The integral part of the age class as a double.		
        double temp;
        double classNum = ((standAge - offset) / _ageClassRange) + 2.0;
        if (modf(classNum, &temp) >= 0.5) {
            classNum = ceil(classNum);
        } else {
            classNum = floor(classNum);
        }

		// If the calculated age class is too great, use the oldest age class. 
		if ((int)classNum > _numAgeClasses) {
			classNum = _numAgeClasses;
		}

		return ((int)classNum);
	}

	void CBMAggregatorLandUnitData::recordAgeArea(Int64 locationId) {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = toAgeClass(standAge);

		AgeAreaRecord ageAreaRecord(locationId, ageClass, _landUnitArea);
		_ageAreaDimension->accumulate(ageAreaRecord);		
	}

    bool CBMAggregatorLandUnitData::hasDisturbanceInfo(std::shared_ptr<flint::IOperationResult> flux) {
        if (!flux->hasDataPackage()) {
            return false;
        }

        auto& disturbanceData = flux->dataPackage().extract<const DynamicObject>();
        for (const auto& disturbanceField : {
            "disturbance", "disturbance_type_code", "pre_disturbance_age_class"
        }) {
            if (!disturbanceData.contains(disturbanceField)) {
                return false;
            }
        }

        return true;
    }

    void CBMAggregatorLandUnitData::recordFluxSet(Int64 locationId) {
        // If Flux set is empty, return immediately.
        if (_landUnitData->getOperationLastAppliedIterator().empty()) {
            return;
        }

        for (auto operationResult : _landUnitData->getOperationLastAppliedIterator()) {
			// Find the module info dimension record.
            const auto& metaData = operationResult->metaData();
            ModuleInfoRecord moduleInfoRecord(
				metaData->libraryType, metaData->libraryInfoId,
				metaData->moduleType, metaData->moduleId, metaData->moduleName);

			auto moduleInfoRecordId = _moduleInfoDimension->accumulate(moduleInfoRecord)->getId();

            Poco::Nullable<Int64> distRecordId;
            if (hasDisturbanceInfo(operationResult)) {
                auto& disturbanceData = operationResult->dataPackage().extract<const DynamicObject>();
                auto disturbanceType = disturbanceData["disturbance"].convert<std::string>();
                int disturbanceTypeCode = disturbanceData["disturbance_type_code"];
                DisturbanceTypeRecord distTypeRecord(disturbanceTypeCode, disturbanceType);
                auto distTypeRecordId = _disturbanceTypeDimension->accumulate(distTypeRecord)->getId();

                Poco::Nullable<int> preDisturbanceAgeClass = disturbanceData["pre_disturbance_age_class"];
                DisturbanceRecord disturbanceRecord(locationId, distTypeRecordId, preDisturbanceAgeClass, _landUnitArea);
                distRecordId = _disturbanceDimension->accumulate(disturbanceRecord)->getId();
            }

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
                    locationId, moduleInfoRecordId, distRecordId,
                    getPoolId(srcPool), getPoolId(dstPool), fluxValue);

                _fluxDimension->accumulate(fluxRecord);
            }
        }

        _landUnitData->clearLastAppliedOperationResults();
    }

	void CBMAggregatorLandUnitData::doError(std::string msg) {
		bool detailsAvailable = _spatialLocationInfo != nullptr;

		auto module = detailsAvailable ? _spatialLocationInfo->getProperty("module").convert<std::string>() : "unknown";
		ErrorRecord errorRec(module, msg);
		auto errorRecId = _errorDimension->accumulate(errorRec)->getId();

		auto locationId = detailsAvailable ? recordLocation(true) : -1;
		LocationErrorRecord locErrRec(locationId, errorRecId);
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
		//Reserve ageClassID 1 for non-forest 1 [-1,-1]
		AgeClassRecord ageClassRecord(-1, -1);
		_ageClassDimension->accumulate(ageClassRecord);
		for (int ageClassNumber = 1; ageClassNumber < _numAgeClasses; ageClassNumber++) {
			int start_age = (ageClassNumber - 1) * _ageClassRange;
			int end_age = ageClassNumber * _ageClassRange - 1;
			AgeClassRecord ageClassRecord(start_age, end_age);
			_ageClassDimension->accumulate(ageClassRecord);
		}
	}

    void CBMAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
