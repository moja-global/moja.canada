SELECT
    ft.name AS forest_type,
    f.a AS a,
    f.b AS b,
    f.a_nonmerch AS a_non_merch,
    f.b_nonmerch AS b_non_merch,
    f.k_nonmerch AS k_non_merch,
    f.cap_nonmerch AS cap_non_merch,
    f.a_sap AS a_sap,
    f.b_sap AS b_sap,
    f.k_sap AS k_sap,
    f.cap_sap AS cap_sap,
    f.a1 AS a1,
    f.a2 AS a2,
    f.a3 AS a3,
    f.b1 AS b1,
    f.b2 AS b2,
    f.b3 AS b3,
    f.c1 AS c1,
    f.c2 AS c2,
    f.c3 AS c3,
    f.min_volume AS min_volume,
    f.max_volume AS max_volume,
    f.low_stemwood_prop AS low_stemwood_prop,
    f.high_stemwood_prop AS high_stemwood_prop,
    f.low_stembark_prop AS low_stembark_prop,
    f.high_stembark_prop AS high_stembark_prop,
    f.low_branches_prop AS low_branches_prop,
    f.high_branches_prop AS high_branches_prop,
    f.low_foliage_prop AS low_foliage_prop,
    f.high_foliage_prop AS high_foliage_prop,
    sp.sw_top_proportion AS softwood_top_prop,
    sp.sw_stump_proportion AS softwood_stump_prop,
    sp.hw_top_proportion AS hardwood_top_prop,
    sp.hw_stump_proportion AS hardwood_stump_prop
FROM vol_to_bio_factor_association fa
INNER JOIN vol_to_bio_factor f
    ON f.id = fa.vol_to_bio_factor_id
INNER JOIN species s
    ON fa.species_id = s.id
INNER JOIN growth_curve_component gcc
    ON s.id = gcc.species_id
INNER JOIN forest_type ft
    ON s.forest_type_id = ft.id
INNER JOIN spatial_unit spu
    ON fa.spatial_unit_id = spu.id
INNER JOIN admin_boundary a
    ON spu.admin_boundary_id = a.id
INNER JOIN stump_parameter sp
    ON a.stump_parameter_id = sp.id
WHERE gcc.growth_curve_id = {var:growth_curve_id}
    AND spu.id = {var:spatial_unit_id}