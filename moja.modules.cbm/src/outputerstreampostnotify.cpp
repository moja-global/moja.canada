//
// OutputerStreamPostNotify.cpp
//

#include "moja/modules/cbm/outputerstreampostnotify.h"
#include "moja/flint/landunitcontroller.h"

#include <iomanip>

#define DL_CHR "\t"
#define STOCK_PRECISION 16

using namespace std;
namespace cbm = moja::modules::cbm;

namespace moja {
namespace modules {
namespace cbm {

	void OutputerStreamPostNotify::configure(const DynamicObject& config) {
		OutputerStream::configure(config);
	};

	void OutputerStreamPostNotify::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::PostNotification, &OutputerStreamPostNotify::onPostNotification, *this);
	};

	void OutputerStreamPostNotify::onPostNotification(short) {
           outputEndStep(_output);
	}

}}} // namespace moja::Modules::CBM

