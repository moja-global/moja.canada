#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"

#include <moja/flint/ivariable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <boost/format.hpp>

namespace moja {
namespace modules {
namespace cbm {

    void CBMDisturbanceEventModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::LocalDomainInit,	&CBMDisturbanceEventModule::onLocalDomainInit,	*this);
		notificationCenter.subscribe(signals::TimingStep,		&CBMDisturbanceEventModule::onTimingStep,		*this);
		notificationCenter.subscribe(signals::DisturbanceEvent,	&CBMDisturbanceEventModule::onDisturbanceEvent,	*this);
	}

    void CBMDisturbanceEventModule::doLocalDomainInit() {
        _softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
        _softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
        _softwoodOther = _landUnitData->getPool("SoftwoodOther");
        _softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
        _softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

        _hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
        _hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
        _hardwoodOther = _landUnitData->getPool("HardwoodOther");
        _hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
        _hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

        _age = _landUnitData->getVariable("age");
    }

	void CBMDisturbanceEventModule::doDisturbanceEvent(DynamicVar n) {
		auto& data = n.extract<const DynamicObject>();

		// Get the disturbance type for either historical or last disturbance event.
		std::string disturbanceType = data["disturbance"];
		int disturbanceCode = data["disturbance_type_code"];
        
        Poco::Nullable<int> preDisturbanceAgeClass;
        if (_landUnitData->hasVariable("age_class")) {
            preDisturbanceAgeClass = _landUnitData->getVariable("age_class")->value().extract<int>();
        }

        DynamicVar metadata = DynamicObject({
			{ "disturbance", disturbanceType },
			{ "disturbance_type_code", disturbanceCode },
            { "pre_disturbance_age_class", preDisturbanceAgeClass }
		});

		auto disturbanceEvent = _landUnitData->createProportionalOperation(metadata);
		auto transferVec = data["transfers"].extract<std::shared_ptr<std::vector<CBMDistEventTransfer>>>();
		for (const auto& transfer : *transferVec) {
			auto srcPool = transfer.sourcePool();
			auto dstPool = transfer.destPool();
			if (srcPool != dstPool) {
				disturbanceEvent->addTransfer(srcPool, dstPool, transfer.proportion());
			}
		}

		_landUnitData->submitOperation(disturbanceEvent);
		_landUnitData->applyOperations();

		double totalBiomass = _hardwoodCoarseRoots->value()
			+ _hardwoodFineRoots->value() + _hardwoodFoliage->value()
			+ _hardwoodMerch->value() + _hardwoodOther->value()
			+ _softwoodCoarseRoots->value() + _softwoodFineRoots->value()
			+ _softwoodFoliage->value() + _softwoodMerch->value()
			+ _softwoodOther->value();

		if (totalBiomass < 0.001) {
			_age->set_value(0);
		}
	}

}}} // namespace moja::modules::cbm
