SELECT
    dt.name AS disturbance_type,
    dma.spatial_unit_id,
    dma.disturbance_matrix_id
FROM disturbance_matrix_association dma
INNER JOIN disturbance_type dt
    ON dma.disturbance_type_id = dt.id