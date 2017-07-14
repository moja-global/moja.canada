SELECT s.id AS spu_id
FROM spatial_unit s
INNER JOIN admin_boundary a
    ON s.admin_boundary_id = a.id
INNER JOIN eco_boundary e
    ON s.eco_boundary_id = e.id
WHERE a.name LIKE {var:admin_boundary}
    AND e.name LIKE {var:eco_boundary}