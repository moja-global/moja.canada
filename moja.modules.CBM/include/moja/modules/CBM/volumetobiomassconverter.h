#ifndef CBM_IConverter_H_
#define CBM_IConverter_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/perdfactor.h"
#include "moja/modules/cbm/componentbiomasscarboncurve.h"
#include "moja/modules/cbm/treespecies.h"
#include "moja/modules/cbm/standgrowthcurve.h"
#include "moja/modules/cbm/smoother.h"

namespace moja {
	namespace modules {
		namespace CBM {			

			/*
			* ADT - to conver the yield growth curve to biomass carbon curve
			* 
			*/
			class CBM_API VolumeToBiomassConverter {

			public:
				VolumeToBiomassConverter();
				virtual ~VolumeToBiomassConverter() { }

				/*
				* Generate stand component biomass carbon curve. The component is either of softwood or hardwood.
				* This version assums that the stand growth curve have all of the required data including:
				* Leading species for each component, PERD factor for each leading species.
				*/
				std::shared_ptr<ComponentBiomassCarbonCurve> generateComponentBiomassCarbonCurve(std::shared_ptr<StandGrowthCurve> standGrowthCurve, SpeciesType speciesType);


				/*
				* Generate stand component biomass carbon curve. The component is either of softwood or hardwood.
				* This version assums that the stand growth curve have some of the required data including:
				* Leading species for each component, but there is no PERD factor for each leading species.
				* Converter will query for the PERD factors based on the leading species ID and CBM defalut SPU ID.
				* There will be a small SQLite database to provide the data
				*/
				std::shared_ptr<ComponentBiomassCarbonCurve> generateComponentBiomassCarbonCurve(std::shared_ptr<StandGrowthCurve> standGrowthCurve, SpeciesType speciesType, int defaultSPUID);

				/*
				* Apply smoother on a carbon curve based on the stand growth yield
				*/
				void DoSmoothing(const StandGrowthCurve& standGrowthCurve, ComponentBiomassCarbonCurve* carbonCurve, SpeciesType sepciesType);

				void setSmootherEnabled(bool value) { _smootherEnabled = value; }
			private:
				bool _smootherEnabled;
				std::unique_ptr<Smoother> _smoother;
			};
		}
	}
}
#endif