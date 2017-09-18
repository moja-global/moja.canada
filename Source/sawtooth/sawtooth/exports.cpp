#include "exports.h"
#include "random.h"
#include "dblayer.h"
#include "sawtoothmodel.h"
#include "parameterset.h"
#include <iostream>
namespace Sawtooth {

	struct SawtoothHandle {
		Parameter::ParameterSet* params;
		SawtoothModel* model;
		Meta::ModelMeta meta;
		Rng::Random* random;
	};

	struct StandHandle {
		std::vector<Stand*> stands;
	};

	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Initialize(Sawtooth_Error* err,
		char* dbPath, Meta::ModelMeta meta, uint64_t randomSeed) {
		try {
			DBConnection conn(dbPath);
			Parameter::ParameterSet* params = new Parameter::ParameterSet(conn,
				meta);
			Sawtooth::Rng::Random* random = new Sawtooth::Rng::Random(
				randomSeed);
			Sawtooth::SawtoothModel* model = new Sawtooth::SawtoothModel(meta,
				*params, *random);

			SawtoothHandle* handle = new SawtoothHandle();
			handle->params = params;
			handle->model = model;
			handle->random = random;
			handle->meta = meta;
			err->Code = Sawtooth_NoError;
			return handle;
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return 0;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return 0;
		}

	}

	extern "C" SAWTOOTH_EXPORT void Sawtooth_Free(Sawtooth_Error* err,
		void* handle) {
		try {
			SawtoothHandle* h = (SawtoothHandle*)handle;
			delete h->random;
			delete h->params;
			delete h->model;
			delete h;
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return;
		}
		err->Code = Sawtooth_NoError;
	}
	
	extern "C" SAWTOOTH_EXPORT void* Sawtooth_Stand_Alloc(
		Sawtooth_Error* err, size_t numStands, size_t maxDensity,
		int** species) {
		try {
			StandHandle* h = new StandHandle();

			for (size_t i = 0; i < numStands; i++) {
				std::vector<int> speciescodes(maxDensity);
				for (size_t j = 0; j < maxDensity; j++) {
					speciescodes[j] = species[i][j];
				}
				Stand* stand = new Stand(1.0, speciescodes, maxDensity);
				h->stands.push_back(stand);
			}
			err->Code = Sawtooth_NoError;
			return h;
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return 0;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return 0;
		}
	}

	extern "C" SAWTOOTH_EXPORT void Sawtooth_Stand_Free(
		Sawtooth_Error* err, void* stands) {
		try {
			StandHandle* h = (StandHandle*)stands;
			for (auto s : h->stands) {
				delete s;
			}
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return;
		}
		err->Code = Sawtooth_NoError;
	}

	extern "C" SAWTOOTH_EXPORT void Sawtooth_Step(
		Sawtooth_Error* err, void* handle, void* stands, size_t numSteps,
		double** tmin, double** tmean, double** vpd, double** etr,
		double** eeq, double** ws, double** ca, double** ndep,
		double** ws_mjjas_z, double* ws_mjjas_n, double** etr_mjjas_z,
		double* etr_mjjas_n, int** disturbances, 
		StandLevelResult* standLevelResult, TreeLevelResult* treeLevelResults) {
		try {
			SawtoothHandle* h = (SawtoothHandle*)handle;
			SawtoothModel* model = h->model;
			Meta::ModelMeta meta = h->meta;
			StandHandle* standHandle = (StandHandle*)stands;

			for (size_t s = 0; s < standHandle->stands.size(); s++) {
				Stand st = *standHandle->stands[s];
				model->InitializeStand(st);
				
				for (size_t t = 0; t < numSteps; t++) {
					Parameter::ClimateVariable cp;
					cp.tmin = tmin[s][t];
					cp.tmean = tmean[s][t];
					cp.vpd = vpd[s][t];
					cp.eeq = eeq[s][t];
					cp.etr = etr[s][t];
					cp.ws = ws[s][t];
					cp.ca = ca[s][t];
					cp.ndep = ndep[s][t];

					if (meta.mortalityModel == Meta::MortalityES2 ||
						meta.mortalityModel == Meta::MortalityMLR35) {
						//more climate variables are required for these mortality models
						//otherwise they may safely be NULL pointers
						cp.etr_mjjas_n = etr_mjjas_n[s];
						cp.etr_mjjas_z = etr_mjjas_z[s][t];
						cp.ws_mjjas_n = ws_mjjas_n[s];
						cp.ws_mjjas_z = ws_mjjas_z[s][t];
					}

					int dist = disturbances[s][t];

					if (treeLevelResults == 0) {
						model->Step(st, t, s, cp, dist,
							*standLevelResult);
					}
					else {
						model->Step(st, t, s, cp, dist,
							*standLevelResult, treeLevelResults[s]);
					}
				}
			}
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return;
		}
		err->Code = Sawtooth_NoError;
	}

	//allocate and run the specified number of stands for the specified number of
	//timesteps.
	extern "C" SAWTOOTH_EXPORT void Sawtooth_Run(
		Sawtooth_Error* err, void* handle, size_t numStands, size_t numSteps,
		size_t maxDensity, int** species, double** tmin, double** tmean, 
		double** vpd, double** etr, double** eeq, double** ws, double** ca,
		double** ndep, double** ws_mjjas_z, double* ws_mjjas_n, double** etr_mjjas_z,
		double* etr_mjjas_n, int** disturbances,
		StandLevelResult* standLevelResult, TreeLevelResult* treeLevelResults)
	{
		try {
			SawtoothHandle* h = (SawtoothHandle*)handle;
			SawtoothModel* model = h->model;
			Meta::ModelMeta meta = h->meta;
			void* stands = Sawtooth_Stand_Alloc(err, numStands,
				maxDensity, species);
			if (err->Code != Sawtooth_NoError) {
				return;
			}

			Sawtooth_Step(err, handle, stands, numSteps, tmin, tmean, vpd,
				etr, eeq, ws, ca, ndep, ws_mjjas_z, ws_mjjas_n, etr_mjjas_z,
				etr_mjjas_n, disturbances, standLevelResult,
				treeLevelResults);
			if (err->Code != Sawtooth_NoError) {
				return;
			}

			Sawtooth_Stand_Free(err, stands);
		}
		catch (const SawtoothException& e) {
			e.SetErrorStruct(err);
			return;
		}
		catch (...) {
			err->Code = Sawtooth_UnknownError;
			return;
		}
		err->Code = Sawtooth_NoError;
	}
}