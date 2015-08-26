#ifndef MOJA_MODULES_CBM_STANDBIOMASSCARBONCURVE_H_
#define MOJA_MODULES_CBM_STANDBIOMASSCARBONCURVE_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/modules/cbm/componentbiomasscarboncurve.h"

namespace moja {
	namespace modules {
		namespace cbm {
			/*
			* ADT - Stand biomass carbon curve by annual age
			*/
			class CBM_API StandBiomassCarbonCurve{
			public:
				StandBiomassCarbonCurve() {};
				virtual ~StandBiomassCarbonCurve() {};

				std::shared_ptr<ComponentBiomassCarbonCurve> softwoodCarbonCurve() const;
				std::shared_ptr<ComponentBiomassCarbonCurve> hardwoodCarbonCurve() const;

				void setSoftwoodComponent(std::shared_ptr<ComponentBiomassCarbonCurve> carbonCurve);
				void setHardwoodComponent(std::shared_ptr<ComponentBiomassCarbonCurve> carbonCurve);
			private:
				/*
				* Softwood component biomass carbon curve
				*/
				std::shared_ptr<ComponentBiomassCarbonCurve> _softwoodComponent;

				/*
				* Hardwood component biomass carbon curve
				*/
				std::shared_ptr<ComponentBiomassCarbonCurve> _hardwoodComponent;				
			};
		}
	}
}
#endif