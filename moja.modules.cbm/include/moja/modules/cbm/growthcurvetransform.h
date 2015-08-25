#ifndef CBM_GrowthCurveTransform_H_
#define CBM_GrowthCurveTransform_H_

#include "moja/datarepository/datarepository.h"
#include "moja/datarepository/tileblockcellindexer.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

#include <unordered_map>

namespace moja {
namespace modules {
namespace CBM {

	class GrowthCurveTransform : public flint::ITransform {
	public:
		void configure(DynamicObject config,
					   const flint::ILandUnitController& landUnitController,
					   datarepository::DataRepository& dataRepository) override;

		void controllerChanged(const flint::ILandUnitController& controller) override;
		const Dynamic& value() const override;

	private:
		const std::string baseSql = "select gccv.growth_curve_id as growth_curve_id, sum(case when c.name in (%1%) then case when cv.name like '?' then 1 %2% else -1000 end else -1000 end) as score from growth_curve_classifier_value gccv inner join classifier_value cv on gccv.classifier_value_id = cv.id inner join classifier c on cv.classifier_id = c.id group by gccv.growth_curve_id having score > 0 order by score desc limit 1";
		const std::string matchSql = "when c.name like '%1%' and cv.name like '%2%' then 4 ";
		const flint::ILandUnitController* _landUnitController;
		datarepository::DataRepository* _dataRepository;
		datarepository::IProviderRelationalInterface::Ptr _provider;
		std::string _csetVarName;
		mutable const flint::IVariable* _csetVar;
		mutable Dynamic _value;
	};

}}}

#endif // CBM_GrowthCurveTransform_H_