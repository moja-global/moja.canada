SELECT
    gcv.age AS age,
    gcv.merchantable_volume AS merchantable_volume
FROM (
    SELECT
        CASE
            WHEN gc.id IS NOT NULL THEN gc.id
            ELSE -1
        END AS growth_curve_component_id
    FROM growth_curve_component gc
    INNER JOIN species s
        ON s.id = gc.species_id
    INNER JOIN forest_type ft
        ON ft.id = s.forest_type_id
    WHERE gc.growth_curve_id = {var:growth_curve_id}
        AND LOWER(ft.name) LIKE LOWER('Hardwood')
) AS gc
INNER JOIN growth_curve_component_value gcv ON gc.growth_curve_component_id = gcv.growth_curve_component_id