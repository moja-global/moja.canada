/**
* @file
* @brief The brief description goes here.
*
* The detailed description if any, goes here
* ******/
#include "moja/modules/cbm/cbmflataggregatorlandunitdata.h"
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
#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    /**
    * @brief Configuration function.
    *
    * Initialise CBMFlatAggregatorLandUnitData._classifierSetVar as config["reporting_classifier_set"] (string) if it exists, \n
    * else initialise it to "classifier_set"
    *
    * @param config DynamicObject&
    * @return void
    * ************************/
	void CBMFlatAggregatorLandUnitData::configure(const DynamicObject& config) {
		if (config.contains("reporting_classifier_set")) {
			_classifierSetVar = config["reporting_classifier_set"].extract<std::string>();
		} else {
			_classifierSetVar = "classifier_set";
		}
	}

    /**
    * @brief Subscribe to the signals localDomainInit,TimingInit,OutputStep and Error 
    * 
    * 
    * @param notificationCenter NotificationCenter&
	* @return void
	* ************************/


	void CBMFlatAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMFlatAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMFlatAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMFlatAggregatorLandUnitData::onOutputStep		, *this);
		notificationCenter.subscribe(signals::Error			 , &CBMFlatAggregatorLandUnitData::onError			, *this);
    }

    /**
    * @brief Record land unit data.
    * 
    * Invoke recordLocation by using isSpinup as a parameter and assign it to a variable location.
    * if isSpinup is true, assign CBMFlatAggregatorLandUnitData._previousAttributes as location variable.
    * Invoke recordPoolsSet and recordFluxSet by using isSpinUp as the parameter.
    * Assign CBMFlatAggregatorLandUnitData._previousAttributes as location variable. 
    * 
    * @param isSpinup bool
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto location = recordLocation(isSpinup);
        if (isSpinup) {
            _previousAttributes = location;
        }

        recordPoolsSet(location);
        recordFluxSet(location);

        _previousAttributes = location;
    }
    /**
    * @brief Add classifierSet into CBMFlatAggregatorLandUnitData._classifierNames.
    * 
    * @param classifierSet DynamicObject&
	* @return void
	* ************************/
	void CBMFlatAggregatorLandUnitData::recordClassifierNames(const DynamicObject& classifierSet) {
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
    * @brief Record location.
    * 
    * Initialise integer variable year as 0.
    * 
    * If isSpinup is false, initialise a constant variable timimg as _landUnitData->timing().
    * Initialise a private variable year as timing->curStartDate().year().
    * 
    * Extract the DynamicObject from _classifierSet value and assign it to landUnitClassifierSet variable.
    * If CBMFlatAggregatorLandUnitData._classifierNames is empty, invoke recordClassifierNames() using the landUnitClassiferSet as a parameter.
    * If each classifier in landUnitClassifierSet is empty,initialise constant variable timeseries as classifer.second (TimeSeries).
    * Initialise a private variable classiferValue as timeseries (string).
    * If not, Initialise it as classifer.second (string).
    * Add classiferValue to CBMFlatAggregatorLandUnitData._classifierSet.
    * 
    * Initialise a string variable landClass as _landClass->value() (string).
    * 
    * If _landUnitData variable contains age_class variable, 
    * The value of the age_class variable is assigned to int variable ageClassId.
    * Initialise a private variable ageClass as CBMFlatAggregatorLandUnitData._ageClassHelper.getAgeClassString() using ageClassId as the parameter.
    * 
    * Invoke locationRecord() using year,classifierSet,landClass,ageClass and CBMFlatAggregatorLandUnitData._landUnitArea as the parameters
    * Invoke accumulate() using locationRecord variable as the parameter.
    * Return locationRecord.
    * 
    * @param isSpinup bool
	* @return FlatAgeAreaRecord
	* ************************/
    FlatAgeAreaRecord CBMFlatAggregatorLandUnitData::recordLocation(bool isSpinup) {
        int year = 0;
        if (!isSpinup) {
            const auto timing = _landUnitData->timing();
            year = timing->curStartDate().year();
        }

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
        
        std::string landClass = _landClass->value().extract<std::string>();

        std::string ageClass = "";
        if (_landUnitData->hasVariable("age_class")) {
            int ageClassId = _landUnitData->getVariable("age_class")->value();
            ageClass = _ageClassHelper.getAgeClassString(ageClassId);
        }

        FlatAgeAreaRecord locationRecord(year, classifierSet, landClass, ageClass, _landUnitArea);
        _ageDimension->accumulate(locationRecord);

        return locationRecord;
    }

    /**
    * @brief recordPoolsSet
    * 
    * Initialise double variable poolValue as pool->value() *  CBMFlatAggregatorLandUnitData._landUnitArea for each pool collection.
    * Invoke poolRecord() using location.getYear(),location.getClassifierValues(),location.getLandClass(),
    * location.getAgeClass(),pool->name() and poolValue as parameters.
    * Invoke CBMFlatAggregatorLandUnitData._poolDimention->accumulate() using poolRecord as a parameter.
    * 
    * @param location FlatAgeAreaRecord&
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::recordPoolsSet(const FlatAgeAreaRecord& location) {
        auto pools = _landUnitData->poolCollection();
        for (auto& pool : _landUnitData->poolCollection()) {
            double poolValue = pool->value() * _landUnitArea;
            if (poolValue == 0) {
                continue;
            }

            FlatPoolRecord poolRecord(location.getYear(), location.getClassifierValues(), location.getLandClass(),
                location.getAgeClass(), pool->name(), poolValue);

            _poolDimension->accumulate(poolRecord);
        }
    }

    /**
    * @brief Check DisturbanceInfo
    * 
    * If flux->hasDataPackage() is false return a false boolean value.
    * Initialise a variable disturbanceData as flux->dataPackage() (DynamicObject).
    * For each constant variable disturbanceField in an array("disturbance","disturbance_type_code"),
    * if disturbanceData variable does not contain disturbanceField, return a false boolean value.
    * Return a true boolean value.
    * 
    * @param flux shared_ptr<IoperationResult>
	* @return bool
	* ************************/
    bool CBMFlatAggregatorLandUnitData::hasDisturbanceInfo(std::shared_ptr<flint::IOperationResult> flux) {
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
    * @param location FlatAgeAreaRecord
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::recordFluxSet(const FlatAgeAreaRecord& location) {
        if (_landUnitData->getOperationLastAppliedIterator().empty()) {
            return;
        }

        for (auto operationResult : _landUnitData->getOperationLastAppliedIterator()) {
            Poco::Nullable<std::string> disturbanceType;
            Poco::Nullable<int> disturbanceCode;
            if (hasDisturbanceInfo(operationResult)) {
                auto& disturbanceData = operationResult->dataPackage().extract<const DynamicObject>();
                disturbanceType = disturbanceData["disturbance"].convert<std::string>();
                disturbanceCode = disturbanceData["disturbance_type_code"].extract<int>();

                FlatDisturbanceRecord disturbanceRecord(location.getYear(), location.getClassifierValues(), location.getLandClass(),
                    location.getAgeClass(), _previousAttributes->getClassifierValues(), _previousAttributes->getLandClass(),
                    _previousAttributes->getAgeClass(), disturbanceType, disturbanceCode, _landUnitArea);

                _disturbanceDimension->accumulate(disturbanceRecord);
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

                FlatFluxRecord fluxRecord(location.getYear(), location.getClassifierValues(), location.getLandClass(),
                    location.getAgeClass(), _previousAttributes->getClassifierValues(), _previousAttributes->getLandClass(),
                    _previousAttributes->getAgeClass(), disturbanceType, disturbanceCode, srcPool->name(), dstPool->name(), fluxValue);

                _fluxDimension->accumulate(fluxRecord);
            }
        }

        _landUnitData->clearLastAppliedOperationResults();
    }

    /**
    * @brief doError
    * 
    * Initialise a boolean variable detailsAvailable as CBMFlatAggregatorLandUnitData._spatialLocationInfo != nullptr.
    * If detailsAvailable is true, get module property,convert to string and assign the value to module.
    * If false assign "unknown" to module.
    * 
    * If detailsAvailable is true, invoke recordLocation() using a true boolean value as a parameter and assign the value to location variable.
    * Invoke errorRecord() using location.getYear(),location.getClassifierValues(),module,msg and CBMFlatAggregatorLandUnitData._landUnitArea as parameters.
    * If not, invoke errorRecord() using integer 0,vector<Nullable<string>>(),module, msg and CBMFlatAggregatorLandUnitData._landUnitArea as parameters.
    * Invoke CBMFlatAggregatorLandUnitData._errorDimension.accumulate() using errorRecord as a parameter.
    * 
    * @param msg string
	* @return void
	* ************************/
	void CBMFlatAggregatorLandUnitData::doError(std::string msg) {
		bool detailsAvailable = _spatialLocationInfo != nullptr;
		auto module = detailsAvailable ? _spatialLocationInfo->getProperty("module").convert<std::string>() : "unknown";
		
        if (detailsAvailable) {
            auto location = recordLocation(true);
            FlatErrorRecord errorRecord(location.getYear(), location.getClassifierValues(),
                module, msg, _landUnitArea);
            
            _errorDimension->accumulate(errorRecord);
        } else {
            FlatErrorRecord errorRecord(0, std::vector<Poco::Nullable<std::string>>(),
                module, msg, _landUnitArea);

            _errorDimension->accumulate(errorRecord);
        }
	}

    /**
    * @brief doTimingInit
    * 
    * Get landUnitArea property and assign it to CBMFlatAggregatorLandUnitData._landUnitArea variable.
    * Invoke recordLandUnitData() using a true boolean value as a parameter.
    * 
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::doTimingInit() {
        _landUnitArea = _spatialLocationInfo->getProperty("landUnitArea");

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

    /**
    * @brief doLocalDomainInit
    * 
    * Initialise CBMFlatAggregatorLandUnitData._spatialLocationInfo as static_pointer_cast<flint::SpatialLocationInfo>(
    * _landUnitData->getVariable("spatialLocationInfo")->value()) (shared_ptr<IFlintData>).
    * 
    * Get variable CBMFlatAggregatorLandUnitData._classifierSetVar and assign it to CBMFlatAggregatorLandUnitData._classifierSet.
    * Get variable unfccc_land_class and assign it to CBMFlatAggregatorLandUnitData._landClass.
    * 
    * If _landUnitData variable contains age_class_range and age_maximum variables,
    * Initialise an integer variable ageClassRange as age_class_range value.
    * Initialise an integer variable ageMaximum as age_maximum value.
    * Invoke AgeClassHelper() using ageClassRange,ageMaximum as parameters and assign it to CBMFlatAggregatorLandUnitData._ageClassHelper.
    * 
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::doLocalDomainInit() {
        _spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(
            _landUnitData->getVariable("spatialLocationInfo")->value()
            .extract<std::shared_ptr<flint::IFlintData>>());

        _classifierSet = _landUnitData->getVariable(_classifierSetVar);
        _landClass = _landUnitData->getVariable("unfccc_land_class");

        if (_landUnitData->hasVariable("age_class_range") && _landUnitData->hasVariable("age_maximum")) {
            int ageClassRange = _landUnitData->getVariable("age_class_range")->value();
            int ageMaximum = _landUnitData->getVariable("age_maximum")->value();
            _ageClassHelper = AgeClassHelper(ageClassRange, ageMaximum);
        }
    }

    /**
    * @brief Invoke recordLandUnitData using a false boolean value as a paramater.
    * 
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
