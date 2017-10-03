#ifndef parameter_set_h
#define parameter_set_h
#include <unordered_map>
#include <tuple>
#include <vector>
#include <memory>
#include "dblayer.h"
#include "constants.h"
#include "disturbance_type.h"
#include "defaultparameter.h"
#include "es1parameter.h"
#include "es2parameter.h"
#include "es3parameter.h"
#include "mlr35Parameter.h"
#include "speciesparameter.h"
#include "cbmparameter.h"
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

			ParameterTable<SpeciesParameter> _SpeciesParameter;

			ParameterTable<DefaultRecruitmentParameter> _DefaultRecruitmentParameter;
			ParameterTable<DefaultGrowthParameter> _DefaultGrowthParameter;
			ParameterTable<DefaultMortalityParameter> _DefaultMortalityParameter;

			ParameterTable<ES1GrowthParameter> _ES1GrowthParameter;
			ParameterTable<ES1MortalityParameter> _ES1MortalityParameter;

			ParameterTable<ES2GrowthParameter> _ES2GrowthParameter;
			ParameterTable<ES2MortalityParameter> _ES2MortalityParameter;

			ParameterTable<ES3GrowthParameter> _ES3GrowthParameter;

			ParameterTable<MLR35MortalityParameter> _MLR35MortalityParameter;

			ParameterTable<CBM::RootParameter> _RootParameter;
			ParameterTable<CBM::TurnoverParameter> _TurnoverParameter;
			ParameterTable<CBM::StumpParameter> _StumpParameter;

			std::unordered_map<int, std::unordered_map<int, double>> _biomassC_utilizationLevel;

		public:
			ParameterSet(DBConnection& conn, Sawtooth_ModelMeta meta) : Conn(conn)
			{
				InitializeSawtoothEquationSet("SpeciesParameters", _SpeciesParameter);
				_constants = LoadConstants();
				LoadDisturbanceTypes();
				switch (meta.growthModel)
				{
				case Sawtooth_GrowthDefault:
					InitializeSawtoothEquationSet("GrowthDefault", _DefaultGrowthParameter);
					break;
				case Sawtooth_GrowthES1:
					InitializeSawtoothEquationSet("GrowthES1", _ES1GrowthParameter);
					break;
				case Sawtooth_GrowthES2:
					InitializeSawtoothEquationSet("GrowthES2", _ES2GrowthParameter);
					break;
				case Sawtooth_GrowthES3:
					InitializeSawtoothEquationSet("GrowthES3", _ES3GrowthParameter);
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
				case Sawtooth_MortalityDefault:
					InitializeSawtoothEquationSet("MortalityDefault", _DefaultMortalityParameter);
					break;
				case Sawtooth_MortalityES1:
					InitializeSawtoothEquationSet("MortalityES1", _ES1MortalityParameter);
					break;
				case Sawtooth_MortalityES2:
					InitializeSawtoothEquationSet("MortalityES2", _ES2MortalityParameter);
					break;
				case Sawtooth_MortalityMLR35:
					InitializeSawtoothEquationSet("MortalityMLR35", _MLR35MortalityParameter);
					break;
				default:
					auto ex = SawtoothException(Sawtooth_ModelMetaError);
					ex.Message << "specified mortality model invalid";
					throw ex;
				}
				switch (meta.recruitmentModel)
				{
				case Sawtooth_RecruitmentDefault:
					InitializeSawtoothEquationSet("RecruitmentDefault",
						_DefaultRecruitmentParameter);
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
				}
			}
			
			const std::shared_ptr<SpeciesParameter> GetSpeciesParameter(int key) const {
				return _SpeciesParameter.GetParameter("SpeciesParameter", key);
			}
			const std::shared_ptr<DefaultRecruitmentParameter> GetDefaultRecruitmentParameter(int key) const {
				return _DefaultRecruitmentParameter.GetParameter("DefaultRecruitmentParameter", key);
			}
			const std::shared_ptr<DefaultGrowthParameter> GetDefaultGrowthParameter(int key) const {
				return _DefaultGrowthParameter.GetParameter("DefaultGrowthParameter", key);
			}
			const std::shared_ptr<DefaultMortalityParameter> GetDefaultMortalityParameter(int key) const {
				return _DefaultMortalityParameter.GetParameter("DefaultMortalityParameter", key);
			}
			const std::shared_ptr<ES1GrowthParameter> GetES1GrowthParameter(int key) const {
				return _ES1GrowthParameter.GetParameter("ES1GrowthParameter", key);
			}
			const std::shared_ptr<ES1MortalityParameter> GetES1MortalityParameter(int key) const {
				return _ES1MortalityParameter.GetParameter("ES1MortalityParameter", key);
			}
			const std::shared_ptr<ES2GrowthParameter> GetES2GrowthParameter(int key) const {
				return _ES2GrowthParameter.GetParameter("ES2GrowthParameter", key);
			}
			const std::shared_ptr<ES2MortalityParameter> GetES2MortalityParameter(int key) const {
				return _ES2MortalityParameter.GetParameter("ES2MortalityParameter", key);
			}
			const std::shared_ptr<ES3GrowthParameter> GetES3GrowthParameter(int key) const {
				return _ES3GrowthParameter.GetParameter("ES3GrowthParameter", key);
			}
			const std::shared_ptr<MLR35MortalityParameter> GetMLR35MortalityParameter(int key) const {
				return _MLR35MortalityParameter.GetParameter("MLR35MortalityParameter", key);
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

			double GetBiomassCUtilizationLevel(int region_id, int species_id) {
				auto region = _biomassC_utilizationLevel.find(region_id);
				if (region == _biomassC_utilizationLevel.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterKeyError);
					ex.Message << "biomass utilization level: specified region_id not found" << region_id;
					throw ex;
				}
				auto species_value = region->second.find(species_id);
				if (species_value == region->second.end()) {
					auto ex = SawtoothException(Sawtooth_ParameterNameError);
					ex.Message << "biomass utilization level: specified species_id not found" << species_id;
					throw ex;
				}
				return species_value->second;
			}
		};
	}
}

#endif 

