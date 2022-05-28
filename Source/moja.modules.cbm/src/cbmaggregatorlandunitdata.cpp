/**
 * @file
 * @brief The brief description goes here.
 *
 * The detailed description if any, goes here
 * ******/

#include "moja/modules/cbm/cbmaggregatorlandunitdata.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ivariable.h>
#include <moja/flint/ioperationresult.h>
#include <moja/flint/ioperationresultflux.h>

#include <moja/flint/itiming.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/lexical_cast.hpp>

namespace moja {
namespace modules {
namespace cbm {

    /**
    * @brief configuration function
    * 
    * Assign the reporting classifier set to the classifier set variable
    * if available. if not, assigns "classifier_set" to it.
    *
    * 
    * @param config DynamicObject&
    * @return void
    * ************************/

	void CBMAggregatorLandUnitData::configure(const DynamicObject& config) {
		if (config.contains("reporting_classifier_set")) {
			_classifierSetVar = config["reporting_classifier_set"].extract<std::string>();
		} else {
			_classifierSetVar = "classifier_set";
		}
	}

    /**
    * @brief subcribe to FLINT
    * 
    * @param notificationCenter NotificationCenter&
    * @return void
    * ************************/

	void CBMAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMAggregatorLandUnitData::onOutputStep		, *this);
		notificationCenter.subscribe(signals::Error			 , &CBMAggregatorLandUnitData::onError			, *this);
    }

    /**
    * @brief Return the pool id.
    * 
    * @param pool IPool*
    * @return Int64
    * ************************/

    Int64 CBMAggregatorLandUnitData::getPoolId(const flint::IPool* pool) {
        PoolInfoRecord poolInfo(pool->name());
        return _poolInfoDimension->search(poolInfo)->getId();
    }

    /**
    * @brief recordLandUnitData
    * 
    * Assign the locationId and records the pools set, flux set
    * and age area using the location id.
    * 
    * @param isSpinup bool
    * @return void
    * ************************/

    void CBMAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto locationId = recordLocation(isSpinup);
        if (isSpinup) {
            _previousLocationId = locationId;
        }

        recordPoolsSet(locationId);
        recordFluxSet(locationId);
		recordAgeArea(locationId);

        _previousLocationId = locationId;
    }

    /**
    * @brief recordClassifierNames
    * 
    * Records classifier names.
    * 
    * 
    * @param classifierSet DynamicObject&
    * @return void
    * ************************/

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

    /**
    * @brief recordLocation
    * 
    * Detailed description here
    * 
    * @param isSpinup bool
    * @return Int64 
    * ************************/

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
                if (classifier.second.type() == typeid(TimeSeries)) {
                    const auto timeseries = classifier.second.extract<TimeSeries>();
                    classifierValue = boost::lexical_cast<std::string>(timeseries.value());
                } else {
                    classifierValue = classifier.second.convert<std::string>();
                }
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

        Poco::Nullable<Int64> ageClassId;
        if (_landUnitData->hasVariable("age_class")) {
            Int64 ageClass = _landUnitData->getVariable("age_class")->value();
            auto ageClassRange = _ageClassHelper.getAgeClass(ageClass);
            AgeClassRecord ageClassRecord(std::get<0>(ageClassRange), std::get<1>(ageClassRange));
            ageClassId = _ageClassDimension->accumulate(ageClassRecord)->getId();
        }

		TemporalLocationRecord locationRecord(
            classifierSetRecordId, dateRecordId, landClassRecordId, ageClassId, _landUnitArea);

        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        return storedLocationRecord->getId();
    }

    /**
    * @brief recordPoolsSet
    * 
    * Detailed description here
    * 
    * @param locationId Int64
    * @return void
    * ************************/

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

    /**
    * @brief recordAgeArea
    * 
    * Detailed description here
    * 
    * @param locationId Int64
    * @return void
    * ************************/

	void CBMAggregatorLandUnitData::recordAgeArea(Int64 locationId) {
		int standAge = _landUnitData->getVariable("age")->value();
		int ageClass = _ageClassHelper.toAgeClass(standAge);
        auto ageClassRange = _ageClassHelper.getAgeClass(ageClass);
        auto ageClassRecord = AgeClassRecord(std::get<0>(ageClassRange), std::get<1>(ageClassRange));
        auto ageClassId = _ageClassDimension->accumulate(ageClassRecord)->getId();
		AgeAreaRecord ageAreaRecord(locationId, ageClassId, _landUnitArea);
		_ageAreaDimension->accumulate(ageAreaRecord);		
	}

    /**
    * @brief hasDisturbanceInfo
    *
    * Detailed description here
    *
    * @param flux shared_ptr<IOperationResult>
    * @return bool
    * ************************/
    bool CBMAggregatorLandUnitData::hasDisturbanceInfo(std::shared_ptr<flint::IOperationResult> flux) {
        if (!flux->hasDataPackage()) {
            return false;
        }

        auto& disturbanceData = flux->dataPackage().extract<const DynamicObject>();
        for (const auto& disturbanceField : {
            "disturbance", "disturbance_type_code"
        }) {
            if (!disturbanceData.contains(disturbanceField)) {
                return false;
            }
        }

        return true;
    }

    /**
    * @brief recordFluxSet
    *
    * Detailed description here
    *
    * @param locationId Int64
    * @return void
    * ************************/

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
                DisturbanceRecord disturbanceRecord(locationId, distTypeRecordId, _previousLocationId, _landUnitArea);
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
    /**
    * @brief doError
    *
    * Detailed description here
    *
    * @param msg string
    * @return void
    * ************************/

	void CBMAggregatorLandUnitData::doError(std::string msg) {
		bool detailsAvailable = _spatialLocationInfo != nullptr;

		auto module = detailsAvailable ? _spatialLocationInfo->getProperty("module").convert<std::string>() : "unknown";
		ErrorRecord errorRec(module, msg);
		auto errorRecId = _errorDimension->accumulate(errorRec)->getId();

        Poco::Nullable<Int64> locationId;
        if (detailsAvailable) {
            locationId = recordLocation(true);
        }

		LocationErrorRecord locErrRec(locationId, errorRecId);
		_locationErrorDimension->accumulate(locErrRec);
	}

    /**
    * @brief initiate timing
    *
    * Detailed description here
    *
    * @return void
    * ************************/

    void CBMAggregatorLandUnitData::doTimingInit() {
        _landUnitArea = _spatialLocationInfo->getProperty("landUnitArea");

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

    /**
    * @brief initiate local domain
    *
    * Initialize spatial location info, classifier set and land class.
    *
    * @return void
    * ************************/

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

    /**
    * @brief recordAgeClass
    *
    * Detailed description here
    *
    * @return void
    * ************************/

	void CBMAggregatorLandUnitData::recordAgeClass() {
        if (_landUnitData->hasVariable("age_class_range") && _landUnitData->hasVariable("age_maximum")) {
            int ageClassRange = _landUnitData->getVariable("age_class_range")->value();
            int ageMaximum = _landUnitData->getVariable("age_maximum")->value();
            _ageClassHelper = AgeClassHelper(ageClassRange, ageMaximum);
        }

        for (auto& ageClass : _ageClassHelper.getAgeClasses()) {
            auto& ageClassRange = ageClass.second;
			AgeClassRecord ageClassRecord(std::get<0>(ageClassRange), std::get<1>(ageClassRange));
			_ageClassDimension->accumulate(ageClassRecord);
		}
	}

    /**
    * @brief Record the land unit data using a false boolean value
    *
    * @return void
    * ************************/

    void CBMAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
