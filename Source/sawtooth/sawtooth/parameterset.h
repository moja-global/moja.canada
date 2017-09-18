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
#include "modelmeta.h"
namespace Sawtooth {
	namespace Parameter {
		template <class T>
		class ParameterTable {
		private:
			std::unordered_map<int, std::shared_ptr<T>> table;
		public:
			void AddParameter(int key, T parameter) {
				if (table.count(key) > 0) {
					throw SawtoothException(Sawtooth_ParameterNameError, "specified key already found");
				}
				table[key] = std::shared_ptr<T>(new T(parameter));
			}
			std::shared_ptr<T> GetParameter(int key) const {
				auto match = table.find(key);
				if (match == table.end()) {
					throw SawtoothException(Sawtooth_ParameterNameError, "specified key not found");
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

			const std::string parameter_query = 
				"SELECT Sawtooth_Species.id, "
				"    Sawtooth_Parameter.name, "
				"    Sawtooth_Parameter.value "
				"from Sawtooth_Parameter "
				"INNER JOIN Sawtooth_Equation_Set "
				"    on Sawtooth_Parameter.equation_set_id = Sawtooth_Equation_Set.id "
				"INNER JOIN Sawtooth_Species "
				"    on Sawtooth_Parameter.species_id = Sawtooth_Species.id "
				"WHERE Sawtooth_Equation_Set.name = ?";

			Constants _constants;

			std::map<int, std::shared_ptr<DisturbanceType>> DisturbanceTypes;

			std::map<int, std::map<std::string, double>> GetGroupedParameters(
				const std::string equationSetName) {

				auto stmt = Conn.prepare(parameter_query);
				sqlite3_bind_text(stmt, 1, equationSetName.c_str(), -1, SQLITE_STATIC);
				auto c = Cursor(stmt);
				std::map<int, std::map<std::string, double>> groupedValues;
				while(c.MoveNext()) {
					int id = c.GetValueInt32("id");
					std::string parameterName = c.GetValueString("name");
					double parameterValue = c.GetValueDouble("value");
					groupedValues[id][parameterName] = parameterValue;
				}
				return groupedValues;
			}
			template<class T>
			void InitializeParameters(std::string equationSetName, 
				ParameterTable<T>& parameterTable) {
				auto grouped = GetGroupedParameters(equationSetName);
				for (auto g : grouped) {
					T param = T(g.second);
					int key = g.first;
					parameterTable.AddParameter(key, param);
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
						throw SawtoothException(Sawtooth_DBQueryError,
							"probability of mortality must be: 0<=PM<=1");
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
							new DisturbanceType(id, p_mortality, eligibleSpecies));

					DisturbanceTypes[id] = dt;
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


		public:
			ParameterSet(DBConnection& conn, Meta::ModelMeta meta) : Conn(conn) 
			{
				InitializeParameters("SpeciesParameters", _SpeciesParameter);
				_constants = LoadConstants();
				LoadDisturbanceTypes();
				switch (meta.growthModel)
				{
				case Meta::GrowthDefault:
					InitializeParameters("GrowthDefault", _DefaultGrowthParameter);
					break;
				case Meta::GrowthES1:
					InitializeParameters("GrowthES1", _ES1GrowthParameter);
					break;
				case Meta::GrowthES2:
					InitializeParameters("GrowthES2", _ES2GrowthParameter);
					break;
				case Meta::GrowthES3:
					InitializeParameters("GrowthES3", _ES3GrowthParameter);
					break;
				default:
					throw SawtoothException(Sawtooth_ModelMetaError, "specified growth model invalid");
				}
				switch (meta.mortalityModel)
				{
				case Meta::MortalityNone:
				case Meta::MortalityConstant:
					break;
				case Meta::MortalityDefault:
					InitializeParameters("MortalityDefault", _DefaultMortalityParameter);
					break;
				case Meta::MortalityES1:
					InitializeParameters("MortalityES1", _ES1MortalityParameter);
					break;
				case Meta::MortalityES2:
					InitializeParameters("MortalityES2", _ES2MortalityParameter);
					break;
				case Meta::MortalityMLR35:
					InitializeParameters("MortalityMLR35", _MLR35MortalityParameter);
					break;
				default:
					throw SawtoothException(Sawtooth_ModelMetaError, "specified mortality model invalid");
				}
				switch (meta.recruitmentModel)
				{
				case Meta::RecruitmentDefault:
					InitializeParameters("RecruitmentDefault", _DefaultRecruitmentParameter);
					break;
				default:
					throw SawtoothException(Sawtooth_ModelMetaError, "specified recruitment model invalid");
				}
			}
			
			const std::shared_ptr<SpeciesParameter> GetSpeciesParameter(int key) const {
				return _SpeciesParameter.GetParameter(key);
			}
			const std::shared_ptr<DefaultRecruitmentParameter> GetDefaultRecruitmentParameter(int key) const {
				return _DefaultRecruitmentParameter.GetParameter(key);
			}
			const std::shared_ptr<DefaultGrowthParameter> GetDefaultGrowthParameter(int key) const {
				return _DefaultGrowthParameter.GetParameter(key);
			}
			const std::shared_ptr<DefaultMortalityParameter> GetDefaultMortalityParameter(int key) const {
				return _DefaultMortalityParameter.GetParameter(key);
			}
			const std::shared_ptr<ES1GrowthParameter> GetES1GrowthParameter(int key) const {
				return _ES1GrowthParameter.GetParameter(key);
			}
			const std::shared_ptr<ES1MortalityParameter> GetES1MortalityParameter(int key) const {
				return _ES1MortalityParameter.GetParameter(key);
			}
			const std::shared_ptr<ES2GrowthParameter> GetES2GrowthParameter(int key) const {
				return _ES2GrowthParameter.GetParameter(key);
			}
			const std::shared_ptr<ES2MortalityParameter> GetES2MortalityParameter(int key) const {
				return _ES2MortalityParameter.GetParameter(key);
			}
			const std::shared_ptr<ES3GrowthParameter> GetES3GrowthParameter(int key) const {
				return _ES3GrowthParameter.GetParameter(key);
			}
			const std::shared_ptr<MLR35MortalityParameter> GetMLR35MortalityParameter(int key) const {
				return _MLR35MortalityParameter.GetParameter(key);
			}

			const Constants GetConstants() const { return _constants; }

			const std::shared_ptr<DisturbanceType> GetDisturbanceType(int id) const {
				return DisturbanceTypes.at(id);
			}
		};
	}
}

#endif 

