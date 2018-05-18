#include "moja/flint/recordaccumulatorwithmutex.h"

#include "moja/modules/cbm/cbmageindicators.h"
#include "moja/modules/cbm/cbmaggregatorlandunitdata.h"
#include "moja/modules/cbm/cbmaggregatorpostgresqlwriter.h"
#include "moja/modules/cbm/cbmaggregatorsqlitewriter.h"
#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/cbmdisturbancelistener.h"
#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/modules/cbm/cbmlandunitdatatransform.h"
#include "moja/modules/cbm/cbmsequencer.h"
#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"
#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmtransitionrulesmodule.h"
#include "moja/modules/cbm/esgymmodule.h"
#include "moja/modules/cbm/esgymspinupsequencer.h"
#include "moja/modules/cbm/growthcurvetransform.h"
#include "moja/modules/cbm/growthmultipliermodule.h"
#include "moja/modules/cbm/libraryfactory.h"
#include "moja/modules/cbm/mossdecaymodule.h"
#include "moja/modules/cbm/mossdisturbancemodule.h"
#include "moja/modules/cbm/mossgrowthmodule.h"
#include "moja/modules/cbm/mossturnovermodule.h"
#include "moja/modules/cbm/outputerstreamfluxpostnotify.h"
#include "moja/modules/cbm/outputerstreampostnotify.h"
#include "moja/modules/cbm/peatlanddecaymodule.h"
#include "moja/modules/cbm/peatlanddisturbancemodule.h"
#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/peatlandpreparemodule.h"
#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/yieldtablegrowthmodule.h"
#include "moja/modules/cbm/sawtoothmodule.h"
#include "moja/modules/cbm/smalltreegrowthmodule.h"
#include "moja/modules/cbm/peatlandspinupnext.h"

#include <atomic>
#include <vector>
#include <Poco/Mutex.h>

namespace moja {
namespace modules {

    struct CBMObjectHolder {
        CBMObjectHolder() : landUnitAggregatorId(1) {
            dateDimension			 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DateRow, cbm::DateRecord>>();
            poolInfoDimension		 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::PoolInfoRow, cbm::PoolInfoRecord>>();
            classifierSetDimension	 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ClassifierSetRow, cbm::ClassifierSetRecord>>();
            classifierNames          = std::make_shared<std::vector<std::string>>();
			classifierNamesLock		 = std::make_shared<Poco::Mutex>();
            landClassDimension       = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::LandClassRow, cbm::LandClassRecord>>();
            locationDimension        = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::TemporalLocationRow, cbm::TemporalLocationRecord>>();
            poolDimension			 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::PoolRow, cbm::PoolRecord>>();
            fluxDimension			 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::FluxRow, cbm::FluxRecord>>();
			ageClassDimension		 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::AgeClassRow, cbm::AgeClassRecord>>();
			ageAreaDimension		 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::AgeAreaRow, cbm::AgeAreaRecord>>();
            moduleInfoDimension      = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ModuleInfoRow, cbm::ModuleInfoRecord>>();
            disturbanceTypeDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceTypeRow, cbm::DisturbanceTypeRecord>>();
			disturbanceDimension	 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceRow, cbm::DisturbanceRecord>>();
			errorDimension			 = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ErrorRow, cbm::ErrorRecord>>();
			locationErrorDimension   = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::LocationErrorRow, cbm::LocationErrorRecord>>();
			gcFactory                = std::make_shared<cbm::StandGrowthCurveFactory>();
        }

        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::DateRow, cbm::DateRecord>> dateDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::PoolInfoRow, cbm::PoolInfoRecord>> poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::ClassifierSetRow, cbm::ClassifierSetRecord>> classifierSetDimension;
        std::shared_ptr<std::vector<std::string>> classifierNames;
		std::shared_ptr<Poco::Mutex> classifierNamesLock;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::LandClassRow, cbm::LandClassRecord>> landClassDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::TemporalLocationRow, cbm::TemporalLocationRecord>> locationDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::PoolRow, cbm::PoolRecord>> poolDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::FluxRow, cbm::FluxRecord>> fluxDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::AgeClassRow, cbm::AgeClassRecord>> ageClassDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::AgeAreaRow, cbm::AgeAreaRecord>> ageAreaDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::ModuleInfoRow, cbm::ModuleInfoRecord>> moduleInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceTypeRow, cbm::DisturbanceTypeRecord>> disturbanceTypeDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceRow, cbm::DisturbanceRecord>> disturbanceDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::ErrorRow, cbm::ErrorRecord>> errorDimension;
		std::shared_ptr<flint::RecordAccumulatorWithMutex2<cbm::LocationErrorRow, cbm::LocationErrorRecord>> locationErrorDimension;
		std::shared_ptr<cbm::StandGrowthCurveFactory> gcFactory;
        std::atomic<int> landUnitAggregatorId;
    };

    static CBMObjectHolder cbmObjectHolder;

    extern "C" {
        MOJA_LIB_API flint::IModule* CreateCBMAggregatorLandUnitData() {
            return new cbm::CBMAggregatorLandUnitData(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.landClassDimension,
                cbmObjectHolder.locationDimension,
                cbmObjectHolder.moduleInfoDimension,
                cbmObjectHolder.disturbanceTypeDimension,
				cbmObjectHolder.disturbanceDimension,
                cbmObjectHolder.classifierNames,
				cbmObjectHolder.classifierNamesLock,
                cbmObjectHolder.poolDimension,
                cbmObjectHolder.fluxDimension,
				cbmObjectHolder.ageClassDimension,
				cbmObjectHolder.ageAreaDimension,
				cbmObjectHolder.errorDimension,
				cbmObjectHolder.locationErrorDimension);
        }
        
        MOJA_LIB_API flint::IModule* CreateCBMAggregatorSQLiteWriter() {
            bool isPrimaryAggregator = cbmObjectHolder.landUnitAggregatorId++ == 1;
            return new cbm::CBMAggregatorSQLiteWriter(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.landClassDimension,
                cbmObjectHolder.locationDimension,
                cbmObjectHolder.moduleInfoDimension,
                cbmObjectHolder.disturbanceTypeDimension,
				cbmObjectHolder.disturbanceDimension,
				cbmObjectHolder.classifierNames,
                cbmObjectHolder.poolDimension,
                cbmObjectHolder.fluxDimension,
				cbmObjectHolder.ageClassDimension,
				cbmObjectHolder.ageAreaDimension,
				cbmObjectHolder.errorDimension,
				cbmObjectHolder.locationErrorDimension,
                isPrimaryAggregator);
        }

        MOJA_LIB_API flint::IModule* CreateCBMAggregatorPostgreSQLWriter() {
            bool isPrimaryAggregator = cbmObjectHolder.landUnitAggregatorId++ == 1;
            return new cbm::CBMAggregatorPostgreSQLWriter(
                cbmObjectHolder.dateDimension,
                cbmObjectHolder.poolInfoDimension,
                cbmObjectHolder.classifierSetDimension,
                cbmObjectHolder.landClassDimension,
                cbmObjectHolder.locationDimension,
                cbmObjectHolder.moduleInfoDimension,
                cbmObjectHolder.disturbanceTypeDimension,
                cbmObjectHolder.disturbanceDimension,
                cbmObjectHolder.classifierNames,
                cbmObjectHolder.poolDimension,
                cbmObjectHolder.fluxDimension,
                cbmObjectHolder.ageClassDimension,
                cbmObjectHolder.ageAreaDimension,
                cbmObjectHolder.errorDimension,
                cbmObjectHolder.locationErrorDimension,
                isPrimaryAggregator);
        }

        MOJA_LIB_API int getModuleRegistrations(moja::flint::ModuleRegistration* outModuleRegistrations) {
            int index = 0;
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorLandUnitData",      &CreateCBMAggregatorLandUnitData };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorPostgreSQLWriter",  &CreateCBMAggregatorPostgreSQLWriter };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorSQLiteWriter",      &CreateCBMAggregatorSQLiteWriter };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDecayModule",                 []() -> flint::IModule* { return new cbm::CBMDecayModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDisturbanceEventModule",	   []() -> flint::IModule* { return new cbm::CBMDisturbanceEventModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDisturbanceListener",	       []() -> flint::IModule* { return new cbm::CBMDisturbanceListener(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMGrowthModule",                []() -> flint::IModule* { return new cbm::YieldTableGrowthModule(cbmObjectHolder.gcFactory); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSequencer",				   []() -> flint::IModule* { return new cbm::CBMSequencer(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamPostNotify",	   []() -> flint::IModule* { return new cbm::OutputerStreamPostNotify(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamFluxPostNotify",   []() -> flint::IModule* { return new cbm::OutputerStreamFluxPostNotify(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupSequencer",			   []() -> flint::IModule* { return new cbm::CBMSpinupSequencer(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMBuildLandUnitModule",		   []() -> flint::IModule* { return new cbm::CBMBuildLandUnitModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupDisturbanceModule",     []() -> flint::IModule* { return new cbm::CBMSpinupDisturbanceModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMLandClassTransitionModule",   []() -> flint::IModule* { return new cbm::CBMLandClassTransitionModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossTurnoverModule",		   []() -> flint::IModule* { return new cbm::MossTurnoverModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossDecayModule",			   []() -> flint::IModule* { return new cbm::MossGrowthModule(cbmObjectHolder.gcFactory); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossGrowthModule",			   []() -> flint::IModule* { return new cbm::MossDecayModule(cbmObjectHolder.gcFactory); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "GrowthMultiplierModule",		   []() -> flint::IModule* { return new cbm::GrowthMultiplierModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandDisturbanceModule",      []() -> flint::IModule* { return new cbm::PeatlandDisturbanceModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "MossDisturbanceModule",		   []() -> flint::IModule* { return new cbm::MossDisturbanceModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandPrepareModule",		   []() -> flint::IModule* { return new cbm::PeatlandPrepareModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandGrowthModule",		   []() -> flint::IModule* { return new cbm::PeatlandGrowthModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandTurnoverModule",		   []() -> flint::IModule* { return new cbm::PeatlandTurnoverModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandDecayModule",			   []() -> flint::IModule* { return new cbm::PeatlandDecayModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMTransitionRulesModule",       []() -> flint::IModule* { return new cbm::CBMTransitionRulesModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "ESGYMModule",					   []() -> flint::IModule* { return new cbm::ESGYMModule(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "ESGYMSpinupSequencer",		   []() -> flint::IModule* { return new cbm::ESGYMSpinupSequencer(); } };
            outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAgeIndicators",		       []() -> flint::IModule* { return new cbm::CBMAgeIndicators(); } };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "SawtoothModule",				   []() -> flint::IModule* { return new cbm::SawtoothModule(); } };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "SmallTreeGrowthModule",		   []() -> flint::IModule* { return new cbm::SmallTreeGrowthModule(); } };
			outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandSpinupNext",			   []() -> flint::IModule* { return new cbm::PeatlandSpinupNext(); } };
            return index;                                                                                  
        }

        MOJA_LIB_API int getTransformRegistrations(flint::TransformRegistration* outTransformRegistrations) {
            int index = 0;
            outTransformRegistrations[index++] = flint::TransformRegistration{ "CBMLandUnitDataTransform", []() -> flint::ITransform* { return new cbm::CBMLandUnitDataTransform(); } };
            outTransformRegistrations[index++] = flint::TransformRegistration{ "GrowthCurveTransform",     []() -> flint::ITransform* { return new cbm::GrowthCurveTransform(); } };
            return index;
        }

        MOJA_LIB_API int getFlintDataRegistrations(moja::flint::FlintDataRegistration* outFlintDataRegistrations) {
            auto index = 0;
            return index;
        }
    }

}}