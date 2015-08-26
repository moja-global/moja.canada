#ifndef MOJA_MODULES_CBM_OUTPUTERSTREAMPOSTNOTIFY_H_
#define MOJA_MODULES_CBM_OUTPUTERSTREAMPOSTNOTIFY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/outputerstream.h"
#include "moja/observer.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

namespace moja {
	namespace modules {
		namespace cbm {

			class CBM_API OutputerStreamPostNotify : public flint::OutputerStream {
			public:
				OutputerStreamPostNotify() : flint::OutputerStream() { }
				virtual ~OutputerStreamPostNotify() { }

				void configure(const DynamicObject& config) override;
				void subscribe(NotificationCenter& notificationCenter) override;

				void onPostNotification(const flint::PostNotificationNotification::Ptr&) override;
			};

		}
	}
} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_OUTPUTERSTREAMPOSTNOTIFY_H_