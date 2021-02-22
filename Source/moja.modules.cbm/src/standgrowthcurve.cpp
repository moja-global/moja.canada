#include "moja/modules/cbm/standgrowthcurve.h"

namespace moja {
namespace modules {
namespace cbm {

    StandGrowthCurve::StandGrowthCurve(Int64 standGrowthCurveID, Int64 spuID) {
        _standGrowthCurveID = standGrowthCurveID;
        _spuID = spuID;
        _standMaxAge = 0;
        _standAgeForMaximumMerchVolume = 0;
        _standMaximumMerchVolume = 0;
    }

    void StandGrowthCurve::addYieldTable(TreeYieldTable& yieldTable) {
		if (yieldTable.totalVolume() > 0) {
			yieldTable.speciesType() == SpeciesType::Softwood
				? _softwoodYieldTables.push_back(yieldTable)
				: _hardwoodYieldTables.push_back(yieldTable);
		}
    }

    bool StandGrowthCurve::hasYieldComponent(SpeciesType componentType) {
        bool found = false;

        if (componentType == SpeciesType::Softwood &&
            _swPERDFactor != nullptr &&
            _softwoodYieldTables.size() > 0) {

            found = true;			
        }
        else if (componentType == SpeciesType::Hardwood &&
                   _hwPERDFactor != nullptr &&
                   _hardwoodYieldTables.size() > 0) {

            found = true;
        }

        return found;
    }

    std::shared_ptr<const PERDFactor> StandGrowthCurve::getPERDFactor(SpeciesType speciesType) const {
        return speciesType == SpeciesType::Softwood ? _swPERDFactor : _hwPERDFactor;
    }

    void StandGrowthCurve::setPERDFactor(std::shared_ptr<PERDFactor> value, SpeciesType speciesType) {
        speciesType == SpeciesType::Softwood ? _swPERDFactor = value
                                             : _hwPERDFactor = value;
    }

    const ForestTypeConfiguration& StandGrowthCurve::getForestTypeConfiguration(SpeciesType speciesType) const {
        return speciesType == SpeciesType::Softwood ? _swForestTypeConfiguration : _hwForestTypeConfiguration;
    }

    void StandGrowthCurve::setForestTypeConfiguration(const ForestTypeConfiguration& value, SpeciesType speciesType) {
        speciesType == SpeciesType::Softwood ? _swForestTypeConfiguration = value
                                             : _hwForestTypeConfiguration = value;
    }

    double StandGrowthCurve::getStandTotalVolumeAtAge(int age) const {
        return _standMerchVolumeAtEachAge.at(
            age > _standMaxAge ? _standMaxAge : age);
    }

    double StandGrowthCurve::getStandSoftwoodVolumeRatioAtAge(int age) const {
        return _standSoftwoodVolumeRatioAtEachAge.at(
            age > _standMaxAge ? _standMaxAge : age);
    }

    void StandGrowthCurve::processStandYieldTables() {
        resolveStandGrowthCurveMaxAge();
        initStandYieldDataStorage();
        checkAndUpdateYieldTables();
        summarizeStandComponentYieldTables();
        updateStandMaximumVolumeAgeInfo();
    }

    void StandGrowthCurve::resolveStandGrowthCurveMaxAge() {
        _standMaxAge = 0;

        for (const auto& yieldData : _softwoodYieldTables) {
            if (yieldData.maxAge() > _standMaxAge) {
                _standMaxAge = yieldData.maxAge();
            }
        }

        for (const auto& yieldData : _hardwoodYieldTables) {
            if (yieldData.maxAge() > _standMaxAge) {
                _standMaxAge = yieldData.maxAge();
            }
        }		
    }

    int StandGrowthCurve::getStandAgeWithMaximumVolume() const {
        return _standAgeForMaximumMerchVolume;
    }

    double StandGrowthCurve::getAnnualStandMaximumVolume() const {
        return _standMaximumMerchVolume;
    }

    void StandGrowthCurve::initStandYieldDataStorage() {
        _standMerchVolumeAtEachAge.resize(_standMaxAge + 1);
        _standSoftwoodVolumeRatioAtEachAge.resize(_standMaxAge + 1);
    }

    /*
    * Both softwood and hardwood components should have valid volume data for each age up to the maximum stand age.
    * Each yield table must have the same pairs of [age, volume], the age should be up to the maximum stand age.
    * If one yield table has less data of volume, repeatedly append the yield data with the last available volume.
    * Use vector.push_back(lastAvailableVolume) instead of use resize() and then assign value. 	
    */
    void StandGrowthCurve::checkAndUpdateYieldTables() {
        size_t tableSize = 0;

        if (!_softwoodYieldTables.empty()) {
            // Process the softwood yield tables.
            for (auto& yieldData : _softwoodYieldTables) {
                auto& yieldsAtEachAge = yieldData.yieldsAtEachAge();

                // Table size = maximum age + 1, each table.
                tableSize = yieldsAtEachAge.size();

                // No need to work on the empty yield table if yield table size is zero.
                if (tableSize > 0 && tableSize != (_standMaxAge + 1)) {
                    for (auto age = tableSize; age <= _standMaxAge; age++) {
                        yieldsAtEachAge.push_back(yieldsAtEachAge.at(tableSize - 1));
                    }
                }
            }
        }

        if (!_hardwoodYieldTables.empty()) {
            // Process the hardwood yield tables.
            for (auto& yieldData : _hardwoodYieldTables) {
                auto& yieldsAtEachAge = yieldData.yieldsAtEachAge();
                tableSize = yieldsAtEachAge.size();
                if (tableSize > 0 && tableSize != (_standMaxAge + 1)) {					
                    for (auto age = tableSize; age <= _standMaxAge; age++) {
                        yieldsAtEachAge.push_back(yieldsAtEachAge.at(tableSize - 1));
                    }					
                }
            }
        }
    }

    void StandGrowthCurve::summarizeStandComponentYieldTables() {
        // First, try to sum up the softwood.
        if (!_softwoodYieldTables.empty()) {
            // Loop over the softwood yield tables.
            for (auto& yieldData : _softwoodYieldTables) {
                const auto& yieldsAtEachAge = yieldData.yieldsAtEachAge();
                if (yieldsAtEachAge.size() > 0) {
                    // Loop over ages of each yield table [0, maxYieldTableAge].
                    for (int age = 0; age <= _standMaxAge; age++) {
                        // Record the stand total volume at this age as the total
                        // softwood volume at this age.
                        _standMerchVolumeAtEachAge[age] += yieldsAtEachAge.at(age);
                    }
                }
            }
        }

        // Second, try to sum up the hardwood and update the stand total volume
        // at an age. Also calculate the softwood ratio at an age.
        double hardwoodVolumeTotalAgAge = 0;
        double softwoodVolumetotalAtAge = 0;
        double softwoodRatioAtAge = 1.0;
        if (!_hardwoodYieldTables.empty()) {
            // Loop over the stand age [0, _standMaxAge]
            for (int age = 0; age <= _standMaxAge; age++) {
                 hardwoodVolumeTotalAgAge = 0;

                for (auto& yieldData : _hardwoodYieldTables) {
                    const auto& yieldsAtEachAge = yieldData.yieldsAtEachAge();
                    // Add up the hardwood volume at this age.
                    if (yieldsAtEachAge.size() > 0) {
                        hardwoodVolumeTotalAgAge += yieldsAtEachAge.at(age);
                    }
                }

                // Get the stand softwood volume at this age.
                softwoodVolumetotalAtAge = _standMerchVolumeAtEachAge.at(age);

                // Calculate the softwood ratio at this age.
                softwoodRatioAtAge = softwoodVolumetotalAtAge
                    / (softwoodVolumetotalAtAge + hardwoodVolumeTotalAgAge);

                if (std::isnan(softwoodRatioAtAge)) {
                    // If it means divided by ZERO, not a number.
                    softwoodRatioAtAge = 1;
                }

                // Update the stand total volume at this age.
                _standMerchVolumeAtEachAge.at(age) += hardwoodVolumeTotalAgAge;

                // Update stand softwood ratio at this age.
                _standSoftwoodVolumeRatioAtEachAge.at(age) = softwoodRatioAtAge;
            }
        } else {
            for (int age = 0; age <= _standMaxAge; age++) {
                _standSoftwoodVolumeRatioAtEachAge.at(age) = softwoodRatioAtAge;
            }
        }
    }

    void StandGrowthCurve::updateStandMaximumVolumeAgeInfo() {
        _standMaximumMerchVolume = 0;
        for (int age = 0; age <= _standMaxAge; age++) {
            // Update the age at which the first maximum stand volume returns.
            if (_standMerchVolumeAtEachAge.at(age) > _standMaximumMerchVolume) {
                _standMaximumMerchVolume = _standMerchVolumeAtEachAge.at(age);
                _standAgeForMaximumMerchVolume = age;
            }
        }
    }

}}}