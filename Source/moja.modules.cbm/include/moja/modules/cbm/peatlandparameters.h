#ifndef MOJA_MODULES_CBM_PLPARAS_H_
#define MOJA_MODULES_CBM_PLPARAS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {
	
	enum class PeatlandType { Bog = 1, PoolFen, RichFen, Swamp };
	enum class PeatlandForestType { Open = 1, Treed, Forested };


	class CBM_API PeatlandParameters {
	public:
		int spuId() const { return _spuId; }
		PeatlandType peatlandType() const { return _peatlandType; }
		PeatlandForestType peatlandTreeClassifier() const { return _peatlandTreeClassifier; }
		

		/// <summary>
		/// Default constructor
		/// </summary>
		PeatlandParameters(){}
		PeatlandParameters(int _spuId, PeatlandType _peatlandType, PeatlandForestType _peatlandTreeClassifier);

		virtual void setValue(const DynamicObject& data) = 0;
		virtual void setDefaultValue(const std::vector<double>& data) = 0;

		private:
			int _spuId;
			PeatlandType _peatlandType;
			PeatlandForestType _peatlandTreeClassifier;
	};
	
}}}
#endif