SELECT
    s.return_interval AS return_interval,
    s.max_rotations AS max_rotations,
    dt.name AS historic_disturbance_type,
    dt.name AS last_pass_disturbance_type,
    s.mean_annual_temperature AS mean_annual_temperature,
    0 AS delay
FROM spinup_parameter s
INNER JOIN disturbance_type dt
    ON s.historic_disturbance_type_id = dt.id
INNER JOIN spatial_unit spu
    ON spu.spinup_parameter_id = s.id
INNER JOIN admin_boundary a
    ON spu.admin_boundary_id = a.id
INNER JOIN eco_boundary e
    ON spu.eco_boundary_id = e.id
WHERE a.name = {var:admin_boundary}
    AND e.name = {var:eco_boundary}