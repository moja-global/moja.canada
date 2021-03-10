#ifndef MOJA_MODULES_CBM_STANDGROWTHCURVE_H_
#define MOJA_MODULES_CBM_STANDGROWTHCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/treeyieldtable.h"
#include "moja/modules/cbm/foresttypeconfiguration.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API StandGrowthCurve {
	public:
		virtual ~StandGrowthCurve() {};

		StandGrowthCurve(Int64 standGrowthCurveID, Int64 spuID);
		
		Int64 standGrowthCurveID() const { return _standGrowthCurveID; }
        Int64 spuID() const { return _spuID; }
		int standMaxAge() const { return _standMaxAge; }		

		void addYieldTable(TreeYieldTable& yieldTable);	
		void processStandYieldTables();
		bool hasYieldComponent(SpeciesType componentType);
		double getStandTotalVolumeAtAge(int age) const;
		double getStandSoftwoodVolumeRatioAtAge(int age) const;

		// Get the stand age at which the stand has the maximum merchantable volume
		int getStandAgeWithMaximumVolume() const;

		// Get the maximum stand merchantable volume at an age
		double getAnnualStandMaximumVolume() const;

		std::shared_ptr<const PERDFactor> getPERDFactor(SpeciesType speciesType) const;
		void setPERDFactor(std::shared_ptr<PERDFactor> value, SpeciesType);

        const ForestTypeConfiguration& getForestTypeConfiguration(SpeciesType speciesType) const;
        void setForestTypeConfiguration(const ForestTypeConfiguration& value, SpeciesType);

    private:
		void resolveStandGrowthCurveMaxAge();
		void initStandYieldDataStorage();
		void checkAndUpdateYieldTables();
		void summarizeStandComponentYieldTables();
		void updateStandMaximumVolumeAgeInfo();

		Int64 _standGrowthCurveID;
        Int64 _spuID;
		int _standMaxAge;
		int _standAgeForMaximumMerchVolume;
		double _standMaximumMerchVolume;	
		bool _okToSmooth;

		std::shared_ptr<PERDFactor> _swPERDFactor;
		std::shared_ptr<PERDFactor> _hwPERDFactor;

        ForestTypeConfiguration _swForestTypeConfiguration;
        ForestTypeConfiguration _hwForestTypeConfiguration;

		std::vector<TreeYieldTable> _softwoodYieldTables;
		std::vector<TreeYieldTable> _hardwoodYieldTables;

		std::vector<double> _standMerchVolumeAtEachAge;
		std::vector<double> _standSoftwoodVolumeRatioAtEachAge;
	};

}}}
#endif