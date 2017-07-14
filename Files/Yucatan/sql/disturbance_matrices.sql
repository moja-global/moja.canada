SELECT
    dm.id AS disturbance_matrix_id,
    source_pool.name AS source_pool_name,
    dest_pool.name AS dest_pool_name,
    dv.proportion
FROM disturbance_matrix dm
INNER JOIN disturbance_matrix_value dv
    ON dm.id = dv.disturbance_matrix_id
INNER JOIN pool source_pool
    ON dv.source_pool_id = source_pool.id
INNER JOIN pool dest_pool
    ON dv.sink_pool_id = dest_pool.id