#include "exports.h"
#include "random.h"
#include "dblayer.h"
#include "sawtoothmodel.h"
#include "parameterset.h"

struct SawtoothHandle {
	Sawtooth::Parameter::ParameterSet* params;
	Sawtooth::SawtoothModel* model;
	Sawtooth_ModelMeta meta;
	Sawtooth::Rng::Random* random;
};

struct StandHandle {
	std::vector<Sawtooth::Stand*> stands;
};

extern "C" SAWTOOTH_EXPORT void* Sawtooth_Initialize(Sawtooth_Error* err,
	const char* dbPath, Sawtooth_ModelMeta meta, uint64_t randomSeed) {
	try {
		Sawtooth::DBConnection conn(dbPath);
		Sawtooth::Parameter::ParameterSet* params = 
			new Sawtooth::Parameter::ParameterSet(conn, meta);
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
	catch (const Sawtooth::SawtoothException& e) {
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
	catch (const Sawtooth::SawtoothException& e) {
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
	Sawtooth_Matrix_Int species, Sawtooth_CBM_Variable* cbmVariables) {
	try {
		StandHandle* h = new StandHandle();

		bool cbmExtended = cbmVariables != NULL;

		for (size_t i = 0; i < numStands; i++) {
			std::vector<int> speciescodes(maxDensity);
			for (size_t j = 0; j < maxDensity; j++) {
				speciescodes[j] = species.GetValue(i, j);
			}
			Sawtooth::Stand* stand;
			if (!cbmExtended) {
				stand = new Sawtooth::Stand(1.0, speciescodes,
					maxDensity);
				h->stands.push_back(stand);
			}
			else {
				stand = new Sawtooth::Stand(1.0, speciescodes,
					maxDensity,
					cbmVariables->StumpParameterId.GetValue(i, 0),
					cbmVariables->RootParameterId.GetValue(i, 0),
					cbmVariables->TurnoverParameterId.GetValue(i, 0),
					cbmVariables->RegionId.GetValue(i, 0));
				h->stands.push_back(stand);
			}
		}
		err->Code = Sawtooth_NoError;
		return h;
	}
	catch (const Sawtooth::SawtoothException& e) {
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
	catch (const Sawtooth::SawtoothException& e) {
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
	Sawtooth_Spatial_Variable spatialVar,
	Sawtooth_StandLevelResult* standLevelResult,
	Sawtooth_TreeLevelResult* treeLevelResults,
	Sawtooth_CBMResult* cbmExtendedResults) {
	try {
		SawtoothHandle* h = (SawtoothHandle*)handle;
		Sawtooth::SawtoothModel* model = h->model;
		Sawtooth_ModelMeta meta = h->meta;
		StandHandle* standHandle = (StandHandle*)stands;

		for (size_t s = 0; s < standHandle->stands.size(); s++) {
			model->InitializeStand(*standHandle->stands[s]);
				
			for (size_t t = 0; t < numSteps; t++) {
				Sawtooth::Parameter::SpatialVariable sp;
				sp.tmin_ann = spatialVar.tmin_ann.GetValue(s, t);
				sp.tmean_gs = spatialVar.tmean_gs.GetValue(s, t);
				sp.vpd = spatialVar.vpd.GetValue(s, t);
				sp.eeq = spatialVar.eeq.GetValue(s, t);
				sp.etp_gs = spatialVar.etp_gs.GetValue(s, t);
				sp.ws_gs = spatialVar.ws_gs.GetValue(s, t);
				sp.ca = spatialVar.ca.GetValue(s, t);
				sp.ndep = spatialVar.ndep.GetValue(s, t);

				if (meta.mortalityModel == Sawtooth_MortalityES2 ||
					meta.mortalityModel == Sawtooth_MortalityMLR35) {
					//more climate variables are required for these mortality models
					//otherwise they may safely empty matrices
					sp.etp_gs_n = spatialVar.etp_gs_n.GetValue(s, 0);
					sp.etp_gs_z = spatialVar.etp_gs_z.GetValue(s, t);
					sp.ws_gs_n = spatialVar.ws_gs_n.GetValue(s, 0);
					sp.ws_gs_z = spatialVar.ws_gs_z.GetValue(s, t);
				}

				if (meta.growthModel == Sawtooth_GrowthES3) {
					sp.tmean_ann = spatialVar.tmean_ann.GetValue(s, t);
					sp.aspect = spatialVar.aspect.GetValue(s,0);
					sp.slope = spatialVar.slope.GetValue(s, 0);
					sp.twi = spatialVar.twi.GetValue(s, 0);
				}

				int dist = spatialVar.disturbances.GetValue(s,t);

				model->Step(*standHandle->stands[s], t, s, sp, dist,
					*standLevelResult,
					cbmExtendedResults == NULL ? NULL : &cbmExtendedResults[s],
					treeLevelResults == NULL ? NULL : &treeLevelResults[s]);
			}
		}
	}
	catch (const Sawtooth::SawtoothException& e) {
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
	size_t maxDensity, Sawtooth_Matrix_Int species,
	Sawtooth_Spatial_Variable spatialVar,
	Sawtooth_CBM_Variable* cbm,
	Sawtooth_StandLevelResult* standLevelResult,
	Sawtooth_TreeLevelResult* treeLevelResults,
	Sawtooth_CBMResult* cbmExtendedResults)
{
	try {
		SawtoothHandle* h = (SawtoothHandle*)handle;
		Sawtooth::SawtoothModel* model = h->model;
		Sawtooth_ModelMeta meta = h->meta;
		void* stands = Sawtooth_Stand_Alloc(err, numStands,
			maxDensity, species, cbm);
		if (err->Code != Sawtooth_NoError) {
			return;
		}

		Sawtooth_Step(err, handle, stands, numSteps, spatialVar,
			standLevelResult, treeLevelResults, cbmExtendedResults);
		if (err->Code != Sawtooth_NoError) {
			return;
		}

		Sawtooth_Stand_Free(err, stands);
	}
	catch (const Sawtooth::SawtoothException& e) {
		e.SetErrorStruct(err);
		return;
	}
	catch (...) {
		err->Code = Sawtooth_UnknownError;
		return;
	}
	err->Code = Sawtooth_NoError;
}
