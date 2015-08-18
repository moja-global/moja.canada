#ifndef CBM_StandGrowthCurve_INCLUDED
#define CBM_StandGrowthCurve_INCLUDED

#include "moja/modules/cbm/_modules.cbmexports.h"
#include "moja/int/modulebase.h"

#include "treespecies.h"
#include "perdfactor.h"
#include "treeyieldtable.h"

namespace moja {
namespace modules {
namespace CBM {
	class CBM_API StandGrowthCurve{
	public:
		StandGrowthCurve();
		virtual ~StandGrowthCurve() {};

		StandGrowthCurve(int standGrowthCurveID);
		
		int standMaxAge() const { return _standMaxAge; }

		const PERDFactor* swPERDFactor() const { return _swPERDFactor.get(); }
		const PERDFactor* hwPERDFactor() const { return _hwPERDFactor.get(); }

		void addYieldTable(TreeYieldTable::Ptr yieldTable);	

		void setPERDFactor(std::unique_ptr<const PERDFactor> value, SpeciesType);

		void processStandYieldTables();
	private:	
		void resolveStandGrowthCurveMaxAge();
		void initStandYieldDataStorage();
		void summarizeStandComponentYieldTables();
		void updateStandMaximumVolumeAgeInfo();

		int _standGrowthCurveID;
		int _standMaxAge;
		int _standAgeForMaximumMerchVolume;
		double _standMaximumMerchVolume;			

		std::unique_ptr<const PERDFactor> _swPERDFactor;
		std::unique_ptr<const PERDFactor> _hwPERDFactor;

		std::vector<TreeYieldTable::Ptr> _softwoodYieldTables;
		std::vector<TreeYieldTable::Ptr> _hardwoodYieldTables;

		std::vector<double> _standMerchVolumeAtEachAge;
		std::vector<double> _standSoftwoodVolumeRatioAtEachAge;
	};
}}}
#endif