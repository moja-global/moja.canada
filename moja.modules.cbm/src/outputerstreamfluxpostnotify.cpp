#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"
#include "moja/flint/landunitcontroller.h"

#include <iomanip>

#define DL_CHR "\t"

using namespace std;
namespace CBM = moja::modules::CBM;

namespace moja {
namespace modules {
namespace CBM {

    void OutputerStreamFluxPostNotify::configure(const DynamicObject& config) {
        OutputerStreamFlux::configure(config);
    }

    void OutputerStreamFluxPostNotify::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.addObserver(
            make_shared<Observer<IModule, flint::PostNotificationNotification>>(
                *this, &IModule::onPostNotification));
    }

    void OutputerStreamFluxPostNotify::onPostNotification(const flint::PostNotificationNotification::Ptr&) {
        if (_fileOpen) {
            outputEndStep(*_fp);
        }

        if (_doCOUT) {
            outputEndStep(cout);
        }
    }

}}} // namespace moja::Modules::CBM
