#ifdef _WIN32
#  define SAWTOOTH_EXPORT __declspec( dllexport )
#else
#  define SAWTOOTH_EXPORT
#endif

#include "modelmeta.h"
#include "results.h"
#include "sawtootherror.h"
#include "sawtoothmatrix.h"
#include <vector>
#ifndef sawtooth_exports_h
#define sawtooth_exports_h

	struct Sawtooth_Spatial_Variable {

		//annual mean temperature [deg C] by stand by timestep
		Sawtooth_Matrix tmean_ann;
		//growing season min annual temperature [deg C] by stand by timestep
		Sawtooth_Matrix tmin_ann;
		//growing season mean temperature [deg C] by stand by timestep
		Sawtooth_Matrix tmean_gs;
		//vapour pressure deficit [hPA] by stand by timestep
		Sawtooth_Matrix vpd;
		//growing season etr evapotranspiration [mm/d] by stand by timestep
		Sawtooth_Matrix etp_gs;
		//eeq by stand by timestep
		Sawtooth_Matrix eeq;
		//growing season soil water content [mm] by stand by timestep
		Sawtooth_Matrix ws_gs;
		//carbon dioxide concentration [ppm] by stand by timestep
		Sawtooth_Matrix ca;
		//nitrogen deposition [kg N ha^-1 yr^-1] by stand by timestep
		Sawtooth_Matrix ndep;
		//warm season z-score soil water content [mm] by stand by timestep
		Sawtooth_Matrix ws_gs_z;
		//warm season mean soil water content [mm] by stand (single column matrix)
		Sawtooth_Matrix ws_gs_n;
		//warm season z-score evapotranspiration [mm/d] by stand by timestep
		Sawtooth_Matrix etp_gs_z;
		//warm season mean evapotranspiration[todo units] by stand (single column matrix)
		Sawtooth_Matrix etp_gs_n;
		//disturbance codes by stand by timestep
		Sawtooth_Matrix_Int disturbances;
		// slope by stand (single column matrix)
		Sawtooth_Matrix slope;
		// TWI by stand (single column matrix)
		Sawtooth_Matrix twi;
		// aspect by stand (single column matrix)
		Sawtooth_Matrix aspect;
	};

	struct Sawtooth_CBM_Variable {

		// matrix containing the id per stand for stump parameters by stand (single column matrix)
		Sawtooth_Matrix_Int StumpParameterId;
		//matrix containing the id per stand for root parameters by stand(single column matrix)
		Sawtooth_Matrix_Int RootParameterId;
		//matrix containing the id per stand for cbm turnover parameters by stand(single column matrix)
		Sawtooth_Matrix_Int TurnoverParameterId;
		//matrix containing the id per stand for cbm region by stand(single column matrix)
		Sawtooth_Matrix_Int RegionId;
	};
	//loads Sawtooth parameters sets up model meta, and random seed
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param dbPath path to a Sawtooth sqlite database
	// @param meta metadata struct describing which component models should be
	// used for this run of Sawtooth
	// @param randomSeed seed for all of the random processes used in Sawtooth
	// @return pointer to the handle created
	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Initialize(Sawtooth_Error* err,
		const char* dbPath, Sawtooth_ModelMeta meta, uint64_t randomSeed);

	//free all memory allocated by Sawtooth_Initialize
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param handle pointer to memory allocated by the Sawtooth_Initialize
	// function
	// @return Sawtooth_Error code
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Free(Sawtooth_Error* err,
		void* handle);

	// allocate stands. This is required for use of the Sawtooth_Step function
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param numStands the number of stands to allocate
	// @param maxDensity the maximum number of trees per stand
	// @param species species matrix, of dimension numStands by maxDensity
	// @param cbmVariables structure of ids corresponding to CBM variables
	// @return pointer to the allocated stands
	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Stand_Alloc(Sawtooth_Error* err,
		size_t numStands, size_t maxDensity, Sawtooth_Matrix_Int species,
		Sawtooth_CBM_Variable* cbmVariables);

	// free stands that were previously allocated by the Sawtooth_Stand_Alloc
	// function
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param stands pointer to the stands allocated by Sawtooth_Stand_Alloc
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Stand_Free(Sawtooth_Error* err,
		void* stands);

	// step sawtooth for the specified number of timesteps with previously 
	// allocated stands
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param handle handle to the Sawtooth configuration and database 
	// parameters as created by the Sawtooth_Initialize function
	// @param stands pointer to stands allocated by the Sawtooth_Stand_Alloc 
	// function
	// @param numSteps the number of steps to run
	// @param spatialVar collection of spatial variables
	// @param standLevelResult collection of stand level result matrices, each
	//  matrix is stand by timestep in dimension 
	//  - allocation by caller
	//  - pointer to single instance
	// @param treeLevelResults array of structs storing tree level information
	//  - 1 struct per stand
	//  - each struct contains matrices of dimension of timestep by maxDensity
	//  - allocated by caller
	//  - optional - unused if set to NULL
	// @param cbmExtendedResults structure containing biomass changes in terms
	//  of CBM-CFS3 pools. 
	//  - 1 struct per stand
	//  - each struct contains a pointer to the timeseries of results of length
	//    numsteps
	//  - allocated by caller
	//  - optional - unused if set to NULL
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Step(Sawtooth_Error* err,
		void* handle, void* stands, size_t numSteps,
		Sawtooth_Spatial_Variable spatialVar,
		Sawtooth_StandLevelResult* standLevelResult,
		Sawtooth_TreeLevelResult* treeLevelResults,
		Sawtooth_CBMResult* cbmExtendedResults);

	// run sawtooth with the specified number of stands and the specified number
	// of timesteps.
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param handle handle to the Sawtooth configuration and database 
	// @param numstands the number of stands to simulate
	// @param numSteps the number of steps to run
	// @param maxDensity the number trees per stand
	// @param species the initial species ids with dimension numstands by
	//  maxDensity
	// @param spatialVar the collection of sawtooth spatial variables
	// @param standLevelResult collection of stand level result matrices, each
	//  matrix is stand by timestep in dimension 
	//  - allocation by caller
	//  - pointer to single instance
	// @param treeLevelResults array of structs storing tree level information
	//  - 1 struct per stand
	//  - each struct contains matrices of dimension of timestep by maxDensity
	//  - allocated by caller
	// @param cbmExtendedResults structure containing biomass changes in terms
	//  of CBM-CFS3 pools. 
	//  - 1 struct per stand
	//  - each struct contains a pointer to the timeseries of results of length
	//    numsteps
	//  - allocated by caller
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Run(Sawtooth_Error* err,
		void* handle, size_t numStands, size_t numSteps, size_t maxDensity,
		Sawtooth_Matrix_Int species, Sawtooth_Spatial_Variable spatialVar,
		Sawtooth_CBM_Variable* cbm,
		Sawtooth_StandLevelResult* standLevelResult,
		Sawtooth_TreeLevelResult* treeLevelResults,
		Sawtooth_CBMResult* cbmExtendedResults);

#endif
