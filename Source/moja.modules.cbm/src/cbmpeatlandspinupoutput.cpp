#include "moja/modules/cbm/cbmpeatlandspinupoutput.h"
#include "moja/modules/cbm/timeseries.h"

#include <moja/flint/variable.h>
#include <moja/flint/ioperation.h>
#include <moja/flint/ipool.h>

#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include <iostream>

namespace moja {
namespace modules {
namespace cbm {
	

    void CBMPeatlandSpinupOutput::configure(const DynamicObject& config) {}

    void CBMPeatlandSpinupOutput::subscribe(NotificationCenter& notificationCenter) { 		
		notificationCenter.subscribe(signals::LocalDomainInit,  &CBMPeatlandSpinupOutput::onLocalDomainInit,  *this);
		notificationCenter.subscribe(signals::TimingInit, &CBMPeatlandSpinupOutput::onTimingInit, *this);			
		notificationCenter.subscribe(signals::TimingEndStep, &CBMPeatlandSpinupOutput::onTimingEndStep, *this);
		notificationCenter.subscribe(signals::LocalDomainShutdown, &CBMPeatlandSpinupOutput::onLocalDomainShutdown, *this);
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

	void CBMPeatlandSpinupOutput::doLocalDomainInit(){
		_peatland_spinup_rotation = _landUnitData->getVariable("peatland_spinup_rotation");
		_tree_age = _landUnitData->getVariable("peatland_smalltree_age");
		_shrub_age = _landUnitData->getVariable("peatland_shrub_age");
		_stand_age = _landUnitData->getVariable("age");			
		
		if(!_isSpinupFileCreated && _landUnitData->hasVariable("spinup_output_file")){
			std::string fileName = _landUnitData->getVariable("spinup_output_file")->value();	
			std::string timeStamp = getTimeStamp();

			if (!fileName.empty()) {				
				fileName = fileName + timeStamp + ".csv";
			}
			else {
				fileName = timeStamp + ".csv";
			}

			MOJA_LOG_INFO << "Spinup output file: " << fileName << std::endl;
			timeStepOutputFile.open(fileName);	

			timeStepOutputFile << "PeatlandID, " << "FireReturnInterval, " << "Rotation, " << "ShrubAge, ";
			int poolIndex = 0;
			for (auto& pool : _landUnitData->poolCollection()) {
				if (poolIndex > 0) timeStepOutputFile << ", ";
				timeStepOutputFile << pool->name();
				poolIndex++;
			}

			timeStepOutputFile << std::endl;	

			_isOutputLog = true;
			_isSpinupFileCreated = true;
		}
	}

    void CBMPeatlandSpinupOutput::doTimingInit() {		
		_runPeatland = _landUnitData->getVariable("run_peatland")->value();
		if (_runPeatland){
			_peatlandID = _landUnitData->getVariable("peatlandId")->value();				
			auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();			
			_fireReturnIntervalValue = fireReturnInterval.isEmpty()? -1 : fireReturnInterval.convert<int>();
		}
    }		

	void CBMPeatlandSpinupOutput::doTimingEndStep() {
		if (_runPeatland && _isOutputLog) {
			int rotation = _peatland_spinup_rotation->value();
			int treeAge = _tree_age->value();
			int shrubAge = _shrub_age->value();
			int standAge = _stand_age->value();

			timeStepOutputFile << _peatlandID <<", "<< _fireReturnIntervalValue <<", "<< rotation << ", "<< shrubAge <<", ";

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
}}}