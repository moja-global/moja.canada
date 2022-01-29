#include "moja/modules/cbm/cbmaggregatorcsvwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ivariable.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/hash.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/TeeStream.h>

namespace moja {
namespace modules {
namespace cbm {

    void CBMAggregatorCsvWriter::configure(const DynamicObject& config) {
        _outputPath = config["output_path"].convert<std::string>();
    }

    void CBMAggregatorCsvWriter::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::SystemInit,      &CBMAggregatorCsvWriter::onSystemInit,      *this);
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorCsvWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorCsvWriter::onSystemShutdown,  *this);
	}

	void CBMAggregatorCsvWriter::doSystemInit() {
        if (!_isPrimaryAggregator) {
            return;
        }

        Poco::File outputDir(_outputPath);
        try {
            outputDir.createDirectories();
        } catch (Poco::FileExistsException&) { }
    }

    void CBMAggregatorCsvWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

    void CBMAggregatorCsvWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        load((boost::format("%1%/flux_%2%.csv")        % _outputPath % _jobId).str(), _classifierNames, _fluxDimension);
        load((boost::format("%1%/pool_%2%.csv")        % _outputPath % _jobId).str(), _classifierNames, _poolDimension);
        load((boost::format("%1%/error_%2%.csv")       % _outputPath % _jobId).str(), _classifierNames, _errorDimension);
        load((boost::format("%1%/age_%2%.csv")         % _outputPath % _jobId).str(), _classifierNames, _ageDimension);
        load((boost::format("%1%/disturbance_%2%.csv") % _outputPath % _jobId).str(), _classifierNames, _disturbanceDimension);

        MOJA_LOG_INFO << "Finished loading results." << std::endl;
    }

    template<typename TAccumulator>
    void CBMAggregatorCsvWriter::load(
        const std::string& outputPath,
        std::shared_ptr<std::vector<std::string>> classifierNames,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % outputPath).str();
        auto tempOutputPath = (boost::format("%1%_%2%") % outputPath % rand()).str();
        auto records = dataDimension->records();
        if (!records.empty()) {
            Poco::File outputFile(tempOutputPath);
            outputFile.createFile();
            Poco::FileOutputStream streamFile(tempOutputPath);
            Poco::TeeOutputStream output(streamFile);

            bool headerWritten = false;
            for (auto& record : records) {
                if (!headerWritten) {
                    output << record.header(*classifierNames);
                    headerWritten = true;
                }

                output << record.asPersistable();
            }

            streamFile.close();

            try {
                outputFile.renameTo(outputPath);
            } catch (...) {}
        }
    }

}}} // namespace moja::modules::cbm
