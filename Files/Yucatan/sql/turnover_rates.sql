SELECT
    t.sw_foliage AS softwood_foliage_fall_rate,
    t.hw_foliage AS hardwood_foliage_fall_rate,
    t.stem_turnover AS stem_annual_turnover_rate,
    t.sw_branch AS softwood_branch_turnover_rate,
    t.hw_branch AS hardwood_branch_turnover_rate,
    t.branch_snag_split AS other_to_branch_snag_split,
    t.stem_snag AS stem_snag_turnover_rate,
    t.branch_snag AS branch_snag_turnover_rate,
    t.coarse_ag_split AS coarse_root_split,
    t.coarse_root AS coarse_root_turn_prop,
    t.fine_ag_split AS fine_root_ag_split,
    t.fine_root AS fine_root_turn_prop
FROM eco_boundary e
INNER JOIN turnover_parameter t
    ON e.turnover_parameter_id = t.id
WHERE e.name = {var:eco_boundary}