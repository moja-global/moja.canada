#ifndef CBM_StandBiomassCarbonCurve_H_
#define CBM_StandBiomassCarbonCurve_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "moja/modules/cbm/componentbiomasscarboncurve.h"

namespace moja {
	namespace modules {
		namespace CBM {
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