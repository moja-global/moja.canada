#ifndef MOJA_MODULES_CBM_CBMSEQUENCER_H_
#define MOJA_MODULES_CBM_CBMSEQUENCER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/datetime.h"
#include "moja/flint/itiming.h"
#include "moja/flint/sequencermodulebase.h"
#include "moja/flint/ilandunitcontroller.h"
#include "moja/notificationcenter.h"

#include <string>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API CBMSequencer : public flint::SequencerModuleBase {
    public:
        CBMSequencer() {};
        virtual ~CBMSequencer() {};

        void configure(flint::ITiming& timing) override {
            _startDate = timing.startDate();
            _endDate = timing.endDate();
            timing.setStepLengthInYears(1);
        };

        bool Run(NotificationCenter& _notificationCenter,
                 flint::ILandUnitController& luc) override;

    private:
        DateTime _startDate;
        DateTime _endDate;
    };

}}} // namespace moja::Modules::CBM
#endif // MOJA_MODULES_CBM_CBMSEQUENCER_H_
