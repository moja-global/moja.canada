#ifndef OutputerStreamFluxPostNotify_H_
#define OutputerStreamFluxPostNotify_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/notification.h"
#include "moja/hash.h"
#include "moja/flint/outputerstreamflux.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

namespace moja {
namespace modules {
namespace CBM {

	class CBM_API OutputerStreamFluxPostNotify : public flint::OutputerStreamFlux {
	public:
		OutputerStreamFluxPostNotify() : flint::OutputerStreamFlux() { }
		virtual ~OutputerStreamFluxPostNotify() { }

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void onPostNotification(const flint::PostNotificationNotification::Ptr&) override;
	};

}}} // namespace moja::Modules::CBM

#endif // OutputerStreamFluxPostNotify_H_