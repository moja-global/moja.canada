SELECT t.branch_snag_split AS slow_mixing_rate
FROM eco_boundary e
INNER JOIN turnover_parameter t
    ON e.turnover_parameter_id = t.id
WHERE e.name LIKE {var:eco_boundary}