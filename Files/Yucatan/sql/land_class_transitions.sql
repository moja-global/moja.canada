SELECT
    dt.name AS disturbance_type,
    lc.code AS land_class_transition,
    lc.is_forest,
    lc.years_to_permanent
FROM disturbance_type dt
INNER JOIN land_class lc
    ON dt.transition_land_class_id = lc.id