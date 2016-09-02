-- Create tables
CREATE TABLE r_change_type_categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR);
CREATE TABLE r_pool_collections (id INTEGER PRIMARY KEY AUTOINCREMENT, description VARCHAR);
CREATE TABLE r_pool_collection_pools (pool_collection_id INTEGER REFERENCES r_pool_collections (id) NOT NULL, pool_id UNSIGNED BIG INT REFERENCES PoolDimension (id) NOT NULL, PRIMARY KEY (pool_collection_id, pool_id));
CREATE TABLE r_flux_indicators (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR NOT NULL, change_type_category_id INTEGER REFERENCES r_change_type_categories (id), source_pool_collection_id INTEGER REFERENCES r_pool_collections (id) NOT NULL, sink_pool_collection_id INTEGER REFERENCES r_pool_collections (id) NOT NULL);

-- Populate change type categories
INSERT INTO r_change_type_categories (name) VALUES ('Annual Process');
INSERT INTO r_change_type_categories (name) VALUES ('Disturbance');
INSERT INTO r_change_type_categories (name) VALUES ('Combined');

-- Populate pool collections
INSERT INTO r_pool_collections (description) VALUES ('All biomass');
INSERT INTO r_pool_collections (description) VALUES ('CO2');
INSERT INTO r_pool_collections (description) VALUES ('CH4');
INSERT INTO r_pool_collections (description) VALUES ('CO');
INSERT INTO r_pool_collections (description) VALUES ('Live biomass');
INSERT INTO r_pool_collections (description) VALUES ('DOM');
INSERT INTO r_pool_collections (description) VALUES ('Softwood biomass');
INSERT INTO r_pool_collections (description) VALUES ('Hardwood biomass');
INSERT INTO r_pool_collections (description) VALUES ('Snags');
INSERT INTO r_pool_collections (description) VALUES ('Merch');
INSERT INTO r_pool_collections (description) VALUES ('Stem snag');
INSERT INTO r_pool_collections (description) VALUES ('Foliage');
INSERT INTO r_pool_collections (description) VALUES ('Aboveground very fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Other');
INSERT INTO r_pool_collections (description) VALUES ('Other litter');
INSERT INTO r_pool_collections (description) VALUES ('Coarse roots');
INSERT INTO r_pool_collections (description) VALUES ('Fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Fine roots');
INSERT INTO r_pool_collections (description) VALUES ('Very fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Aboveground slow soil / CO2');
INSERT INTO r_pool_collections (description) VALUES ('Belowground slow soil / CO2');
INSERT INTO r_pool_collections (description) VALUES ('Belowground very fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Aboveground fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Belowground fast soil');
INSERT INTO r_pool_collections (description) VALUES ('Medium soil');
INSERT INTO r_pool_collections (description) VALUES ('Aboveground slow soil');
INSERT INTO r_pool_collections (description) VALUES ('Belowground slow soil');
INSERT INTO r_pool_collections (description) VALUES ('Softwood stem snag');
INSERT INTO r_pool_collections (description) VALUES ('Softwood branch snag');
INSERT INTO r_pool_collections (description) VALUES ('Hardwood stem snag');
INSERT INTO r_pool_collections (description) VALUES ('Hardwood branch snag');
INSERT INTO r_pool_collections (description) VALUES ('Aboveground live biomass');
INSERT INTO r_pool_collections (description) VALUES ('Belowground live biomass');
INSERT INTO r_pool_collections (description) VALUES ('Atmosphere');
INSERT INTO r_pool_collections (description) VALUES ('Overmature decline');
INSERT INTO r_pool_collections (description) VALUES ('Products');
INSERT INTO r_pool_collections (description) VALUES ('Black carbon');
INSERT INTO r_pool_collections (description) VALUES ('Peat');
INSERT INTO r_pool_collections (description) VALUES ('Soil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('All biomass')
    AND LOWER(p.poolname) IN ('softwoodmerch', 'softwoodfoliage', 'softwoodother', 'softwoodcoarseroots',
                              'softwoodfineroots', 'hardwoodmerch', 'hardwoodfoliage', 'hardwoodother',
                              'hardwoodcoarseroots', 'hardwoodfineroots', 'abovegroundveryfastsoil',
                              'belowgroundveryfastsoil', 'abovegroundfastsoil', 'belowgroundfastsoil',
                              'mediumsoil', 'abovegroundslowsoil', 'belowgroundslowsoil', 'softwoodstemsnag',
                              'softwoodbranchsnag', 'hardwoodstemsnag', 'hardwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('CO2')
    AND LOWER(p.poolname) LIKE LOWER('CO2');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('CH4')
    AND LOWER(p.poolname) LIKE LOWER ('CH4');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('CO')
    AND LOWER(p.poolname) LIKE LOWER('CO');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Live biomass')
    AND LOWER(p.poolname) IN ('softwoodmerch', 'softwoodfoliage', 'softwoodother', 'softwoodcoarseroots',
                              'softwoodfineroots', 'hardwoodmerch', 'hardwoodfoliage', 'hardwoodother',
                              'hardwoodcoarseroots', 'hardwoodfineroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('DOM')
    AND LOWER(p.poolname) IN ('abovegroundveryfastsoil', 'belowgroundveryfastsoil', 'abovegroundfastsoil',
                              'belowgroundfastsoil', 'mediumsoil', 'abovegroundslowsoil', 'belowgroundslowsoil',
                              'softwoodstemsnag', 'softwoodbranchsnag', 'hardwoodstemsnag', 'hardwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Soil')
    AND LOWER(p.poolname) IN ('abovegroundveryfastsoil', 'belowgroundveryfastsoil', 'abovegroundfastsoil',
                              'belowgroundfastsoil', 'mediumsoil', 'abovegroundslowsoil', 'belowgroundslowsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Softwood biomass')
    AND LOWER(p.poolname) IN ('softwoodmerch', 'softwoodfoliage', 'softwoodother', 'softwoodcoarseroots', 'softwoodfineroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Hardwood biomass')
    AND LOWER(p.poolname) IN ('hardwoodmerch', 'hardwoodfoliage', 'hardwoodother', 'hardwoodcoarseroots', 'hardwoodfineroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Snags')
    AND LOWER(p.poolname) IN ('softwoodstemsnag', 'softwoodbranchsnag', 'hardwoodstemsnag', 'hardwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Merch')
    AND LOWER(p.poolname) IN ('softwoodmerch', 'hardwoodmerch');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Stem snag')
    AND LOWER(p.poolname) IN ('softwoodstemsnag', 'hardwoodstemsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Foliage')
    AND LOWER(p.poolname) IN ('softwoodfoliage', 'hardwoodfoliage');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Aboveground very fast soil')
    AND LOWER(p.poolname) IN ('abovegroundveryfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Other')
    AND LOWER(p.poolname) IN ('softwoodother', 'hardwoodother');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Other litter')
    AND LOWER(p.poolname) IN ('abovegroundfastsoil', 'softwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Coarse roots')
    AND LOWER(p.poolname) IN ('softwoodcoarseroots', 'hardwoodcoarseroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Fast soil')
    AND LOWER(p.poolname) IN ('abovegroundfastsoil', 'belowgroundfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Fine roots')
    AND LOWER(p.poolname) IN ('softwoodfineroots', 'hardwoodfineroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Very fast soil')
    AND LOWER(p.poolname) IN ('abovegroundveryfastsoil', 'belowgroundveryfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Aboveground slow soil / CO2')
    AND LOWER(p.poolname) IN ('abovegroundslowsoil', 'co2');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Belowground slow soil / CO2')
    AND LOWER(p.poolname) IN ('belowgroundslowsoil', 'co2');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Belowground very fast soil')
    AND LOWER(p.poolname) LIKE LOWER('belowgroundveryfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Aboveground fast soil')
    AND LOWER(p.poolname) LIKE LOWER('abovegroundfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Belowground fast soil')
    AND LOWER(p.poolname) LIKE LOWER('belowgroundfastsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Medium soil')
    AND LOWER(p.poolname) LIKE LOWER('mediumsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Aboveground slow soil')
    AND LOWER(p.poolname) LIKE LOWER('abovegroundslowsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Belowground slow soil')
    AND LOWER(p.poolname) LIKE LOWER('belowgroundslowsoil');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Softwood stem snag')
    AND LOWER(p.poolname) LIKE LOWER('softwoodstemsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Softwood branch snag')
    AND LOWER(p.poolname) LIKE LOWER('softwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Hardwood stem snag')
    AND LOWER(p.poolname) LIKE LOWER('hardwoodstemsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Hardwood branch snag')
    AND LOWER(p.poolname) LIKE LOWER('hardwoodbranchsnag');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Aboveground live biomass')
    AND LOWER(p.poolname) IN ('softwoodmerch', 'softwoodfoliage', 'softwoodother', 'hardwoodmerch', 'hardwoodfoliage', 'hardwoodother');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Belowground live biomass')
    AND LOWER(p.poolname) IN ('softwoodcoarseroots', 'softwoodfineroots', 'hardwoodcoarseroots', 'hardwoodfineroots');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Atmosphere')
    AND LOWER(p.poolname) IN ('atmosphere');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Overmature decline')
    AND LOWER(p.poolname) LIKE LOWER('overmaturelosses');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Products')
    AND LOWER(p.poolname) LIKE LOWER('products');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Black carbon')
    AND LOWER(p.poolname) LIKE LOWER('blackcarbon');

INSERT INTO r_pool_collection_pools (pool_collection_id, pool_id)
SELECT pc.id AS pool_collection_id, p.id AS pool_id
FROM r_pool_collections pc, pooldimension p
WHERE LOWER(pc.description) LIKE LOWER('Peat')
    AND LOWER(p.poolname) LIKE LOWER('peat');

-- Populate flux indicators
INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'CO2 Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('All biomass')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'CH4 Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('All biomass')
    AND LOWER(snk.description) LIKE LOWER('CH4');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'CO Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('All biomass')
    AND LOWER(snk.description) LIKE LOWER('CO');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Bio CO2 Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Live biomass')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Bio CH4 Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Live biomass')
    AND LOWER(snk.description) LIKE LOWER('CH4');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Bio CO Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Live biomass')
    AND LOWER(snk.description) LIKE LOWER('CO');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CO2 Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('DOM')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CH4 Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Soil')
    AND LOWER(snk.description) LIKE LOWER('CH4');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CO Emission' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Soil')
    AND LOWER(snk.description) LIKE LOWER('CO');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CO2 Emission - Annual Process' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('DOM')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CH4 Emission - Annual Process' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Soil')
    AND LOWER(snk.description) LIKE LOWER('CH4');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM CO Emission - Annual Process' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Soil')
    AND LOWER(snk.description) LIKE LOWER('CO');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Soft Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Softwood biomass')
    AND LOWER(snk.description) LIKE LOWER('Products');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Hard Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Hardwood biomass')
    AND LOWER(snk.description) LIKE LOWER('Products');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'DOM Production' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Snags')
    AND LOWER(snk.description) LIKE LOWER('Products');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Biomass to Soil' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Live biomass')
    AND LOWER(snk.description) LIKE LOWER('DOM');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Merch Litter Input' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Merch')
    AND LOWER(snk.description) LIKE LOWER('Stem snag');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Foliage Litter Input' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Foliage')
    AND LOWER(snk.description) LIKE LOWER('Aboveground very fast soil');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Other Litter Input' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Other')
    AND LOWER(snk.description) LIKE LOWER('Other litter');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Coarse Litter Input' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Coarse roots')
    AND LOWER(snk.description) LIKE LOWER('Fast soil');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Fine Litter Input' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Fine roots')
    AND LOWER(snk.description) LIKE LOWER('Very fast soil');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'V Fast AG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Aboveground very fast soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'V Fast BG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Belowground very fast soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Fast AG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Aboveground fast soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Fast BG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Belowground fast soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Medium to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Medium soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Slow AG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Aboveground slow soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Slow BG to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Belowground slow soil')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'SW Stem Snag to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Softwood stem snag')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'SW Branch Snag to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Softwood branch snag')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'HW Stem Snag to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Hardwood stem snag')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'HW Branch Snag to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Hardwood branch snag')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Merch to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Merch')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Foliage to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Foliage')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Other to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Other')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Coarse to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Coarse roots')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Fine to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Disturbance')
    AND LOWER(src.description) LIKE LOWER('Fine roots')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Black Carbon to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Black carbon')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Peat to Air' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Combined')
    AND LOWER(src.description) LIKE LOWER('Peat')
    AND LOWER(snk.description) LIKE LOWER('CO2');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Gross Growth AG' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Atmosphere')
    AND LOWER(snk.description) LIKE LOWER('Aboveground live biomass');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Gross Growth BG' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Atmosphere')
    AND LOWER(snk.description) LIKE LOWER('Belowground live biomass');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Total decline' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Live biomass')
    AND LOWER(snk.description) LIKE LOWER('Overmature decline');

INSERT INTO r_flux_indicators (name, change_type_category_id, source_pool_collection_id, sink_pool_collection_id)
SELECT 'Accountable decline' AS name, ct.id AS change_type_category_id, src.id AS source_pool_collection_id, snk.id AS sink_pool_collection_id
FROM r_change_type_categories ct, r_pool_collections src, r_pool_collections snk
WHERE LOWER(ct.name) LIKE LOWER('Annual Process')
    AND LOWER(src.description) LIKE LOWER('Overmature decline')
    AND LOWER(snk.description) LIKE LOWER('DOM');

-- Create flux indicator reporting view
CREATE VIEW flux_indicators AS
SELECT
    fi.id AS flux_indicator_id,
    fi.name AS indicator,
    d.year,
    SUM(CASE
            WHEN LOWER(ct.name) LIKE LOWER('Annual Process') AND m.disturbancetype <= 0 THEN f.fluxvalue
            WHEN LOWER(ct.name) LIKE LOWER('Disturbance') AND m.disturbancetype > 0 THEN f.fluxvalue
            WHEN LOWER(ct.name) LIKE LOWER('Combined') THEN f.fluxvalue
            ELSE 0
        END
    ) AS flux
FROM r_flux_indicators fi
INNER JOIN r_pool_collection_pools p_src
    ON fi.source_pool_collection_id = p_src.pool_collection_id
INNER JOIN r_pool_collection_pools p_dst
    ON fi.sink_pool_collection_id = p_dst.pool_collection_id
INNER JOIN fluxes f
    ON f.poolsrcdimid = p_src.pool_id
    AND f.pooldstdimid = p_dst.pool_id
INNER JOIN moduleinfodimension m
    ON f.moduleinfodimid = m.id
INNER JOIN datedimension d
    ON f.datedimid = d.id
INNER JOIN r_change_type_categories ct
    ON fi.change_type_category_id = ct.id
GROUP BY
    fi.id,
    fi.name,
    d.year;

-- Populate stock change reporting tables
CREATE TABLE r_flux_indicator_collections (id INTEGER PRIMARY KEY AUTOINCREMENT, description VARCHAR);
CREATE TABLE r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id INTEGER REFERENCES r_flux_indicator_collections (id) NOT NULL, flux_indicator_id INTEGER REFERENCES r_flux_indicators (id) NOT NULL, PRIMARY KEY (flux_indicator_collection_id, flux_indicator_id));
CREATE TABLE r_stock_changes (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR NOT NULL, add_flux_indicator_collection_id INTEGER REFERENCES r_flux_indicator_collections (id), sub_flux_indicator_collection_id INTEGER REFERENCES r_flux_indicator_collections (id));

INSERT INTO r_flux_indicator_collections (description) VALUES ('Gross growth');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Accountable decline');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Total decline');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Ecosystem removals');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Biomass removals');
INSERT INTO r_flux_indicator_collections (description) VALUES ('DOM removals');
INSERT INTO r_flux_indicator_collections (description) VALUES ('DOM emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('DOM emissions - annual processes');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Biomass to soil');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Decomposition releases');
INSERT INTO r_flux_indicator_collections (description) VALUES ('All production');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Live production');
INSERT INTO r_flux_indicator_collections (description) VALUES ('DOM production');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Soft production');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Hard production');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Disturbance emissions and DOM annual process emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Deadwood to air');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Litter to air');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Soil to air');
INSERT INTO r_flux_indicator_collections (description) VALUES ('All emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('Biomass emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('CO2 emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('CO emissions');
INSERT INTO r_flux_indicator_collections (description) VALUES ('CH4 emissions');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Gross growth')
    AND LOWER(fi.name) IN ('gross growth ag', 'gross growth bg');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Accountable decline')
    AND LOWER(fi.name) IN ('accountable decline');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Total decline')
    AND LOWER(fi.name) IN ('total decline');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Ecosystem removals')
    AND LOWER(fi.name) IN ('bio co2 emission', 'bio ch4 emission', 'bio co emission', 'dom co2 emission', 'dom ch4 emission', 'dom co emission', 'soft production', 'hard production', 'dom production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Biomass removals')
    AND LOWER(fi.name) IN ('bio co2 emission', 'bio ch4 emission', 'bio co emission', 'soft production', 'hard production', 'biomass to soil');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('DOM removals')
    AND LOWER(fi.name) IN ('dom co2 emission', 'dom ch4 emission', 'dom co emission', 'dom production', 'biomass to soil');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('DOM emissions')
    AND LOWER(fi.name) IN ('dom co2 emission', 'dom ch4 emission', 'dom co emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('DOM emissions - annual processes')
    AND LOWER(fi.name) IN ('dom co2 emission - annual process', 'dom ch4 emission - annual process', 'dom co emission - annual process');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Biomass to soil')
    AND LOWER(fi.name) IN ('biomass to soil');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Decomposition releases')
    AND LOWER(fi.name) IN ('dom co2 emission - annual process', 'dom ch4 emission - annual process', 'dom co emission - annual process');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('All production')
    AND LOWER(fi.name) IN ('soft production', 'hard production', 'dom production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Live production')
    AND LOWER(fi.name) IN ('soft production', 'hard production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('DOM production')
    AND LOWER(fi.name) IN ('dom production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Soft production')
    AND LOWER(fi.name) IN ('soft production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Hard production')
    AND LOWER(fi.name) IN ('hard production');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Disturbance emissions and DOM annual process emissions')
    AND LOWER(fi.name) IN ('soft production', 'hard production', 'dom production', 'dom co2 emission', 'dom ch4 emission', 'dom co emission', 'bio co2 emission', 'bio ch4 emission', 'bio co emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Deadwood to Air')
    AND LOWER(fi.name) IN ('fast ag to air', 'fast bg to air', 'medium to air', 'sw stem snag to air', 'sw branch snag to air', 'hw stem snag to air', 'hw branch snag to air');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Litter to Air')
    AND LOWER(fi.name) IN ('v fast ag to air', 'slow ag to air');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Soil to air')
    AND LOWER(fi.name) IN ('v fast bg to air', 'slow bg to air', 'black carbon to air');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('All emissions')
    AND LOWER(fi.name) IN ('bio co2 emission', 'bio ch4 emission', 'bio co emission', 'dom co2 emission', 'dom ch4 emission', 'dom co emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('Biomass emissions')
    AND LOWER(fi.name) IN ('bio co2 emission', 'bio ch4 emission', 'bio co emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('CO2 emissions')
    AND LOWER(fi.name) IN ('bio co2 emission', 'dom co2 emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('CO emissions')
    AND LOWER(fi.name) IN ('bio co emission', 'dom co emission');

INSERT INTO r_flux_indicator_collection_flux_indicators (flux_indicator_collection_id, flux_indicator_id)
SELECT fic.id AS flux_indicator_collection_id, fi.id AS flux_indicator_id
FROM r_flux_indicator_collections fic, r_flux_indicators fi
WHERE LOWER(fic.description) LIKE LOWER('CH4 emissions')
    AND LOWER(fi.name) IN ('bio ch4 emission', 'dom ch4 emission');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Delta Total Ecosystem' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) LIKE LOWER('Gross growth')
    AND LOWER(fic_sub.description) LIKE LOWER('Ecosystem removals');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Delta Total Biomass' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) LIKE LOWER('Gross growth')
    AND LOWER(fic_sub.description) LIKE LOWER('Biomass removals');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Delta Total DOM' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) LIKE LOWER('Gross growth')
    AND LOWER(fic_sub.description) LIKE LOWER('DOM removals');

-- Accountable decline is the root decline from overmature losses, when a growth curve
-- turns downward. Total decline includes root losses following a decline in aboveground
-- biomass due to disturbance; in CBM3, the root losses from disturbance disappear, so
-- gross growth is really total growth minus these unaccounted subtractions.
INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'NPP' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) IN ('gross growth', 'accountable decline')
    AND LOWER(fic_sub.description) LIKE LOWER('Total decline');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'NEP' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) IN ('gross growth', 'accountable decline')
    AND LOWER(fic_sub.description) IN ('total decline', 'dom emissions - annual processes');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'NBP' AS name, fic_add.id AS add_flux_indicator_collection_id, fic_sub.id AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add, r_flux_indicator_collections fic_sub
WHERE LOWER(fic_add.description) IN ('gross growth', 'accountable decline')
    AND LOWER(fic_sub.description) IN ('total decline', 'disturbance emissions and dom annual process emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Litterfall' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Biomass to soil');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Decomposition Releases' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Decomposition releases');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Harvest (Biomass + Snags)' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('All production');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Harvest (Biomass)' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Live production');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Harvest (Snags)' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('DOM production');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Softwood Harvest (Biomass)' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Soft production');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Hardwood Harvest (Biomass)' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Hard production');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Deadwood to Atmosphere' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Deadwood to air');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Soil to Atmosphere' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Soil to air');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('All emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total Biomass Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Biomass emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total DOM Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('DOM emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total CO2 Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('CO2 emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total CO Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('CO emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Total CH4 Emissions' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('CH4 emissions');

INSERT INTO r_stock_changes (name, add_flux_indicator_collection_id, sub_flux_indicator_collection_id)
SELECT 'Disturbance Losses' AS name, fic_add.id AS add_flux_indicator_collection_id, NULL AS sub_flux_indicator_collection_id
FROM r_flux_indicator_collections fic_add
WHERE LOWER(fic_add.description) LIKE LOWER('Ecosystem removals');

-- Create stock change reporting view
CREATE VIEW stock_change_indicators AS
SELECT
    a.indicator,
    a.year,
    a.flux - COALESCE(s.flux, 0) AS flux
FROM (
    SELECT
        flux_add.indicator,
        flux_add.year,
        SUM(flux_add.flux) AS flux
    FROM (
        SELECT
            sc.name AS indicator,
            fi.flux_indicator_id,
            fi.year,
            fi.flux
        FROM r_stock_changes sc
        INNER JOIN r_flux_indicator_collection_flux_indicators fic
            ON sc.add_flux_indicator_collection_id = fic.flux_indicator_collection_id
        INNER JOIN flux_indicators fi
            ON fic.flux_indicator_id = fi.flux_indicator_id
        GROUP BY
            sc.name,
            fi.flux_indicator_id,
            fi.year,
            fi.flux
    ) AS flux_add
    GROUP BY
        flux_add.indicator,
        flux_add.year
) a LEFT JOIN (
    SELECT
        flux_sub.indicator,
        flux_sub.year,
        SUM(flux_sub.flux) AS flux
    FROM (
        SELECT
            sc.name AS indicator,
            fi.flux_indicator_id,
            fi.year,
            fi.flux
        FROM r_stock_changes sc
        INNER JOIN r_flux_indicator_collection_flux_indicators fic
            ON sc.sub_flux_indicator_collection_id = fic.flux_indicator_collection_id
        INNER JOIN flux_indicators fi
            ON fic.flux_indicator_id = fi.flux_indicator_id
        GROUP BY
            sc.name,
            fi.flux_indicator_id,
            fi.year,
            fi.flux
    ) AS flux_sub
    GROUP BY
        flux_sub.indicator,
        flux_sub.year
) s
    ON LOWER(a.indicator) LIKE LOWER(s.indicator)
    AND a.year = s.year;

-- Pool indicators
CREATE TABLE r_pool_indicators (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR NOT NULL, pool_collection_id INTEGER REFERENCES r_pool_collections (id));

INSERT INTO r_pool_indicators (name, pool_collection_id)
SELECT 'Total Ecosystem' AS name, pc.id AS pool_collection_id
FROM r_pool_collections pc
WHERE LOWER(pc.description) LIKE LOWER('All biomass');

INSERT INTO r_pool_indicators (name, pool_collection_id)
SELECT 'Total Biomass' AS name, pc.id AS pool_collection_id
FROM r_pool_collections pc
WHERE LOWER(pc.description) LIKE LOWER('Live biomass');

-- Create pool indicators reporting view
CREATE VIEW pool_indicators AS
SELECT
    pid.id AS indicator_id,
    pid.name AS indicator,
    d.year,
    SUM(p.poolvalue) AS pool_value
FROM r_pool_indicators pid
INNER JOIN r_pool_collection_pools pcp
    ON pid.pool_collection_id = pcp.pool_collection_id
INNER JOIN pools p
    ON pcp.pool_id = p.poolid
INNER JOIN datedimension d
    ON p.datedimid = d.id
GROUP BY
    pid.id,
    pid.name,
    d.year;
