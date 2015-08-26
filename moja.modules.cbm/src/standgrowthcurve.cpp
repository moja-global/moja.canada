#include "moja/modules/cbm/standgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {

	StandGrowthCurve::StandGrowthCurve(){}

	StandGrowthCurve::StandGrowthCurve(Int64 standGrowthCurveID){
		_standGrowthCurveID = standGrowthCurveID;		
		 _standMaxAge = 0;
		 _standAgeForMaximumMerchVolume = 0;
		 _standMaximumMerchVolume = 0;
	}

	void StandGrowthCurve::addYieldTable(TreeYieldTable::Ptr yieldTable){
		if (yieldTable->speciesType() == SpeciesType::Softwood) {
			_softwoodYieldTables.push_back(yieldTable);
		}
		else {
			_hardwoodYieldTables.push_back(yieldTable);
		}
	}

	bool StandGrowthCurve::hasYieldComponent(SpeciesType componentType){
		bool found = false;

		if (componentType == SpeciesType::Softwood && _swPERDFactor != nullptr && _softwoodYieldTables.size() > 0){
			found = true;			
		}
		else if (componentType == SpeciesType::Hardwood && _hwPERDFactor != nullptr &&  _hardwoodYieldTables.size() > 0){
			found = true;
		}

		return found;
	}

	std::shared_ptr<const PERDFactor> StandGrowthCurve::getPERDFactor(SpeciesType speciesType) const
	{
		if (speciesType == SpeciesType::Softwood){
			return _swPERDFactor;
		}
		else if (speciesType == SpeciesType::Hardwood){
			return _hwPERDFactor;
		}
		// TODO: Check this, should there be a default return? There was a compiler warning on control path not returning
		return {};
	}

	void StandGrowthCurve::setPERDFactor(std::shared_ptr<PERDFactor> value, SpeciesType speciesType){
		if (speciesType == SpeciesType::Softwood){
			_swPERDFactor = value;
		}
		else if (speciesType == SpeciesType::Hardwood){
			_hwPERDFactor = value;
		}
	}

	double StandGrowthCurve::getStandTotalVolumeAtAge(int age) const {
		return _standMerchVolumeAtEachAge[age];
	}

	double StandGrowthCurve::getStandSoftwoodVolumeRationAtAge(int age) const {
		return _standSoftwoodVolumeRatioAtEachAge[age];
	}

	void StandGrowthCurve::processStandYieldTables(){
		resolveStandGrowthCurveMaxAge();
		initStandYieldDataStorage();
		checkAndUpdateYieldTables();
		summarizeStandComponentYieldTables();
		updateStandMaximumVolumeAgeInfo();
	}

	void StandGrowthCurve::resolveStandGrowthCurveMaxAge(){
		_standMaxAge = 0;

		for (const auto& yieldData : _softwoodYieldTables) {
			if (yieldData->maxAge() > _standMaxAge) {
				_standMaxAge = yieldData->maxAge();
			}
		}

		for (const auto& yieldData : _hardwoodYieldTables) {
			if (yieldData->maxAge() > _standMaxAge) {
				_standMaxAge = yieldData->maxAge();
			}
		}		
	}

	int StandGrowthCurve::getStandAgeWithMaximumVolume() const {
		return _standAgeForMaximumMerchVolume;
	}

	double StandGrowthCurve::getAnnualStandMaximumVolume() const {
		return _standMaximumMerchVolume;
	}

	void StandGrowthCurve::initStandYieldDataStorage(){
		_standMerchVolumeAtEachAge.resize(_standMaxAge + 1);
		_standSoftwoodVolumeRatioAtEachAge.resize(_standMaxAge + 1);
	}

	/*
	* Both softwood and hardwood components should have valid volume data for each age up to the maximum stand age.
	* Each yield table must have the same pairs of [age, volume], the age should be up to the maximum stand age.
	* If one yield table has less data of volume, repeatedly append the yield data with the last available volume.
	* Use vector.push_back(lastAvailableVolume) instead of use resize() and then assign value. 	
	*/
	void StandGrowthCurve::checkAndUpdateYieldTables(){			
		int tableSize = 0;

		if (!_softwoodYieldTables.empty()){
			//loop over the softwood yield tables
			for (auto& yieldData : _softwoodYieldTables) {
				auto& yieldsAtEachAge = yieldData->yieldsAtEachAge();

				// table size = maximum age + 1, each table
				tableSize = yieldsAtEachAge.size();

				// no need to work on the empty yield table if yield table size is zero
				if (tableSize > 0 && tableSize != (_standMaxAge + 1)) {
					for (int age = tableSize; age <= _standMaxAge; age++) {
						yieldsAtEachAge.push_back(yieldsAtEachAge[tableSize - 1]);
					}
				}
			}
		}

		if (!_hardwoodYieldTables.empty()){
			//loop over the softwood yield tables
			for (auto& yieldData : _hardwoodYieldTables) {
				auto& yieldsAtEachAge = yieldData->yieldsAtEachAge();
				tableSize = yieldsAtEachAge.size();
				if (tableSize > 0 && tableSize != (_standMaxAge + 1)) {					
					for (int age = tableSize; age <= _standMaxAge; age++) {
						yieldsAtEachAge.push_back(yieldsAtEachAge[tableSize-1]);
					}					
				}
			}
		}
	}

	void StandGrowthCurve::summarizeStandComponentYieldTables(){
		//firstly, try to sumup the softwood
		if (!_softwoodYieldTables.empty()){
			//loop over the softwood yield tables
			for (auto& yieldData : _softwoodYieldTables) {
				const auto& yieldsAtEachAge = yieldData->yieldsAtEachAge();
				if (yieldsAtEachAge.size() > 0) {
					//loop over ages of each yield table [0, maxYieldTableAge]
					for (int age = 0; age <= _standMaxAge; age++) {
						//record the stand total volume at this age as the total softwood volume at this age
						_standMerchVolumeAtEachAge[age] += yieldsAtEachAge[age];
					}
				}
			}
		}

		//secondly, try to sumup the hardwood and update the stand total volume at an age
		//also calculate the softwood ratio at an age

		double hardwoodVolumeTotalAgAge = 0;
		double softwoodVolumetotalAtAge = 0;
		double softwoodRatioAtAge = 0;

		if (!_hardwoodYieldTables.empty()){
			//loop over the stand age [0, _standMaxAge]
			for (int age = 0; age <= _standMaxAge; age++) {
				 hardwoodVolumeTotalAgAge = 0;

				for (auto& yieldData : _hardwoodYieldTables) {
					const auto& yieldsAtEachAge = yieldData->yieldsAtEachAge();
					//addup the hardwood volume at this age
					if (yieldsAtEachAge.size() > 0)
						hardwoodVolumeTotalAgAge += yieldsAtEachAge[age];
				}

				//get the stand softwood volume at this age
				softwoodVolumetotalAtAge = _standMerchVolumeAtEachAge[age];

				//calculate the softwood ratio at this age
				softwoodRatioAtAge = softwoodVolumetotalAtAge / (softwoodVolumetotalAtAge + hardwoodVolumeTotalAgAge);

				if (std::isnan(softwoodRatioAtAge)){
					//if it means divided by ZERO, not a number
					softwoodRatioAtAge = 1;
				}

				//update the stand total volume at this age
				_standMerchVolumeAtEachAge[age] += hardwoodVolumeTotalAgAge;

				//update stand softwood ratio at this age 
				_standSoftwoodVolumeRatioAtEachAge[age] = softwoodRatioAtAge;
			}
		} else{
			for (int age = 0; age <= _standMaxAge; age++){
				_standSoftwoodVolumeRatioAtEachAge[age] = softwoodRatioAtAge;
			}
		}
	}

	void StandGrowthCurve::updateStandMaximumVolumeAgeInfo(){
		_standMaximumMerchVolume = 0;
		for (int age = 0; age <= _standMaxAge; age++) {
			//update the age at which the first maximum stand volume returns
			if(_standMerchVolumeAtEachAge[age] > _standMaximumMerchVolume)
			{
				_standMaximumMerchVolume = _standMerchVolumeAtEachAge[age];
				_standAgeForMaximumMerchVolume = age;
			}
		}
	}

}}}