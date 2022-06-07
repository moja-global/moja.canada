/**
* @file
* @brief 
*
* ******/
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

			 /**
             * @brief Configuration function.
             * 
             * @param config DynamicObject&
             * @return void
             * ************************/
			void CBMPeatlandSpinupOutput::configure(const DynamicObject& config) {}

			/**
            * @brief Subscribe to the signals LocalDomianInit,TimingInit,TimigEndStep,TimingStep,
			* LocalDomainShutDown,DisturbanceEvent and PrePostDisturbanceEvent.
            * 
            * @param notificationCenter NotificationCenter&
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &CBMPeatlandSpinupOutput::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::TimingInit, &CBMPeatlandSpinupOutput::onTimingInit, *this);
				notificationCenter.subscribe(signals::TimingEndStep, &CBMPeatlandSpinupOutput::onTimingEndStep, *this);
				notificationCenter.subscribe(signals::TimingStep, &CBMPeatlandSpinupOutput::onTimingStep, *this);
				notificationCenter.subscribe(signals::LocalDomainShutdown, &CBMPeatlandSpinupOutput::onLocalDomainShutdown, *this);
				notificationCenter.subscribe(signals::DisturbanceEvent, &CBMPeatlandSpinupOutput::onDisturbanceEvent, *this);
				notificationCenter.subscribe(signals::PrePostDisturbanceEvent, &CBMPeatlandSpinupOutput::onPrePostDisturbanceEvent, *this);
			}

			/**
            * @brief Get TimeStamp Using Local Date Time.
            * 
            * @return string
            * ************************/
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

			/**
            * @brief Perform at start of simulation.
            * 
			* Assign CBMPeatlandSpinupOutput._peatland_spinup_rotation as peatland_spinup_rotation variable,
			* CBMPeatlandSpinupOutput._tree_age as peatland_smalltree_age variable,CBMPeatlandSpinupOutput._shrub_age as
			* peatland_shrub_age variable and CBMPeatlandSpinupOutput._stand_age as age variable. \n
			* if CBMPeatlandSpinupOutput._isSpinupFileCreated is false and _landUnitData has spinup_output_file variable. \n
			* Assign a string variable fileName. \n
			* Check if the file exist, open the file using CBMPeatlandSpinupOutput._timeStepOutputFile and 
			* add each pool from the poolCollection to CBMPeatlandSpinupOutput.timeStepOutputFile. \n
			* Else open the file using CBMPeatlandSpinupOutput._timeStepOutputFile. \n
			* Assign CBMPeatlandSpinupOutput._isOutputLog and CBMPeatlandSpinupOutput._isSpinupFileCreated to true.
			* 
            * @return void
            * ************************/
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

			/**
            * @brief Perform at start of simulation.
            * 
			* Initialise a variable peatland_class as peatland_class value. \n
			* If peatland_class is empty,assign CBMPeatlandSpinupOutput._peatlandId as -1. \n
			* Else assign it as peatland_class (int). \n
			* If CBMPeatlandSpinupOutput._peatland is greater than 0,
			* Assign CBMPeatlandSpinupOutput._runPeatland to true and fireReturnInterval as fire_return_interval value. \n
			* If fireReturnInterval is empty, assign CBMPeatlandSpinupOutput._fireReturnIntervalValue as -1. \n
			* Else assign it as fireReturnInterval (int).
			* 
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::doTimingInit() {
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				_peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();
				if (_peatlandId > 0) {
					_runPeatland = true;
					auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
					_fireReturnIntervalValue = fireReturnInterval.isEmpty() ? -1 : fireReturnInterval.convert<int>();
				}
			}
			/**
            * @brief Perform on each timing step.
			* 
			* Initialise a boolean variable cached as peat_pool_cached value.\n
			* if CBMPeatlandSpinupOutput._isOutputLog,CBMPeatlandSpinupOutput._isSpinupFileCreated are true and cached is false,
			* invoke outputPoolValues().
            * 
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::doTimingStep() {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}
			/**
            * @brief When a disturbance event occurs.
			* 
			* Initialise a boolean variable cached as peat_pool_cached value.\n
			* if CBMPeatlandSpinupOutput._isOutputLog,CBMPeatlandSpinupOutput._isSpinupFileCreated are true and cached is false
			* invoke outputPoolValues().
            * 
			* @param n DynamicVar
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::doDisturbanceEvent(DynamicVar n) {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}
			/**
            * @brief doPrePostDisturbanceEvent.
			* 
			* Initialise a boolean variable cached as peat_pool_cached value.\n
			* if CBMPeatlandSpinupOutput._isOutputLog,CBMPeatlandSpinupOutput._isSpinupFileCreated are true and cached is false
			* invoke outputPoolValues().
            * 
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::doPrePostDisturbanceEvent() {
				bool cached = _landUnitData->getVariable("peat_pool_cached")->value();
				if (_isOutputLog && _isSpinupFileCreated && !cached) {
					outputPoolValues();
				}
			}

			/**
            * @brief Print Pool Values.
			* 
            * If CBMPeatlandSpinupOutput._runPeatland and CBMPeatlandSpinupOutput._isOutputLog are true,
			* Intialise int variables rotation as CBMPeatlandSpinupOutput._peatland_spin_rotation value,
			* treeAge as CBMPeatlandSpinupOutput._tree_age value, shrubAge as _shrub_age value and
			* standAge as CBMPeatlandSpinupOutput._stand_age value. \n
			* Add CBMPeatlandSpinupOutput._peatland, CBMPeatlandSpinupOutput._fireReturnintervalValue, rotation and shrubAge to
			* CBMPeatlandSpinupOutput.timeStepOutputFile. \n
			* Add each pool from the pool collection to CBMPeatlandSpinupOutput.timeStepOutputFile.
			* 
			* 
			* 
            * @return void
            * ************************/
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

			/**
            * @brief Event called when the Thread dies.
            * 
			* if CBMPeatlandSpinupOutput.isSpinUpFileCreated,
			* invoke CBMPeatlandSpinupOutput.timeStepOutputFile.flush()and CBMPeatlandSpinupOutput.timeStepOutputFile.close().
			* 
            * @return void
            * ************************/
			void CBMPeatlandSpinupOutput::doLocalDomainShutdown() {
				if (_isSpinupFileCreated) {
					timeStepOutputFile.flush();
					timeStepOutputFile.close();
				}
			}
		}
	}
}