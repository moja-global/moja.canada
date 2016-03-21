#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"
#include "moja/flint/landunitcontroller.h"

#include <iomanip>

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
		notificationCenter.connect_signal(signals::PostNotification, &OutputerStreamFluxPostNotify::onPostNotification, *this);
	}

    void OutputerStreamFluxPostNotify::onPostNotification(const std::string) {
        if (_fileOpen) {
            outputEndStep(*_fp);
        }

        if (_outputToScreen) {
            outputEndStep(cout);
        }
    }

}}} // namespace moja::Modules::cbm
