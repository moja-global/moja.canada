#ifndef MOJA_MODULES_CBM_OUTPUTERSTREAMFLUXPOSTNOTIFY_H_
#define MOJA_MODULES_CBM_OUTPUTERSTREAMFLUXPOSTNOTIFY_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/hash.h"
#include "moja/flint/outputerstreamflux.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API OutputerStreamFluxPostNotify : public flint::OutputerStreamFlux {
	public:
		OutputerStreamFluxPostNotify() : flint::OutputerStreamFlux() { }
		virtual ~OutputerStreamFluxPostNotify() { }

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void onPostNotification(short preMessageSignal) override;
	};

}}} // namespace moja::Modules::CBM

#endif // MOJA_MODULES_CBM_OUTPUTERSTREAMFLUXPOSTNOTIFY_H_