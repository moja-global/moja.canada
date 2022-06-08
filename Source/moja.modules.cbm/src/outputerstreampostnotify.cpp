//
// OutputerStreamPostNotify.cpp
//

#include "moja/modules/cbm/outputerstreampostnotify.h"

#include <moja/flint/landunitcontroller.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#define DL_CHR "\t"
#define STOCK_PRECISION 16

using namespace std;
namespace cbm = moja::modules::cbm;

namespace moja {
namespace modules {
namespace cbm {

	/**
	 * @brief Configuration function
	 * 
	 * Invoke OutputerStream::configure with parameter config
	 * 
	 * @return void 
	 * ***************/
	void OutputerStreamPostNotify::configure(const DynamicObject& config) {
		OutputerStream::configure(config);
	}

	/**
	 * @brief Subscribe to the signal PostNotification
	 * 
	 * @return void
	 * *****************/
	void OutputerStreamPostNotify::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::PostNotification, &OutputerStreamPostNotify::onPostNotification, *this);
	}

	/**
	 * @brief Invoke on posting the notification
	 * 
	 * @param short 
	 * @return void
	 * *****************/
	void OutputerStreamPostNotify::onPostNotification(short) {
           outputEndStep("onPostNotification",_output);
	}

}}} // namespace moja::Modules::CBM

