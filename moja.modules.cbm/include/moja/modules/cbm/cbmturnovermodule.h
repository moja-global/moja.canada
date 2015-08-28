#ifndef MOJA_MODULES_CBM_CBMTURNOVERMODULE_H_
#define MOJA_MODULES_CBM_CBMTURNOVERMODULE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "overmaturedeclinelosses.h"

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API CBMTurnoverModule : public moja::flint::ModuleBase {
    public:
        CBMTurnoverModule() : ModuleBase() {}
        virtual ~CBMTurnoverModule() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        void onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& n) override;
        void onTimingInit(const flint::TimingInitNotification::Ptr& n) override;
        void onTimingStep(const flint::TimingStepNotification::Ptr& n) override;
        void onTimingEndStep(const flint::TimingEndStepNotification::Ptr& n) override;

        std::shared_ptr<OvermatureDeclineLosses> getOvermatrueDeclineLosses(
            double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges,
            double coarseRootCarbonChanges, double fineRootCarbonChanges);

    private:
        const flint::IPool* _softwoodStemSnag;
        const flint::IPool* _softwoodBranchSnag;
        const flint::IPool* _softwoodMerch;
        const flint::IPool* _softwoodFoliage;
        const flint::IPool* _softwoodOther;
        const flint::IPool* _softwoodCoarseRoots;
        const flint::IPool* _softwoodFineRoots;

        const flint::IPool* _hardwoodStemSnag;
        const flint::IPool* _hardwoodBranchSnag;
        const flint::IPool* _hardwoodMerch;
        const flint::IPool* _hardwoodFoliage;
        const flint::IPool* _hardwoodOther;
        const flint::IPool* _hardwoodCoarseRoots;
        const flint::IPool* _hardwoodFineRoots;

        const flint::IPool* _aboveGroundVeryFastSoil;
        const flint::IPool* _aboveGroundFastSoil;

        const flint::IPool* _belowGroundVeryFastSoil;
        const flint::IPool* _belowGroundFastSoil;

        const flint::IPool* _mediumSoil;

        double _softwoodFoliageFallRate;
        double _hardwoodFoliageFallRate;
        double _stemAnnualTurnOverRate;
        double _softwoodBranchTurnOverRate;
        double _hardwoodBranchTurnOverRate;

        double _otherToBranchSnagSplit;
        double _stemSnagTurnoverRate;
        double _branchSnagTurnoverRate;

        double _coarseRootSplit;
        double _coarseRootTurnProp;
        double _fineRootAGSplit;
        double _fineRootTurnProp;

        //record previous biomass pool
        double preStandSoftwoodMerch;
        double preStandSoftwoodOther;
        double preStandSoftwoodFoliage;
        double preStandSoftwoodCoarseRoots;
        double preStandSoftwoodFineRoots;
        double preStandHardwoodMerch;
        double preStandHardwoodOther;
        double preStandHardwoodFoliage;
        double preStandHardwoodCoarseRoots;
        double preStandHardwoodFineRoots;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMTURNOVERMODULE_H_
