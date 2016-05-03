-- select DISTINCT(F.source_poolinfo_dimension_id_fk) from flux_reporting_results AS F
select DISTINCT(F.source_poolinfo_dimension_id_fk),P.name from flux_reporting_results AS F
INNER JOIN poolinfo_dimension AS P on P.poolinfo_dimension_id_pk = F.source_poolinfo_dimension_id_fk
order by F.source_poolinfo_dimension_id_fk

select DISTINCT(F.sink_poolinfo_dimension_id_fk) from flux_reporting_results AS F
order by F.sink_poolinfo_dimension_id_fk



select DISTINCT(F.sink_poolinfo_dimension_id_fk),P.name from flux_reporting_results AS F
INNER JOIN poolinfo_dimension AS P on P.poolinfo_dimension_id_pk = F.sink_poolinfo_dimension_id_fk
order by F.sink_poolinfo_dimension_id_fk
