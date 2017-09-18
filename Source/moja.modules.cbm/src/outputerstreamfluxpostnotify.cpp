#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#define DL_CHR "\t"

using namespace std;
namespace cbm = moja::modules::cbm;

namespace moja {
namespace modules {
namespace cbm {

    void OutputerStreamFluxPostNotify::configure(const DynamicObject& config) {
        OutputerStreamFlux::configure(config);
    }

    void OutputerStreamFluxPostNotify::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::PostNotification, &OutputerStreamFluxPostNotify::onPostNotification, *this);
	}

    void OutputerStreamFluxPostNotify::onPostNotification(short) {
		outputEndStep(_output);
    }

}}} // namespace moja::Modules::cbm
