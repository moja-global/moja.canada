#ifndef CBMLandUnitDataTransform_H_
#define CBMLandUnitDataTransform_H_

#include "moja/flint/itransform.h"
#include "moja/datarepository/datarepository.h"
#include "moja/datarepository/tileblockcellindexer.h"
#include "moja/flint/ilandunitcontroller.h"

#include <unordered_map>

namespace moja {
	namespace modules {
		namespace CBM {

			class CBMLandUnitDataTransform : public flint::ITransform {
			public:
				void configure(DynamicObject config, const flint::ILandUnitController& landUnitController, datarepository::DataRepository& dataRepository) override;
				void controllerChanged(const flint::ILandUnitController& controller) override;
				const Dynamic& value() const override;

			private:
				const flint::ILandUnitController* _landUnitController;
				datarepository::DataRepository* _dataRepository;
				datarepository::IProviderRelationalInterface::Ptr _provider;
				const flint::IVariable* _varToUse;
				std::string _varName;

				mutable Dynamic _results;
				mutable DynamicObject _resultsObject;
			};

		}
	}
}

#endif // CBMLandUnitDataTransform_H_