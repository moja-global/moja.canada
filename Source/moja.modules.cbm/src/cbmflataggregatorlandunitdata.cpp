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

	void CBMFlatAggregatorLandUnitData::configure(const DynamicObject& config) {
		if (config.contains("reporting_classifier_set")) {
			_classifierSetVar = config["reporting_classifier_set"].extract<std::string>();
		} else {
			_classifierSetVar = "classifier_set";
		}
	}

	void CBMFlatAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMFlatAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMFlatAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMFlatAggregatorLandUnitData::onOutputStep		, *this);
		notificationCenter.subscribe(signals::Error			 , &CBMFlatAggregatorLandUnitData::onError			, *this);
    }

    void CBMFlatAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto location = recordLocation(isSpinup);
        if (isSpinup) {
            _previousAttributes = location;
        }

        recordPoolsSet(location);
        recordFluxSet(location);

        _previousAttributes = location;
    }

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

    void CBMFlatAggregatorLandUnitData::doTimingInit() {
        _landUnitArea = _spatialLocationInfo->getProperty("landUnitArea");

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

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

    void CBMFlatAggregatorLandUnitData::doOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
