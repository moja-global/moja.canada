#include <moja/logging.h>
#include <moja/exception.h>

#include "moja/modules/cbm/timeseries.h"

#include <cstring>
#include <Poco/Bugcheck.h>

namespace moja {
namespace modules {
namespace cbm {

class TimeSeries::TimeSeriesPrep {
public:
    TimeSeriesPrep(int yr0, int dataPerYr, int nYrs, bool subSame, DateOrigin origin,
                   int extraSteps, const std::vector<boost::optional<double>> raw);

    TimeSeriesPrep() :
        _extrap(ExtrapType::NearestYr), _origin(DateOrigin::StartSim), _yr0(0), _nYrs(0),
        _dataPerYr(0), _mult(1.0), _subSame(false), _prepared(false), _extraStepsIfSprout(0) {}

    ~TimeSeriesPrep() = default;

    void setTiming(const flint::ITiming* timing);
    const std::vector<double>& series();
    double value();
    int yr0() const;
    int nYrs() const;
    int dataPerYr() const;
    bool subSame() const;
    const std::vector<boost::optional<double>> raw() const;
    DateOrigin origin() const;
    ExtrapType extrap() const;

private:
    void preparedTS();
    void calcSizes();
    std::vector<double> fillGapsMult();
    void computeOneDataPoint(const std::vector<double>& data, std::vector<double>& prep);
    void computeAvgYr(const std::vector<double>& data);
    void computeWholeYrsInterp(const std::vector<double>& data, std::vector<double>& prep);
    void computeWholeYrsBefore(const std::vector<double>& data, std::vector<double>& prep);
    void computeWholeYrsAfter(const std::vector<double>& data, std::vector<double>& prep);
    void computeOneWholeYr(const std::vector<double>& data, std::vector<double>& prep, int dix, int pix);
    void computeOddStepsBefore(const std::vector<double>& data, std::vector<double>& prep);
    void computeOddStepsAfter(const std::vector<double>& data, std::vector<double>& prep);
    void copyOddStepsBefore(const std::vector<double>& src, std::vector<double>& prep, int srcIx);
    void copyOddStepsAfter(const std::vector<double>& src, std::vector<double>& prep, int srcIx, int pix);
    int dixFromYr(const std::vector<double>& data, int yr);
    void interpOneYr(const double* d, double* p, int prepIxLo, int prepIxHi);

    void interpOneYrWithCheck(
        const std::vector<double>& data, std::vector<double>& prep,
        int dix, int pix, int prepIxLo, int prepIxHi);

    ExtrapType _extrap;	// NearestYr, CycleYrs, AvgYr
    DateOrigin _origin;	// StartSim, Calendar, Sprout
    int _yr0;  // Raw data is dataPerYr pieces of data in each of the years
    int _nYrs; //       yr0 .. yr0 + nYrsTS - 1
    int _dataPerYr;
    double _mult;
    bool _subSame;
    const flint::ITiming* _timing;
    const std::vector<boost::optional<double>> _raw; // User's raw data
    std::vector<double> _series;
    bool _prepared;

    // About the current prepTS
    int	_prepFirstYr;
    int _prepFirstStep;
    int _prepLastYr;
    int _prepStepsPerYr;

    // Used for computing the time series prepTS by PrepareTS:
    std::vector<double> _avgData; // one year of averaged data (if required)
    std::vector<double> _avgPrep; // one year of prepared averaged data

    int	_dataStYr; // Start year of data
    int _dataEnYr; // End year of data

    // Prep timing
    int	_prepNSteps;		//   Number of time series steps required by simulation
    int _prepStYr;			//   Start year for ts
    int _prepEnYr;			//   End year for ts

    // Partition prep into whole years and odds steps
    int	_oddStepsBefore;	//   Part year before first whole year [0..prepStepsPerYr - 1]
    int _wholeYrsBefore;	//   Extrapolate before data
    int _wholeYrsInterp;	//   Interpolate data
    int _wholeYrsAfter;		//   Extrapolate after data
    int _oddStepsAfter;		//   Part year after  last  whole year [0..prepStepsPerYr - 1]
    int	_nWholeYrs;			//   wholeYrsBefore + wholeYrsInterp + wholeYrsAfter
    int _firstWholeYr;
    int _extraStepsIfSprout;
};

TimeSeries::TimeSeries() : _impl(std::make_shared<TimeSeriesPrep>()) {}

TimeSeries::TimeSeries(int yr0, int dataPerYr, int nYrs, bool subSame,
                       const std::vector<boost::optional<double>> raw,
                       DateOrigin origin, int extraSteps) :
    _impl(std::make_shared<TimeSeriesPrep>(yr0, dataPerYr, nYrs, subSame, origin, extraSteps, raw)) {}

const std::vector<double>& TimeSeries::series() const { return _impl->series(); }
double TimeSeries::value() const { return _impl->value(); }
int TimeSeries::yr0() const { return _impl->yr0(); }
int TimeSeries::nYrs() const { return _impl->nYrs(); }
int TimeSeries::dataPerYr() const { return _impl->dataPerYr(); }
bool TimeSeries::subSame() const { return _impl->subSame(); }
const std::vector<boost::optional<double>> TimeSeries::raw() const { return _impl->raw(); }
DateOrigin TimeSeries::origin() const { return _impl->origin(); }
ExtrapType TimeSeries::extrap() const { return _impl->extrap(); }

void TimeSeries::setTiming(const flint::ITiming* timing) {
    _impl->setTiming(timing);
}

TimeSeries::TimeSeriesPrep::TimeSeriesPrep(
    int yr0, int dataPerYr, int nYrs, bool subSame, DateOrigin origin,
    int extraSteps, const std::vector<boost::optional<double>> raw) :
    _extrap(ExtrapType::NearestYr), _origin(origin), _yr0(yr0), _nYrs(nYrs), _dataPerYr(dataPerYr),
    _mult(1.0), _subSame(subSame), _raw(raw), _prepared(false), _extraStepsIfSprout(extraSteps) {}

void TimeSeries::TimeSeriesPrep::setTiming(const flint::ITiming* timing) {
	_timing = timing;
	_prepNSteps = _timing->nSteps();
	if (_origin == DateOrigin::Sprout)
		_prepNSteps += _extraStepsIfSprout;

#if 1 // TODO: Jim
	auto timingWithInitStep = _timing->startDate();
	switch (timing->stepping())
	{
		case flint::TimeStepping::Monthly:		timingWithInitStep.addMonths(-1); break;
		case flint::TimeStepping::Annual:		timingWithInitStep.addYears(-1); break;
		case flint::TimeStepping::Daily:		timingWithInitStep.addDays(-1); break;
	}

	_prepFirstYr	= timingWithInitStep.year();
	_prepFirstStep	= timingWithInitStep.month();
	_prepLastYr		= timing->endDate().year();
#else
	_prepFirstYr = _timing->startDate().year();
	_prepFirstStep = _timing->startDate().month();
	_prepLastYr = _timing->endDate().year();
#endif

	switch (timing->stepping())
	{
		case flint::TimeStepping::Monthly: _prepStepsPerYr = 12; break;
		case flint::TimeStepping::Annual: _prepStepsPerYr = 1; break;
		case flint::TimeStepping::Daily:
			throw NotImplementedException("Daily timing not implemented yet");
			break;
	}

	// Compute sizing parameters
	calcSizes();
	_prepared = false;
}


const std::vector<double>& TimeSeries::TimeSeriesPrep::series() {
    if (!_prepared) {
        preparedTS();
    }

    return _series;
}

double TimeSeries::TimeSeriesPrep::value() {
    if (!_prepared) {
        preparedTS();
    }

    return _series[_timing->step()];
}

int TimeSeries::TimeSeriesPrep::yr0() const { return _yr0; }
int TimeSeries::TimeSeriesPrep::nYrs() const { return _nYrs; }
int TimeSeries::TimeSeriesPrep::dataPerYr() const { return _dataPerYr; }
bool TimeSeries::TimeSeriesPrep::subSame() const { return _subSame; }
const std::vector<boost::optional<double>> TimeSeries::TimeSeriesPrep::raw() const { return _raw; }
DateOrigin TimeSeries::TimeSeriesPrep::origin() const { return _origin; }
ExtrapType TimeSeries::TimeSeriesPrep::extrap() const { return _extrap; }

void TimeSeries::TimeSeriesPrep::preparedTS() {
    // Computations with raw data.
    auto data = fillGapsMult();
    _series.resize(_prepNSteps);

    if (data.size() == 1) {
        computeOneDataPoint(data, _series);
        return;
    }

    if (_extrap == ExtrapType::AvgYr
        && (   _wholeYrsBefore > 0
            || _wholeYrsAfter  > 0
            || _oddStepsBefore > 0
            || _oddStepsAfter  > 0)) {
        computeAvgYr(data);
    }

    // Compute prepared data. Must go in this order!
    if (_wholeYrsInterp > 0) computeWholeYrsInterp(data, _series);
    if (_wholeYrsBefore > 0) computeWholeYrsBefore(data, _series);
    if (_wholeYrsAfter  > 0)  computeWholeYrsAfter(data, _series);
    if (_oddStepsBefore > 0) computeOddStepsBefore(data, _series);
    if (_oddStepsAfter  > 0)  computeOddStepsAfter(data, _series);
    _prepared = true;
}

#if 0
{
	void TimeSeries::PrepareTSCalcSizes(int firstYr, int firstStep, int lastYr) {
		// Already set: prepNSteps, prepFirstYr, prepFirstStep, prepStepsPerYr
		ASSERT(z.tExtrapTS == kNearestYr || z.tExtrapTS == kCycleYrs || z.tExtrapTS == kAvgYr);
		int prepStStep; // Unlike firstStep, is 0-indexed [0..prepStepsPerYr - 1]

		dataStYr = z.yr0TS;
		dataEnYr = z.yr0TS + z.nYrsTS - 1;
		synched = (prepStepsPerYr == z.dataPerYrTS);
		// Set prepStYr, prepEnYr
		if (z.tOriginTS == kCalendar) {
			prepStYr = firstYr;
			prepStStep = firstStep - 2;
			if (prepStStep < 0) {
				--prepStYr;
				prepStStep = prepStepsPerYr - 1;
			}
			prepEnYr = lastYr;
		}
		else {
			ASSERT(z.tOriginTS == kStartSim || z.tOriginTS == kSprout);
			prepStYr = -1;
			prepStStep = prepStepsPerYr - 1;
			prepEnYr = (prepNSteps - 2) / prepStepsPerYr;
			ASSERT(prepEnYr >= 0);
		}
		ASSERT(prepEnYr >= prepStYr);
		// Identify whole years in prep
		nWholeYrs = prepEnYr - prepStYr - 1;  // Start with interior years
		if (prepStStep == 0 || nWholeYrs < 0) {
			firstWholeYr = prepStYr;
			oddStepsBefore = 0;
			++nWholeYrs;
		}
		else {
			firstWholeYr = prepStYr + 1;
			oddStepsBefore = prepStepsPerYr - prepStStep;
		}
		oddStepsAfter = prepNSteps - oddStepsBefore - nWholeYrs * prepStepsPerYr;
		while (oddStepsAfter >= prepStepsPerYr) {
			oddStepsAfter -= prepStepsPerYr;
			++nWholeYrs;
		}
		ASSERT(oddStepsBefore + nWholeYrs * prepStepsPerYr + oddStepsAfter == prepNSteps);
		// Align prep whole years with user's data
		wholeYrsBefore = z.yr0TS - firstWholeYr;
		if (wholeYrsBefore < 0) wholeYrsBefore = 0;
		else if (wholeYrsBefore > nWholeYrs) wholeYrsBefore = nWholeYrs;

		wholeYrsAfter = (firstWholeYr + nWholeYrs) - (z.yr0TS + z.nYrsTS);
		if (wholeYrsAfter < 0) wholeYrsAfter = 0;
		else if (wholeYrsAfter > nWholeYrs) wholeYrsAfter = nWholeYrs;

		wholeYrsInterp = nWholeYrs - wholeYrsBefore - wholeYrsAfter;
		ASSERT(wholeYrsInterp >= 0);
		ASSERT(wholeYrsInterp <= z.nYrsTS);
	}
}
#endif

void TimeSeries::TimeSeriesPrep::calcSizes() {
    // Already set: prepNSteps, prepFirstYr, prepFirstStep, prepStepsPerYr
	poco_assert(   _extrap == ExtrapType::NearestYr
                || _extrap == ExtrapType::CycleYrs
                || _extrap == ExtrapType::AvgYr);

    int prepStStep; // Unlike firstStep, is 0-indexed [0..prepStepsPerYr - 1]

    _dataStYr = _yr0;
    _dataEnYr = _dataStYr + _nYrs - 1;
    // Set prepStYr, prepEnYr
    if (_origin == DateOrigin::Calendar) {
        _prepStYr = _prepFirstYr;
        prepStStep = _prepFirstStep - 1;
        _prepEnYr = _prepLastYr;
    } else {
        poco_assert(_origin == DateOrigin::StartSim || _origin == DateOrigin::Sprout);
		_prepStYr = -1;					// 0;
        prepStStep = _prepStepsPerYr - 1; // 0;
        _prepEnYr = (_prepNSteps - 1) / _prepStepsPerYr;
        poco_assert(_prepEnYr >= 0);
    }
    poco_assert(_prepEnYr >= _prepStYr);

    // Identify whole years in prep
    _nWholeYrs = _prepEnYr - _prepStYr - 1;  // Start with interior years
    if (prepStStep == 0 || _nWholeYrs < 0) {
        _firstWholeYr = _prepStYr;
        _oddStepsBefore = 0;
        ++_nWholeYrs;
    } else {
        _firstWholeYr = _prepStYr + 1;
        _oddStepsBefore = _prepStepsPerYr - prepStStep;
    }

    _oddStepsAfter = _prepNSteps - _oddStepsBefore - _nWholeYrs * _prepStepsPerYr;
    while (_oddStepsAfter >= _prepStepsPerYr) {
        _oddStepsAfter -= _prepStepsPerYr;
        ++_nWholeYrs;
    }
    poco_assert(_oddStepsBefore + _nWholeYrs * _prepStepsPerYr + _oddStepsAfter == _prepNSteps);

    // Align prep whole years with user's data
    _wholeYrsBefore = _dataStYr - _firstWholeYr;
    if (_wholeYrsBefore < 0) _wholeYrsBefore = 0;
    else if (_wholeYrsBefore > _nWholeYrs) _wholeYrsBefore = _nWholeYrs;

    _wholeYrsAfter = (_firstWholeYr + _nWholeYrs) - (_dataStYr + _nYrs);
    if (_wholeYrsAfter < 0) _wholeYrsAfter = 0;
    else if (_wholeYrsAfter > _nWholeYrs) _wholeYrsAfter = _nWholeYrs;

    _wholeYrsInterp = _nWholeYrs - _wholeYrsBefore - _wholeYrsAfter;

    poco_assert(_wholeYrsInterp >= 0);
    poco_assert(_wholeYrsInterp <= _nYrs);
}

std::vector<double> TimeSeries::TimeSeriesPrep::fillGapsMult() {
    // - Later interpolations and extrapolations require the TS table to be complete.
    // - Empty cells are assigned the average of the column.

    std::vector<double> result(_raw.size());
    for (auto c = 0; c < _dataPerYr; ++c) {              // Calculate col sum
        double colAvg = 0.0;
        int n = 0;
        int ix = c;
        for (auto r = 0; r < _nYrs; ++r) {
            if (_raw[ix].is_initialized()) {
                colAvg += _raw[ix].get();
                ++n;
            }
            ix += _dataPerYr;
        }

        if (n == 0)
            throw std::runtime_error("No data for timeseries column");

        colAvg /= n;
        ix = c;
        for (auto r = 0; r < _nYrs; ++r) {
            result[ix] = (_raw[ix].is_initialized() ? _raw[ix].get() : colAvg) * _mult;
            ix += _dataPerYr;
        }
    }
    return result;
}

void TimeSeries::TimeSeriesPrep::computeOneDataPoint(const std::vector<double>& data,
                                                     std::vector<double>& prep) {
    poco_assert(data.size() == 1);
    double val = data.front();
    if (!_subSame)
        val /= static_cast<double>(_prepStepsPerYr);
    std::fill(std::begin(prep), std::end(prep), val);
}

void TimeSeries::TimeSeriesPrep::computeAvgYr(const std::vector<double>& data) {
    poco_assert(_nYrs >= 1);
    // Compute raw avg -> avgData
    _avgData.resize(_dataPerYr);
    if (_nYrs == 1) {
        std::copy(std::begin(data), std::end(data), std::begin(_avgData));
    }
    else {
        double nYrsInv = 1.0 / static_cast<double>(_nYrs);
        for (int i = _dataPerYr - 1; i >= 0; --i) {
            double sum = 0.0;
            int k = i;
            for (int j = _nYrs - 1; j >= 0; --j, k += _dataPerYr)
                sum += data[k];
            _avgData[i] = sum * nYrsInv;
        }
    }
    // Compute prepared avg -> avgPrep
    _avgPrep.resize(_prepStepsPerYr);
    interpOneYr(&_avgData[0], &_avgPrep[0], 0, _prepStepsPerYr - 1);
}

// Interpolate whole years from user's data
void TimeSeries::TimeSeriesPrep::computeWholeYrsInterp(const std::vector<double>& data,
                                                       std::vector<double>& prep) {
    poco_assert(_wholeYrsInterp > 0);
    int dix = (_firstWholeYr + _wholeYrsBefore - _yr0) * _dataPerYr;
    int pix = _oddStepsBefore + _wholeYrsBefore * _prepStepsPerYr;
    for (int i = _wholeYrsInterp - 1; i >= 0; --i) {
        interpOneYrWithCheck(data, prep, dix, pix, 0, _prepStepsPerYr - 1);
        dix += _dataPerYr;
        pix += _prepStepsPerYr;
    }
}

// Extrapolate whole years before user's data
void TimeSeries::TimeSeriesPrep::computeWholeYrsBefore(const std::vector<double>& data,
                                                       std::vector<double>& prep) {
    poco_assert(_wholeYrsBefore > 0);
    // PrepareTSComputeWholeYrsInterp must already have run!
    int nbOneYr = _prepStepsPerYr * sizeof(double);
    // Set to last whole-year-before
    int lastYrBefore = _firstWholeYr + _wholeYrsBefore - 1;
    int pix = _oddStepsBefore + (_wholeYrsBefore - 1) * _prepStepsPerYr;
    // Calculated years
    int nCalcYrs = 0;
    int	pixCopyIncr = 0;
    switch (_extrap) {
    case ExtrapType::NearestYr:
        if (_wholeYrsInterp == 0) {
            int dix = dixFromYr(data, lastYrBefore);
            computeOneWholeYr(data, prep, dix, pix);
            pix -= _prepStepsPerYr;
            nCalcYrs = 1;
        }
        else
            nCalcYrs = 0;
        pixCopyIncr = _prepStepsPerYr;
        break;
    case ExtrapType::CycleYrs:
        nCalcYrs = _nYrs - _wholeYrsInterp;
        if (nCalcYrs > _wholeYrsBefore)
            nCalcYrs = _wholeYrsBefore;
        if (nCalcYrs <= 0)
            nCalcYrs = 0;
        else {
            int dix = dixFromYr(data, lastYrBefore);
            for (int i = nCalcYrs; i > 0; --i) {
                computeOneWholeYr(data, prep, dix, pix);
                dix -= _dataPerYr;
                if (dix < 0)
                    dix = static_cast<int>(data.size()) - _dataPerYr;
                pix -= _prepStepsPerYr;
            }
        }
        pixCopyIncr = _prepStepsPerYr * _nYrs;
        break;
    case ExtrapType::AvgYr:
        std::memcpy(&prep[pix], &_avgPrep[0], nbOneYr);
        pix -= _prepStepsPerYr;
        nCalcYrs = 1;
        pixCopyIncr = _prepStepsPerYr;
        break;
    default:
        throw std::invalid_argument("_extrap");
    }
    // Copied years
    for (int i = _wholeYrsBefore - nCalcYrs; i > 0; --i) {
        memcpy(&prep[pix], &prep[pix + pixCopyIncr], nbOneYr);
        pix -= _prepStepsPerYr;
    }
}

// Extrapolate whole years after user's data
void TimeSeries::TimeSeriesPrep::computeWholeYrsAfter(const std::vector<double>& data,
                                                      std::vector<double>& prep) {
    poco_assert(_wholeYrsAfter > 0);
    // PrepareTSComputeWholeYrsInterp must already have run!
    int nbOneYr = _prepStepsPerYr * sizeof(double);
    // Set to first whole-year-after
    int firstYrAfter = _firstWholeYr + _wholeYrsBefore + _wholeYrsInterp;
    int pix = _oddStepsBefore + (_wholeYrsBefore + _wholeYrsInterp) * _prepStepsPerYr;
    // Calculated years
    int nCalcYrs = 0;
    int	pixCopyDecr = 0;
    switch (_extrap) {
    case ExtrapType::NearestYr:
        if (_wholeYrsInterp == 0) {
            int dix = dixFromYr(data, firstYrAfter);
            computeOneWholeYr(data, prep, dix, pix);
            pix += _prepStepsPerYr;
            nCalcYrs = 1;
        }
        else
            nCalcYrs = 0;
        pixCopyDecr = _prepStepsPerYr;
        break;
    case ExtrapType::CycleYrs:
        nCalcYrs = _nYrs - _wholeYrsInterp;
        if (nCalcYrs > _wholeYrsAfter)
            nCalcYrs = _wholeYrsAfter;
        if (nCalcYrs <= 0) {
            nCalcYrs = 0;
        }
        else {
            int dix = dixFromYr(data, firstYrAfter);
            for (int i = nCalcYrs; i > 0; --i) {
                computeOneWholeYr(data, prep, dix, pix);
                dix += _dataPerYr;
                if (dix >= static_cast<int>(data.size()))
                    dix = 0;
                pix += _prepStepsPerYr;
            }
        }
        pixCopyDecr = _prepStepsPerYr * _nYrs;
        break;
    case ExtrapType::AvgYr:
        memcpy(&prep.data()[pix], &_avgPrep.data()[0], nbOneYr);
        pix += _prepStepsPerYr;
        nCalcYrs = 1;
        pixCopyDecr = _prepStepsPerYr;
        break;
    default:
        throw std::invalid_argument("_extrap");
    }
    // Copied years
    for (int i = _wholeYrsAfter - nCalcYrs; i > 0; --i) {
        memcpy(&prep.data()[pix], &prep.data()[pix - pixCopyDecr], nbOneYr);
        pix += _prepStepsPerYr;
    }
}

void TimeSeries::TimeSeriesPrep::computeOneWholeYr(const std::vector<double>& data,
                                                   std::vector<double>& prep,
                                                   int dix, int pix) {
    interpOneYrWithCheck(data, prep, dix, pix, 0, _prepStepsPerYr - 1);
}

// Partial year at start of prep
void TimeSeries::TimeSeriesPrep::computeOddStepsBefore(const std::vector<double>& data,
                                                       std::vector<double>& prep) {
    poco_assert(_oddStepsBefore > 0);
    if (_oddStepsBefore <= 0) {
        return;
    }

    // prepareTSComputeWholeYrsInterp, prepareTSComputeWholeYrsBefore, and
    // prepareTSComputeWholeYrsAfter must already have run (ie all whole years done)!

    // Interpolated
    if (_yr0 <= _prepStYr && _prepStYr <= _dataEnYr) {
        int dix = dixFromYr(data, _prepStYr);
        interpOneYrWithCheck(data, prep, dix, 0, _prepStepsPerYr - _oddStepsBefore, _prepStepsPerYr - 1);
    }
    else { // Extrapolated
        switch (_extrap) {
        case ExtrapType::NearestYr:
            if (_nWholeYrs >= 1) {
                copyOddStepsBefore(prep, prep, _prepStepsPerYr);
            }
            else {
                int dix = dixFromYr(data, _prepStYr);
                interpOneYrWithCheck(data, prep, dix, 0, _prepStepsPerYr - _oddStepsBefore, _prepStepsPerYr - 1);
            }
            break;
        case ExtrapType::CycleYrs:
            if (_nYrs <= _nWholeYrs) {
                copyOddStepsBefore(prep, prep, _nYrs * _prepStepsPerYr);
            }
            else {
                int dix = dixFromYr(data, _prepStYr);
                interpOneYrWithCheck(data, prep, dix, 0, _prepStepsPerYr - _oddStepsBefore, _prepStepsPerYr - 1);
            }
            break;
        case ExtrapType::AvgYr:
            copyOddStepsBefore(_avgPrep, prep, _prepStepsPerYr - _oddStepsBefore);
            break;
        }
    }
}

void TimeSeries::TimeSeriesPrep::copyOddStepsBefore(const std::vector<double>& src,
                                                    std::vector<double>& prep,
                                                    int srcIx) {
    memcpy(&prep.data()[0], &src.data()[srcIx], _oddStepsBefore * sizeof(double));
}

void TimeSeries::TimeSeriesPrep::copyOddStepsAfter(const std::vector<double>& src,
                                                   std::vector<double>& prep,
                                                   int srcIx, int pix) {
    std::memcpy(&prep.data()[pix], &src.data()[srcIx], _oddStepsAfter * sizeof(double));
}

// Partial year at end of prep
void TimeSeries::TimeSeriesPrep::computeOddStepsAfter(const std::vector<double>& data,
                                                      std::vector<double>& prep) {
    poco_assert(_oddStepsAfter > 0);
    if (_oddStepsAfter <= 0) {
        return;
    }

    // PrepareTSComputeWholeYrsInterp, PrepareTSComputeWholeYrsBefore, and
    // PrepareTSComputeWholeYrsAfter must already have run (ie all whole years done)!
    int pix = _prepNSteps - _oddStepsAfter;

    // Interpolated
    if (_yr0 <= _prepEnYr && _prepEnYr <= _dataEnYr) {
        int dix = dixFromYr(data, _prepEnYr);
        interpOneYrWithCheck(data, prep, dix, pix, 0, _oddStepsAfter - 1);
    }
    else { // Extrapolated
        switch (_extrap) {
        case ExtrapType::NearestYr:
            if (_nWholeYrs >= 1) {
                copyOddStepsAfter(prep, prep, pix - _prepStepsPerYr, pix);
            }
            else {
                int dix = dixFromYr(data, _prepEnYr);
                interpOneYrWithCheck(data, prep, dix, pix, 0, _oddStepsAfter - 1);
            }
            break;
        case ExtrapType::CycleYrs:
            if (_nYrs <= _nWholeYrs) {
                copyOddStepsAfter(prep, prep, pix - _nYrs * _prepStepsPerYr, pix);
            }
            else {
                int dix = dixFromYr(data, _prepEnYr);
                interpOneYrWithCheck(data, prep, dix, pix, 0, _oddStepsAfter - 1);
            }
            break;
        case ExtrapType::AvgYr:
            copyOddStepsAfter(_avgPrep, prep, 0, pix);
            break;
        }
    }
}

void TimeSeries::TimeSeriesPrep::interpOneYr(const double* d, double* p, int prepIxLo, int prepIxHi) {
    // - For a full year's calculation (prepIxLo = 0, prepIxHi = prepStepsPerYr - 1):
    //     In:  d[0, .. , dataPerYr  - 1] 		<-- user's data
    //     Out: p[0, .. , prepStepsPerYr - 1]   <-- prepared time series
    //   Thus
    //		dataPerYr  input  blocks (block = step, effectively a "point")
    //		prepStepsPerYr output blocks
    //   On year calculated on its own, with no influence from other years.
    // - However in general we only want output blocks prepIxLo .. prepIxHi, which are put in
    //		p[0, .. , prepIxHi - prepIxLo]
    // - Relatively slow, so minimize calls to this routine.
    // - For each output block:
    //    * The part of [0, dataPerYr] that is covered by the output block is [st, en]
    //    * The input block at st is stIx, at en is enIx.
    //    * Output block value = (Sum over each input block that falls during the output block) of
    //			Height of input block * Fraction of input block that is during the output block
    // - st and en are calculated by multiplication each output step, so htere is no accumulation
    //   of roundoff error as might occur if en was calculated by adding to the previous value.
    // - kEpsilon counters roundoff error in conversion from flo to int by boosting st and en.
    // - st and en are in [0..10,000], less than 10^4. Double prescion, so has 15 significant
    //   figures. So set kEpsilon safely more than 10-^(15 - 4).
    poco_assert(prepIxHi >= prepIxLo);
    poco_assert(prepIxHi <= prepIxLo + _prepStepsPerYr - 1);
    const double kEpsilon = 0.0000000001;
    const double kEpsilonPlus = 0.0000000002;	// 50% to 100% more than kEpsilon

    double nInBlocksPerOutBlock = static_cast<double>(_dataPerYr) / static_cast<double>(_prepStepsPerYr);
    double nOutBlocksPerInBlock = 1.0 / nInBlocksPerOutBlock;

    int n = prepIxHi - prepIxLo;
    double st0 = prepIxLo * nInBlocksPerOutBlock + kEpsilon;
    double st = st0 + (n + 1) * nInBlocksPerOutBlock;
    double en;
    for (int i = n; i >= 0; --i) {
        en = st;
        st = st0 + i * nInBlocksPerOutBlock;
        int stIx = static_cast<int>(st);
        int enIx = static_cast<int>(en);
        if (enIx >= _dataPerYr)
            enIx = _dataPerYr - 1;
        poco_assert(0 <= stIx && stIx < _dataPerYr);
        poco_assert(0 <= enIx && enIx < _dataPerYr);
        double sum;
        if (stIx == enIx)							// No interior input blocks
            sum = d[stIx] * nInBlocksPerOutBlock;
        else {										// Some interior input blocks
            double stDiff = st - static_cast<double>(stIx) - kEpsilonPlus;
            if (stDiff > 0.0) {						// - Partial input block to left of interior
                sum = d[stIx] * (1.0 - stDiff);
                ++stIx;
            }
            else
                sum = 0.0;
            for (int j = stIx; j < enIx; ++j)		// - Whole input blocks in interior
                sum += d[j];
            double enDiff = en - static_cast<double>(enIx) - kEpsilonPlus;
            if (enDiff > 0.0) 						// - Partial input block to right of interior
                sum += d[enIx] * enDiff;
        }
        if (_subSame) sum *= nOutBlocksPerInBlock;
        p[i] = sum;
    }
}

void TimeSeries::TimeSeriesPrep::interpOneYrWithCheck(const std::vector<double>& data,
                                                      std::vector<double>& prep,
                                                      int dix, int pix,
                                                      int prepIxLo, int prepIxHi) {
    interpOneYr(&data.data()[dix], &prep.data()[pix], prepIxLo, prepIxHi);
}

int TimeSeries::TimeSeriesPrep::dixFromYr(const std::vector<double>& data, int yr) {
    // - Returns dix (data index) for the data for the simulation year yr.
    // - Does not apply if tExtrapTS = AvgYr.
    // yr is before user data begins
    if (yr <= _yr0) {
        switch (_extrap) {
        case ExtrapType::NearestYr:	return 0;
        case ExtrapType::CycleYrs: {
            int corresYr = (yr - _yr0) % _nYrs;
            if (corresYr < 0)
                corresYr += _nYrs;
            return _dataPerYr * corresYr;
        }
        case ExtrapType::AvgYr:		return 0;
        }
    }

    // yr is after user data ends
    if (yr >= _dataEnYr) {
        switch (_extrap) {
        case ExtrapType::NearestYr:	return static_cast<int>(data.size()) - _dataPerYr;
        case ExtrapType::CycleYrs:	return _dataPerYr * ((yr - _yr0) % _nYrs);
        case ExtrapType::AvgYr:		return 0;
        }
    }

    // yr is during user data
    return _dataPerYr * (yr - _yr0);
}

} // namespace cbm
} // namespace modules
} // namespace moja