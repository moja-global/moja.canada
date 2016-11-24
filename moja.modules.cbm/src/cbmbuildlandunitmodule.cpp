#include "moja/modules/cbm/cbmbuildlandunitmodule.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMBuildLandUnitModule::configure(const DynamicObject& config) { }

    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit		, &CBMBuildLandUnitModule::onLocalDomainInit	, *this);
		notificationCenter.subscribe(signals::PreTimingSequence		, &CBMBuildLandUnitModule::onPreTimingSequence	, *this);
	}

    void CBMBuildLandUnitModule::onLocalDomainInit() {
        _initialAge = _landUnitData->getVariable("initial_age");
        _buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
        _initialCSet = _landUnitData->getVariable("initial_classifier_set");
        _cset = _landUnitData->getVariable("classifier_set");
        _initialHistoricLandClass = _landUnitData->getVariable("initial_historic_land_class");
        _initialCurrentLandClass = _landUnitData->getVariable("initial_current_land_class");
        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
        _isForest = _landUnitData->getVariable("is_forest");
    }

    void CBMBuildLandUnitModule::onPreTimingSequence() {
        auto initialCSet = _initialCSet->value();
        _cset->set_value(initialCSet);

        auto historicLandClass = _initialHistoricLandClass->value();
        _historicLandClass->set_value(historicLandClass);

        auto currentLandClass = _initialCurrentLandClass->value();
        if (currentLandClass.isEmpty()) {
            _currentLandClass->set_value(historicLandClass);
        } else {
            _currentLandClass->set_value(currentLandClass);
        }

        _isForest->set_value(true);

        bool success = !_initialAge->value().isEmpty() && !_initialCSet->value().isEmpty();
        _buildWorked->set_value(success);
    }

}}} // namespace moja::modules::cbm
