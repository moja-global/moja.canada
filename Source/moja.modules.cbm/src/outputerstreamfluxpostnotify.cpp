#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#define DL_CHR "\t"

using namespace std;
namespace cbm = moja::modules::cbm;

namespace moja {
namespace modules {
namespace cbm {

   /**
	 * @brief Configuration function
	 * 
	 * Invoke OutputerStreamFlux::configure with parameter config
	 * 
	 * @return void 
	 * ***************/
    void OutputerStreamFluxPostNotify::configure(const DynamicObject& config) {
        OutputerStreamFlux::configure(config);
    }

   /**
	 * @brief Subscribe to the signal PostNotification
	 * 
	 * @return void
	 * *****************/
    void OutputerStreamFluxPostNotify::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.subscribe(signals::PostNotification, &OutputerStreamFluxPostNotify::onPostNotification, *this);
	}

   /**
	 * @brief Invoke on posting the notification
	 * 
	 * @param short 
	 * @return void
	 * *****************/
    void OutputerStreamFluxPostNotify::onPostNotification(short) {
		outputEndStep(_output);
    }

}}} // namespace moja::Modules::cbm
