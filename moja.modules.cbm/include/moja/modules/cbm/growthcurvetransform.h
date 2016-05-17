#ifndef MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_
#define MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_

#include "moja/datarepository/iproviderrelationalinterface.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

namespace moja {
namespace modules {
namespace cbm {

class GrowthCurveTransform : public flint::ITransform {
public:
	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;
	const Dynamic& value() const override;

private:
	const std::string baseSql = "select gccv.growth_curve_id as growth_curve_id, sum(case when c.name in (%1%) then case when cv.value like '?' then 1 %2% else -1000 end else -1000 end) as score from growth_curve_classifier_value gccv inner join classifier_value cv on gccv.classifier_value_id = cv.id inner join classifier c on cv.classifier_id = c.id group by gccv.growth_curve_id having score > 0 order by score desc limit 1";
	const std::string matchSql = "when c.name like '%1%' and cv.value like '%2%' then 4 ";
	const flint::ILandUnitController* _landUnitController;
	datarepository::DataRepository* _dataRepository;
	datarepository::IProviderRelationalInterface::Ptr _provider;
	std::string _csetVarName;
	mutable const flint::IVariable* _csetVar;
	mutable Dynamic _value;
};

}
}
}

#endif // MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_