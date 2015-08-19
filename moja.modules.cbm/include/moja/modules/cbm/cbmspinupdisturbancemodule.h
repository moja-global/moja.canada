#ifndef CBM_SpinupDisturbanceModule_H_
#define CBM_SpinupDisturbanceModule_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "cbmdisturbanceeventmodule.h"

namespace moja {
namespace modules {
namespace CBM {

	/*
	Response to the historical and last disturbance events in CBM spinup
	*/
	class CBM_API CBMSpinupDisturbanceModule : public moja::flint::ModuleBase {
	public:
		CBMSpinupDisturbanceModule(){};
		virtual ~CBMSpinupDisturbanceModule(){};		

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void onDisturbanceEvent(const flint::DisturbanceEventNotification::Ptr& n) override;
		void onTimingInit(const flint::TimingInitNotification::Ptr&) override;
	private:	
		typedef std::vector<CBMDistEventTransfer::Ptr> matrix_vector;	
		typedef std::unordered_map<int, matrix_vector> event_map;

		event_map _events;		
	};
}}}
#endif