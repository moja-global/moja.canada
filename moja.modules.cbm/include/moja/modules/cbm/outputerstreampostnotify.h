#ifndef OutputerStreamPostNotify_H_
#define OutputerStreamPostNotify_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/outputerstream.h"
#include "moja/observer.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

namespace moja {
	namespace modules {
		namespace CBM {

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
} // namespace moja::Modules::CBM

#endif // OutputerStreamPostNotify_H_