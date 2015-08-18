#include "moja/modules/cbm/cbmturnovermodule.h"
#include "moja/observer.h"

namespace moja {
namespace modules {
namespace CBM {

	void CBMTurnoverModule::configure(const DynamicObject& config) { }

	void CBMTurnoverModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::LocalDomainInitNotification>>(*this, &IModule::onLocalDomainInit));
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingStepNotification>>(*this, &IModule::onTimingStep));
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingInitNotification>>(*this, &IModule::onTimingInit));

		//added to update prvious biomass pool values at the end of time step
		notificationCenter.addObserver(std::make_shared<Observer<IModule, flint::TimingEndStepNotification>>(*this, &IModule::onTimingEndStep));
	}

	void CBMTurnoverModule::onLocalDomainInit(const flint::LocalDomainInitNotification::Ptr& init) {
		_softwoodStemSnag			= _landUnitData->getPool("SoftwoodStemSnag");
		_softwoodBranchSnag			= _landUnitData->getPool("SoftwoodBranchSnag");
		_softwoodMerch				= _landUnitData->getPool("SoftwoodMerch");
		_softwoodFoliage			= _landUnitData->getPool("SoftwoodFoliage");
		_softwoodOther				= _landUnitData->getPool("SoftwoodOther");
		_softwoodCoarseRoots		= _landUnitData->getPool("SoftwoodCoarseRoots");
		_softwoodFineRoots			= _landUnitData->getPool("SoftwoodFineRoots");

		_hardwoodStemSnag			= _landUnitData->getPool("HardwoodStemSnag");
		_hardwoodBranchSnag			= _landUnitData->getPool("HardwoodBranchSnag");
		_hardwoodMerch				= _landUnitData->getPool("HardwoodMerch");
		_hardwoodFoliage			= _landUnitData->getPool("HardwoodFoliage");
		_hardwoodOther				= _landUnitData->getPool("HardwoodOther");
		_hardwoodCoarseRoots		= _landUnitData->getPool("HardwoodCoarseRoots");
		_hardwoodFineRoots			= _landUnitData->getPool("HardwoodFineRoots");

		_aboveGroundVeryFastSoil	= _landUnitData->getPool("AboveGroundVeryFastSoil");
		_aboveGroundFastSoil		= _landUnitData->getPool("AboveGroundFastSoil");

		_belowGroundVeryFastSoil	= _landUnitData->getPool("BelowGroundVeryFastSoil");
		_belowGroundFastSoil		= _landUnitData->getPool("BelowGroundFastSoil");

		_mediumSoil					= _landUnitData->getPool("MediumSoil");
	}

	void CBMTurnoverModule::onTimingInit(const flint::TimingInitNotification::Ptr& n)
	{
		//Initiallly record current biomass pool value as previous
		preStandSoftwoodMerch = _softwoodMerch->value();
		preStandSoftwoodOther = _softwoodOther->value();
		preStandSoftwoodFoliage = _softwoodFoliage->value();
		preStandSoftwoodCoarseRoots = _softwoodCoarseRoots->value();
		preStandSoftwoodFineRoots = _softwoodFineRoots->value();
		preStandHardwoodMerch = _hardwoodMerch->value();
		preStandHardwoodOther = _hardwoodOther->value();
		preStandHardwoodFoliage = _hardwoodFoliage->value();
		preStandHardwoodCoarseRoots = _hardwoodCoarseRoots->value();
		preStandHardwoodFineRoots = _hardwoodFineRoots->value();

		const auto& turnoverRates = _landUnitData->getVariable("TurnoverRates")->value()
			.extract<const std::vector<DynamicObject>>();

		_softwoodFoliageFallRate	= turnoverRates[0]["SoftwoodFoliageFallRate"];
		_hardwoodFoliageFallRate	= turnoverRates[0]["HardwoodFoliageFallRate"];
		_stemAnnualTurnOverRate		= turnoverRates[0]["StemAnnualTurnOverRate"];
		_softwoodBranchTurnOverRate = turnoverRates[0]["SoftwoodBranchTurnOverRate"];
		_hardwoodBranchTurnOverRate = turnoverRates[0]["HardwoodBranchTurnOverRate"];

		_otherToBranchSnagSplit		= turnoverRates[0]["OtherToBranchSnagSplit"];
		_stemSnagTurnoverRate		= turnoverRates[0]["StemSnagTurnoverRate"];
		_branchSnagTurnoverRate		= turnoverRates[0]["BranchSnagTurnoverRate"];

		_coarseRootSplit			= turnoverRates[0]["CoarseRootSplit"];
		_coarseRootTurnProp			= turnoverRates[0]["CoarseRootTurnProp"];
		_fineRootAGSplit			= turnoverRates[0]["FineRootAGSplit"];
		_fineRootTurnProp			= turnoverRates[0]["FineRootTurnProp"];
	}

	void CBMTurnoverModule::onTimingStep(const flint::TimingStepNotification::Ptr& step) {
		//get current biomass pool values
		double curStandSoftwoodMerch = _softwoodMerch->value();		
		double curStandSoftwoodFoliage = _softwoodFoliage->value();
		double curStandSoftwoodOther = _softwoodOther->value();
		double curStandSoftwoodCoarseRoots = _softwoodCoarseRoots->value();
		double curStandSoftwoodFineRoots = _softwoodFineRoots->value();

		double curStandHardwoodMerch = _hardwoodMerch->value();		
		double curStandHardwoodFoliage = _hardwoodFoliage->value();
		double curStandHardwoodOther = _hardwoodOther->value();
		double curStandHardwoodCoarseRoots = _hardwoodCoarseRoots->value();
		double curStandHardwoodFineRoots = _hardwoodFineRoots->value();

		//update the biomas pool changes (current - previous)
		double changesOfStandSoftwoodMerch = curStandSoftwoodMerch - preStandSoftwoodMerch;		
		double changesOfStandSoftwoodFoliage = curStandSoftwoodFoliage - preStandSoftwoodFoliage;
		double changesOfStandSoftwoodOther = curStandSoftwoodOther - preStandSoftwoodOther;
		double changesOfStandSoftwoodCoarseRoots = curStandSoftwoodCoarseRoots - preStandSoftwoodCoarseRoots;
		double changesOfStandSoftwoodFineRoots = curStandSoftwoodFineRoots - preStandSoftwoodFineRoots;

		double changesOfStandHardwoodMerch = curStandHardwoodMerch - preStandHardwoodMerch;		
		double changesOfStandHardwoodFoliage = curStandHardwoodFoliage - preStandHardwoodFoliage;
		double changesOfStandHardwoodOther = curStandHardwoodOther - preStandHardwoodOther;
		double changesOfStandHardwoodCoarseRoots = curStandHardwoodCoarseRoots - preStandHardwoodCoarseRoots;
		double changesOfStandHardwoodFineRoots = curStandHardwoodFineRoots - preStandHardwoodFineRoots;

		//handle overmature turnover right after the growth module run
		auto overmatureLoss = _landUnitData->createStockOperation();
		bool softwoodLossHappened = (changesOfStandSoftwoodMerch + changesOfStandSoftwoodOther + changesOfStandSoftwoodFoliage + changesOfStandSoftwoodCoarseRoots + changesOfStandSoftwoodFineRoots) < 0;

		bool hardwoodLossHappened = (changesOfStandHardwoodMerch + changesOfStandHardwoodOther + changesOfStandHardwoodFoliage + changesOfStandHardwoodCoarseRoots + changesOfStandHardwoodFineRoots) < 0;

		if (softwoodLossHappened){
			//handle softwood overmature decline - turnover
			std::shared_ptr<OvermatureDeclineLosses> swlosses = getOvermatrueDeclineLosses(changesOfStandSoftwoodMerch, changesOfStandSoftwoodOther, changesOfStandSoftwoodFoliage, changesOfStandSoftwoodCoarseRoots, changesOfStandSoftwoodFineRoots);

			overmatureLoss
				->addTransfer(_softwoodMerch, _softwoodStemSnag, swlosses->merchToStemSnags())
				->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, swlosses->foliageToAGVeryFast())
				->addTransfer(_softwoodOther, _softwoodBranchSnag, swlosses->otherToBranchSnag())
				->addTransfer(_softwoodOther, _aboveGroundFastSoil, swlosses->otherToAGFast())
				->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, swlosses->coarseRootToAGFast())
				->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, swlosses->coarseRootToBGFast())
				->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, swlosses->fineRootToAGVeryFast())
				->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, swlosses->fineRootToBGVeryFast());
		}

		if (hardwoodLossHappened){
			//handle hardwood overmature decline - turnover
			std::shared_ptr<OvermatureDeclineLosses> hwlosses = getOvermatrueDeclineLosses(changesOfStandHardwoodMerch, changesOfStandHardwoodOther, changesOfStandHardwoodFoliage, changesOfStandHardwoodCoarseRoots, changesOfStandHardwoodFineRoots);

			overmatureLoss->addTransfer(_hardwoodMerch, _hardwoodStemSnag, hwlosses->merchToStemSnags())
				->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, hwlosses->foliageToAGVeryFast())
				->addTransfer(_hardwoodOther, _hardwoodBranchSnag, hwlosses->otherToBranchSnag())
				->addTransfer(_hardwoodOther, _aboveGroundFastSoil, hwlosses->otherToAGFast())
				->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, hwlosses->coarseRootToAGFast())
				->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, hwlosses->coarseRootToBGFast())
				->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, hwlosses->fineRootToAGVeryFast())
				->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, hwlosses->fineRootToBGVeryFast());
		}

		if (softwoodLossHappened || hardwoodLossHappened){
			//submit the overmature loss operation
			_landUnitData->submitOperation(overmatureLoss);
		}

		//do biomass turnover, half of the changes (either positive or negative growth) are applied
		auto biomassPoolTurnover = _landUnitData->createStockOperation();	
		biomassPoolTurnover
			->addTransfer(_softwoodMerch, _softwoodStemSnag, (preStandSoftwoodMerch + 0.5 * changesOfStandSoftwoodMerch) * _stemAnnualTurnOverRate)
			->addTransfer(_softwoodFoliage, _aboveGroundVeryFastSoil, (preStandSoftwoodFoliage + 0.5 * changesOfStandSoftwoodFoliage)* _softwoodFoliageFallRate)
			->addTransfer(_softwoodOther, _softwoodBranchSnag, (preStandSoftwoodOther + 0.5 * changesOfStandSoftwoodOther) * _otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodOther, _aboveGroundFastSoil, (preStandSoftwoodOther + 0.5 * changesOfStandSoftwoodOther) * (1- _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil, (preStandSoftwoodCoarseRoots + 0.5 * changesOfStandSoftwoodCoarseRoots) *_coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil, (preStandSoftwoodCoarseRoots + 0.5 * changesOfStandSoftwoodCoarseRoots) * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots, _aboveGroundVeryFastSoil, (preStandSoftwoodFineRoots + 0.5 * changesOfStandSoftwoodFineRoots) * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_softwoodFineRoots, _belowGroundVeryFastSoil, (preStandSoftwoodFineRoots + 0.5 * changesOfStandSoftwoodFineRoots) * (1 - _fineRootAGSplit) * _fineRootTurnProp)		

			->addTransfer(_hardwoodMerch, _hardwoodStemSnag, (preStandHardwoodMerch + 0.5*changesOfStandHardwoodMerch) * _stemAnnualTurnOverRate)
			->addTransfer(_hardwoodFoliage, _aboveGroundVeryFastSoil, (preStandHardwoodFoliage + 0.5 * changesOfStandHardwoodFoliage) * _hardwoodFoliageFallRate)
			->addTransfer(_hardwoodOther, _hardwoodBranchSnag, (preStandHardwoodOther + 0.5 * changesOfStandHardwoodOther) *  _otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
			->addTransfer(_hardwoodOther, _aboveGroundFastSoil, (preStandHardwoodOther + 0.5 * changesOfStandHardwoodOther) * (1-_otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
			->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil, (preStandHardwoodCoarseRoots + 0.5 * changesOfStandHardwoodCoarseRoots) *_coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil, (preStandHardwoodCoarseRoots + 0.5 * changesOfStandHardwoodCoarseRoots) * (1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _aboveGroundVeryFastSoil, (preStandHardwoodFineRoots + 0.5 * changesOfStandHardwoodFineRoots) * _fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_hardwoodFineRoots, _belowGroundVeryFastSoil, (preStandHardwoodFineRoots + 0.5 * changesOfStandHardwoodFineRoots) * (1 - _fineRootAGSplit) * _fineRootTurnProp);
		
		_landUnitData->submitOperation(biomassPoolTurnover);		

		//do DOM turnover
		auto domTurnover = _landUnitData->createProportionalOperation();
		domTurnover
			->addTransfer(_softwoodStemSnag,	_mediumSoil,				_stemSnagTurnoverRate)
			->addTransfer(_softwoodBranchSnag,	_aboveGroundFastSoil,		_branchSnagTurnoverRate)
			->addTransfer(_hardwoodStemSnag,	_mediumSoil,				_stemSnagTurnoverRate)
			->addTransfer(_hardwoodBranchSnag,	_aboveGroundFastSoil,		_branchSnagTurnoverRate);

		_landUnitData->submitOperation(domTurnover);

		/*
		auto biomassTurnover = _landUnitData->createProportionalOperation();
		biomassTurnover
			->addTransfer(_softwoodMerch,		_softwoodStemSnag,			_stemAnnualTurnOverRate)
			->addTransfer(_softwoodFoliage,		_aboveGroundVeryFastSoil,	_softwoodFoliageFallRate)
			->addTransfer(_softwoodOther,		_softwoodBranchSnag,		(1 - _otherToBranchSnagSplit) * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodOther,		_aboveGroundFastSoil,		_otherToBranchSnagSplit * _softwoodBranchTurnOverRate)
			->addTransfer(_softwoodCoarseRoots, _aboveGroundFastSoil,		_coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_softwoodCoarseRoots, _belowGroundFastSoil,		(1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_softwoodFineRoots,	_aboveGroundVeryFastSoil,	_fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_softwoodFineRoots,	_belowGroundVeryFastSoil,	(1 - _fineRootAGSplit) * _fineRootTurnProp)

			->addTransfer(_hardwoodMerch,		_hardwoodStemSnag,			_stemAnnualTurnOverRate)
			->addTransfer(_hardwoodFoliage,		_aboveGroundVeryFastSoil,	_hardwoodFoliageFallRate)
			->addTransfer(_hardwoodOther,		_hardwoodBranchSnag,		(1 - _otherToBranchSnagSplit) * _hardwoodBranchTurnOverRate)
			->addTransfer(_hardwoodOther,		_aboveGroundFastSoil,		_otherToBranchSnagSplit * _hardwoodBranchTurnOverRate)
			->addTransfer(_hardwoodCoarseRoots, _aboveGroundFastSoil,		_coarseRootSplit * _coarseRootTurnProp)
			->addTransfer(_hardwoodCoarseRoots, _belowGroundFastSoil,		(1 - _coarseRootSplit) * _coarseRootTurnProp)
			->addTransfer(_hardwoodFineRoots,	_aboveGroundVeryFastSoil,	_fineRootAGSplit * _fineRootTurnProp)
			->addTransfer(_hardwoodFineRoots,	_belowGroundVeryFastSoil,	(1 - _fineRootAGSplit) * _fineRootTurnProp);

		_landUnitData->submitOperation(biomassTurnover);
		*/
	}

	void CBMTurnoverModule::onTimingEndStep(const flint::TimingEndStepNotification::Ptr& n){
		//Record current biomass pool value as previous for next step....

		preStandSoftwoodMerch = _softwoodMerch->value();
		preStandSoftwoodOther = _softwoodOther->value();
		preStandSoftwoodFoliage = _softwoodFoliage->value();
		preStandSoftwoodCoarseRoots = _softwoodCoarseRoots->value();
		preStandSoftwoodFineRoots = _softwoodFineRoots->value();
		preStandHardwoodMerch = _hardwoodMerch->value();
		preStandHardwoodOther = _hardwoodOther->value();
		preStandHardwoodFoliage = _hardwoodFoliage->value();
		preStandHardwoodCoarseRoots = _hardwoodCoarseRoots->value();
		preStandHardwoodFineRoots = _hardwoodFineRoots->value();
	}

	std::shared_ptr<OvermatureDeclineLosses> CBMTurnoverModule::getOvermatrueDeclineLosses(double merchCarbonChanges, double foliageCarbonChanges, double otherCarbonChanges, double coarseRootCarbonChanges, double fineRootCarbonChanges){
		std::shared_ptr<OvermatureDeclineLosses> losses = std::make_shared<OvermatureDeclineLosses>();

		double changes = merchCarbonChanges + foliageCarbonChanges + otherCarbonChanges + coarseRootCarbonChanges + fineRootCarbonChanges;

		if (changes < 0){
			losses->setLossesPresent(true);
		}

		if (merchCarbonChanges < 0)
		{
			losses->setMerchToStemSnags(-merchCarbonChanges);
		}

		if (foliageCarbonChanges < 0)
		{
			losses->setFoliageToAGVeryFast(-foliageCarbonChanges);
		}

		if (otherCarbonChanges < 0)
		{
			losses->setOtherToBranchSnag(-otherCarbonChanges * (1 - _otherToBranchSnagSplit));
			losses->setOtherToAGFast(-otherCarbonChanges * _otherToBranchSnagSplit);
		}

		if (coarseRootCarbonChanges < 0)
		{
			losses->setCoarseRootToAGFast(-coarseRootCarbonChanges * _coarseRootSplit);
			losses->setCoarseRootToBGFast(-coarseRootCarbonChanges * (1 - _coarseRootSplit));
		}

		if (fineRootCarbonChanges < 0)
		{
			losses->setFineRootToAGVeryFast(-fineRootCarbonChanges * _fineRootAGSplit);
			losses->setFineRootToBGVeryFast(-fineRootCarbonChanges * (1 - _fineRootAGSplit));
		}

		return losses;
	};

}}} // namespace moja::flint
