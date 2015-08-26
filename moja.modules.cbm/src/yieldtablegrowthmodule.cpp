#include "moja/flint/variable.h"
#include "moja/observer.h"

#include "moja/modules/cbm/yieldtablegrowthmodule.h"

namespace moja {
namespace modules {
namespace cbm {	

	void YieldTableGrowthModule::configure(const DynamicObject& config) { }

	void YieldTableGrowthModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(*this, &IModule::onLocalDomainInit));
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(*this, &IModule::onTimingStep));
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));
	}

	void YieldTableGrowthModule::onTimingInit(const flint::TimingInitNotification::Ptr&) {
		//get the stand growth curve ID associated to the pixel/svo
		const auto& standGrowthCurveID = _landUnitData->getVariable("StandGrowthCurveID")->value()
			.extract<const std::vector<DynamicObject>>();

		if (standGrowthCurveID.size() == 0) {
			_standGrowthCurveID = -1;
		} else {
			const auto& gcId = standGrowthCurveID[0]["StandGrowthCurveID"];
			_standGrowthCurveID = gcId.isEmpty() ? -1 : Int64(gcId);
		}

		const auto& landAge = _landUnitData->getVariable("InitialAge")->value()
			.extract<const std::vector<DynamicObject>>();

		int standAge = landAge[0]["age"];
		_age->set_value(standAge);

		//try to get the stand growth curve and related yield table data from memory
		if (_standGrowthCurveID > 0) {
			bool carbonCurveFound = _volumeToBioGrowth->isBiomassCarbonCurveAvailable(_standGrowthCurveID);
			if (!carbonCurveFound) {
				std::shared_ptr<StandGrowthCurve> standGrowthCurve = createStandGrowthCurve(_standGrowthCurveID);

				//pre-process the standGrowthCurve here		
				standGrowthCurve->processStandYieldTables();

				//Process and convert yield volume to carbon curves
				_volumeToBioGrowth->generateBiomassCarbonCurve(standGrowthCurve);
			}
		}
	}

	void YieldTableGrowthModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& init) {
		_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
		_softwoodOther = _landUnitData->getPool("SoftwoodOther");
		_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
		_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
		_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

		_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
		_hardwoodOther = _landUnitData->getPool("HardwoodOther");
		_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
		_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
		_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

		_atmosphere = _landUnitData->getPool("atmosphere");		

		_age = _landUnitData->getVariable("Age");

		//TODO, validate this
		_volumeToBioGrowth = std::make_shared<VolumeToBiomassCarbonGrowth>();

		// ??? TODO, option to start the initialization to preprocess all growth curves here
	}

	void YieldTableGrowthModule::onTimingStep(const flint::TimingStepNotification::Ptr& step) {
		if (_standGrowthCurveID < 0) {
			return;
		}				

		//get stand age for current pixel/svo/stand
		int standAge = _age->value();

		double standSoftwoodMerch = _softwoodMerch->value();
		double standSoftwoodOther = _softwoodOther->value();
		double standSoftwoodFoliage = _softwoodFoliage->value();
		double standSWCoarseRootsCarbon = _softwoodCoarseRoots->value();
		double standSWFineRootsCarbon = _softwoodFineRoots->value();
		double standHardwoodMerch = _hardwoodMerch->value();
		double standHardwoodOther = _hardwoodOther->value();
		double standHardwoodFoliage = _hardwoodFoliage->value();
		double standHWCoarseRootsCarbon = _hardwoodCoarseRoots->value();
		double standHWFineRootsCarbon = _hardwoodFineRoots->value();

		//get the above ground biomass carbon growth increment
		std::shared_ptr<AboveGroundBiomassCarbonIncrement> abIncrement = _volumeToBioGrowth->getAGBiomassCarbonIncrements(_standGrowthCurveID, standAge);
	
		double swm = abIncrement->softwoodMerch();
		double swo = abIncrement->softwoodOther();
		double swf = abIncrement->softwoodFoliage();	
		double hwm = abIncrement->hardwoodMerch();
		double hwo = abIncrement->hardwoodOther();
		double hwf = abIncrement->hardwoodFoliage();
	
		//the MAX function calls below to enforce the biomass carbon changes keeps a POSITIVE value for the pool value
		swm = std::max(swm, -standSoftwoodMerch);
		swo = std::max(swo, -standSoftwoodOther);
		swf = std::max(swf, -standSoftwoodFoliage);
		hwm = std::max(hwm, -standHardwoodMerch);
		hwo = std::max(hwo, -standHardwoodOther);
		hwf = std::max(hwf, -standHardwoodFoliage);

		//compute the total biomass carbon for softwood and hardwood component
		double totalSWAgBioCarbon = standSoftwoodMerch + swm + standSoftwoodFoliage + swf + standSoftwoodOther + swo;
		double totalHWAgBioCarbon = standHardwoodMerch + hwm + standHardwoodFoliage + hwf + standHardwoodOther + hwo;

		//get the root biomass carbon increment based on the total above ground biomass
		std::shared_ptr<RootBiomassCarbonIncrement> bgIncrement = _volumeToBioGrowth->getBGBiomassCarbonIncrements(totalSWAgBioCarbon, standSWCoarseRootsCarbon, standSWFineRootsCarbon, totalHWAgBioCarbon, standHWCoarseRootsCarbon, standHWFineRootsCarbon);

		double swcr = bgIncrement->softwoodFineRoots();
		double swfr = bgIncrement->softwoodFineRoots();
		double hwcr = bgIncrement->hardwoodFineRoots();
		double hwfr = bgIncrement->hardwoodFineRoots();

		auto growth = _landUnitData->createStockOperation();	

		growth
			->addTransfer(_atmosphere, _softwoodMerch, swm)
			->addTransfer(_atmosphere, _softwoodOther, swo)
			->addTransfer(_atmosphere, _softwoodFoliage, swf)
			->addTransfer(_atmosphere, _softwoodCoarseRoots, swcr)
			->addTransfer(_atmosphere, _softwoodFineRoots, swfr)

			->addTransfer(_atmosphere, _hardwoodMerch, hwm)
			->addTransfer(_atmosphere, _hardwoodOther, hwo)
			->addTransfer(_atmosphere, _hardwoodFoliage, hwf)
			->addTransfer(_atmosphere, _hardwoodCoarseRoots, hwcr)
			->addTransfer(_atmosphere, _hardwoodFineRoots, hwfr);

		_landUnitData->submitOperation(growth);			
		
		//overmature is handled in the following turnover module
		/*
		std::shared_ptr<OvermatureDeclineLosses> swlosses = _volumeToBioGrowth->getOvermatrueDeclineLosses( swm, swf, swo, swcr, swfr);
		std::shared_ptr<OvermatureDeclineLosses> hwlosses = _volumeToBioGrowth->getOvermatrueDeclineLosses( hwm, hwf, hwo, hwcr, hwfr);

		auto loss = _landUnitData->createStockOperation();
				
		if (swlosses->lossesPresent()){
			loss
				->addTransfer(_softwoodMerch, _softwoodStemSnag, swlosses->merchToStemSnags())
				->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, swlosses->foliageToAGVeryFast())
				->addTransfer(_softwoodOther, _softwoodBranchSnag, swlosses->otherToBranchSnag())
				->addTransfer(_softwoodOther, _aboveGroundFastSoil, swlosses->otherToAGFast())
				->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, swlosses->coarseRootToAGFast())
				->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, swlosses->coarseRootToBGFast())
				->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, swlosses->fineRootToAGVeryFast())
				->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, swlosses->fineRootToBGVeryFast());
		}
		if (hwlosses->lossesPresent()){
			loss->addTransfer(_hardwoodMerch, _hardwoodStemSnag, hwlosses->merchToStemSnags())
				->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, hwlosses->foliageToAGVeryFast())
				->addTransfer(_hardwoodOther, _hardwoodBranchSnag, hwlosses->otherToBranchSnag())
				->addTransfer(_hardwoodOther, _aboveGroundFastSoil, hwlosses->otherToAGFast())
				->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, hwlosses->coarseRootToAGFast())
				->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, hwlosses->coarseRootToBGFast())
				->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, hwlosses->fineRootToAGVeryFast())
				->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, hwlosses->fineRootToBGVeryFast());
		}
		_landUnitData->submitOperation(loss);
		*/
		_age->set_value(standAge + 1);
	}

	std::shared_ptr<StandGrowthCurve> YieldTableGrowthModule::createStandGrowthCurve(Int64 standGrowthCurveID){
		auto standGrowthCurve = std::make_shared<StandGrowthCurve>(standGrowthCurveID);

		// Get the table of softwood merchantable volumes associated to the stand growth curve.
		const auto& softwoodYieldTable = _landUnitData->getVariable("SoftwoodYieldTable")->value()
			.extract<const std::vector<DynamicObject>>();

		auto swTreeYieldTable = std::make_shared<TreeYieldTable>(softwoodYieldTable, SpeciesType::Softwood);
		standGrowthCurve->addYieldTable(swTreeYieldTable);

		// Get the table of hardwood merchantable volumes associated to the stand growth curve.
		const auto& hardwoodYieldTable = _landUnitData->getVariable("HardwoodYieldTable")->value()
			.extract<const std::vector<DynamicObject>>();

		auto hwTreeYieldTable = std::make_shared<TreeYieldTable>(hardwoodYieldTable, SpeciesType::Hardwood);
		standGrowthCurve->addYieldTable(hwTreeYieldTable);
		
		// Query for the appropriate PERD factor data.
		const auto& vol2bio = _landUnitData->getVariable("VolumeToBiomassParameters")->value()
			.extract<const std::vector<DynamicObject>>();

		for (const auto& row : vol2bio) {
			auto perdFactor = std::make_unique<PERDFactor>();
			perdFactor->setValue(row);

			std::string forestType = row["ForestType"].convert<std::string>();
			if (forestType == "Softwood") {
				standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Softwood);
			}
			else if (forestType == "Hardwood") {
				standGrowthCurve->setPERDFactor(std::move(perdFactor), SpeciesType::Hardwood);
			}
		}

		return standGrowthCurve;
	}
	 
}}}