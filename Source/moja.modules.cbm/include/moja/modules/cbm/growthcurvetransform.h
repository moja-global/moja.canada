#ifndef MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_
#define MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_

#include "moja/datarepository/iproviderrelationalinterface.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

#include <Poco/LRUCache.h>
#include <Poco/ThreadLocal.h>

namespace moja {
namespace modules {
namespace cbm {

class GrowthCurveTransform : public flint::ITransform {
public:
	void configure(DynamicObject config,
		const flint::ILandUnitController& landUnitController,
		datarepository::DataRepository& dataRepository) override;

	void controllerChanged(const flint::ILandUnitController& controller) override;
	const DynamicVar& value() const override;

private:
	const flint::ILandUnitController* _landUnitController;
	datarepository::DataRepository* _dataRepository;
	std::shared_ptr<datarepository::IProviderRelationalInterface> _provider;
	mutable const flint::IVariable* _csetVar;
	mutable DynamicVar _value;
    mutable Poco::ThreadLocal<Poco::LRUCache<std::string, DynamicVar>> _cache;

    const std::string buildSql(const DynamicObject& classifierSet) const;
    const std::string buildDebuggingInfo(const DynamicObject& classifierSet) const;

    const std::string _baseSql = R"(
        SELECT growth_curve_id
        FROM (
            SELECT
                gccv.growth_curve_id AS growth_curve_id,
                SUM(CASE
                        WHEN c.name IN (%1%) THEN
                            CASE
                                WHEN cv.value LIKE '?' THEN 1
                                %2%
                                ELSE -1000
                            END
                        ELSE -1000
                    END
                ) AS score
            FROM growth_curve_classifier_value gccv
            INNER JOIN classifier_value cv
                ON gccv.classifier_value_id = cv.id
            INNER JOIN classifier c
                ON cv.classifier_id = c.id
            GROUP BY gccv.growth_curve_id
        ) AS growth_curve_ranking
        WHERE score > 0
        ORDER BY score DESC
        LIMIT 1
    )";

    const std::string _matchSql = "WHEN c.name LIKE '%1%' AND cv.value LIKE '%2%' THEN 4 ";
};

}}}

#endif // MOJA_MODULES_CBM_GROWTHCURVETRANSFORM_H_