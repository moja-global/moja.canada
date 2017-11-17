#ifndef sawtooth_parameter_set_h
#define sawtooth_parameter_set_h
#include <unordered_map>
#include <tuple>
#include <vector>
#include <memory>
#include "dblayer.h"
#include "constants.h"
#include "disturbance_type.h"
#include "parameter_d1.h"
#include "parameter_d2.h"
#include "parameter_es1.h"
#include "parameter_es2.h"
#include "parameter_es3.h"
#include "parameter_mlr35.h"
#include "parameter_core.h"
#include "parameter_cbm.h"
#include "modelmeta.h"

namespace Sawtooth {
	namespace Parameter {

		

		template <class T>
		class ParameterTable {
		private:

			std::unordered_map<int, std::shared_ptr<T>> Table;
		public:
			void AddParameter(const std::string& tableName, int key, T parameter) {
				if (Table.count(key) > 0) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "'" << tableName << "' specified key already exists {" << key << "}";
					throw ex;
				}
				Table[key] = std::shared_ptr<T>(new T(parameter));
			}
			std::shared_ptr<T> GetParameter(const std::string& tableName, int key) const {
				auto match = Table.find(key);
				if (match == Table.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "'" << tableName << "' specified key not found {" << key << "}";
					throw ex;
				}
				return match->second;
			}
			const std::unordered_map<int, std::shared_ptr<T>> GetCollection() const {
				return Table;
			}
		};

		class ParameterSet {
		private:

			DBConnection& Conn;

			const std::string constants_query = "select name, value from Sawtooth_Constants";

			const std::string disturbanceTypes_query = "select id, type, severity, p_mortality from Sawtooth_Disturbance_Type";

			const std::string disturbance_species_query = "select disturbance_type, species_id from Sawtooth_Disturbance_Species";

			const std::string equation_set_query = 
				"SELECT Sawtooth_Species.id, "
				"Sawtooth_Parameter.name, "
				"Sawtooth_Parameter.value "
				"from Sawtooth_Parameter "
				"INNER JOIN Sawtooth_Equation_Set "
				"on Sawtooth_Parameter.equation_set_id = Sawtooth_Equation_Set.id "
				"INNER JOIN Sawtooth_Species "
				"on Sawtooth_Parameter.species_id = Sawtooth_Species.id "
				"WHERE Sawtooth_Equation_Set.name = ?";

			const std::string cbm_stump_parameter_query =
				"SELECT stump_parameter.id, "
				"stump_parameter.sw_top_proportion, "
				"stump_parameter.sw_stump_proportion, "
				"stump_parameter.hw_top_proportion, "
				"stump_parameter.hw_stump_proportion "
				"FROM stump_parameter";

			const std::string cbm_root_parameter_query =
				"SELECT root_parameter.id, "
				"biomass_to_carbon_rate.rate as biomass_to_carbon, "
				"root_parameter.hw_a, "
				"root_parameter.sw_a, "
				"root_parameter.hw_b, "
				"root_parameter.frp_a, "
				"root_parameter.frp_b, "
				"root_parameter.frp_c "
				"FROM root_parameter, biomass_to_carbon_rate";

			const std::string cbm_turnover_parameter_query =
				"SELECT turnover_parameter.id, "
				"turnover_parameter.sw_foliage as SoftwoodFoliageFallRate, "
				"turnover_parameter.hw_foliage as HardwoodFoliageFallRate, "
				"turnover_parameter.stem_turnover as StemAnnualTurnoverRate, "
				"turnover_parameter.sw_branch as SoftwoodBranchTurnoverRate, "
				"turnover_parameter.hw_branch as HardwoodBranchTurnoverRate, "
				"turnover_parameter.coarse_root as CoarseRootTurnProp, "
				"turnover_parameter.fine_root as FineRootTurnProp "
				"from turnover_parameter";

			const std::string cbm_disturbance_matrix_association_query =
				"SELECT * FROM disturbance_matrix_association";

			const std::string cbm_disturbance_matrix_bio_loss_query =
				"SELECT disturbance_matrix_value.disturbance_matrix_id, "
				"disturbance_matrix_value.source_pool_id, "
				"sum(disturbance_matrix_value.proportion) as lossProportion "
				"FROM disturbance_matrix_value "
				"WHERE disturbance_matrix_value.source_pool_id <= 10 AND "
				"disturbance_matrix_value.source_pool_id <> disturbance_matrix_value.sink_pool_id "
				"GROUP BY disturbance_matrix_value.disturbance_matrix_id, disturbance_matrix_value.source_pool_id";

			const std::string biomassC_UtilizationQuery =
				"SELECT Sawtooth_BiomassC_Utilization.spatial_unit_id, "
				"Sawtooth_BiomassC_Utilization.species_id, "
				"Sawtooth_BiomassC_Utilization.value "
				"FROM Sawtooth_BiomassC_Utilization";

			Constants _constants;

			std::map<int, std::shared_ptr<DisturbanceType>> DisturbanceTypes;

			std::map<int, EquationSet> GetGroupedEquationSet(
				const std::string& equationSetName) {

				auto stmt = Conn.prepare(equation_set_query);
				sqlite3_bind_text(stmt, 1, equationSetName.c_str(), -1, SQLITE_STATIC);
				auto c = Cursor(stmt);
				std::map<int, EquationSet> groupedValues;
				while(c.MoveNext()) {
					int id = c.GetValueInt32("id");
					std::string parameterName = c.GetValueString("name");
					double parameterValue = c.GetValueDouble("value");

					auto groupMatch = groupedValues.find(id);
					if (groupMatch == groupedValues.end()) {
						auto e = EquationSet(equationSetName);
						e.AddValue(parameterName, parameterValue);
						groupedValues[id] = e;
					}
					else {
						groupedValues[id].AddValue(parameterName,
							parameterValue);
					}
				}
				return groupedValues;
			}

			template<class T>
			void InitializeSawtoothEquationSet(const std::string& equationSetName, 
				ParameterTable<T>& parameterTable) {
				auto grouped = GetGroupedEquationSet(equationSetName);
				for (auto g : grouped) {
					T param = T(g.second);
					int key = g.first;
					parameterTable.AddParameter(equationSetName, key, param);
				}
			}

			Constants LoadConstants() {
				auto stmt = Conn.prepare(constants_query);
				auto c = Cursor(stmt);
				std::map<std::string, double> result;
				while (c.MoveNext()) {
					std::string name = c.GetValueString("name");
					double value = c.GetValueDouble("value");
					result[name] = value;
				}
				return Constants(result);
			}

			std::map<int, std::vector<int>> GetDisturbanceTypeSpecies() {
				std::map<int, std::vector<int>> result;
				auto stmt = Conn.prepare(disturbance_species_query);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {
					int disturbanceType = c.GetValueInt32("disturbance_type");
					int species = c.GetValueInt32("species_id");
					result[disturbanceType].push_back(species);
				}
				return result;
			}

			void LoadDisturbanceTypes() {
				std::map<int, std::vector<int>> speciesLookup 
					= GetDisturbanceTypeSpecies();

				auto stmt = Conn.prepare(disturbanceTypes_query);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {

					int id = c.GetValueInt32("id");
					int type = c.GetValueInt32("type");
					double p_mortality = c.GetValueDouble("p_mortality");
					if (p_mortality > 1.0 || p_mortality < 0.0) {
						auto ex = SawtoothException(Sawtooth_DBQueryError);
						ex.Message << "disturbance probability of mortality must be: 0<=PM<=1";
						throw ex;
					}
					std::vector<int> eligibleSpecies;
					const auto species = speciesLookup.find(type);
					if (species != speciesLookup.end()) {
						for (auto s : species->second) {
							eligibleSpecies.push_back(s);
						}
					}
					std::shared_ptr<DisturbanceType> dt = 
						std::shared_ptr<DisturbanceType>(
							new DisturbanceType(id, p_mortality,
								eligibleSpecies));

					DisturbanceTypes[id] = dt;
				}
			}

			void LoadBiomassCUtilizationLevels() {
				auto stmt = Conn.prepare(biomassC_UtilizationQuery);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {
					int region_id = c.GetValueInt32("spatial_unit_id");
					int species_id = c.GetValueInt32("species_id");
					double value = c.GetValueDouble("value");
					_biomassC_utilizationLevel[region_id][species_id] = value;
				}
			}

			template<class T>
			void LoadCBMParameters(const std::string& name, const std::string& query, ParameterTable<T>& p) {
				auto stmt = Conn.prepare(query);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {
					T v(c);
					p.AddParameter(name, v.id, v);
				}
			}

			void LoadDisturbanceMatrixAssocations() {
				auto stmt = Conn.prepare(cbm_disturbance_matrix_association_query);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {
					int spatial_unit_id = c.GetValueInt32("spatial_unit_id");
					int disturbance_type_id = c.GetValueInt32("disturbance_type_id");
					int disturbance_matrix_id = c.GetValueInt32("disturbance_matrix_id");
					dmAssociations[spatial_unit_id][disturbance_type_id] = disturbance_matrix_id;
				}
			}

			void LoadDisturbanceMatrixBiomassLosses() {
				//populate with the set of dmids
				for (const auto pair1 : dmAssociations) {

					for (const auto pair2 : pair1.second) {
						//for every unique disturbance matrix id found in the 
						//dm associations add a record to the DM losses 
						//collection
						int dmid = pair2.second;
						if (!DMBiomassLossProportions.count(dmid)) {
							//add an empty struct which by default has 0 losses
							DMBiomassLossProportions[dmid] = Sawtooth_CBMBiomassPools();
						}
					}
				}
				auto stmt = Conn.prepare(cbm_disturbance_matrix_bio_loss_query);
				auto c = Cursor(stmt);
				while (c.MoveNext()) {
					//add the loss proportions found in the dm query
					int disturbance_matrix_id = c.GetValueInt32("disturbance_matrix_id");
					int source_pool_id = c.GetValueInt32("source_pool_id");
					double lossProportion = c.GetValueDouble("lossProportion");
					if      (source_pool_id == 1) DMBiomassLossProportions[disturbance_matrix_id].SWM  = lossProportion;
					else if (source_pool_id == 2) DMBiomassLossProportions[disturbance_matrix_id].SWF  = lossProportion;
					else if (source_pool_id == 3) DMBiomassLossProportions[disturbance_matrix_id].SWO  = lossProportion;
					else if (source_pool_id == 4) DMBiomassLossProportions[disturbance_matrix_id].SWCR = lossProportion;
					else if (source_pool_id == 5) DMBiomassLossProportions[disturbance_matrix_id].SWFR = lossProportion;
					else if (source_pool_id == 6) DMBiomassLossProportions[disturbance_matrix_id].HWM  = lossProportion;
					else if (source_pool_id == 7) DMBiomassLossProportions[disturbance_matrix_id].HWF  = lossProportion;
					else if (source_pool_id == 8) DMBiomassLossProportions[disturbance_matrix_id].HWO  = lossProportion;
					else if (source_pool_id == 9) DMBiomassLossProportions[disturbance_matrix_id].HWCR = lossProportion;
					else if (source_pool_id == 10) DMBiomassLossProportions[disturbance_matrix_id].HWFR = lossProportion;
					else {
						auto ex = SawtoothException(Sawtooth_DBQueryError);
						ex.Message << "expected source pool between 1 and 10. Got " << source_pool_id;
						throw ex;
					}
				}
			}

			ParameterTable<ParameterCore> _ParameterCore;

			ParameterTable<ParameterRecruitmentD1> _ParameterRecruitmentD1;
			ParameterTable<ParameterGrowthD1> _ParameterGrowthD1;
			ParameterTable<ParameterMortalityD1> _ParameterMortalityD1;

			ParameterTable<ParameterRecruitmentD2> _ParameterRecruitmentD2;
			ParameterTable<ParameterGrowthD2> _ParameterGrowthD2;
			ParameterTable<ParameterMortalityD2> _ParameterMortalityD2;

			ParameterTable<ParameterGrowthES1> _ParameterGrowthES1;
			ParameterTable<ParameterMortalityES1> _ParameterMortalityES1;

			ParameterTable<ParameterGrowthES2> _ParameterGrowthES2;
			ParameterTable<ParameterMortalityES2> _ParameterMortalityES2;

			ParameterTable<ParameterGrowthES3> _ParameterGrowthES3;

			ParameterTable<ParameterMortalityMLR35> _ParameterMortalityMLR35;

			ParameterTable<CBM::RootParameter> _RootParameter;
			ParameterTable<CBM::TurnoverParameter> _TurnoverParameter;
			ParameterTable<CBM::StumpParameter> _StumpParameter;
			std::unordered_map<int, Sawtooth_CBMBiomassPools> DMBiomassLossProportions;
			std::unordered_map<int, std::unordered_map<int, int>> dmAssociations;
			std::unordered_map<int, std::unordered_map<int, double>> _biomassC_utilizationLevel;
			std::unordered_set<int> SoftwoodSpecies;
			std::unordered_set<int> HardwoodSpecies;

		public:
			ParameterSet(DBConnection& conn, Sawtooth_ModelMeta meta) : Conn(conn)
			{
				InitializeSawtoothEquationSet("Core", _ParameterCore);
				_constants = LoadConstants();
				LoadDisturbanceTypes();
				switch (meta.growthModel)
				{
				case Sawtooth_GrowthD1:
					InitializeSawtoothEquationSet("GrowthD1", _ParameterGrowthD1);
					break;
				case Sawtooth_GrowthD2:
					InitializeSawtoothEquationSet("GrowthD2", _ParameterGrowthD2);
					break;
				case Sawtooth_GrowthES1:
					InitializeSawtoothEquationSet("GrowthES1", _ParameterGrowthES1);
					break;
				case Sawtooth_GrowthES2:
					InitializeSawtoothEquationSet("GrowthES2", _ParameterGrowthES2);
					break;
				case Sawtooth_GrowthES3:
					InitializeSawtoothEquationSet("GrowthES3", _ParameterGrowthES3);
					break;
				default:
					auto ex = SawtoothException(Sawtooth_ModelMetaError);
					ex.Message<< "specified growth model invalid";
					throw ex;
				}
				switch (meta.mortalityModel)
				{
				case Sawtooth_MortalityNone:
				case Sawtooth_MortalityConstant:
					break;
				case Sawtooth_MortalityD1:
					InitializeSawtoothEquationSet("MortalityD1", _ParameterMortalityD1);
					break;
				case Sawtooth_MortalityD2:
					InitializeSawtoothEquationSet("MortalityD2", _ParameterMortalityD2);
					break;
				case Sawtooth_MortalityES1:
					InitializeSawtoothEquationSet("MortalityES1", _ParameterMortalityES1);
					break;
				case Sawtooth_MortalityES2:
					InitializeSawtoothEquationSet("MortalityES2", _ParameterMortalityES2);
					break;
				case Sawtooth_MortalityMLR35:
					InitializeSawtoothEquationSet("MortalityMLR35", _ParameterMortalityMLR35);
					break;
				default:
					auto ex = SawtoothException(Sawtooth_ModelMetaError);
					ex.Message << "specified mortality model invalid";
					throw ex;
				}
				switch (meta.recruitmentModel)
				{
				case Sawtooth_RecruitmentD1:
					InitializeSawtoothEquationSet("RecruitmentD1",
						_ParameterRecruitmentD1);
					break;
				case Sawtooth_RecruitmentD2:
					InitializeSawtoothEquationSet("RecruitmentD1",
						_ParameterRecruitmentD1);
					break;
				default:
					auto ex = SawtoothException(Sawtooth_ModelMetaError);
					ex.Message << "specified recruitment model invalid";
					throw ex;
				}
				if (meta.CBMEnabled) {
					LoadCBMParameters("CBMRootParameters", cbm_root_parameter_query, _RootParameter);
					LoadCBMParameters("CBMTurnoverParameter", cbm_turnover_parameter_query, _TurnoverParameter);
					LoadCBMParameters("CBMStumpParameter", cbm_stump_parameter_query, _StumpParameter);
					LoadBiomassCUtilizationLevels();
					LoadDisturbanceMatrixAssocations();
					LoadDisturbanceMatrixBiomassLosses();
					for (const auto s : _ParameterCore.GetCollection()) {
						if (s.second->DeciduousFlag) {
							HardwoodSpecies.insert(s.first);
						}
						else {
							SoftwoodSpecies.insert(s.first);
						}
					}
				}
			}
			
			const std::shared_ptr<ParameterCore> GetParameterCore(int key) const {
				return _ParameterCore.GetParameter("ParameterCore", key);
			}
			const std::shared_ptr<ParameterRecruitmentD1> GetParameterRecruitmentD1(int key) const {
				return _ParameterRecruitmentD1.GetParameter("ParameterRecruitmentD1", key);
			}
			const std::shared_ptr<ParameterGrowthD1> GetParameterGrowthD1(int key) const {
				return _ParameterGrowthD1.GetParameter("ParameterGrowthD1", key);
			}
			const std::shared_ptr<ParameterMortalityD1> GetParameterMortalityD1(int key) const {
				return _ParameterMortalityD1.GetParameter("ParameterMortalityD1", key);
			}
			const std::shared_ptr<ParameterRecruitmentD2> GetParameterRecruitmentD2(int key) const {
				return _ParameterRecruitmentD2.GetParameter("ParameterRecruitmentD2", key);
			}
			const std::shared_ptr<ParameterGrowthD2> GetParameterGrowthD2(int key) const {
				return _ParameterGrowthD2.GetParameter("ParameterGrowthD2", key);
			}
			const std::shared_ptr<ParameterMortalityD2> GetParameterMortalityD2(int key) const {
				return _ParameterMortalityD2.GetParameter("ParameterMortalityD2", key);
			}
			const std::shared_ptr<ParameterGrowthES1> GetParameterGrowthES1(int key) const {
				return _ParameterGrowthES1.GetParameter("ParameterGrowthES1", key);
			}
			const std::shared_ptr<ParameterMortalityES1> GetParameterMortalityES1(int key) const {
				return _ParameterMortalityES1.GetParameter("ParameterMortalityES1", key);
			}
			const std::shared_ptr<ParameterGrowthES2> GetParameterGrowthES2(int key) const {
				return _ParameterGrowthES2.GetParameter("ParameterGrowthES2", key);
			}
			const std::shared_ptr<ParameterMortalityES2> GetParameterMortalityES2(int key) const {
				return _ParameterMortalityES2.GetParameter("ParameterMortalityES2", key);
			}
			const std::shared_ptr<ParameterGrowthES3> GetParameterGrowthES3(int key) const {
				return _ParameterGrowthES3.GetParameter("ParameterGrowthES3", key);
			}
			const std::shared_ptr<ParameterMortalityMLR35> GetParameterMortalityMLR35(int key) const {
				return _ParameterMortalityMLR35.GetParameter("ParameterMortalityMLR35", key);
			}

			const Constants GetConstants() const { return _constants; }

			const std::shared_ptr<DisturbanceType> GetDisturbanceType(int id) const {
				auto match = DisturbanceTypes.find(id);
				if (match == DisturbanceTypes.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterNameError);
					ex.Message << "specified disturbance type id not found" << id;
					throw ex;
				}
				return match->second;
			}

			const std::shared_ptr<CBM::RootParameter> GetRootParameter(int id) const {
				return _RootParameter.GetParameter("RootParameter", id);
			}

			const std::shared_ptr <CBM::TurnoverParameter> GetTurnoverParameter(int id) const {
				return _TurnoverParameter.GetParameter("TurnoverParameter", id);
			}

			const std::shared_ptr<CBM::StumpParameter> GetStumpParameter(int id) const {
				return _StumpParameter.GetParameter("StumpParameter", id);
			}

			double GetBiomassCUtilizationLevel(int region_id, int species_id) const {
				auto region = _biomassC_utilizationLevel.find(region_id);
				if (region == _biomassC_utilizationLevel.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "biomass utilization level: specified region_id not found" << region_id;
					throw ex;
				}
				auto species_value = region->second.find(species_id);
				if (species_value == region->second.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "biomass utilization level: specified species_id not found" << species_id;
					throw ex;
				}
				return species_value->second;
			}

			Sawtooth_CBMBiomassPools GetDisturbanceBiomassLossProportions(int region_id, int disturbance_type_id) const {
				auto regionMatch = dmAssociations.find(region_id);
				if (regionMatch == dmAssociations.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "disturbance biomass loss proportions: specified region_id not found" << region_id;
					throw ex;
				}
				auto dist_type_match = regionMatch->second.find(disturbance_type_id);
				if (dist_type_match == regionMatch->second.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "disturbance biomass loss proportions: specified disturbance_type_id not found" << disturbance_type_id;
					throw ex;
				}
				int dmid = dist_type_match->second;
				auto match = DMBiomassLossProportions.find(dmid);
				if (match == DMBiomassLossProportions.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "disturbance biomass loss proportions: specified disturbance matrix id not found" << dmid;
					throw ex;
				}
				return match->second;
			}

			const std::unordered_set<int> GetSoftwoodSpecies() const {
				return SoftwoodSpecies;
			}

			const std::unordered_set<int> GetHardwoodSpecies() const {
				return HardwoodSpecies;
			}
		};
	}
}

#endif 

