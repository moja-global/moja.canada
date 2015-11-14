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

        std::shared_ptr<OvermatureDeclineLosses> getOvermatureDeclineLosses(
            double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges,
            double coarseRootCarbonChanges, double fineRootCarbonChanges);

    private:
        flint::IPool::ConstPtr _softwoodStemSnag;
        flint::IPool::ConstPtr _softwoodBranchSnag;
        flint::IPool::ConstPtr _softwoodMerch;
        flint::IPool::ConstPtr _softwoodFoliage;
        flint::IPool::ConstPtr _softwoodOther;
        flint::IPool::ConstPtr _softwoodCoarseRoots;
        flint::IPool::ConstPtr _softwoodFineRoots;

        flint::IPool::ConstPtr _hardwoodStemSnag;
        flint::IPool::ConstPtr _hardwoodBranchSnag;
        flint::IPool::ConstPtr _hardwoodMerch;
        flint::IPool::ConstPtr _hardwoodFoliage;
        flint::IPool::ConstPtr _hardwoodOther;
        flint::IPool::ConstPtr _hardwoodCoarseRoots;
        flint::IPool::ConstPtr _hardwoodFineRoots;

        flint::IPool::ConstPtr _aboveGroundVeryFastSoil;
        flint::IPool::ConstPtr _aboveGroundFastSoil;

        flint::IPool::ConstPtr _belowGroundVeryFastSoil;
        flint::IPool::ConstPtr _belowGroundFastSoil;

        flint::IPool::ConstPtr _mediumSoil;

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
