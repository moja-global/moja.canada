#ifndef MOJA_MODULES_CBM_TIMESERIESUTILS_H_
#define MOJA_MODULES_CBM_TIMESERIESUTILS_H_

#include <moja/_core_exports.h>
#include <moja/dynamic.h>
#include <moja/exception.h>

#include "moja/modules/cbm/timeseries.h"

namespace moja {
namespace modules {
namespace cbm {

/**
 * <summary>    Converts a DynamicObject containing TimeSeries configuration to an
 *              actual TimeSeries object. Configuration must be in the format:
 *              {
 *                  "$time_series": {
 *                      "year_0": &lt;first timestep of data&gt;,
 *                      "data_per_year": &lt;data points per year&gt;,
 *                      "n_years": &lt;years of data&gt;,
 *                      "sub_same": &lt;sub-timesteps use whole year value&gt;,
 *                      "values": &lt;the raw timeseries data&gt;
 *                  }
 *              }
 * </summary>
 *
 * <exception cref="InvalidArgumentException">
 * Thrown when the DynamicObject does not contain all required configuration.
 * </exception>
 *
 * <param name="s"> The DynamicObject to process. </param>
 *
 * <returns>    A TimeSeries object matching the specified configuration. </returns>
 */
inline TimeSeries convertDynamicObjectToTimeSeries(const DynamicObject& s) {
    if (!(s.size() == 1 && s.begin()->first == "$time_series")) {
        throw InvalidArgumentException();
    }
    auto varStruct = s.begin()->second.extract<const DynamicObject>();

    int yr0 = varStruct["year_0"];
    int dataPerYr = varStruct["data_per_year"];
    int nYrs = varStruct["n_years"];
    bool subSame = varStruct["sub_same"];

	auto arr = varStruct["values"].extract<DynamicVector>();
	std::vector<boost::optional<double>> values;
	for (auto item : arr) {
        if (item.isEmpty()) {
            values.push_back(boost::optional<double>{});
        } else {
			values.push_back(item.convert<double>());
        }
	}

	TimeSeries timeSeries(yr0, dataPerYr, nYrs, subSame, values);
    return timeSeries;
}

inline TimeSeries convertDynamicToTimeSeries(const DynamicVar& value) {
    if (!value.isStruct()) {
        throw InvalidArgumentException();
    }

    auto& s = value.extract<const DynamicObject>();
    return convertDynamicObjectToTimeSeries(s);
}

inline DynamicVar convertTimeSeriesToDynamic(const TimeSeries& timeseries) {

	DynamicVector ts_values = DynamicVector();
	
	for (auto val : timeseries.raw()) {
		if (val)
			ts_values.push_back(val.get());
		else
			ts_values.push_back(DynamicVar());	// json NULL to allow for nodata value
	}

	DynamicObject ts_body = DynamicObject({
		{ "year_0",			timeseries.yr0() },
		{ "data_per_year",	timeseries.dataPerYr() },
		{ "n_years",		timeseries.nYrs() },
		{ "sub_same",		timeseries.subSame() },
		{ "values",			ts_values }
	});

	DynamicObject ts_container = DynamicObject({
		{ "$time_series", ts_body}
	});
	return ts_container;
}

} // namespace cbm
} // namespace modules
} // namespace moja

#endif // MOJA_MODULES_CBM_TIMESERIESUTILS_H_