#include <moja/flint/ivariable.h>
#include <moja/flint/ipool.h>
#include <moja/flint/ioperation.h>

#include <moja/signals.h>
#include <moja/notificationcenter.h>

#include "moja/modules/cbm/peatlandspinupnext.h"
#include "moja/modules/cbm/printpools.h"

namespace moja {
	namespace modules {
		namespace cbm {

			void PeatlandSpinupNext::configure(const DynamicObject& config) { }

			void PeatlandSpinupNext::subscribe(NotificationCenter& notificationCenter) {
				notificationCenter.subscribe(signals::LocalDomainInit, &PeatlandSpinupNext::onLocalDomainInit, *this);
				notificationCenter.subscribe(signals::PrePostDisturbanceEvent, &PeatlandSpinupNext::onPrePostDisturbanceEvent, *this);
			}

			void PeatlandSpinupNext::doLocalDomainInit() {
				_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
				_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
				_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");
				_softwoodOther = _landUnitData->getPool("SoftwoodOther");
				_softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
				_softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");

				_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
				_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
				_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");
				_hardwoodOther = _landUnitData->getPool("HardwoodOther");
				_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
				_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");

				_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
				_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
				_softwoodStem = _landUnitData->getPool("SoftwoodStem");
				_hardwoodStem = _landUnitData->getPool("HardwoodStem");

				_woodyFoliageLive = _landUnitData->getPool("WoodyFoliageLive");
				_woodyStemsBranchesLive = _landUnitData->getPool("WoodyStemsBranchesLive");
				_woodyRootsLive = _landUnitData->getPool("WoodyRootsLive");
				_sedgeFoliageLive = _landUnitData->getPool("SedgeFoliageLive");
				_sedgeRootsLive = _landUnitData->getPool("SedgeRootsLive");
				_sphagnumMossLive = _landUnitData->getPool("SphagnumMossLive");
				_featherMossLive = _landUnitData->getPool("FeatherMossLive");

				_woodyFoliageDead = _landUnitData->getPool("WoodyFoliageDead");
				_woodyFineDead = _landUnitData->getPool("WoodyFineDead");
				_woodyCoarseDead = _landUnitData->getPool("WoodyCoarseDead");
				_woodyRootsDead = _landUnitData->getPool("WoodyRootsDead");
				_sedgeFoliageDead = _landUnitData->getPool("SedgeFoliageDead");
				_sedgeRootsDead = _landUnitData->getPool("SedgeRootsDead");
				_feathermossDead = _landUnitData->getPool("FeathermossDead");

				_acrotelm_o = _landUnitData->getPool("Acrotelm_O");
				_catotelm_a = _landUnitData->getPool("Catotelm_A");
				_acrotelm_a = _landUnitData->getPool("Acrotelm_A");
				_catotelm_o = _landUnitData->getPool("Catotelm_O");

				_atmosphere = _landUnitData->getPool("Atmosphere");
			}

			void PeatlandSpinupNext::doPrePostDisturbanceEvent() {
				auto loadInitialFlag = _landUnitData->getVariable("load_peatpool_initials")->value();
				if (loadInitialFlag) {
					//PrintPools::printPeatlandPools("Year ", *_landUnitData);

					//in case of loading initial peat pool value, this step is skipped. 
					//initial acrotelm and catotelm pool values will be loadded in peatland prepare module

					//now, load initial value is not preferred
					return;
				}

				//get the current peatland ID
				auto& peatland_class = _landUnitData->getVariable("peatland_class")->value();
				auto peatlandId = peatland_class.isEmpty() ? -1 : peatland_class.convert<int>();
				bool runPeatland = peatlandId > 0;

				if (runPeatland) {
					//get the mean anual temperture variable
					auto meanAT = _landUnitData->getVariable("mean_annual_temperature")->value();
					double defaultMAT = _landUnitData->getVariable("default_mean_annual_temperature")->value();
					meanAnnualTemperature = meanAT.isEmpty() ? defaultMAT : meanAT.convert<double>();

					//get fire return interval
					auto fireReturnInterval = _landUnitData->getVariable("fire_return_interval")->value();
					int defaultFRI = _landUnitData->getVariable("default_fire_return_interval")->value();
					f_r = fireReturnInterval.isEmpty() ? defaultFRI : fireReturnInterval.convert<int>();
					f_fr = 1.0 / f_r;

					// get turnover rate
					getTreeTurnoverRate(Peatlands(peatlandId));

					// get related parameters
					getAndUpdateParameter();

					// prepare for speeding peatland spinup
					getNonOpenPeatlandRemovals(Peatlands(peatlandId));

					//check values in current peat pools
					getCurrentDeadPoolValues();

					//reset some of the dead pools
					resetSlowPools();

					// transfer carbon between pools
					int spinupFactor = _landUnitData->getVariable("peatland_spinup_factor")->value();
					if (spinupFactor > 0) {
						//when factor is set either 8000 or 10000
						populatePeatlandDeadPoolsV3();
					}
				}
			}

			void PeatlandSpinupNext::getAndUpdateParameter() {
				// get the data by variable "peatland_decay_parameters"
				const auto& peatlandDecayParams = _landUnitData->getVariable("peatland_decay_parameters")->value();

				//create the PeaglandDecayParameters, set the value from the variable
				decayParas = std::make_shared<PeatlandDecayParameters>();
				decayParas->setValue(peatlandDecayParams.extract<DynamicObject>());

				//compute the applied parameters
				decayParas->updateAppliedDecayParameters(meanAnnualTemperature);

				// get the data by variable "peatland_turnover_parameters"
				const auto& peatlandTurnoverParams = _landUnitData->getVariable("peatland_turnover_parameters")->value();

				//create the PeaglandGrowthParameters, set the value from the variable
				turnoverParas = std::make_shared<PeatlandTurnoverParameters>();
				turnoverParas->setValue(peatlandTurnoverParams.extract<DynamicObject>());

				// get the data by variable "peatland_growth_parameters"
				const auto& peatlandGrowthParams = _landUnitData->getVariable("peatland_growth_parameters")->value();

				//create the PeatlandGrowthParameters, set the value from the variable
				growthParas = std::make_shared<PeatlandGrowthParameters>();
				growthParas->setValue(peatlandGrowthParams.extract<DynamicObject>());

				// get the data by variable "peatland_fire_parameters"
				const auto& peatlandFireParams = _landUnitData->getVariable("peatland_fire_parameters")->value();

				//create the PeatlandFireParameters, set the value from the variable
				fireParas = std::make_shared<PeatlandFireParameters>();
				if (!peatlandFireParams.isEmpty()) {
					fireParas->setValue(peatlandFireParams.extract<DynamicObject>());
				}
			}

			void PeatlandSpinupNext::getTreeTurnoverRate(Peatlands peatlandId) {
				if (peatlandId == Peatlands::OPEN_PEATLAND_BOG ||
					peatlandId == Peatlands::OPEN_PEATLAND_POORFEN ||
					peatlandId == Peatlands::OPEN_PEATLAND_RICHFEN)
				{
					return;
				}

				// Get turnover parameter for treed and forested peatland only
				_turnoverRates = _landUnitData->getVariable("turnover_rates");
				auto& turnoverRates = _turnoverRates->value().extract<DynamicObject>();

				_otherToBranchSnagSplit = turnoverRates["sw_other_to_branch_snag_split"];
				_stemAnnualTurnOverRate = turnoverRates["sw_stem_turnover"];
				_softwoodFoliageFallRate = turnoverRates["sw_foliage_turnover"];
				_hardwoodFoliageFallRate = turnoverRates["hw_foliage_turnover"];
				_softwoodBranchTurnOverRate = turnoverRates["sw_branch_turnover"];
				_hardwoodBranchTurnOverRate = turnoverRates["hw_branch_turnover"];
				_coarseRootTurnProp = turnoverRates["sw_coarse_root_turnover"];
				_fineRootTurnProp = turnoverRates["sw_fine_root_turnover"];
				_stemSnagTurnoverRate = turnoverRates["sw_stem_snag_turnover"];
				_branchSnagTurnoverRate = turnoverRates["sw_branch_snag_turnover"];
			}

			//all of the slow dead pools are directly assigned by above computing based on live pools
			//reset current slow pool value to receive the new computed value
			void PeatlandSpinupNext::resetSlowPools() {
				auto peatlandDeadPoolReset = _landUnitData->createProportionalOperation();
				peatlandDeadPoolReset
					->addTransfer(_woodyFoliageDead, _atmosphere, 1.0)
					->addTransfer(_woodyFineDead, _atmosphere, 1.0)
					->addTransfer(_woodyRootsDead, _atmosphere, 1.0)
					->addTransfer(_sedgeFoliageDead, _atmosphere, 1.0)
					->addTransfer(_sedgeRootsDead, _atmosphere, 1.0)
					->addTransfer(_feathermossDead, _atmosphere, 1.0)
					->addTransfer(_woodyCoarseDead, _atmosphere, 1.0)
					->addTransfer(_acrotelm_o, _atmosphere, 1.0)
					->addTransfer(_catotelm_a, _atmosphere, 1.0);
				_landUnitData->submitOperation(peatlandDeadPoolReset);
				_landUnitData->applyOperations();
			}

			//Get removals (live biomass turnover) from small tree or big tree peatlands
			//The removals will be input to some peatland pools
			void PeatlandSpinupNext::getNonOpenPeatlandRemovals(Peatlands peatlandId)
			{
				switch (peatlandId) {
				case Peatlands::TREED_PEATLAND_BOG:
				case Peatlands::TREED_PEATLAND_POORFEN:
				case Peatlands::TREED_PEATLAND_RICHFEN:
				case Peatlands::TREED_PEATLAND_SWAMP:
					smallTreeOn = 1;
					smallTreeFoliageRemoval = _softwoodFoliageFallRate * _softwoodFoliage->value() + _hardwoodFoliageFallRate * _hardwoodFoliage->value();
					smallTreeBranchSnagRemoval = _branchSnagTurnoverRate * (_softwoodBranchSnag->value() + _hardwoodBranchSnag->value());
					smallTreeOtherRemovalToWFD = (1 - _otherToBranchSnagSplit) * (_softwoodOther->value() * _softwoodBranchTurnOverRate + _hardwoodOther->value() * _hardwoodBranchTurnOverRate);
					smallTreeCoarseRootRemoval = _coarseRootTurnProp * (_softwoodCoarseRoots->value() + _hardwoodCoarseRoots->value());
					smallTreeFineRootRemoval = _fineRootTurnProp * (_softwoodFineRoots->value() + _hardwoodFineRoots->value());
					smallTreeStemSnagRemoval = _stemSnagTurnoverRate * (_softwoodStemSnag->value() + _hardwoodStemSnag->value());
					break;
				case Peatlands::FOREST_PEATLAND_BOG:
				case Peatlands::FOREST_PEATLAND_POORFEN:
				case Peatlands::FOREST_PEATLAND_RICHFEN:
				case Peatlands::FOREST_PEATLAND_SWAMP:
					largeTreeOn = 1;
					largeTreeFoliageRemoval = _softwoodFoliageFallRate * _softwoodFoliage->value() + _hardwoodFoliageFallRate * _hardwoodFoliage->value();
					largeTreeBranchSnagRemoval = _branchSnagTurnoverRate * (_softwoodBranchSnag->value() + _hardwoodBranchSnag->value());
					largeTreeOtherRemovalToWFD = (1 - _otherToBranchSnagSplit) * (_softwoodOther->value() * _softwoodBranchTurnOverRate + _hardwoodOther->value() * _hardwoodBranchTurnOverRate);
					largeTreeCoarseRootRemoval = _coarseRootTurnProp * (_softwoodCoarseRoots->value() + _hardwoodCoarseRoots->value());
					largeTreeFineRootRemoval = _fineRootTurnProp * (_softwoodFineRoots->value() + _hardwoodFineRoots->value());
					largeTreeStemSnagRemoval = _stemSnagTurnoverRate * (_softwoodStemSnag->value() + _hardwoodStemSnag->value());
					break;
				}
			}

			void PeatlandSpinupNext::getCurrentDeadPoolValues() {
				auto wdyFoliageDead = _woodyFoliageDead->value();
				auto wdyStemBranchDead = _woodyFineDead->value();
				auto wdyRootsDead = _woodyRootsDead->value();
				auto sedgeFoliageDead = _sedgeFoliageDead->value();
				auto sedgeRootsDead = _sedgeRootsDead->value();
				auto featherMossDead = _feathermossDead->value();
				auto woodyCoarseDead = _woodyCoarseDead->value();
				auto actotelm = _acrotelm_o->value();
				auto catotelm = _catotelm_a->value();
			}

			// Latest implementation of the spinup next procedure
			void PeatlandSpinupNext::populatePeatlandDeadPoolsV3() {
				auto RT_D_W_Fol = 1 / (turnoverParas->Pfe() * decayParas->kwfe() * modifyQ10(decayParas->Q10wf()) + turnoverParas->Pfn() * decayParas->kwfne() * modifyQ10(decayParas->Q10wf()));
				auto RT_D_W_StemBranch = 1 / (decayParas->kwsb() * modifyQ10(decayParas->Q10wsb()));
				auto RT_D_W_Roots = 1 / (decayParas->kwr() * modifyQ10(decayParas->Q10wr()));
				auto RT_D_S_Fol = 1 / (decayParas->ksf() * modifyQ10(decayParas->Q10sf()));
				auto RT_D_S_Roots = 1 / (decayParas->ksr() * modifyQ10(decayParas->Q10sr()));
				auto RT_D_Feath = 1 / (decayParas->kfm() * modifyQ10(decayParas->Q10fm()));
				auto RT_Acro = 1 / (decayParas->ka() * modifyQ10(decayParas->Q10a()));
				auto TR_Acro = 1 / RT_Acro;
				auto RT_Cato = 1 / (decayParas->kc() * modifyQ10(decayParas->Q10c()));
				auto TR_Cato = 1 / RT_Cato;
				auto RT_D_CWD = 1 / (decayParas->kwc() * modifyQ10(decayParas->Q10wc()));

				//special factor applied to functions to populate peat pool
				int spinupFactor = _landUnitData->getVariable("peatland_spinup_factor")->value();

				auto wdyFoliageLive = _woodyFoliageLive->value();
				auto wdyFoliageDead = (
					wdyFoliageLive * (turnoverParas->Pfe() * turnoverParas->Pel() + turnoverParas->Pnl() * turnoverParas->Pfn()) +
					smallTreeOn * smallTreeFoliageRemoval +
					largeTreeOn * largeTreeFoliageRemoval) * (RT_D_W_Fol - 1) * (f_r / (f_r + RT_D_W_Fol));
				auto DWFol = wdyFoliageDead / (1 - (1 / RT_D_W_Fol));
				auto wdyFoliageDeadToAcrotelm = decayParas->Pt() * DWFol * modifyQ10(decayParas->Q10wf()) *
					(turnoverParas->Pfe() * decayParas->kwfe() + turnoverParas->Pfn() * decayParas->kwfne());

				auto wdyStemBranchLive = _woodyStemsBranchesLive->value();
				auto wdyStemBranchDead = (
					(wdyStemBranchLive * growthParas->NPPagls() / growthParas->Bagls()) +
					(smallTreeOn * (smallTreeStemSnagRemoval + smallTreeBranchSnagRemoval + smallTreeOtherRemovalToWFD)) +
					(largeTreeOn * (largeTreeBranchSnagRemoval + largeTreeOtherRemovalToWFD))) * (RT_D_W_StemBranch - 1) * (f_r / (f_r + RT_D_W_StemBranch));
				auto DWStemBranch = wdyStemBranchDead / (1 - (1 / RT_D_W_StemBranch));
				auto wdyStemBranchDeadToAcrotelm = decayParas->Pt() * DWStemBranch *
					decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb()));

				auto woodyCoarseDead = largeTreeOn * largeTreeStemSnagRemoval * (RT_D_CWD - 1) * (f_r / (f_r + RT_D_CWD));
				auto DCWD = woodyCoarseDead / (1 - (1 / RT_D_CWD));
				auto wdyCoarseDeadToAcrotelm = decayParas->Pt() * DCWD * decayParas->kwc() * (modifyQ10(decayParas->Q10wc()));

				auto wdyRootsLive = _woodyRootsLive->value();
				auto LWRoots = wdyRootsLive / (1 - turnoverParas->Mbgls());
				auto wdyRootsLiveToAcrotelm = LWRoots * f_fr * fireParas->CTwr();

				auto wdyRootsDead = (
					LWRoots * turnoverParas->Mbgls() +
					(smallTreeOn * (smallTreeFineRootRemoval + smallTreeCoarseRootRemoval)) +
					(largeTreeOn * (largeTreeFineRootRemoval + largeTreeCoarseRootRemoval))) * (RT_D_W_Roots - 1) * (f_r / (f_r + fireParas->CCdwr() * RT_D_W_Roots));
				auto DWRoots = wdyRootsDead / (1 - (1 / RT_D_W_Roots));
				auto wdyRootsDeadToAcrotelm = decayParas->Pt() * DWRoots * decayParas->kwr() * (modifyQ10(decayParas->Q10wr()));

				auto sedgeFoliageLive = _sedgeFoliageLive->value();
				auto temp1 = sedgeFoliageLive / (1 - turnoverParas->Mags());
				auto sedgeFoliageDead = temp1 * turnoverParas->Mags() * (RT_D_S_Fol - 1) * (f_r / (f_r + fireParas->CCdsf() * RT_D_S_Fol));
				auto DSFol = sedgeFoliageDead / (1 - (1 / RT_D_S_Fol));
				auto sedgeFoliageDeadToAcrotelm = decayParas->Pt() * DSFol * decayParas->ksf() * (modifyQ10(decayParas->Q10sf()));

				auto sedgeRootsLive = _sedgeRootsLive->value();
				auto temp2 = sedgeRootsLive / (1 - turnoverParas->Mbgs());
				auto sedgeRootsDead = temp2 * turnoverParas->Mbgs() * (RT_D_S_Roots - 1) * (f_r / (f_r + fireParas->CCdsr() * RT_D_S_Roots));
				auto DSRoots = sedgeRootsDead / (1 - (1 / RT_D_S_Roots));
				auto sedgeRootsDeadToAcrotelm = decayParas->Pt() * DSRoots * decayParas->ksr() * (modifyQ10(decayParas->Q10sr()));

				auto featherMossLive = _featherMossLive->value();
				auto featherMossDead = featherMossLive * (RT_D_Feath - 1) * (f_r / (f_r + fireParas->CCdfm() * RT_D_Feath));
				auto DFeath = featherMossDead / (1 - (1 / RT_D_Feath));
				auto featherMossDeadToAcrotelm = DFeath * decayParas->kfm() * (modifyQ10(decayParas->Q10fm()));

				auto sphagnumMossLive = _sphagnumMossLive->value();
				auto sphagnumMossLiveToAcrotelm = sphagnumMossLive;

				//following is for test8 from Kelly
				/*
				auto lwtd = _landUnitData->getVariable("peatland_longterm_wtd")->value().convert<double>();
				lwtd = lwtd < 0 ? (-1 * lwtd) : 0;
				auto a = this->turnoverParas->a();
				auto b = this->turnoverParas->b();
				auto toAcrotelm_new = 10 * (a * pow(lwtd, b));
				*/

				auto toAcrotelm = (
					wdyFoliageDeadToAcrotelm +
					wdyStemBranchDeadToAcrotelm +
					wdyCoarseDeadToAcrotelm +
					wdyRootsDeadToAcrotelm +
					sedgeFoliageDeadToAcrotelm +
					sedgeRootsDeadToAcrotelm +
					sphagnumMossLiveToAcrotelm +
					featherMossDeadToAcrotelm +
					wdyRootsLiveToAcrotelm) * (1 - exp(-1 * TR_Acro * spinupFactor)) * (RT_Acro - 1) * (f_r / (f_r + fireParas->CCa() * RT_Acro));

				// transfer carbon from acrotelm to catotelm 	
				auto Acro = toAcrotelm / (1 - 1 / RT_Acro);
				auto ac2caAmount = (decayParas->Pt() * Acro * decayParas->ka() * modifyQ10(decayParas->Q10a())) * (1 - exp(-1 * TR_Cato * spinupFactor)) * (RT_Cato - 1);

				//make sure ac2caAmount >=0
				ac2caAmount = ac2caAmount > 0 ? ac2caAmount : 0;

				// transfer carbons to peatland dead pool by stock amount
				// origianl toAcrotelm is replaced by toAcrotelm_new for test8
				auto peatlandSpinnupOne = _landUnitData->createStockOperation();
				peatlandSpinnupOne->addTransfer(_atmosphere, _woodyFoliageDead, wdyFoliageDead)
					->addTransfer(_atmosphere, _woodyFineDead, wdyStemBranchDead)
					->addTransfer(_atmosphere, _woodyCoarseDead, woodyCoarseDead)
					->addTransfer(_atmosphere, _woodyRootsDead, wdyRootsDead)
					->addTransfer(_atmosphere, _sedgeFoliageDead, sedgeFoliageDead)
					->addTransfer(_atmosphere, _sedgeRootsDead, sedgeRootsDead)
					->addTransfer(_atmosphere, _feathermossDead, featherMossDead)
					->addTransfer(_atmosphere, _acrotelm_o, toAcrotelm)
					->addTransfer(_atmosphere, _catotelm_a, ac2caAmount);
				_landUnitData->submitOperation(peatlandSpinnupOne);
				_landUnitData->applyOperations();
			}

			/*
			// second version of spinup next procedure
			void PeatlandSpinupNext::populatePeatlandDeadPoolsV2() {
				auto RT_D_W_Fol = 1 / (turnoverParas->Pfe() * decayParas->kwfe() * modifyQ10(decayParas->Q10wf()) + turnoverParas->Pfn() * decayParas->kwfne()* modifyQ10(decayParas->Q10wf()));
				auto RT_D_W_StemBranch = 1 / (decayParas->kwsb() * modifyQ10(decayParas->Q10wsb()));
				auto RT_D_W_Roots = 1 / (decayParas->kwr() * modifyQ10(decayParas->Q10wr()));
				auto RT_D_S_Fol = 1 / (decayParas->ksf() * modifyQ10(decayParas->Q10sf()));
				auto RT_D_S_Roots = 1 / (decayParas->ksr() * modifyQ10(decayParas->Q10sr()));
				auto RT_D_Feath = 1 / (decayParas->kfm() * modifyQ10(decayParas->Q10fm()));
				auto RT_Acro = 1 / (decayParas->ka() * modifyQ10(decayParas->Q10a()));
				auto TR_Acro = 1 / RT_Acro;
				auto RT_Cato = 1 / (decayParas->kc() * modifyQ10(decayParas->Q10c()));
				auto TR_Cato = 1 / RT_Cato;

				//special factor applied to functions to populate peat pool
				int spinupFactor= _landUnitData->getVariable("peatland_spinup_factor")->value();

				auto wdyFoliageLive = _woodyFoliageLive->value();
				auto wdyFoliageDead = (
					wdyFoliageLive * (turnoverParas->Pfe()  * turnoverParas->Pel() + turnoverParas->Pnl() * turnoverParas->Pfn()) +
					smallTreeOn * turnoverParas->Mstf() * smallTreeFoliageRemoval  +
					largeTreeOn * largeTreeFoliageRemoval) * (RT_D_W_Fol - 1) * (f_r / (f_r + RT_D_W_Fol));
				auto DWFol = wdyFoliageDead / (1 - (1 / RT_D_W_Fol));
				auto wdyFoliageDeadToAcrotelm = decayParas->Pt() * DWFol *
					(turnoverParas->Pfe() * decayParas->kwfe() * (modifyQ10(decayParas->Q10wf())) +
						turnoverParas->Pfn() * decayParas->kwfne() * (modifyQ10(decayParas->Q10wf())));

				auto wdyStemBranchLive = _woodyStemsBranchesLive->value();
				auto wdyStemBranchDead = (
					(wdyStemBranchLive * growthParas->NPPagls() / growthParas->Bagls()) +
					(smallTreeOn * (turnoverParas->Msto() * smallTreeOtherRemovalToWFD + turnoverParas->Msts() * smallTreeBranchSnagRemoval)) +
					(largeTreeOn * (largeTreeBranchSnagRemoval + largeTreeOtherRemovalToWFD))) * (RT_D_W_StemBranch - 1) * (f_r / (f_r + RT_D_W_StemBranch));
				auto DWStemBranch = wdyStemBranchDead / (1 - (1 / RT_D_W_StemBranch));
				auto wdyStemBranchDeadToAcrotelm = decayParas->Pt() * DWStemBranch *
					decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb()));

				auto wdyRootsLive = _woodyRootsLive->value();
				auto LWRoots = wdyRootsLive / (1 - turnoverParas->Mbgls());
				auto wdyRootsLiveToAcrotelm = LWRoots * f_fr * fireParas->CTwr();

				auto wdyRootsDead = (
					LWRoots* turnoverParas->Mbgls() +
					(smallTreeOn * (turnoverParas->Mstfr() * smallTreeFineRootRemoval + turnoverParas->Mstcr() * smallTreeCoarseRootRemoval) ) +
					(largeTreeOn * (largeTreeFineRootRemoval + largeTreeCoarseRootRemoval))) * (RT_D_W_Roots - 1) * (f_r / (f_r + fireParas->CCdwr() * RT_D_W_Roots));
				auto DWRoots = wdyRootsDead / (1 - (1 / RT_D_W_Roots));
				auto wdyRootsDeadToAcrotelm = decayParas->Pt() * DWRoots * decayParas->kwr() * (modifyQ10(decayParas->Q10wr()));

				auto sedgeFoliageLive = _sedgeFoliageLive->value();
				auto temp1 = sedgeFoliageLive / (1 - turnoverParas->Mags());
				auto sedgeFoliageDead =  temp1 * turnoverParas->Mags() * (RT_D_S_Fol - 1) * (f_r / (f_r + fireParas->CCdsf() * RT_D_S_Fol));
				auto DSFol = sedgeFoliageDead / (1 - (1 / RT_D_S_Fol));
				auto sedgeFoliageDeadToAcrotelm = decayParas->Pt() * DSFol * decayParas->ksf() * (modifyQ10(decayParas->Q10sf()));

				auto sedgeRootsLive = _sedgeRootsLive->value();
				auto temp2 = sedgeRootsLive / (1 - turnoverParas->Mbgs());
				auto sedgeRootsDead = temp2 * turnoverParas->Mbgs() * (RT_D_S_Roots - 1) * (f_r / (f_r + fireParas->CCdsr() * RT_D_S_Roots));
				auto DSRoots = sedgeRootsDead / (1 - (1 / RT_D_S_Roots));
				auto sedgeRootsDeadToAcrotelm = decayParas->Pt() * DSRoots * decayParas->ksr() * (modifyQ10(decayParas->Q10sr()));

				auto featherMossLive = _featherMossLive->value();
				auto featherMossDead = featherMossLive * (RT_D_Feath - 1) * (f_r / (f_r + fireParas->CCdfm() * RT_D_Feath));
				auto DFeath = featherMossDead / (1 - (1 / RT_D_Feath));
				auto featherMossDeadToAcrotelm = DFeath * decayParas->kfm() * (modifyQ10(decayParas->Q10fm()));

				auto sphagnumMossLive = _sphagnumMossLive->value();
				auto sphagnumMossLiveToAcrotelm = sphagnumMossLive;

				auto toAcrotelm = (
					wdyFoliageDeadToAcrotelm +
					wdyStemBranchDeadToAcrotelm +
					wdyRootsDeadToAcrotelm +
					sedgeFoliageDeadToAcrotelm +
					sedgeRootsDeadToAcrotelm +
					sphagnumMossLiveToAcrotelm +
					featherMossDeadToAcrotelm +
					wdyRootsLiveToAcrotelm ) * (1 - exp(-1 * TR_Acro * spinupFactor)) * (RT_Acro - 1) * (f_r / (f_r + fireParas->CCa() * RT_Acro));

				// transfer carbon from acrotelm to catotelm
				auto Acro = toAcrotelm / (1 - 1 / RT_Acro);
				auto ac2caAmount = (decayParas->Pt() * Acro * decayParas->ka()  * modifyQ10(decayParas->Q10a())) * (1 - exp(-1 * TR_Cato * spinupFactor)) * (RT_Cato - 1);

				//make sure ac2caAmount >=0
				ac2caAmount = ac2caAmount > 0 ? ac2caAmount : 0;

				// transfer carbons to peatland dead pool by stock amount
				auto peatlandSpinnupOne = _landUnitData->createStockOperation();
				peatlandSpinnupOne->addTransfer(_atmosphere, _woodyFoliageDead, wdyFoliageDead)
					->addTransfer(_atmosphere, _woodyFineDead, wdyStemBranchDead)
					->addTransfer(_atmosphere, _woodyRootsDead, wdyRootsDead)
					->addTransfer(_atmosphere, _sedgeFoliageDead, sedgeFoliageDead)
					->addTransfer(_atmosphere, _sedgeRootsDead, sedgeRootsDead)
					->addTransfer(_atmosphere, _feathermossDead, featherMossDead)
					->addTransfer(_atmosphere, _acrotelm_o, toAcrotelm)
					->addTransfer(_atmosphere, _catotelm_a, ac2caAmount);
				_landUnitData->submitOperation(peatlandSpinnupOne);
				_landUnitData->applyOperations();
			}

			// frist version of spinup next procedure
			void PeatlandSpinupNext::populatePeatlandDeadPoolsV1() {
				double fireReturnReciprocal = f_fr;
				auto wdyFoliageLive = _woodyFoliageLive->value();
				auto denominator1 = (turnoverParas->Pfe() * decayParas->kwfe() * (modifyQ10(decayParas->Q10wf())) +
					turnoverParas->Pfn() * decayParas->kwfne() * (modifyQ10(decayParas->Q10wf())) +
					fireReturnReciprocal * fireParas->CCdwf());

				auto wdyFoliageDead = (
					(wdyFoliageLive * (turnoverParas->Pfe()  * turnoverParas->Pel() + turnoverParas->Pnl() * turnoverParas->Pfn())) +
					(smallTreeOn * turnoverParas->Mstf() * smallTreeFoliageRemoval ) +
					(largeTreeOn * largeTreeFoliageRemoval)) / denominator1;

				auto wdyStemBranchLive = _woodyStemsBranchesLive->value();
				auto denominator2 = (decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb())) +
					fireReturnReciprocal * fireParas->CCdwsb());
				auto wdyStemBranchDead = (
					(wdyStemBranchLive * growthParas->NPPagls() / growthParas->Bagls()) +
					(smallTreeOn * (turnoverParas->Msto() * smallTreeOtherRemovalToWFD + turnoverParas->Msts() * smallTreeBranchSnagRemoval) ) +
					(largeTreeOn * (largeTreeBranchSnagRemoval + largeTreeOtherRemovalToWFD) )) / denominator2;

				auto wdyRootsLive = _woodyRootsLive->value();
				auto denominator3 = (decayParas->kwr() * (modifyQ10(decayParas->Q10wr())) +
					fireReturnReciprocal * fireParas->CCdwr());
				auto wdyRootsDead = (
					(wdyRootsLive * turnoverParas->Mbgls())+
					(smallTreeOn * (turnoverParas->Mstfr() * smallTreeFineRootRemoval + turnoverParas->Mstcr() * smallTreeCoarseRootRemoval) ) +
					(largeTreeOn * (largeTreeFineRootRemoval + largeTreeCoarseRootRemoval) )) /denominator3;

				auto sedgeFoliageLive = _sedgeFoliageLive->value();
				auto denominator4 = (decayParas->ksf() * (modifyQ10(decayParas->Q10sf())) +
					fireReturnReciprocal * fireParas->CCdsf());
				auto sedgeFoliageDead = sedgeFoliageLive * turnoverParas->Mags() / denominator4;

				auto sedgeRootsLive = _sedgeRootsLive->value();
				auto denominator5 = (decayParas->ksr() *  (modifyQ10(decayParas->Q10sr())) +
					fireReturnReciprocal * fireParas->CCdsr());
				auto sedgeRootsDead = sedgeRootsLive * turnoverParas->Mbgs() / denominator5;

				auto featherMossLive = _featherMossLive->value();
				auto denominator6 = (decayParas->kfm() * (modifyQ10(decayParas->Q10fm())) +
					fireReturnReciprocal * fireParas->CCdfm());
				auto featherMossDead = featherMossLive / denominator6;

				auto wdyFoliageDeadToAcrotelm = decayParas->Pt() * wdyFoliageDead *
					(turnoverParas->Pfe() * decayParas->kwfe() * (modifyQ10(decayParas->Q10wf())) +
					turnoverParas->Pfn() * decayParas->kwfne() * (modifyQ10(decayParas->Q10wf())));

				auto wdyStemBranchDeadToAcrotelm = decayParas->Pt() * wdyStemBranchDead *
					decayParas->kwsb() * (modifyQ10(decayParas->Q10wsb()));

				auto wdyRootsDeadToAcrotelm = decayParas->Pt() * wdyRootsDead *
					decayParas->kwr() * (modifyQ10(decayParas->Q10wr()));

				auto sedgeFoliageDeadToAcrotelm = decayParas->Pt() * sedgeFoliageDead *
					decayParas->ksf() * (modifyQ10(decayParas->Q10sf()));

				auto sedgeRootsDeadToAcrotelm = decayParas->Pt() * sedgeRootsDead *
					decayParas->ksr() * (modifyQ10(decayParas->Q10sr()));

				auto sphagnumMossLive = _sphagnumMossLive->value();
				auto sphagnumMossLiveToAcrotelm = sphagnumMossLive;

				auto featherMossDeadToAcrotelm = featherMossDead * decayParas->kfm() * (modifyQ10(decayParas->Q10fm()));

				auto wdyRootsLiveToAcrotelm = wdyRootsLive * fireReturnReciprocal * fireParas->CTwr();

				auto sedgeRootsLiveToAcrotelm = sedgeRootsLive * fireReturnReciprocal * fireParas->CTsr();

				auto denominator7 = (decayParas->ka() * (modifyQ10(decayParas->Q10a())) + (fireReturnReciprocal) * fireParas->CCa());
				auto toAcrotelm = (
					wdyFoliageDeadToAcrotelm +
					wdyStemBranchDeadToAcrotelm +
					wdyRootsDeadToAcrotelm +
					sedgeFoliageDeadToAcrotelm +
					sedgeRootsDeadToAcrotelm +
					sphagnumMossLiveToAcrotelm +
					featherMossDeadToAcrotelm +
					wdyRootsLiveToAcrotelm +
					sedgeRootsLiveToAcrotelm) / denominator7;

				// transfer carbon from acrotelm to catotelm
				auto ac2caAmount = (decayParas->Pt() * toAcrotelm * decayParas->ka()  * (modifyQ10(decayParas->Q10a())) - 0.3) /
					(decayParas->kc() * (modifyQ10(decayParas->Q10c())));

				//make sure ac2caAmount >=0
				ac2caAmount = ac2caAmount > 0 ? ac2caAmount : 0;

				// transfer carbons to peatland dead pool by stock amount
				auto peatlandSpinnupOne = _landUnitData->createStockOperation();
				peatlandSpinnupOne->addTransfer(_atmosphere, _woodyFoliageDead, wdyFoliageDead)
					->addTransfer(_atmosphere, _woodyFineDead, wdyStemBranchDead)
					->addTransfer(_atmosphere, _woodyRootsDead, wdyRootsDead)
					->addTransfer(_atmosphere, _sedgeFoliageDead, sedgeFoliageDead)
					->addTransfer(_atmosphere, _sedgeRootsDead, sedgeRootsDead)
					->addTransfer(_atmosphere, _feathermossDead, featherMossDead)
					->addTransfer(_atmosphere, _acrotelm_o, toAcrotelm)
					->addTransfer(_atmosphere, _catotelm_a, ac2caAmount);

				_landUnitData->submitOperation(peatlandSpinnupOne);
				_landUnitData->applyOperations();
			}
			*/

		}
	}
} // namespace moja::modules::cbm
