#include "moja/modules/cbm/CBMAggregatorLandUnitData.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorLandUnitData::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorLandUnitData::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::TimingInit	 , &CBMAggregatorLandUnitData::onTimingInit		, *this);
        notificationCenter.subscribe(signals::OutputStep	 , &CBMAggregatorLandUnitData::onOutputStep		, *this);
    }

    Int64 CBMAggregatorLandUnitData::getPoolId(flint::IPool::ConstPtr pool) {
        auto poolInfo = std::make_shared<PoolInfoRecord>(pool->name());
        return _poolInfoDimension->search(poolInfo)->getId();
    }

    void CBMAggregatorLandUnitData::recordLandUnitData(bool isSpinup) {
        auto locationId = recordLocation(isSpinup);
        recordPoolsSet(locationId, isSpinup);
        recordFluxSet(locationId);
    }

    Int64 CBMAggregatorLandUnitData::recordLocation(bool isSpinup) {
        Int64 dateRecordId = -1;
        if (!isSpinup) {
            // Find the date dimension record.
            const auto timing = _landUnitData->timing();
            auto dateRecord = std::make_shared<DateRecord>(
                timing->step(), timing->curStartDate().year(),
                timing->curStartDate().month(), timing->curStartDate().day(),
                timing->fractionOfStep(), timing->stepLengthInYears());

            auto storedDateRecord = _dateDimension->accumulate(dateRecord);
            auto dateRecordId = storedDateRecord->getId();
        }

        // Classifier set information.
        const auto& landUnitClassifierSet = _classifierSet->value().extract<DynamicObject>();
        std::vector<std::string> classifierSet;
        bool firstPass = _classifierNames->empty();
        for (const auto& classifier : landUnitClassifierSet) {
            if (firstPass) {
                std::string name = classifier.first;
                std::replace(name.begin(), name.end(), '.', ' ');
                std::replace(name.begin(), name.end(), ' ', '_');
                _classifierNames->insert(name);
            }

            classifierSet.push_back(classifier.second);
        }

        auto cSetRecord = std::make_shared<ClassifierSetRecord>(classifierSet);
        auto storedCSetRecord = _classifierSetDimension->accumulate(cSetRecord);
        auto classifierSetRecordId = storedCSetRecord->getId();

        std::string landClass = _landClass->value().extract<std::string>();
        auto landClassRecord = std::make_shared<LandClassRecord>(landClass);
        auto storedLandClassRecord = _landClassDimension->accumulate(landClassRecord);
        auto landClassRecordId = storedLandClassRecord->getId();

        auto locationRecord = std::make_shared<TemporalLocationRecord>(
            classifierSetRecordId, dateRecordId, landClassRecordId, _landUnitArea);

        auto storedLocationRecord = _locationDimension->accumulate(locationRecord);
        return storedLocationRecord->getId();
    }

    void CBMAggregatorLandUnitData::recordPoolsSet(Int64 locationId, bool isSpinup) {
        auto pools = _landUnitData->poolCollection();
        for (auto& pool : _landUnitData->poolCollection()) {
            auto poolInfo = std::make_shared<PoolInfoRecord>(pool->name());
            auto poolId = _poolInfoDimension->search(poolInfo)->getId();
            double poolValue = pool->value() * _landUnitArea;
            auto poolRecord = std::make_shared<PoolRecord>(locationId, poolId, poolValue);
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
            for (auto it : operationResult->operationResultFluxCollection()) {
                auto srcIx = it->source();
                auto dstIx = it->sink();
                if (srcIx == dstIx) {
                    continue; // don't process diagonal - flux to & from same pool is ignored
                }

                auto fluxValue = it->value() * _landUnitArea;
                auto srcPool = _landUnitData->getPool(srcIx);
                auto dstPool = _landUnitData->getPool(dstIx);

                // Find the module info dimension record.
                auto moduleInfoRecord = std::make_shared<ModuleInfoRecord>(
                    metaData->libraryType, metaData->libraryInfoId,
                    metaData->moduleType, metaData->moduleId, metaData->moduleName,
                    metaData->disturbanceType);

                auto storedModuleInfoRecord = _moduleInfoDimension->accumulate(moduleInfoRecord);
                auto moduleInfoRecordId = storedModuleInfoRecord->getId();

                // Now have the required dimensions - look for the flux record.
                auto fluxRecord = std::make_shared<FluxRecord>(
                    locationId, moduleInfoRecordId, getPoolId(srcPool),
                    getPoolId(dstPool), fluxValue);

                _fluxDimension->accumulate(fluxRecord);
            }
        }
    }

    void CBMAggregatorLandUnitData::onTimingInit() {
        _landUnitArea = _spatialLocationInfo->_landUnitArea;

        // Record post-spinup pool values.
        recordLandUnitData(true);
    }

    void CBMAggregatorLandUnitData::onLocalDomainInit() {
        for (auto& pool : _landUnitData->poolCollection()) {
            auto poolInfoRecord = std::make_shared<PoolInfoRecord>(pool->name());
            _poolInfoDimension->insert(pool->idx(), poolInfoRecord);
        }

        _spatialLocationInfo = std::static_pointer_cast<flint::SpatialLocationInfo>(
            _landUnitData->getVariable("spatialLocationInfo")->value()
            .extract<std::shared_ptr<flint::IFlintData>>());

        _classifierSet = _landUnitData->getVariable("classifier_set");
        _landClass = _landUnitData->getVariable("unfccc_land_class");
    }

    void CBMAggregatorLandUnitData::onOutputStep() {
        recordLandUnitData(false);
    }

}}} // namespace moja::modules::cbm
