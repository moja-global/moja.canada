/**
* @file
*/
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
     * Configuration function
     * 
     * Assign CBMFlatAggregatorLandUnitData._classifierSetVar as value of variable "reporting_classifier_set" in parameter config if it exists, \n
     * else to string "classifier_set"
     * @param config DynamicObject&
     *  @return void
     **************************/
	void CBMFlatAggregatorLandUnitData::configure(const DynamicObject& config) {
		if (config.contains("reporting_classifier_set")) {
			_classifierSetVar = config["reporting_classifier_set"].extract<std::string>();
		} else {
			_classifierSetVar = "classifier_set";
		}
	}

    /**
     * Subscribe to the signals LocalDomainInit, TimingInit, OutputStep and Error 
     * 
     * @param notificationCenter NotificationCenter&
     * @return void
     **************************/
	void CBMFlatAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMFlatAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMFlatAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMFlatAggregatorLandUnitData::onOutputStep		, *this);
		notificationCenter.subscribe(signals::Error			 , &CBMFlatAggregatorLandUnitData::onError			, *this);
    }

    /**
    * If parameter isSpinup is true, assign CBMFlatAggregatorLandUnitData._previousAttributes the result of the function CBMFlatAggregatorLandUnitData.recordLocation() \n
    * Invoke CBMFlatAggregatorLandUnitData.recordPoolsSet() and CBMFlatAggregatorLandUnitData.recordFluxSet() by using isSpinUp as the parameter \n
    * Assign CBMFlatAggregatorLandUnitData._previousAttributes the result of the function CBMFlatAggregatorLandUnitData.recordLocation()
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
    * Add each classifier in parameter classifierSet to CBMFlatAggregatorLandUnitData._classifierNames, \n 
    * if CBMFlatAggregatorLandUnitData._classifierNames is not empty
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
    * If parameter isSpinup is false, get the current year of the simulation from _landUnitData \n
    * If CBMAggregatorLandUnitData._classifierNames is empty, invoke CBMAggregatorLandUnitData.recordClassifierNames() \n
    * For each classifier in  CBMAggregatorLandUnitData._classifierSet, append classifier.second to a variable classifierSet \n
    * Instantiate an object of FlatAgeAreaRecord with year, classifierSet, value of CBMFlatAggregatorLandUnitData._landClass, 
    * value of variable "age_class" in _landUnitData if it exists else "", _landUnitArea, invoke accumulate() on 
    * CBMFlatAggregatorLandUnitData._ageDimension with argument object and return the object
    * 
    * @param isSpinup bool
    * @return Int64 
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
    * For each pool in poolCollection() of _landUnitData, calculate the poolValue as value of pool * _landUnitArea \n
    * If poolValue is not 0, instantiate an object of FlatPoolRecord and call accumulate on CBMFlatAggregatorLandUnitData._poolDimension with argument object
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
    * If hasDataPackage() on parameter flux is false return false \n
    * Extract DynamicObject from flux->dataPackage(), if it contains both "disturbance" and "disturbance_type_code", return true, else return false
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
    * If getOperationLastAppliedIterator() on _landUnitData is not empty, \n
    * for each operationResult in _landUnitData->getOperationLastAppliedIterator(), instantiate an object of FlatDisturbanceRecord and invoke accumulate() on 
    * CBMFlatAggregatorLandUnitData._disturbanceDimension with argument object \n
    * For each operation in operationResult, if the source and destination pools are not the same, instantiate an object of FlatFluxRecord and invoke accumulate() on 
    * CBMFlatAggregatorLandUnitData._fluxDimension with the argument object \n
    * Invoke clearLastAppliedOperationResults() on _landUnitData
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
    * If CBMFlatAggregatorLandUnitData._spatialLocationInfo is not nullptr, then instantiate an object of FlatErrorRecord with the year, classifier  
    * of the current location using CBMFlatAggregatorLandUnitData.recordLocation(), value of property "module" in CBMFlatAggregatorLandUnitData._spatialLocationInfo \n
    * else use the 0 and null string for the parameter 1 and 2 \n
    * Invoke accumulate() on CBMFlatAggregatorLandUnitData._errorDimension with argument object
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
    * Assign CBMFlatAggregatorLandUnitData._landUnitArea value of property "landUnitArea" in CBMFlatAggregatorLandUnitData._spatialLocationInfo \n
    * Invoke CBMFlatAggregatorLandUnitData.recordLandUnitData() using a true boolean argument
    * 
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::doTimingInit() {
        _landUnitArea = _spatialLocationInfo->getProperty("landUnitArea");

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

    /**
    * Assign CBMFlatAggregatorLandUnitData._spatialLocationInfo, CBMFlatAggregatorLandUnitData._landClass values of variables 
    * "spatialLocationInfo" and "unfccc_land_class", CBMFlatAggregatorLandUnitData._classifierSet value of  CBMFlatAggregatorLandUnitData._classifierSet in _landUnitData \n
    * If _landUnitData contains variables "age_class_range" and "age_maximum", create an object of AgeClassHelper, \n
    * assign it to CBMFlatAggregatorLandUnitData._ageClassHelper
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
    * Invoke CBMFlatAggregatorLandUnitData.recordLandUnitData() with argument as boolean false
    * 
	* @return void
	* ************************/
    void CBMFlatAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
