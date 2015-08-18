#ifndef CBM_Smoother_H_
#define CBM_Smoother_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"
#include "standgrowthcurve.h"
#include "componentbiomasscarboncurve.h"

namespace moja {
	namespace modules {
		namespace CBM {
			class CBM_API Smoother {
			public:
				Smoother(){}
				virtual ~Smoother() {};

				const int extendedRegionSize = 15;
				const int smoothSampleSize = 50;
				const int numerOfParameters = 2;

				void smooth(const StandGrowthCurve& standGrowthCurve, ComponentBiomassCarbonCurve* carbonCurve, SpeciesType speciesType);
			
				int getComponentSmoothingSubstitutionRegionPoint(const StandGrowthCurve& standGrowthCurve, SpeciesType speciesType);

				void prepareSmoothingInputData(const ComponentBiomassCarbonCurve& carbonCurve, int substitutionPoint, int standMaxAge);

				void clearAndReserveDataSpace(int workingFitingRange);

				void minimize(double yValues[], double startingVals[]);

				//to be passed as a function pointer
				static double weibull_2Parameter(double t, double *p);

				int getFinalFittingRegionAndReplaceData(ComponentBiomassCarbonCurve& carbonCurve, int substitutionPoint, double merchCWeibullParameters[], double foliageCWeibullParameters[], double totalAGBioCWeibullParameters[]);

				std::vector<double>& smoothingMerchC()      { return _smoothingMerchC; };
				std::vector<double>& smoothingFoliageC()    { return _smoothingFoliageC; };
				std::vector<double>& smoothingOtherC()      { return _smoothingOtherC; };
				std::vector<double>& smoothingTotalAGBioC() { return _smoothingTotalAGBioC; };
				std::vector<double>& smoothingAageSerials() { return _smoothingAageSerials; };

			private:
				//smoother uses following data vectors as working space to generate the data to add/replace the original data
				std::vector<double> _smoothingMerchC;
				std::vector<double> _smoothingFoliageC;
				std::vector<double> _smoothingOtherC;
				std::vector<double> _smoothingTotalAGBioC;
				std::vector<double> _smoothingAageSerials;

				double _smoothingMaxMerchC;
				double _smoothingMaxFoliageC;
				double _smoothingMaxOtherC;
				double _smoothingMaxTotalAGC;
				double _smoothingMaxAge;
			};
		}
	}
}
#endif