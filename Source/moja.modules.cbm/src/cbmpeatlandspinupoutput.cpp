#include "moja/modules/cbm/cbmpeatlandspinupoutput.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <iostream>
#include <boost/filesystem.hpp>

namespace moja {
	namespace modules {
		namespace cbm {

			/*
			This class is specially used to output pool value in the peatland spinup
			Fire return interval value must be defined with valid number, no spatial reference.
			For test and verify purpose
			*/

			void CBMPeatlandSpinupOutput::configure(const DynamicObject& config) {}

			void CBMPeatlandSpinupOutput::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMPeatlandSpinupOutput::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMPeatlandSpinupOutput::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingEndStep, &CBMPeatlandSpinupOutput::onTimingEndStep, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMPeatlandSpinupOutput::onTimingStep, *this);
				notificationCenter.subscribe(signals::LocalDomainShutdown, &CBMPeatlandSpinupOutput::onLocalDomainShutdown, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMPeatlandSpinupOutput::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::PrePostDisturbanceEvent, &CBMPeatlandSpinupOutput::onPrePostDisturbanceEvent, *this);
			}

			std::string CBMPeatlandSpinupOutput::getTimeStamp() {
				Poco::LocalDateTime t = Poco::LocalDateTime();
				int year = t.year();
				int month = t.month();
				int day = t.day();
				int hour = t.hour();
				int min = t.minute();
				int sec = t.second();
				std::string s(Poco::format("%4d_%02d_%02d_%02d_%02d_%02d", year, month, day, hour, min, sec));
				return s;
			}

			void CBMPeatlandSpinupOutput::doLocalDomainInit() {
				_peatland_spinup_rotation = _landUnitData->getVariable("peatland_spinup_rotation");
				_tree_age = _landUnitData->getVariable("peatland_smalltree_age");
				_shrub_age = _landUnitData->getVariable("peatland_shrub_age");
				_stand_age = _landUnitData->getVariable("age");

				auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
				int fireReturnIntervalValue = fireReturnInterval.isEmpty() ? 1 : fireReturnInterval.convert<int>();
				auto test_run_id = _landUnitData->getVariable("test_run_id")->value();
				fireReturnIntervalValue = test_run_id;
				if (!_isSpinupFileCreated && _landUnitData->hasVariable("spinup_output_file")) {
					std::string fileName = _landUnitData->getVariable("spinup_output_file")->value();
					std::string timeStamp = getTimeStamp();

					if (!fileName.empty() && fireReturnIntervalValue > 0) {
						fileName = fileName + std::to_string(fireReturnIntervalValue) + ".csv";
					}
					else {
						fileName = timeStamp + ".csv";
					}

					fileNameFixed = fileName;
					bool fileExisted = boost::filesystem::exists(fileName);
					if (!fileExisted) {
						MOJA_LOG_INFO << "Spinup output file: " << fileName << std::endl;
						timeStepOutputFile.open(fileName, std::ios_base::app);

						timeStepOutputFile << "PeatlandID, " << "FireReturnInterval, " << "Rotation, " << "ShrubAge, ";
						int poolIndex = 0;
						for (auto& pool : _landUnitData->poolCollection()) {
							if (poolIndex > 0) timeStepOutputFile << ", ";
							timeStepOutputFile << pool->name();
							poolIndex++;
						}
						timeStepOutputFile << std::endl;
					}
					else {
						timeStepOutputFile.open(fileName, std::ios_base::app);
					}
					_isOutputLog = true;
					_isSpinupFileCreated = true;
				}
			}

			void CBMPeatlandSpinupOutput::doTimingInit() {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();
				if (_peatlandId > 0) {
					_runPeatland = true;
					auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
					_fireReturnIntervalValue = fireReturnInterval.isEmpty() ? -1 : fireReturnInterval.convert<int>();
				}
			}

			void CBMPeatlandSpinupOutput::doTimingStep() {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}
			void CBMPeatlandSpinupOutput::doDisturbanceEvent(DynamicVar n) {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}
			void CBMPeatlandSpinupOutput::doPrePostDisturbanceEvent() {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}
			void CBMPeatlandSpinupOutput::outputPoolValues() {
				if (_runPeatland && _isOutputLog) {
					int rotation = _peatland_spinup_rotation->value();
					int treeAge = _tree_age->value();
					int shrubAge = _shrub_age->value();
					int standAge = _stand_age->value();

					timeStepOutputFile << _peatlandId << ", " << _fireReturnIntervalValue << ", " << rotation << ", " << shrubAge << ", ";

					int poolIndex = 0;
					for (auto& pool : _landUnitData->poolCollection()) {
						if (poolIndex > 0) timeStepOutputFile << ", ";
						timeStepOutputFile << pool->value();
						poolIndex++;
					}
					timeStepOutputFile << std::endl;
				}
			}

			void CBMPeatlandSpinupOutput::doLocalDomainShutdown() {
				if (_isSpinupFileCreated) {
					timeStepOutputFile.flush();
					timeStepOutputFile.close();
				}
			}
		}
	}
}