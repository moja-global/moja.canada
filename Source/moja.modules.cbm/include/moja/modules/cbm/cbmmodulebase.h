#ifndef MOJA_MODULES_CBM_CBMMODULEBASE_H_
#define MOJA_MODULES_CBM_CBMMODULEBASE_H_

#include "moja/flint/modulebase.h"
#include "moja/flint/flintexceptions.h"
#include "moja/exception.h"

#include <boost/exception_ptr.hpp>

namespace moja {
namespace modules {
namespace cbm {

class CBMModuleBase : public flint::ModuleBase {
public:
	virtual ~CBMModuleBase() = default;

	void onSystemInit() override							 { doWithHandling([this]() { this->doSystemInit(); }); }
	void onSystemShutdown() override						 { doWithHandling([this]() { this->doSystemShutdown(); }); }
	void onLocalDomainInit() override						 { doWithHandling([this]() { this->doLocalDomainInit(); }); }
	void onLocalDomainShutdown() override					 { doWithHandling([this]() { this->doLocalDomainShutdown(); }); }
	void onLocalDomainProcessingUnitInit() override			 { doWithHandling([this]() { this->doLocalDomainProcessingUnitInit(); });	}
	void onLocalDomainProcessingUnitShutdown() override		 { doWithHandling([this]() { this->doLocalDomainProcessingUnitShutdown(); }); }
	void onPreTimingSequence() override						 { doWithHandling([this]() { this->doPreTimingSequence(); }); }
	void onTimingInit() override							 { doWithHandling([this]() { this->doTimingInit(); }); }
	void onTimingPrePostInit() override						 { doWithHandling([this]() { this->doTimingPrePostInit(); }); }
	void onTimingPostInit() override						 { doWithHandling([this]() { this->doTimingPostInit(); }); }
	void onTimingPostInit2() override						 { doWithHandling([this]() { this->doTimingPostInit2(); }); }
	void onTimingShutdown() override						 { doWithHandling([this]() { this->doTimingShutdown(); }); }
	void onTimingStep() override							 { doWithHandling([this]() { this->doTimingStep(); }); }
	void onTimingPreEndStep() override						 { doWithHandling([this]() { this->doTimingPreEndStep(); }); }
	void onTimingEndStep() override							 { doWithHandling([this]() { this->doTimingEndStep(); }); }
	void onTimingPostStep() override						 { doWithHandling([this]() { this->doTimingPostStep(); }); }
	void onOutputStep() override							 { doWithHandling([this]() { this->doOutputStep(); }); }
	void onPrePostDisturbanceEvent() override				 { doWithHandling([this]() { this->doPrePostDisturbanceEvent(); }); }
	void onPostDisturbanceEvent() override					 { doWithHandling([this]() { this->doPostDisturbanceEvent(); }); }
	void onPostDisturbanceEvent2() override					 { doWithHandling([this]() { this->doPostDisturbanceEvent2(); }); }
	void onError(std::string msg) override					 { doWithHandling([this, msg]() { this->doError(msg); }); }
	void onDisturbanceEvent(DynamicVar e) override			 { doWithHandling([this, e]()   { this->doDisturbanceEvent(e); }); }
	void onPostNotification(short preMessageSignal) override { doWithHandling([this, preMessageSignal]() { this->doPostNotification(preMessageSignal); }); }

	virtual void doSystemInit() {}
	virtual void doSystemShutdown() {}
	virtual void doLocalDomainInit() {}
	virtual void doLocalDomainShutdown() {}
	virtual void doLocalDomainProcessingUnitInit() {}
	virtual void doLocalDomainProcessingUnitShutdown() {}
	virtual void doPreTimingSequence() {}
	virtual void doTimingInit() {}
	virtual void doTimingPrePostInit() {}
	virtual void doTimingPostInit() {}
	virtual void doTimingPostInit2() {}
	virtual void doTimingShutdown() {}
	virtual void doTimingStep() {}
	virtual void doTimingPreEndStep() {}
	virtual void doTimingEndStep() {}
	virtual void doTimingPostStep() {}
	virtual void doOutputStep() {}
	virtual void doError(std::string msg) {}
	virtual void doDisturbanceEvent(DynamicVar) {}
	virtual void doPrePostDisturbanceEvent() {}
	virtual void doPostDisturbanceEvent() {}
	virtual void doPostDisturbanceEvent2() {}
	virtual void doPostNotification(short preMessageSignal) {}
    
private:
    void doWithHandling(const std::function<void()>& fn) {
        try {
            fn();
        }
        catch (flint::SimulationError&) { throw; }
        catch (moja::Exception& e) { raiseModuleError(e); }
        catch (boost::exception& e) { raiseModuleError(e); }
        catch (std::exception& e) { raiseModuleError(e); }
    }

    void raiseModuleError(moja::Exception& e) {
        BOOST_THROW_EXCEPTION(flint::SimulationError()
            << flint::Details(e.displayText())
            << flint::LibraryName("moja.modules.cbm")
            << flint::ModuleName(metaData().moduleName)
            << flint::ErrorCode(0));
    }

    void raiseModuleError(boost::exception& e) {
        BOOST_THROW_EXCEPTION(flint::SimulationError()
            << flint::Details(boost::diagnostic_information(e))
            << flint::LibraryName("moja.modules.cbm")
            << flint::ModuleName(metaData().moduleName)
            << flint::ErrorCode(0));
    }

    void raiseModuleError(std::exception& e) {
        BOOST_THROW_EXCEPTION(flint::SimulationError()
            << flint::Details(e.what())
            << flint::LibraryName("moja.modules.cbm")
            << flint::ModuleName(metaData().moduleName)
            << flint::ErrorCode(0));
    }
};

}}} // namespace moja::Modules::cbm
#endif // MOJA_MODULES_CBM_CBMMODULEBASE_H_
