/**
 * @file 
 * @brief The brief description goes here.
 * 
 * The detailed description if any, goes here 
 * ******/

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

     /**
     * @brief configuration function.
     * 
     * Detailed description here
     * 
     * @param config DynamicObject&
     * @return void
     * ************************/
    CBMFlatFile::CBMFlatFile(const std::string& path, const std::string& header) : _path(path) {
        _tempPath = (boost::format("%1%_%2%") % path % rand()).str();
        _outputFile = std::make_unique<Poco::File>(_tempPath);
        _outputFile->createFile();
        _streamFile = std::make_unique<Poco::FileOutputStream>(_tempPath);
        _outputStream = std::make_unique<Poco::TeeOutputStream>(*_streamFile);
        write(header);
    }

    void CBMFlatFile::write(const std::string& text) {
        (*_outputStream) << text;
    }

    void CBMFlatFile::save() {
        _streamFile->close();
        _outputFile->renameTo(_path);
    }

    void CBMAggregatorCsvWriter::configure(const DynamicObject& config) {
        _outputPath = config["output_path"].convert<std::string>();
    }

    /**
     * @brief subscribe to FLINT.
     * 
     * Detailed description here
     * 
     * @param notificationCenter NotificationCenter&
     * @return void
     * ************************/

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

    /**
     * @brief initiate local domain.
     * 
     * Detailed description here
     * 
     * @return void
     * ************************/

    void CBMAggregatorCsvWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

     /**
     * @brief perform system shut down.
     * 
     * Detailed description here
     * 
     * @return void
     * ************************/
    void CBMAggregatorCsvWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        load((boost::format("%1%/flux_%2%")        % _outputPath % _jobId).str(), _classifierNames, _fluxDimension);
        load((boost::format("%1%/pool_%2%")        % _outputPath % _jobId).str(), _classifierNames, _poolDimension);
        load((boost::format("%1%/error_%2%")       % _outputPath % _jobId).str(), _classifierNames, _errorDimension);
        load((boost::format("%1%/age_%2%")         % _outputPath % _jobId).str(), _classifierNames, _ageDimension);
        load((boost::format("%1%/disturbance_%2%") % _outputPath % _jobId).str(), _classifierNames, _disturbanceDimension);

        MOJA_LOG_INFO << "Finished loading results." << std::endl;
    }

    /**
     * @brief load classifier.
     * 
     * Detailed description here
     * 
     * @return void
     * ************************/

    template<typename TAccumulator>
    void CBMAggregatorCsvWriter::load(
        const std::string& outputPath,
        std::shared_ptr<std::vector<std::string>> classifierNames,
        std::shared_ptr<TAccumulator> dataDimension) {

        MOJA_LOG_INFO << (boost::format("Loading %1%") % outputPath).str();
        std::unordered_map<int, std::shared_ptr<CBMFlatFile>> flatFiles;

        auto records = dataDimension->records();
        if (!records.empty()) {
            for (auto& record : records) {
                if (flatFiles.find(record.getYear()) == flatFiles.end()) {
                    auto yearOutputPath = (boost::format("%1%_%2%.csv") % outputPath % record.getYear()).str();
                    flatFiles[record.getYear()] = std::make_shared<CBMFlatFile>(yearOutputPath, record.header(*classifierNames));
                }

                flatFiles[record.getYear()]->write(record.asPersistable());
            }

            for (auto& flatFile : flatFiles) {
                flatFile.second->save();
            }
        }
    }

}}} // namespace moja::modules::cbm
