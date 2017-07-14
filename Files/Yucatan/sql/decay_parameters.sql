SELECT
    p.name AS pool,
    dp.base_decay_rate AS organic_matter_decay_rate,
    dp.prop_to_atmosphere AS prop_to_atmosphere,
    dp.q10 AS q10,
    dp.reference_temp AS reference_temp,
    dp.max_rate AS max_decay_rate_soft
FROM decay_parameter dp
INNER JOIN dom_pool dom
    ON dp.dom_pool_id = dom.id
INNER JOIN pool p
    ON p.id = dom.pool_id