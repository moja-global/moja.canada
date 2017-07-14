SELECT spu.id
FROM spatial_unit spu
INNER JOIN admin_boundary a
    ON spu.admin_boundary_id = a.id
INNER JOIN eco_boundary e
    ON spu.eco_boundary_id = e.id
WHERE a.name = {var:admin_boundary}
    AND e.name = {var:eco_boundary}