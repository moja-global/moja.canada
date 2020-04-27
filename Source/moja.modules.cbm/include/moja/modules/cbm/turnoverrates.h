#ifndef MOJA_MODULES_CBM_TURNOVERRATES_H_
#define MOJA_MODULES_CBM_TURNOVERRATES_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/flint/flintexceptions.h"
#include "moja/exception.h"

namespace moja {
namespace modules {
namespace cbm {
	
	class CBM_API TurnoverRates {
	public:
        TurnoverRates(DynamicObject data) {
            _swFoliageTurnover = data["sw_foliage_turnover"];
            _swStemTurnover = data["sw_stem_turnover"];
            _swBranchTurnover = data["sw_branch_turnover"];
            _swStemSnagTurnover = data["sw_stem_snag_turnover"];
            _swBranchSnagTurnover = data["sw_branch_snag_turnover"];
            _swCoarseRootTurnover = data["sw_coarse_root_turnover"];
            _swFineRootTurnover = data["sw_fine_root_turnover"];
            _swBranchSnagSplit = data["sw_other_to_branch_snag_split"];
            _swCoarseRootSplit = data["sw_coarse_root_split"];
            _swFineRootSplit = data["sw_fine_root_ag_split"];
            _hwFoliageTurnover = data["hw_foliage_turnover"];
            _hwStemTurnover = data["hw_stem_turnover"];
            _hwBranchTurnover = data["hw_branch_turnover"];
            _hwStemSnagTurnover = data["hw_stem_snag_turnover"];
            _hwBranchSnagTurnover = data["hw_branch_snag_turnover"];
            _hwCoarseRootTurnover = data["hw_coarse_root_turnover"];
            _hwFineRootTurnover = data["hw_fine_root_turnover"];
            _hwBranchSnagSplit = data["hw_other_to_branch_snag_split"];
            _hwCoarseRootSplit = data["hw_coarse_root_split"];
            _hwFineRootSplit = data["hw_fine_root_ag_split"];
        }

        double swFoliageTurnover() const { return _swFoliageTurnover; }
        double swStemTurnover() const { return _swStemTurnover; }
        double swBranchTurnover() const { return _swBranchTurnover; }
        double swStemSnagTurnover() const { return _swStemSnagTurnover; }
        double swBranchSnagTurnover() const { return _swBranchSnagTurnover; }
        double swCoarseRootTurnover() const { return _swCoarseRootTurnover; }
        double swFineRootTurnover() const { return _swFineRootTurnover; }
        double swBranchSnagSplit() const { return _swBranchSnagSplit; }
        double swCoarseRootSplit() const { return _swCoarseRootSplit; }
        double swFineRootSplit() const { return _swFineRootSplit; }
        double hwFoliageTurnover() const { return _hwFoliageTurnover; }
        double hwStemTurnover() const { return _hwStemTurnover; }
        double hwBranchTurnover() const { return _hwBranchTurnover; }
        double hwStemSnagTurnover() const { return _hwStemSnagTurnover; }
        double hwBranchSnagTurnover() const { return _hwBranchSnagTurnover; }
        double hwCoarseRootTurnover() const { return _hwCoarseRootTurnover; }
        double hwFineRootTurnover() const { return _hwFineRootTurnover; }
        double hwBranchSnagSplit() const { return _hwBranchSnagSplit; }
        double hwCoarseRootSplit() const { return _hwCoarseRootSplit; }
        double hwFineRootSplit() const { return _hwFineRootSplit; }

		private:
            double _swFoliageTurnover = 0;
            double _swStemTurnover = 0;
            double _swBranchTurnover = 0;
            double _swStemSnagTurnover = 0;
            double _swBranchSnagTurnover = 0;
            double _swCoarseRootTurnover = 0;
            double _swFineRootTurnover = 0;
            double _swBranchSnagSplit = 0;
            double _swCoarseRootSplit = 0;
            double _swFineRootSplit = 0;
            double _hwFoliageTurnover = 0;
            double _hwStemTurnover = 0;
            double _hwBranchTurnover = 0;
            double _hwStemSnagTurnover = 0;
            double _hwBranchSnagTurnover = 0;
            double _hwCoarseRootTurnover = 0;
            double _hwFineRootTurnover = 0;
            double _hwBranchSnagSplit = 0;
            double _hwCoarseRootSplit = 0;
            double _hwFineRootSplit = 0;
    };
	
}}}
#endif