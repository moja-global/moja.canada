#ifdef _WIN32
#  define SAWTOOTH_EXPORT __declspec( dllexport )
#else
#  define SAWTOOTH_EXPORT
#endif

#include "modelmeta.h"
#include "results.h"
#include "sawtootherror.h"
#include <vector>
#ifndef exports_h
#define exports_h
namespace Sawtooth {

	//loads Sawtooth parameters sets up model meta, and random seed
	// @param err structure containing error information (if any) that occurs
	// during function call
	// @param dbPath path to a Sawtooth sqlite database
	// @param meta metadata struct describing which component models should be
	// used for this run of Sawtooth
	// @param randomSeed seed for all of the random processes used in Sawtooth
	// @return pointer to the handle created
	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Initialize(Sawtooth_Error* err,
		char* dbPath, Meta::ModelMeta meta, uint64_t randomSeed);

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
	// @param species species array, of dimension numStands by maxDensity
	// @return pointer to the allocated stands
	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Stand_Alloc(Sawtooth_Error* err,
		size_t numStands, size_t maxDensity, int** species);

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
	// @param tmin min annual temperature [deg C] by stand by timestep
	// @param tmean mean annual temperature [deg C] by stand by timestep
	// @param vpd vapour pressure deficit [hPA] by stand by timestep
	// @param etr evapotranspiration [mm/d] by stand by timestep
	// @param eeq by stand by timestep
	// @param ws soil water content [mm] by stand by timestep
	// @param ca carbon dioxide concentration [ppm] by stand by timestep
	// @param ndep nitrogen deposition [kg N ha^-1 yr^-1] by stand by timestep
	// @param ws_mjjas_z warm season z-score soil water content [mm] by stand
	//  by timestep
	// @param ws_mjjas_n warm season mean soil water content [mm] by stand
	// @param etr_mjjas_z warm season z-score evapotranspiration [todo units]
	//  by stand by timestep
	// @param etr_mjjas_n warm season mean evapotranspiration [todo units] by
	//  stand
	// @param disturbances disturbance codes by stand by timestep
	// @param standLevelResult collection of stand level result matrices, each
	//  matrix is stand by timestep in dimension 
	//  - allocation by caller
	//  - pointer to single instance
	// @param treeLevelResults array of structs storing tree level information
	//  - 1 struct per stand
	//  - each struct contains matrices of dimension of timestep by maxDensity
	//  - allocated by caller
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Step(Sawtooth_Error* err,
		void* handle, void* stands, size_t numSteps, double** tmin, 
		double** tmean, double** vpd, double** etr, double** eeq, double** ws,
		double** ca, double** ndep, double** ws_mjjas_z, double* ws_mjjas_n,
		double** etr_mjjas_z, double* etr_mjjas_n, int** disturbances,
		StandLevelResult* standLevelResult, TreeLevelResult* treeLevelResults);

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
	// @param tmin min annual temperature [deg C] by stand by timestep
	// @param tmean mean annual temperature [deg C] by stand by timestep
	// @param vpd vapour pressure deficit [hPA] by stand by timestep
	// @param etr evapotranspiration [mm/d] by stand by timestep
	// @param eeq by stand by timestep
	// @param ws soil water content [mm] by stand by timestep
	// @param ca carbon dioxide concentration [ppm] by stand by timestep
	// @param ndep nitrogen deposition [kg N ha^-1 yr^-1] by stand by timestep
	// @param ws_mjjas_z warm season z-score soil water content [mm] by stand
	//  by timestep
	// @param ws_mjjas_n warm season mean soil water content [mm] by stand
	// @param etr_mjjas_z warm season z-score evapotranspiration [todo units]
	//  by stand by timestep
	// @param etr_mjjas_n warm season mean evapotranspiration [todo units] by
	//  stand
	// @param disturbances disturbance codes by stand by timestep
	// @param standLevelResult collection of stand level result matrices, each
	//  matrix is stand by timestep in dimension 
	//  - allocation by caller
	//  - pointer to single instance
	// @param treeLevelResults array of structs storing tree level information
	//  - 1 struct per stand
	//  - each struct contains matrices of dimension of timestep by maxDensity
	//  - allocated by caller
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Run(Sawtooth_Error* err,
		void* handle, size_t numStands, size_t numSteps, size_t maxDensity,
		int** species, double** tmin, double** tmean, double** vpd,
		double** etr, double** eeq, double** ws, double** ca, double** ndep,
		double** ws_mjjas_z, double* ws_mjjas_n, double** etr_mjjas_z,
		double* etr_mjjas_n, int** disturbances,
		StandLevelResult* standLevelResult, TreeLevelResult* treeLevelResults);
}
#endif
