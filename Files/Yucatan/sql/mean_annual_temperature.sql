SELECT sp.mean_annual_temperature
FROM spatial_unit spu
INNER JOIN spinup_parameter sp
    ON spu.spinup_parameter_id = sp.id
WHERE spu.id = {var:spatial_unit_id}