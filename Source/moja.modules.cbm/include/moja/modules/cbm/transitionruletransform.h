#ifndef MOJA_MODULES_CBM_TRANSITIONRULETRANSFORM_H_
#define MOJA_MODULES_CBM_TRANSITIONRULETRANSFORM_H_

#include "moja/datarepository/iproviderrelationalinterface.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/flint/itransform.h"

#include <Poco/LRUCache.h>
#include <Poco/ThreadLocal.h>

namespace moja {
namespace modules {
namespace cbm {

class TransitionRuleTransform : public flint::ITransform {
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

    const std::string _baseSql = R"(
        SELECT
            disturbance_type,
            transition_id
        FROM (
            SELECT
                ROW_NUMBER() OVER (PARTITION BY disturbance_type ORDER BY score DESC) AS tr_rank,
                disturbance_type,
                transition_id
            FROM (
                SELECT
                    dt.name AS disturbance_type,
                    tr.transition_id,
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
                FROM transition_rule tr
                INNER JOIN transition_rule_classifier_value trv
                    ON trv.transition_rule_id = tr.id
                INNER JOIN classifier_value cv
                    ON trv.classifier_value_id = cv.id
                INNER JOIN classifier c
                    ON cv.classifier_id = c.id
                INNER JOIN disturbance_type dt
                    ON tr.disturbance_type_id = dt.id
                GROUP BY
                    tr.transition_id,
                    tr.disturbance_type_id
                HAVING SUM(
                    CASE
                        WHEN c.name IN (%1%) THEN
                            CASE
                                WHEN cv.value LIKE '?' THEN 1
                                %2%
                                ELSE -1000
                            END
                        ELSE -1000
                    END
                ) > 0
            ) AS transition_ranking
        ) AS best_matches
        WHERE tr_rank = 1
    )";

    const std::string _matchSql = "WHEN c.name LIKE '%1%' AND cv.value LIKE '%2%' THEN 4 ";
};

}}}

#endif // MOJA_MODULES_CBM_TRANSITIONRULETRANSFORM_H_