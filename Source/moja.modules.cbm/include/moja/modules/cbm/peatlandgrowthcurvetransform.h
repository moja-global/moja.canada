#ifndef MOJA_MODULES_CBM_PEATLANDGROWTHCURVETRANSFORM_H_
#define MOJA_MODULES_CBM_PEATLANDGROWTHCURVETRANSFORM_H_

#include "moja/datarepository/iproviderrelationalinterface.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

namespace moja {
namespace modules {
namespace cbm {

class PeatlandGrowthCurveTransform : public flint::ITransform {
public:
	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;
	const DynamicVar& value() const override;

private:
	const flint::ILandUnitController* _landUnitController;
	datarepository::DataRepository* _dataRepository;
	mutable const flint::IVariable* _gcbmGrowthCurveVar;
    mutable const flint::IVariable* _blackSpruceGrowthCurveVar;
    mutable DynamicVar _value;
};

}}}

#endif // MOJA_MODULES_CBM_PEATLANDGROWTHCURVETRANSFORM_H_