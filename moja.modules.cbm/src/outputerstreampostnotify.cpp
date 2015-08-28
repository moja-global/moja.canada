//
// OutputerStreamPostNotify.cpp
//

#include "moja/modules/cbm/outputerstreampostnotify.h"
#include "moja/flint/landunitcontroller.h"

#include <iomanip>

#define DL_CHR "\t"
#define STOCK_PRECISION 16

using namespace std;
namespace CBM = moja::modules::CBM;

namespace moja {
namespace modules {
namespace CBM {

	void OutputerStreamPostNotify::configure(const DynamicObject& config) {
		OutputerStream::configure(config);
	};

	void OutputerStreamPostNotify::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.addObserver(
            make_shared<Observer<IModule, flint::PostNotificationNotification>>(
                *this, &IModule::onPostNotification));
	};

	void OutputerStreamPostNotify::onPostNotification(const flint::PostNotificationNotification::Ptr&) {
        if (_fileOpen) {
            outputEndStep(*_fp);
        }

        if (_doCOUT) {
            outputEndStep(cout);
        }
	}

}}} // namespace moja::Modules::CBM
