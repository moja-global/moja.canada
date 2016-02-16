#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/observer.h"

namespace moja {
namespace modules {
namespace cbm {

    void CBMBuildLandUnitModule::configure(const DynamicObject& config) { }

    void CBMBuildLandUnitModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connect_signal(signals::LocalDomainInit		, &CBMBuildLandUnitModule::onLocalDomainInit	, *this);
		notificationCenter.connect_signal(signals::PreTimingSequence	, &CBMBuildLandUnitModule::onPreTimingSequence	, *this);
	}

    void CBMBuildLandUnitModule::onLocalDomainInit() {
        _initialAge = _landUnitData->getVariable("initial_age");
        _buildWorked = _landUnitData->getVariable("landUnitBuildSuccess");
        _initialGCID = _landUnitData->getVariable("initial_growth_curve_id");
        _gcid = _landUnitData->getVariable("growth_curve_id");
        _cset = _landUnitData->getVariable("classifier_set");
        _initialHistoricLandClass = _landUnitData->getVariable("initial_historic_land_class");
        _initialCurrentLandClass = _landUnitData->getVariable("initial_current_land_class");
        _historicLandClass = _landUnitData->getVariable("historic_land_class");
        _currentLandClass = _landUnitData->getVariable("current_land_class");
    }

    void CBMBuildLandUnitModule::onPreTimingSequence() {
        auto historicLandClass = _initialHistoricLandClass->value();
        _historicLandClass->set_value(historicLandClass);

        auto currentLandClass = _initialCurrentLandClass->value();
        if (currentLandClass.isEmpty()) {
            _currentLandClass->set_value(historicLandClass);
        } else {
            _currentLandClass->set_value(currentLandClass);
        }

        auto gcid = _initialGCID->value();
        _gcid->set_value(gcid.isEmpty() ? -1 : gcid);

        bool success = !_initialAge->value().isEmpty() && !_cset->value().isEmpty();
        // What CBM variables need to be set between land unit simulations?
        // External variables are handled already, just internal variables. State values? Classifiers?
        _buildWorked->set_value(success);
    }

}}} // namespace moja::modules::cbm
