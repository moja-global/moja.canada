#ifndef MOJA_MODULES_CBM_TIMESERIES_H_
#define MOJA_MODULES_CBM_TIMESERIES_H_

#include <moja/flint/itiming.h>

#include "moja/modules/cbm/_modules.cbm_exports.h"

#include <boost/optional.hpp>
#include <vector>

namespace moja {
namespace modules {
namespace cbm {

enum class ExtrapType {
    NearestYr = 0,
    CycleYrs,
    AvgYr
};


/**
 * <summary>    The date origin of the timeseries. Calendar allows yr0 to be  
 *              specified as a year. StartSim means that yr0 is a timestep in
 *              the same units as dataPerYr. </summary>
 */
enum class DateOrigin {
    Calendar = 0,
    StartSim,
    Sprout
};

enum class Frequency {
    Annual,
    Monthly,
    Daily,
    Observations
};

enum class InterpType {
    Linear,
    ColAverage
};

enum class SubSamplingMode {
    None,
    Interpolate
};

/**
 * <summary>    Encapsulates a timeseries of data. Normal usage allows modules
 *              to retrieve the most relevant data for the current timestep of
 *              the simulation. </summary>
 */
class CBM_API TimeSeries {
public:
    TimeSeries();

    /**
     * <summary>    Constructs a TimeSeries with the specified parameters. </summary>
     *
     * <param name="yr0">           The first timestep of data in the TimeSeries. Must be a
     *                              multiple of dataPerYr for origin=DateOrigin::StartSim;
     *                              for example, data starting in the second year of a monthly
     *                              simulation would have yr0=12. </param>
     * <param name="dataPerYr">     The number of data points per year in the TimeSeries. </param>
     * <param name="nYrs">          The number of years of data in the TimeSeries. </param>
     * <param name="subSame">       If true, values are divided between the sub-timesteps. False
     *                              means that the value for sub-timesteps is the same as for the
     *                              full timestep. </param>
     * <param name="raw">           The raw data making up the TimeSeries. </param>
     * <param name="origin">        (Optional) The units of the TimeSeries origin (yr0), most
     *                              commonly either StartSim (timestep - the default) or
     *                              Calendar (year). </param>
     * <param name="extraSteps">    (Optional) the extra steps. </param>
     */
    TimeSeries(int yr0, int dataPerYr, int nYrs, bool subSame,
               const std::vector<boost::optional<double>> raw,
               DateOrigin origin = DateOrigin::StartSim,
               int extraSteps = 0);

    ~TimeSeries() = default;

    /**
     * <summary>    Sets the simulation timing for the TimeSeries. The value returned
     *              by the TimeSeries will be the most appropriate for the current timestep
     *              in the timing object and the configuration of the TimeSeries; for example,
     *              if the TimeSeries is set up for yearly data, and the timing object is
     *              also in yearly steps and is currently on timestep 5, value() will return
     *              the timestep 5 value. </summary>
     *
     * <param name="timing">    The timing. </param>
     */
    void setTiming(const flint::ITiming* timing);

    /**
     * <summary>    Alternate way to set the timing for the TimeSeries. The most often used
     *              method is to use the setTiming(const ITiming*) version. </summary>
     *
     * <param name="start"> The start Date/Time. </param>
     * <param name="end">   The end Date/Time. </param>
     */
    void setTiming(DateTime start, DateTime end);

    /**
     * <summary>    Gets the values for all timesteps in the entire TimeSeries. </summary>
     */
    const std::vector<double>& series() const;

    /**
     * <summary>    Gets the most current value for this point in time in the
     *              simulation. </summary>
     */
    double value() const;

    /**
     * <summary>    The first year or timestep of data for the TimeSeries. </summary>
     */
    int yr0() const;

    /**
     * <summary>    The number of years in the TimeSeries. </summary>
     */
    int nYrs() const;

    /**
     * <summary>    The number of data points per year in the TimeSeries. </summary>
     */
    int dataPerYr() const;

    /**
     * <summary>    True means that sub-timesteps get the full year value; false
     *              means the yearly value will be divided evenly among sub-timesteps. </summary>
     */
    bool subSame() const;

    /**
     * <summary>    The raw data for the TimeSeries. </summary>
     */
    const std::vector<boost::optional<double>> raw() const;

    /**
     * <summary>    The origin of the TimeSeries. </summary>
     */
    DateOrigin origin() const;

    ExtrapType extrap() const;

private:
    class TimeSeriesPrep;
    std::shared_ptr<TimeSeriesPrep> _impl;
};

struct CORE_API Observation {
    DateTime date;
    boost::optional<double> value;
};

class CORE_API TimeSeries2 {
public:
    TimeSeries2() : _interpType(InterpType::ColAverage),
                    _extraptype(ExtrapType::NearestYr),
                    _frequency(Frequency::Monthly),
                    _multiplier(1.0) {}

    TimeSeries2(DateTime start,
                const std::vector<boost::optional<double>>& data,
                Frequency frequency = Frequency::Monthly,
                ExtrapType extraptype = ExtrapType::NearestYr,
                InterpType interpType = InterpType::ColAverage) :
        _interpType(interpType), _extraptype(extraptype), _frequency(frequency),
        _start(start), _multiplier(1.0), _series(data) {}

    explicit TimeSeries2(const std::vector<Observation>& observations,
                         ExtrapType extraptype = ExtrapType::NearestYr,
                         InterpType interpType = InterpType::Linear) :
        _interpType(interpType), _extraptype(extraptype), _frequency(Frequency::Observations),
        _multiplier(1.0), _observations(observations) {}

    void setData(DateTime start,
                 const std::vector<boost::optional<double>>& data,
                 Frequency frequency = Frequency::Monthly) {}

    void setObservations(const std::vector<Observation>& observations) {}
    void addObservation(Observation observation) {}

    double multiplier() const;
    void setMultiplier(double multiplier);

    std::vector<Observation> observations() const;
    std::vector<double> series(DateTime start, DateTime end) const;
    double value() const; // by age(duration??) for date???

    ExtrapType extrapType() const;
    void setExtrapType(ExtrapType extraptype);

    InterpType interpType() const;
    void setInterpType(InterpType interpType);

    Frequency frequency() const;

private:
    InterpType _interpType;
    ExtrapType _extraptype;
    Frequency _frequency;
    DateTime _start;
    double _multiplier; 
    std::vector<boost::optional<double>> _series;
    std::vector<Observation> _observations;
};

inline double TimeSeries2::multiplier() const {
    return _multiplier;
}

inline void TimeSeries2::setMultiplier(double multiplier) {
    _multiplier = multiplier;
}

inline std::vector<Observation> TimeSeries2::observations() const {
    switch (_frequency)
    {
    case Frequency::Observations: {
        return _observations;
    } break;
    case Frequency::Annual:  {
        auto observations = std::vector<Observation>();
        auto date = _start;
        for (auto value : _series) {
            // Represents the end of the annual step.
            auto observationDate = DateTime(date.year() + 1, 1, 1).addMicroseconds(-1);
            observations.push_back({ observationDate, value });
            date.addYears(1);
        }
        return observations;
    } break;
    case Frequency::Monthly: {
        auto observations = std::vector<Observation>();
        auto date = _start;
        for (auto value : _series) {
            // Represents the end of the monthly step.
            auto observationDate = DateTime(date.year(), date.month(), 1).addMonths(1).addMicroseconds(-1);
            observations.push_back({ observationDate, value });
            date.addMonths(1);
        }
        return observations;
    } break;
    case Frequency::Daily: break;
    default: break;
    }
    return std::vector<Observation>();
}

inline std::vector<double> TimeSeries2::series(DateTime start, DateTime end) const {
    // - Later interpolations and extrapolations require the TS table to be complete.
    // - Empty cells are assigned the average of the column.
    auto dataPerYr = -1;
    switch (_frequency)
    {
    case Frequency::Annual: dataPerYr = 1; break;
    case Frequency::Monthly: dataPerYr = 12; break;
    default: break;
    }
    auto nYrs = _series.size() / dataPerYr;

    std::vector<double> result(_series.size());
    for (auto c = 0; c < dataPerYr; ++c) { // Calculate col sum
        double colAvg = 0.0;
        int n = 0;
        int ix = c;
        for (auto r = 0; r < nYrs; ++r) {
            if (_series[ix].is_initialized()) {
                colAvg += _series[ix].get();
                ++n;
            }
            ix += dataPerYr;
        }

        if (n == 0) {
            throw std::runtime_error("No data for timeseries column");
        }

        colAvg /= n;
        ix = c;
        for (auto r = 0; r < nYrs; ++r) {
            result[ix] = (_series[ix].is_initialized() ? _series[ix].get() : colAvg) * _multiplier;
            ix += dataPerYr;
        }
    }
        
    return std::vector<double>();
}

inline double TimeSeries2::value() const {
    return 0.0;
}

inline ExtrapType TimeSeries2::extrapType() const {
    return _extraptype;
}

inline void TimeSeries2::setExtrapType(ExtrapType extraptype) {
    _extraptype = extraptype;
}

inline InterpType TimeSeries2::interpType() const {
    return _interpType;
}

inline void TimeSeries2::setInterpType(InterpType interpType) {
    _interpType = interpType;
}

inline Frequency TimeSeries2::frequency() const	{
    return _frequency;
}

} // namespace cbm
} // namespace modules
} // namespace moja

#endif // MOJA_MODULES_CBM_TIMESERIES_H_
