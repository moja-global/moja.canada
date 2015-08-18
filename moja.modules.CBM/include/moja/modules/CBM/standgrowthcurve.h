#ifndef CBM_StandGrowthCurve_H_
#define CBM_StandGrowthCurve_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

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

		StandGrowthCurve(Int64 standGrowthCurveID);
		
		Int64 standGrowthCurveID() const { return _standGrowthCurveID; }
		int standMaxAge() const { return _standMaxAge; }		

		void addYieldTable(TreeYieldTable::Ptr yieldTable);	
		
		void processStandYieldTables();

		bool hasYieldComponent(SpeciesType componentType);

		double getStandTotalVolumeAtAge(int age) const ;

		double getStandSoftwoodVolumeRationAtAge(int age) const;

		/*
		* Get the stand age at which the stand has the maximum merchantable volume
		*/
		int getStandAgeWithMaximumVolume() const;

		/*
		* Get the maximum stand merchantable volume at an age
		*/
		double getAnnualStandMaximumVolume() const;

		std::shared_ptr<const PERDFactor> getPERDFactor(SpeciesType speciesType) const;
		void setPERDFactor(std::shared_ptr<PERDFactor> value, SpeciesType);

		//bool okToSmooth() const { return _okToSmooth; }
		//void setOkToSmooth(bool value) { _okToSmooth = value; }
	private:	
		void resolveStandGrowthCurveMaxAge();
		void initStandYieldDataStorage();
		void checkAndUpdateYieldTables();
		void summarizeStandComponentYieldTables();
		void updateStandMaximumVolumeAgeInfo();

		Int64 _standGrowthCurveID;
		int _standMaxAge;
		int _standAgeForMaximumMerchVolume;
		double _standMaximumMerchVolume;	
		bool _okToSmooth;

		std::shared_ptr<PERDFactor> _swPERDFactor;
		std::shared_ptr<PERDFactor> _hwPERDFactor;

		std::vector<TreeYieldTable::Ptr> _softwoodYieldTables;
		std::vector<TreeYieldTable::Ptr> _hardwoodYieldTables;

		std::vector<double> _standMerchVolumeAtEachAge;
		std::vector<double> _standSoftwoodVolumeRatioAtEachAge;
	};
}}}
#endif