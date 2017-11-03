#ifndef sawtooth_equation_set_h
#define sawtooth_equation_set_h

#include <string>
#include <map>
#include "sawtoothexception.h"

namespace Sawtooth {
	namespace Parameter {
		//wrapper for std::map purely for better error handling
		class EquationSet {
		private:
			std::string EquationSetName;
			std::map<std::string, double> Values;
		public:
			EquationSet() { }
			EquationSet(std::string equationSetName) {
				EquationSetName = equationSetName;
			}
			void AddValue(const std::string& name, double value) {
				Values[name] = value;
			}
			double at(const std::string& key) const {
				auto match = Values.find(key);
				if (match == Values.end()) {
					auto err = SawtoothException(Sawtooth_ParameterNameError);
					err.Message << "parameter '" << key <<
						"' not found in equation set '" <<
						EquationSetName << "'";
					throw err;
				}
				return match->second;
			}
		};
	}
}
#endif