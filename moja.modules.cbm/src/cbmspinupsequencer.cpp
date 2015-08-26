#include "moja/modules/cbm/cbmspinupsequencer.h"

using namespace moja::flint;

namespace moja {
	namespace modules {
		namespace cbm {

			bool CBMSpinupSequencer::getSpinupParameters(flint::ILandUnitDataWrapper& landUnitData){
				const auto& spinupParas = landUnitData.getVariable("SpinupParameters")->value()
					.extract<const std::vector<DynamicObject>>();

				if (spinupParas.empty()) {
					return false;
				}

				_ageReturnInterval = spinupParas[0][CBMSpinupSequencer::returnInverval];
				_maxRotationValue = spinupParas[0][CBMSpinupSequencer::maxRotation];
				_histroricDistTypeID = spinupParas[0][CBMSpinupSequencer::histroricDistTypeID];
				_lastDistTypeID = spinupParas[0][CBMSpinupSequencer::lastDistTypeID];
				
				_miniumRotation = landUnitData.getVariable("MinimumRotation")->value();

				_age = landUnitData.getVariable("Age");
				_aboveGroundSlowSoil = landUnitData.getPool("AboveGroundSlowSoil");
				_belowGroundSlowSoil = landUnitData.getPool("BelowGroundSlowSoil");		

				//get the stand age of this landunit
				const auto& landAge = landUnitData.getVariable("InitialAge")->value()
					.extract<const std::vector<DynamicObject>>();
				_standAge = landAge[0]["age"];
				_age->set_value(_standAge);
				
				return true;
			}

			bool CBMSpinupSequencer::Run(NotificationCenter& notificationCenter, ILandUnitController& luc) {
				//get spinup parameters for this landunit
				if (!getSpinupParameters(*_landUnitData)) {
					return false;
				}
			
				bool slowPoolStabled = false;
				bool lastRotation = false;

				int currentRotation = 0;
				double aboveGroundSlowSoil = 0;
				double belowGroundSlowSoil = 0;
				double currentSlowPoolValue = 0;	

				// record total slow pool carbon at the end of previous spinup pass (every 125 steps)
				double _lastSlowPoolValue = 0;	

				notificationCenter.postNotification(std::make_shared<TimingInitNotification>(&luc, _ageReturnInterval, startDate, endDate));
				notificationCenter.postNotification(std::make_shared<TimingPostInitNotification>());

				//loop up to the maximum number of rotation/pass
				while (++currentRotation <= _maxRotationValue) {
					//fire spinup pass, each pass is up to the stand age return interval
					fireSpinupSequenceEvent(notificationCenter, _ageReturnInterval);

					//at the end of each pass, set the current stand age as 0
					_age->set_value(0);

					//get the slow pool values at the end of age interval 
					aboveGroundSlowSoil = _aboveGroundSlowSoil->value();
					belowGroundSlowSoil = _belowGroundSlowSoil->value();
					currentSlowPoolValue = aboveGroundSlowSoil + belowGroundSlowSoil;

					//check if the slow pool is stable
					slowPoolStabled = isSlowPoolStable(_lastSlowPoolValue, currentSlowPoolValue);

					//update previous toal slow pool value
					_lastSlowPoolValue = currentSlowPoolValue;

					if (slowPoolStabled && currentRotation >= _miniumRotation){
						//slow pool is stable, and the minimum rotations are done
						//std::cout << "Slow pool is stable at rotation: " << currentRotation << std::endl;
						
						//set the last rotation flag as true
						lastRotation = true;						
					}								

					if (currentRotation == _maxRotationValue){
						if (!slowPoolStabled){
							//std::cout << "Slow pool is not stable at maximum rotation: " << currentRotation << std::endl;
						}
						//whenever the max rotations are reached, set the last rotation flag as true even if the slow pool is not stable
						lastRotation = true;
					}

					if (lastRotation){
						//CBM spinup is done, notify to simulate the last disturbance							
						notificationCenter.postNotification(std::make_shared<flint::DisturbanceEventNotification>(DynamicObject({ { "disturbance", _lastDistTypeID }})));
						break; //exit the while(rotation) loop
					}
					else{
						//CBM spinup is not done, notify to simulate the historic disturbance		
						notificationCenter.postNotification(std::make_shared<flint::DisturbanceEventNotification>(DynamicObject({ { "disturbance", _histroricDistTypeID }})));	
					}				
				}

				if (lastRotation){				
					//fire up the spinup sequencer to grow the stand to the original stand age
					fireSpinupSequenceEvent(notificationCenter, _standAge);
				}
				
				//notice to report stand pool values here when spinup is done
				notificationCenter.postNotification(std::make_shared<flint::OutputStepNotification>());

				auto tEnd = std::make_shared<flint::TimingShutdownNotification>();
				notificationCenter.postNotification(tEnd);

				return true;				
			}

			bool CBMSpinupSequencer::isSlowPoolStable(double lastSlowPoolValue, double currentSlowPoolValue){
				bool stable = false;
				if (lastSlowPoolValue != 0){
					double var =  currentSlowPoolValue / lastSlowPoolValue;					
					if (var > 0.999 && var < 1.001){
						stable = true;
						//std::cout << "Spinup stable check value: " << var << std::endl;
					}
				}
				return stable;
			}

			void  CBMSpinupSequencer::fireSpinupSequenceEvent(NotificationCenter& notificationCenter, int maximumSteps){
				auto curStepDate = startDate;
				auto endStepDate = startDate;
				const auto timing = _landUnitData->timing();
				for (int curStep = 1; curStep < maximumSteps; curStep++)
				{
					timing->set_startStepDate(curStepDate);
					timing->set_endStepDate(endStepDate);
					timing->set_curStartDate(curStepDate);
					timing->set_curEndDate(endStepDate);
					timing->set_stepLengthInYears(1);
					timing->set_step(curStep);
					timing->set_fractionOfStep(1);

					auto useStartDate = curStepDate;

					notificationCenter.postNotification(std::make_shared<flint::TimingStepNotification>(curStep, 1, useStartDate, endStepDate), std::make_shared<PostNotificationNotification>("TimingStepNotification"));
					notificationCenter.postNotification(std::make_shared<TimingPreEndStepNotification>(endStepDate));
					notificationCenter.postNotification(std::make_shared<flint::TimingEndStepNotification>(endStepDate));
					notificationCenter.postNotification(std::make_shared<flint::TimingPostStepNotification>(endStepDate));

					curStepDate.addYears(1);
					endStepDate = curStepDate;
					endStepDate.addYears(1);
				}
			}
		}
	}
} // namespace moja::modules::cbm
