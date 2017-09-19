
#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include "moja/modules/cbm/rootbiomassequation.h"

#include "sawtooth/exports.h"

namespace moja {
namespace modules {
namespace cbm {

	class CBM_API SawtoothModule : public CBMModuleBase {
	public:
		SawtoothModule() : CBMModuleBase() { }
		virtual ~SawtoothModule() = default;

		void configure(const DynamicObject& config) override;
		void subscribe(NotificationCenter& notificationCenter) override;

		void doLocalDomainInit() override;
		void doTimingInit() override;
		void doTimingStep() override;
		void onTimingShutdown() override;

	private:
		void* Sawtooth_Handle;
		void* Sawtooth_Stand_Handle;
		size_t Sawtooth_Max_Density;
		Sawtooth_Error sawtooth_error;
		int** speciesList;

		moja::flint::IVariable* tmin;
		moja::flint::IVariable* tmean;
		moja::flint::IVariable* vpd;
		moja::flint::IVariable* etr;
		moja::flint::IVariable* eeq;
		moja::flint::IVariable* ws;
		moja::flint::IVariable* ca;
		moja::flint::IVariable* ndep;
		moja::flint::IVariable* ws_mjjas_z;
		moja::flint::IVariable* ws_mjjas_n;
		moja::flint::IVariable* etr_mjjas_z;
		moja::flint::IVariable* etr_mjjas_n;
		moja::flint::IVariable* disturbance;

		const flint::IPool* _softwoodMerch;
		const flint::IPool* _softwoodOther;
		const flint::IPool* _softwoodFoliage;
		const flint::IPool* _softwoodCoarseRoots;
		const flint::IPool* _softwoodFineRoots;

		const flint::IPool* _hardwoodMerch;
		const flint::IPool* _hardwoodOther;
		const flint::IPool* _hardwoodFoliage;
		const flint::IPool* _hardwoodCoarseRoots;
		const flint::IPool* _hardwoodFineRoots;

		const flint::IPool* _aboveGroundVeryFastSoil;
		const flint::IPool* _aboveGroundFastSoil;
		const flint::IPool* _belowGroundVeryFastSoil;
		const flint::IPool* _belowGroundFastSoil;
		const flint::IPool* _softwoodStemSnag;
		const flint::IPool* _softwoodBranchSnag;
		const flint::IPool* _hardwoodStemSnag;
		const flint::IPool* _hardwoodBranchSnag;
		const flint::IPool* _mediumSoil;
		const flint::IPool* _atmosphere;

		flint::IVariable* _turnoverRates;
		flint::IVariable* _regenDelay;
		flint::IVariable* _currentLandClass;

		flint::IVariable* _isForest;

		flint::IVariable* _cbm_species_id;
		flint::IVariable* _standSPUID;

		std::shared_ptr<SoftwoodRootBiomassEquation> SWRootBio;
		std::shared_ptr<HardwoodRootBiomassEquation> HWRootBio;

		Sawtooth_ModelMeta InitializeModelMeta(const DynamicObject& config);

	};
}}}