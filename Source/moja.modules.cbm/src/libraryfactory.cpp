#include <moja/flint/mojalibapi.h>
#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/logging.h>

#include "moja/modules/cbm/cbmageindicators.h"
#include "moja/modules/cbm/cbmaggregatorcsvwriter.h"
#include "moja/modules/cbm/cbmaggregatorlandunitdata.h"
#include "moja/modules/cbm/cbmaggregatorlibpqxxwriter.h"
#include "moja/modules/cbm/cbmaggregatorpostgresqlwriter.h"
#include "moja/modules/cbm/cbmaggregatorsqlitewriter.h"
#include "moja/modules/cbm/cbmbuildlandunitmodule.h"
#include "moja/modules/cbm/cbmdecaymodule.h"
#include "moja/modules/cbm/cbmdisturbanceeventmodule.h"
#include "moja/modules/cbm/cbmdisturbancelistener.h"
#include "moja/modules/cbm/cbmflataggregatorlandunitdata.h"
#include "moja/modules/cbm/cbmlandclasstransitionmodule.h"
#include "moja/modules/cbm/cbmlandunitdatatransform.h"
#include "moja/modules/cbm/cbmpartitioningmodule.h"
#include "moja/modules/cbm/cbmpeatlandspinupoutput.h"
#include "moja/modules/cbm/cbmsequencer.h"
#include "moja/modules/cbm/cbmspinupdisturbancemodule.h"
#include "moja/modules/cbm/cbmspinupsequencer.h"
#include "moja/modules/cbm/cbmtransitionrulesmodule.h"
#include "moja/modules/cbm/disturbancemonitormodule.h"
#include "moja/modules/cbm/dynamicgrowthcurvetransform.h"
#include "moja/modules/cbm/dynamicgrowthcurvelookuptransform.h"
#include "moja/modules/cbm/esgymmodule.h"
#include "moja/modules/cbm/esgymspinupsequencer.h"
#include "moja/modules/cbm/flatrecord.h"
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
#include "moja/modules/cbm/peatlandgrowthcurvetransform.h"
#include "moja/modules/cbm/peatlandgrowthmodule.h"
#include "moja/modules/cbm/peatlandspinupnext.h"
#include "moja/modules/cbm/peatlandspinupturnovermodule.h"
#include "moja/modules/cbm/peatlandturnovermodule.h"
#include "moja/modules/cbm/record.h"
#include "moja/modules/cbm/smalltreegrowthmodule.h"
#include "moja/modules/cbm/standmaturitymodule.h"
#include "moja/modules/cbm/standgrowthcurvefactory.h"
#include "moja/modules/cbm/timeseriesidxfromflintdatatransform.h"
#include "moja/modules/cbm/transitionruletransform.h"
#include "moja/modules/cbm/version.h"
#include "moja/modules/cbm/volumetobiomasscarbongrowth.h"
#include "moja/modules/cbm/yieldtablegrowthmodule.h"

#include <atomic>
#include <vector>
#include <map>
#include <Poco/Mutex.h>
#include <Poco/LRUCache.h>

namespace moja {
	namespace modules {

		struct CBMObjectHolder {
			CBMObjectHolder() : landUnitAggregatorId(1) {
				dateDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DateRow, cbm::DateRecord>>();
				poolInfoDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::PoolInfoRow, cbm::PoolInfoRecord>>();
				classifierSetDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ClassifierSetRow, cbm::ClassifierSetRecord>>();
				classifierNames = std::make_shared<std::vector<std::string>>();
				classifierNamesLock = std::make_shared<Poco::Mutex>();
				landClassDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::LandClassRow, cbm::LandClassRecord>>();
				locationDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::TemporalLocationRow, cbm::TemporalLocationRecord>>();
				poolDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::PoolRow, cbm::PoolRecord>>();
				fluxDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::FluxRow, cbm::FluxRecord>>();
				ageClassDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::AgeClassRow, cbm::AgeClassRecord>>();
				ageAreaDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::AgeAreaRow, cbm::AgeAreaRecord>>();
				moduleInfoDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ModuleInfoRow, cbm::ModuleInfoRecord>>();
				disturbanceTypeDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceTypeRow, cbm::DisturbanceTypeRecord>>();
				disturbanceDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::DisturbanceRow, cbm::DisturbanceRecord>>();
				errorDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::ErrorRow, cbm::ErrorRecord>>();
				locationErrorDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<cbm::LocationErrorRow, cbm::LocationErrorRecord>>();
				gcFactory = std::make_shared<cbm::StandGrowthCurveFactory>();
				volToBioCarbonGrowth = std::make_shared<cbm::VolumeToBiomassCarbonGrowth>();
				dynamicGcIdLock = std::make_shared<Poco::Mutex>();
				dynamicGcIdCache = std::make_shared<std::map<std::tuple<std::string, double, double>, DynamicVar>>();
				dynamicGcCache = std::make_shared<std::map<int, std::map<std::string, DynamicVar>>>();
				nextDynamicGcId = std::make_shared<std::atomic<int>>(1);
				flatFluxDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatFluxRecord>>();
				flatPoolDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatPoolRecord>>();
				flatErrorDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatErrorRecord>>();
				flatAgeDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatAgeAreaRecord>>();
				flatDisturbanceDimension = std::make_shared<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatDisturbanceRecord>>();
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
			std::shared_ptr<cbm::VolumeToBiomassCarbonGrowth> volToBioCarbonGrowth;
			std::atomic<int> landUnitAggregatorId;
			std::shared_ptr<std::atomic<int>> nextDynamicGcId;
			std::shared_ptr<Poco::Mutex> dynamicGcIdLock;
			std::shared_ptr<std::map<std::tuple<std::string, double, double>, DynamicVar>> dynamicGcIdCache;
			std::shared_ptr<std::map<int, std::map<std::string, DynamicVar>>> dynamicGcCache;
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatFluxRecord>> flatFluxDimension;
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatPoolRecord>> flatPoolDimension;
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatErrorRecord>> flatErrorDimension;
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatAgeAreaRecord>> flatAgeDimension;
			std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, cbm::FlatDisturbanceRecord>> flatDisturbanceDimension;
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

			MOJA_LIB_API flint::IModule* CreateCBMFlatAggregatorLandUnitData() {
				return new cbm::CBMFlatAggregatorLandUnitData(
					cbmObjectHolder.flatFluxDimension,
					cbmObjectHolder.flatPoolDimension,
					cbmObjectHolder.flatErrorDimension,
					cbmObjectHolder.flatAgeDimension,
					cbmObjectHolder.flatDisturbanceDimension,
					cbmObjectHolder.classifierNames,
					cbmObjectHolder.classifierNamesLock);
			}

			MOJA_LIB_API flint::IModule* CreateCBMAggregatorCsvWriter() {
				bool isPrimaryAggregator = cbmObjectHolder.landUnitAggregatorId++ == 1;
				return new cbm::CBMAggregatorCsvWriter(
					cbmObjectHolder.flatFluxDimension,
					cbmObjectHolder.flatPoolDimension,
					cbmObjectHolder.flatErrorDimension,
					cbmObjectHolder.flatAgeDimension,
					cbmObjectHolder.flatDisturbanceDimension,
					cbmObjectHolder.classifierNames,
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

			MOJA_LIB_API flint::IModule* CreateCBMAggregatorLibPQXXWriter() {
				bool isPrimaryAggregator = cbmObjectHolder.landUnitAggregatorId++ == 1;
				return new cbm::CBMAggregatorLibPQXXWriter(
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
				MOJA_LOG_INFO << "GCBM version: " << CBM_VERSION;

				int index = 0;
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMFlatAggregatorLandUnitData",  &CreateCBMFlatAggregatorLandUnitData };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorLandUnitData",      &CreateCBMAggregatorLandUnitData };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorLibPQXXWriter",     &CreateCBMAggregatorLibPQXXWriter };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorPostgreSQLWriter",  &CreateCBMAggregatorPostgreSQLWriter };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorCsvWriter",         &CreateCBMAggregatorCsvWriter };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAggregatorSQLiteWriter",      &CreateCBMAggregatorSQLiteWriter };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDecayModule",                 []() -> flint::IModule* { return new cbm::CBMDecayModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDisturbanceEventModule",	   []() -> flint::IModule* { return new cbm::CBMDisturbanceEventModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMDisturbanceListener",	       []() -> flint::IModule* { return new cbm::CBMDisturbanceListener(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMGrowthModule",                []() -> flint::IModule* { return new cbm::YieldTableGrowthModule(cbmObjectHolder.gcFactory, cbmObjectHolder.volToBioCarbonGrowth); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSequencer",				   []() -> flint::IModule* { return new cbm::CBMSequencer(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "DisturbanceMonitor",             []() -> flint::IModule* { return new cbm::DisturbanceMonitorModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamPostNotify",	   []() -> flint::IModule* { return new cbm::OutputerStreamPostNotify(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "OutputerStreamFluxPostNotify",   []() -> flint::IModule* { return new cbm::OutputerStreamFluxPostNotify(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupSequencer",			   []() -> flint::IModule* { return new cbm::CBMSpinupSequencer(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMBuildLandUnitModule",		   []() -> flint::IModule* { return new cbm::CBMBuildLandUnitModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMSpinupDisturbanceModule",     []() -> flint::IModule* { return new cbm::CBMSpinupDisturbanceModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMLandClassTransitionModule",   []() -> flint::IModule* { return new cbm::CBMLandClassTransitionModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossTurnoverModule",		   []() -> flint::IModule* { return new cbm::MossTurnoverModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossDecayModule",			   []() -> flint::IModule* { return new cbm::MossDecayModule(cbmObjectHolder.gcFactory); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMMossGrowthModule",			   []() -> flint::IModule* { return new cbm::MossGrowthModule(cbmObjectHolder.gcFactory); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMPartitioningModule",          []() -> flint::IModule* { return new cbm::CBMPartitioningModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "GrowthMultiplierModule",		   []() -> flint::IModule* { return new cbm::GrowthMultiplierModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandDisturbanceModule",      []() -> flint::IModule* { return new cbm::PeatlandDisturbanceModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "MossDisturbanceModule",		   []() -> flint::IModule* { return new cbm::MossDisturbanceModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandSpinupTurnOverModule",   []() -> flint::IModule* { return new cbm::PeatlandSpinupTurnOverModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandGrowthModule",		   []() -> flint::IModule* { return new cbm::PeatlandGrowthModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandTurnoverModule",		   []() -> flint::IModule* { return new cbm::PeatlandTurnoverModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandDecayModule",			   []() -> flint::IModule* { return new cbm::PeatlandDecayModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "StandMaturityModule",            []() -> flint::IModule* { return new cbm::StandMaturityModule(cbmObjectHolder.gcFactory, cbmObjectHolder.volToBioCarbonGrowth); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMTransitionRulesModule",       []() -> flint::IModule* { return new cbm::CBMTransitionRulesModule(cbmObjectHolder.gcFactory, cbmObjectHolder.volToBioCarbonGrowth); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "ESGYMModule",					   []() -> flint::IModule* { return new cbm::ESGYMModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "ESGYMSpinupSequencer",		   []() -> flint::IModule* { return new cbm::ESGYMSpinupSequencer(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMAgeIndicators",		       []() -> flint::IModule* { return new cbm::CBMAgeIndicators(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "SmallTreeGrowthModule",		   []() -> flint::IModule* { return new cbm::SmallTreeGrowthModule(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "PeatlandSpinupNext",			   []() -> flint::IModule* { return new cbm::PeatlandSpinupNext(); } };
				outModuleRegistrations[index++] = flint::ModuleRegistration{ "CBMPeatlandSpinupOutput",		   []() -> flint::IModule* { return new cbm::CBMPeatlandSpinupOutput(); } };
				return index;
			}

			MOJA_LIB_API int getTransformRegistrations(flint::TransformRegistration* outTransformRegistrations) {
				int index = 0;
				outTransformRegistrations[index++] = flint::TransformRegistration{ "CBMLandUnitDataTransform",             []() -> flint::ITransform* { return new cbm::CBMLandUnitDataTransform(); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "DynamicGrowthCurveTransform",          []() -> flint::ITransform* { return new cbm::DynamicGrowthCurveTransform(cbmObjectHolder.dynamicGcIdCache, cbmObjectHolder.dynamicGcCache, cbmObjectHolder.dynamicGcIdLock, cbmObjectHolder.nextDynamicGcId); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "DynamicGrowthCurveLookupTransform",    []() -> flint::ITransform* { return new cbm::DynamicGrowthCurveLookupTransform(cbmObjectHolder.dynamicGcCache); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "GrowthCurveTransform",                 []() -> flint::ITransform* { return new cbm::GrowthCurveTransform(); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "PeatlandGrowthCurveTransform",         []() -> flint::ITransform* { return new cbm::PeatlandGrowthCurveTransform(); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "TransitionRuleTransform",              []() -> flint::ITransform* { return new cbm::TransitionRuleTransform(); } };
				outTransformRegistrations[index++] = flint::TransformRegistration{ "TimeSeriesIdxFromFlintDataTransform",  []() -> flint::ITransform* { return new cbm::TimeSeriesIdxFromFlintDataTransform(); } };
				return index;
			}

			MOJA_LIB_API int getFlintDataRegistrations(moja::flint::FlintDataRegistration* outFlintDataRegistrations) {
				auto index = 0;
            return index;
        }
    }

}}
