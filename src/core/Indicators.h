/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

typedef Cacheable<SeriesAbstr> CacheableSeries;
typedef std::auto_ptr<CacheableSeries> CacheableSeriesPtr;

class IndicatorCacheable : public CacheableSeries {
 public:
  IndicatorCacheable(SeriesAbstr* series, const Id& id)
      : CacheableSeries(series, id) {}
};

template <class T>
class CacheableBuilderY : public CacheableBuilder<T> {
 private:
  const Id _id;

 public:
  CacheableBuilderY(const Id& id) : _id(id) {}
  virtual bool isConsistent(const Cacheable<T>& cacheable) const {
    return true;
  }

 public:
  virtual const Id& id() const { return _id; }
};

typedef CacheableBuilderY<SeriesAbstr> CacheableBuilderX;

class MakeFromSeries : public CacheableBuilderX {
 private:
  const SeriesImpl& _series;

 protected:
  MakeFromSeries(const SeriesImpl& series, const Id& id)
      : _series(series), CacheableBuilderX(id) {}

  const SeriesImpl& getSeries() const { return _series; }
};

class MakeFromSeriesWithOnePeriod : public MakeFromSeries {
 private:
  const unsigned int _period;

 public:
  MakeFromSeriesWithOnePeriod(const SeriesImpl& series, unsigned int period,
                              const std::string& name)
      : MakeFromSeries(series, calculateId(series, period, name)),
        _period(period) {}

 protected:
  const Id calculateId(const SeriesImpl& series, unsigned int period,
                       const std::string& name) {
    std::ostringstream o;
    o << series.getId() << " - " << name << " - " << period;

    return Id(o.str());
  }

  unsigned int getPeriod() const { return _period; }
};

class MakeFromBars : public CacheableBuilderX {
 private:
  const BarsImpl& _bars;

 protected:
  MakeFromBars(const BarsImpl& bars, const Id& id)
      : _bars(bars), CacheableBuilderX(id) {}

  const BarsImpl& getBars() const { return _bars; }
};

class MakeFromBarsWithOnePeriod : public MakeFromBars {
 private:
  const unsigned int _period;

 public:
  MakeFromBarsWithOnePeriod(const BarsImpl& bars, unsigned int period,
                            const std::string& name)
      : MakeFromBars(bars, calculateId(bars, period, name)), _period(period) {}

 protected:
  const Id calculateId(const BarsImpl& bars, unsigned int period,
                       const std::string& name) {
    std::ostringstream o;
    o << bars.getId() << " - " << name << " - " << period;

    return Id(o.str());
  }

  unsigned int getPeriod() const { return _period; }
};

typedef int (*TA_LOOKBACK_INT)(int);
typedef TA_RetCode (*TA_FUNC1)(int, int, const double[], int, int*, int*,
                               double[]);
typedef TA_RetCode (*TA_FUNC1Int)(int, int, const double[], int, int*, int*,
                                  int[]);

template <TA_FUNC1 T>
class TAFunc1Constants {
 public:
  static const char* _id;
};

template <>
const char* TAFunc1Constants<TA_MAX>::_id = "Max";
template <>
const char* TAFunc1Constants<TA_MIN>::_id = "Min";
template <>
const char* TAFunc1Constants<TA_DEMA>::_id = "DEMA";
template <>
const char* TAFunc1Constants<TA_KAMA>::_id = "KAMA";
template <>
const char* TAFunc1Constants<TA_MIDPOINT>::_id = "Mid Point";
template <>
const char* TAFunc1Constants<TA_TEMA>::_id = "TEMA";
template <>
const char* TAFunc1Constants<TA_TRIMA>::_id = "TRIMA";
template <>
const char* TAFunc1Constants<TA_MOM>::_id = "MOM";
template <>
const char* TAFunc1Constants<TA_ROC>::_id = "ROC";
template <>
const char* TAFunc1Constants<TA_ROCP>::_id = "ROCP";
template <>
const char* TAFunc1Constants<TA_ROCR>::_id = "ROCR";
template <>
const char* TAFunc1Constants<TA_ROCR100>::_id = "ROCR100";
template <>
const char* TAFunc1Constants<TA_RSI>::_id = "RSI";
template <>
const char* TAFunc1Constants<TA_TRIX>::_id = "TRIX";
template <>
const char* TAFunc1Constants<TA_LINEARREG>::_id = "Linear Regression";
template <>
const char* TAFunc1Constants<TA_LINEARREG_SLOPE>::_id =
    "Linear Regression Slope";
template <>
const char* TAFunc1Constants<TA_LINEARREG_ANGLE>::_id =
    "Linear Regression Angle";
template <>
const char* TAFunc1Constants<TA_LINEARREG_INTERCEPT>::_id =
    "Linear Regression Intercept";
template <>
const char* TAFunc1Constants<TA_TSF>::_id = "Time Series Forecast";
template <>
const char* TAFunc1Constants<TA_CMO>::_id = "Chande Momentum Oscillator";

template <TA_FUNC1Int T>
class TAFunc1IntConstants {
 public:
  static const char* _id;
};

template <>
const char* TAFunc1IntConstants<TA_MAXINDEX>::_id = "Max index";
template <>
const char* TAFunc1IntConstants<TA_MININDEX>::_id = "Min index";

template <TA_LOOKBACK_INT TA_LB, TA_FUNC1 TA_FUNC>
class MakeSeriesTAFunc1 : public MakeFromSeriesWithOnePeriod {
 private:
  /**
   * Simple Moving Average - calculated iteratively
   */
  // MAX value over a period
  class XSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    XSeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_LB(period)) {
        int begIdx;
        int nbElement;
        TA_FUNC(0, unsyncSize() - 1, series.getArray(), period, &begIdx,
                &nbElement, &at(TA_LB(period)));
      }
    }
  };

 public:
  MakeSeriesTAFunc1(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period,
                                    TAFunc1Constants<TA_FUNC>::_id) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new XSeries(getSeries(), getPeriod(), id()), id()));
  }
};

template <TA_LOOKBACK_INT TA_LB, TA_FUNC1Int TA_FUNC>
class MakeSeriesTAFunc1Int : public MakeFromSeriesWithOnePeriod {
 private:
  /**
   * Simple Moving Average - calculated iteratively
   */
  // MAX value over a period
  class XSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    XSeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_LB(period)) {
        vector<int> x(unsyncSize());

        int begIdx;
        int nbElement;
        TA_FUNC(0, unsyncSize() - 1, series.getArray(), period, &begIdx,
                &nbElement, &x.at(TA_LB(period)));

        std::copy(x.begin(), x.end(), _v.begin());
      }
    }
  };

 public:
  MakeSeriesTAFunc1Int(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period,
                                    TAFunc1IntConstants<TA_FUNC>::_id) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new XSeries(getSeries(), getPeriod(), id()), id()));
  }
};

typedef MakeSeriesTAFunc1Int<TA_MAXINDEX_Lookback, TA_MAXINDEX>
    MakeMaxIndexSeries;
typedef MakeSeriesTAFunc1Int<TA_MININDEX_Lookback, TA_MININDEX>
    MakeMinIndexSeries;

class MakeBBandUpperSeries : public MakeFromSeries {
 private:
  class BBandUpperSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    BBandUpperSeries(const SeriesImpl& series, unsigned int period,
                     double stdDev, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl median(unsyncSize(), SynchronizerPtr());
      SeriesImpl lower(unsyncSize(), SynchronizerPtr());
      unsigned int k = (unsigned int)TA_BBANDS_Lookback(period, stdDev, stdDev,
                                                        TA_MAType_SMA);
      if (unsyncSize() > k)
        TA_BBANDS(0, unsyncSize() - 1, series.getArray(), period, stdDev,
                  stdDev, TA_MAType_SMA, &begIdx, &nbElement, &at(k),
                  &median.at(k), &lower.at(k));
    }
  };

 private:
  const unsigned int _period;
  const double _stdDev;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double stdDev) {
    std::ostringstream o;
    o << series.getId() << " - BBand upper - " << period << ", " << stdDev;

    return Id(o.str());
  }

 public:
  MakeBBandUpperSeries(const SeriesImpl& series, unsigned int period,
                       double stdDev)
      : MakeFromSeries(series, calculateId(series, period, stdDev)),
        _period(period),
        _stdDev(stdDev) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new BBandUpperSeries(getSeries(), _period, _stdDev, id()), id()));
  }
};

class MakeBBandLowerSeries : public MakeFromSeries {
 private:
  class BBandLowerSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    BBandLowerSeries(const SeriesImpl& series, unsigned int period,
                     double stdDev, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl median(unsyncSize(), SynchronizerPtr());
      SeriesImpl upper(unsyncSize(), SynchronizerPtr());
      unsigned int k = (unsigned int)TA_BBANDS_Lookback(period, stdDev, stdDev,
                                                        TA_MAType_SMA);
      if (unsyncSize() > k)
        TA_BBANDS(0, unsyncSize() - 1, series.getArray(), period, stdDev,
                  stdDev, TA_MAType_SMA, &begIdx, &nbElement, &upper.at(k),
                  &median.at(k), &at(k));
    }
  };

 private:
  const unsigned int _period;
  const double _stdDev;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double stdDev) {
    std::ostringstream o;
    o << series.getId() << " - BBand lower - " << period << ", " << stdDev;

    return Id(o.str());
  }

 public:
  MakeBBandLowerSeries(const SeriesImpl& series, unsigned int period,
                       double stdDev)
      : MakeFromSeries(series, calculateId(series, period, stdDev)),
        _period(period),
        _stdDev(stdDev) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new BBandLowerSeries(getSeries(), _period, _stdDev, id()), Id()));
  }
};

class MakeEMASeries : public MakeFromSeries {
 private:
  /**
   * Exponential Moving Average
   */
  class EMASeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires

    EMASeries(const SeriesAbstr& series, unsigned int period, double exp,
              const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      // function_requires< SeriesImpl< T > >();

      if (period < unsyncSize()) {
        double f = 0;

        const std::vector<double>& v(series.getVector());

        // TODO: use std::accumulate
        unsigned int n = 0;
        for (; n < period; n++) f += v[n];

        at(period - 1) = f / (double)period;

        for (; n < unsyncSize(); n++) {
          double x = exp * (v[n] - at(n - 1)) + at(n - 1);
          at(n) = x;
        }
      }
    }
  };

 private:
  unsigned int _period;
  double _exp;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double exp) {
    std::ostringstream o;
    o << series.getId() << " - EMA - " << period << ", " << exp;

    return Id(o.str());
  }

 public:
  MakeEMASeries(const SeriesImpl& series, unsigned int period, double exp)
      : MakeFromSeries(series, calculateId(series, period, exp)),
        _period(period),
        _exp(exp) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new EMASeries(getSeries(), _period, _exp, id()), id()));
  }
};

class MakeFixedExpEMASeries : public MakeEMASeries {
 public:
  MakeFixedExpEMASeries(const SeriesImpl& series, unsigned int period)
      : MakeEMASeries(series, period, 2 / ((double)period + 1)) {}
};

class MakeHTTrendlineSeries : public MakeFromSeries {
 private:
  // Hilbert transform - Instantaneous Trendline
  class HTTrendlineSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTTrendlineSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_HT_TRENDLINE_Lookback())
        TA_HT_TRENDLINE(0, unsyncSize() - 1, series.getArray(), &begIdx,
                        &nbElement, &at(TA_HT_TRENDLINE_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HTTrendline");
  }

 public:
  MakeHTTrendlineSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTTrendlineSeries(getSeries(), id()), Id()));
  }
};

class MakeMAMASeries : public MakeFromSeries {
 private:
  // TODO: implement TA_MA (all moving average?)

  class MAMASeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MAMASeries(const SeriesImpl& series, double fastLimit, double slowLimit,
               const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl fama(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MAMA_Lookback(fastLimit, slowLimit);

      if (unsyncSize() > k)
        TA_MAMA(0, unsyncSize() - 1, series.getArray(), fastLimit, slowLimit,
                &begIdx, &nbElement, &at(k), &fama.at(k));
    }
  };

 private:
  const double _slowLimit;
  const double _fastLimit;

 private:
  static const Id calculateId(const SeriesImpl& series, double fastLimit,
                              double slowLimit) {
    std::ostringstream o;
    o << series.getId() << " - MAMA - " << fastLimit << ", " << slowLimit;

    return Id(o.str());
  }

 public:
  MakeMAMASeries(const SeriesImpl& series, double fastLimit, double slowLimit)
      : MakeFromSeries(series, calculateId(series, fastLimit, slowLimit)),
        _fastLimit(fastLimit),
        _slowLimit(slowLimit) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MAMASeries(getSeries(), _fastLimit, _slowLimit, id()), Id()));
  }
};

class MakeFAMASeries : public MakeFromSeries {
 private:
  // TODO: implement TA_MA (all moving average?)

  class FAMASeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    FAMASeries(const SeriesImpl& series, double fastLimit, double slowLimit,
               const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl mama(unsyncSize(), SynchronizerPtr());
      unsigned int k = (unsigned int)TA_MAMA_Lookback(fastLimit, slowLimit);

      if (unsyncSize() > k)
        TA_MAMA(0, unsyncSize() - 1, series.getArray(), fastLimit, slowLimit,
                &begIdx, &nbElement, &mama.at(k), &at(k));
    }
  };

 private:
  const double _slowLimit;
  const double _fastLimit;

 private:
  static const Id calculateId(const SeriesImpl& series, double fastLimit,
                              double slowLimit) {
    std::ostringstream o;
    o << series.getId() << " - FAMA - " << fastLimit << ", " << slowLimit;

    return Id(o.str());
  }

 public:
  MakeFAMASeries(const SeriesImpl& series, double fastLimit, double slowLimit)
      : MakeFromSeries(series, calculateId(series, fastLimit, slowLimit)),
        _fastLimit(fastLimit),
        _slowLimit(slowLimit) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new FAMASeries(getSeries(), _fastLimit, _slowLimit, id()), id()));
  }
};

typedef TA_RetCode (*TA_FUNC_HIGH_LOW)(int, int, const double[], const double[],
                                       int, int*, int*, double[]);
typedef TA_RetCode (*TA_FUNC_HIGH_LOW_CLOSE)(int, int, const double[],
                                             const double[], const double[],
                                             int, int*, int*, double[]);

template <TA_FUNC_HIGH_LOW T>
class TAFuncHighLowConstants {
 public:
  static const char* _id;
};

template <TA_FUNC_HIGH_LOW_CLOSE T>
class TAFuncHighLowCloseConstants {
 public:
  static const char* _id;
};

template <>
const char* TAFuncHighLowConstants<TA_MIDPRICE>::_id = "Mid Price";
template <>
const char* TAFuncHighLowConstants<TA_MINUS_DM>::_id = "Minus DM";
template <>
const char* TAFuncHighLowConstants<TA_PLUS_DM>::_id = "Plus DM";

template <>
const char* TAFuncHighLowCloseConstants<TA_ATR>::_id = "ATR";
template <>
const char* TAFuncHighLowCloseConstants<TA_ADX>::_id = "ADX";
template <>
const char* TAFuncHighLowCloseConstants<TA_ADXR>::_id = "ADXR";
template <>
const char* TAFuncHighLowCloseConstants<TA_CCI>::_id = "CCI";
template <>
const char* TAFuncHighLowCloseConstants<TA_DX>::_id = "DX";
template <>
const char* TAFuncHighLowCloseConstants<TA_MINUS_DI>::_id = "Minus DI";
template <>
const char* TAFuncHighLowCloseConstants<TA_PLUS_DI>::_id = "Plus DI";
template <>
const char* TAFuncHighLowCloseConstants<TA_WILLR>::_id = "Williams R";
template <>
const char* TAFuncHighLowCloseConstants<TA_NATR>::_id =
    "Normalized Average True Range";

template <TA_LOOKBACK_INT TA_LB, TA_FUNC_HIGH_LOW TA_FUNC>
class MakeBarsHighLowSeries : public MakeFromBarsWithOnePeriod {
 private:
  class XSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    XSeries(const BarsImpl& bars, unsigned int period, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int begIdx;
      int nbElement;
      const TA_Real* inHigh = bars.highSeries().getArray();
      const TA_Real* inLow = bars.lowSeries().getArray();

      if (unsyncSize() > (unsigned int)TA_LB(period))
        TA_FUNC(0, unsyncSize() - 1, inHigh, inLow, period, &begIdx, &nbElement,
                &at(TA_LB(period)));
    }
  };

 public:
  MakeBarsHighLowSeries(const BarsImpl& bars, unsigned int period)
      : MakeFromBarsWithOnePeriod(bars, period,
                                  TAFuncHighLowConstants<TA_FUNC>::_id) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new XSeries(getBars(), getPeriod(), id()), id()));
  }
};

class MakeSARSeries : public MakeFromBars {
 private:
  class SARSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    SARSeries(const BarsImpl& bars, double acceleration, double maximum,
              const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int begIdx;
      int nbElement;
      const TA_Real* inHigh = bars.highSeries().getArray();
      const TA_Real* inLow = bars.lowSeries().getArray();

      if (unsyncSize() > (unsigned int)TA_SAR_Lookback(acceleration, maximum))
        TA_SAR(0, unsyncSize() - 1, inHigh, inLow, acceleration, maximum,
               &begIdx, &nbElement,
               &at(TA_SAR_Lookback(acceleration, maximum)));
    }
  };

 private:
  const double _acceleration;
  const double _maximum;

 private:
  static const Id calculateId(const BarsImpl& bars, double acceleration,
                              double maximum) {
    std::ostringstream o;
    o << bars.getId() << " - SAR - " << acceleration << ", " << maximum;

    return Id(o.str());
  }

 public:
  MakeSARSeries(const BarsImpl& bars, double acceleration, double maximum)
      : MakeFromBars(bars, calculateId(bars, acceleration, maximum)),
        _acceleration(acceleration),
        _maximum(maximum) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new SARSeries(getBars(), _acceleration, _maximum, id()), id()));
  }
};

class MakeSMASeries : public MakeFromSeriesWithOnePeriod {
 private:
  /**
   * Simple Moving Average - calculated iteratively
   */
  class SMASeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    SMASeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      // function_requires< SeriesImpl< T > >();

      if (period < unsyncSize()) {
        double f = 0;

        const std::vector<double>& v(series.getVector());

        // TODO: use std::accumulate
        unsigned int n = 0;
        for (; n < period; n++) f += v[n];

        at(period - 1) = f / (double)period;

        for (; n < unsyncSize(); n++)
          at(n) = at(n - 1) + (v[n] - v[n - period]) / (double)period;
      }
    }
  };

 public:
  MakeSMASeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "SMA") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new SMASeries(getSeries(), getPeriod(), id()), id()));
  }
};

class MakeT3Series : public MakeFromSeries {
 private:
  // Triple exponential moving average (T3)
  class T3Series : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    T3Series(const SeriesImpl& series, unsigned int period, double vFactor,
             const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_T3_Lookback(period, vFactor))
        TA_T3(0, unsyncSize() - 1, series.getArray(), period, vFactor, &begIdx,
              &nbElement, &at(TA_T3_Lookback(period, vFactor)));
    }
  };

 private:
  const unsigned int _period;
  const double _vFactor;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double vFactor) {
    std::ostringstream o;
    o << series.getId() << " - T3 - " << period << ", " << vFactor;

    return Id(o.str());
  }

 public:
  MakeT3Series(const SeriesImpl& series, unsigned int period, double vFactor)
      : MakeFromSeries(series, calculateId(series, period, vFactor)),
        _period(period),
        _vFactor(vFactor) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new T3Series(getSeries(), _period, _vFactor, id()), id()));
  }
};

class MakeWMASeries : public MakeFromSeries {
 private:
  // WLD has a bug - the series starts one index later than it should,
  // for example for period 3, it starts at index 3 (it should start at 2)
  class WMASeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    WMASeries(const SeriesImpl& series, unsigned int period, SeriesAbstrPtr sma,
              const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      // function_requires< SeriesImpl< T > >();

      assert(sma);

      if (period < unsyncSize()) {
        double f = 0;

        const std::vector<double>& v(series.getVector());
        const std::vector<double>& vsma(sma->getVector());

        assert(v.size() == vsma.size() && v.size() == series.unsyncSize());

        // TODO: use std::accumulate
        double sigma = (period + 1) * period / 2;
        unsigned int n = 0;
        for (; n < period; n++) f += (n + 1) * v[n];

        at(period - 1) = f / sigma;

        for (; n < unsyncSize(); n++) {
          double x = at(n - 1) - vsma[n - 1] * (float)period / sigma +
                     period * v[n] / sigma;
          at(n) = x;
        }
      }
    }
  };

 private:
  unsigned int _period;
  SeriesAbstrPtr _sma;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period) {
    std::ostringstream o;
    o << series.getId() << " - WMA - " << period;

    return Id(o.str());
  }

 public:
  MakeWMASeries(const SeriesImpl& series, unsigned int period,
                SeriesAbstrPtr sma)
      : MakeFromSeries(series, calculateId(series, period)),
        _period(period),
        _sma(sma) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new WMASeries(getSeries(), _period, _sma, id()), id()));
  }
};

template <TA_LOOKBACK_INT TA_LB, TA_FUNC_HIGH_LOW_CLOSE TA_FUNC>
class MakeBarsHighLowCloseSeries : public MakeFromBarsWithOnePeriod {
 private:
  // ATR - average true range
  class XSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    XSeries(const BarsImpl& bars, unsigned int period, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int begIdx;
      int nbElement;
      const TA_Real* inHigh = bars.highSeries().getArray();
      const TA_Real* inLow = bars.lowSeries().getArray();
      const TA_Real* inClose = bars.closeSeries().getArray();

      if (unsyncSize() > (unsigned int)TA_LB(period))
        TA_FUNC(0, unsyncSize() - 1, inHigh, inLow, inClose, period, &begIdx,
                &nbElement, &at(TA_LB(period)));
    }
  };

 public:
  MakeBarsHighLowCloseSeries(const BarsImpl& bars, unsigned int period)
      : MakeFromBarsWithOnePeriod(bars, period,
                                  TAFuncHighLowCloseConstants<TA_FUNC>::_id) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new XSeries(getBars(), getPeriod(), id()), id()));
  }
};

class MakeTRSeries : public MakeFromBars {
 private:
  // TR - true range
  class TRangeSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    TRangeSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int begIdx;
      int nbElement;
      const TA_Real* inHigh = bars.highSeries().getArray();
      const TA_Real* inLow = bars.lowSeries().getArray();
      const TA_Real* inClose = bars.closeSeries().getArray();

      if (unsyncSize() > (unsigned int)TA_TRANGE_Lookback())
        TA_TRANGE(0, unsyncSize() - 1, inHigh, inLow, inClose, &begIdx,
                  &nbElement, &at(TA_TRANGE_Lookback()));
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    return Id(bars.getId() + " - True Range");
  }

 public:
  MakeTRSeries(const BarsImpl& bars) : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new TRangeSeries(getBars(), id()), id()));
  }
};

class MakeAPOSeries : public MakeFromSeries {
 private:
  // TA_APO - Absolute Price Oscillator
  // TODO: what to do with the MAType?
  class APOSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    APOSeries(const SeriesImpl& series, unsigned int fastPeriod,
              unsigned int slowPeriod, MAType maType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_APO_Lookback(fastPeriod, slowPeriod,
                                                       (TA_MAType)maType))
        TA_APO(0, unsyncSize() - 1, series.getArray(), fastPeriod, slowPeriod,
               (TA_MAType)maType, &begIdx, &nbElement,
               &at(TA_APO_Lookback(fastPeriod, slowPeriod, (TA_MAType)maType)));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const unsigned int _slowPeriod;
  const MAType _maType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              unsigned int slowPeriod, MAType maType) {
    std::ostringstream o;
    o << series.getId() << " - APO - " << fastPeriod << ", " << slowPeriod
      << ", " << maType;

    return Id(o.str());
  }

 public:
  MakeAPOSeries(const SeriesImpl& series, unsigned int fastPeriod,
                unsigned int slowPeriod, MAType maType)
      : MakeFromSeries(series,
                       calculateId(series, fastPeriod, slowPeriod, maType)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod),
        _maType(maType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new APOSeries(getSeries(), _fastPeriod, _slowPeriod, _maType, id()),
        id()));
  }
};

class MakeAroonDownSeries : public MakeFromSeriesWithOnePeriod {
 private:
  // TODO: optimize these series
  class AroonDownSeries : public SeriesImpl {
   public:
    AroonDownSeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int x;
      for (unsigned int n = period; n < unsyncSize(); n++) {
        x = period - n + series.minIndexInInterval(n - period, period + 1);
        at(n) = x < 0 ? 0 : x * 100 / period;
      }
    }
  };

 public:
  MakeAroonDownSeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "Aroon Down") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new AroonDownSeries(getSeries(), getPeriod(), id()), id()));
  }
};

class MakeAroonUpSeries : public MakeFromSeriesWithOnePeriod {
 private:
  // TODO: optimize these series
  class AroonUpSeries : public SeriesImpl {
   public:
    AroonUpSeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int x;
      for (unsigned int n = period; n < unsyncSize(); n++) {
        x = period - n + series.maxIndexInInterval(n - period, period + 1);
        at(n) = x < 0 ? 0 : x * 100 / period;
      }
    }
  };

 public:
  MakeAroonUpSeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "Aroon Up") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new AroonUpSeries(getSeries(), getPeriod(), id()), id()));
  }
};

// TODO: Aroon oscillator

/*class AroonOscillatorSeries : public SeriesImpl
{
  friend class SeriesManager;
private:
  // TODO: enforce that T is of type series using function_requires
  AroonOscillatorSeries( const SeriesImpl& series, unsigned int period )
  : SeriesImpl( series.unsyncSize(), calculateId(series,period ) )
  {
    const Series* down = dm.getAroonDownSeries(  series, period );
    const Series* up = dm.getAroonUpSeries( series, period );
    // subtract series down from up and put the result in this
    dm.subtractSeries( *up, *down, *this );
  }

  static const Id calculateId( const SeriesImpl& series, unsigned int period )
  {
    std::ostringstream o;
    o << period;

    return Id( series.getId() + " - Aroon Oscillator - " ) + o.str() ;
  }

public:
  virtual ~AroonOscillatorSeries()
  {
  }
};
*/

class MakeMACDSeries : public MakeFromSeries {
 private:
  // MACD - moving average convergence/divergence
  // TODO: use the cache to calculate these only once
  class MACDSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDSeries(const SeriesImpl& series, unsigned int fastPeriod,
               unsigned int slowPeriod, unsigned int signalPeriod, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k =
          (unsigned int)TA_MACD_Lookback(fastPeriod, slowPeriod, signalPeriod);

      if (unsyncSize() > k)
        TA_MACD(0, unsyncSize() - 1, series.getArray(), fastPeriod, slowPeriod,
                signalPeriod, &begIdx, &nbElement, &at(k), &(macdSignal.at(k)),
                &(macdHist.at(k)));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const unsigned int _slowPeriod;
  const unsigned int _signalPeriod;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              unsigned int slowPeriod,
                              unsigned int signalPeriod) {
    std::ostringstream o;
    o << series.getId() << " - MACD - " << fastPeriod << ", " << slowPeriod
      << ", " << signalPeriod;

    return Id(o.str());
  }

 public:
  MakeMACDSeries(const SeriesImpl& series, unsigned int fastPeriod,
                 unsigned int slowPeriod, unsigned int signalPeriod)
      : MakeFromSeries(
            series, calculateId(series, fastPeriod, slowPeriod, signalPeriod)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod),
        _signalPeriod(signalPeriod) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new MACDSeries(getSeries(), _fastPeriod,
                                              _slowPeriod, _signalPeriod, id()),
                               id()));
  }
};

class MakeMACDSignalSeries : public MakeFromSeries {
 private:
  // MACD - moving average convergence/divergence signal
  // TODO: what is this?
  // TODO: use the cache to calculate these only once
  class MACDSignalSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDSignalSeries(const SeriesImpl& series, unsigned int fastPeriod,
                     unsigned int slowPeriod, unsigned int signalPeriod,
                     const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k =
          (unsigned int)TA_MACD_Lookback(fastPeriod, slowPeriod, signalPeriod);

      if (unsyncSize() > k)
        TA_MACD(0, unsyncSize() - 1, series.getArray(), fastPeriod, slowPeriod,
                signalPeriod, &begIdx, &nbElement, &(macd.at(k)), &at(k),
                &(macdHist.at(k)));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const unsigned int _slowPeriod;
  const unsigned int _signalPeriod;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              unsigned int slowPeriod,
                              unsigned int signalPeriod) {
    std::ostringstream o;
    o << series.getId() << " - MACD Signal - " << fastPeriod << ", "
      << slowPeriod << ", " << signalPeriod;

    return Id(o.str());
  }

 public:
  MakeMACDSignalSeries(const SeriesImpl& series, unsigned int fastPeriod,
                       unsigned int slowPeriod, unsigned int signalPeriod)
      : MakeFromSeries(
            series, calculateId(series, fastPeriod, slowPeriod, signalPeriod)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod),
        _signalPeriod(signalPeriod) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDSignalSeries(getSeries(), _fastPeriod, _slowPeriod,
                             _signalPeriod, id()),
        id()));
  }
};

class MakeMACDHistSeries : public MakeFromSeries {
 private:
  // MACD hist - moving average convergence/divergence hist
  // TODO: is this histogram?
  // TODO: use the cache to calculate these only once
  class MACDHistSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDHistSeries(const SeriesImpl& series, unsigned int fastPeriod,
                   unsigned int slowPeriod, unsigned int signalPeriod,
                   const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());

      unsigned int k =
          (unsigned int)TA_MACD_Lookback(fastPeriod, slowPeriod, signalPeriod);

      if (unsyncSize() > k)
        TA_MACD(0, unsyncSize() - 1, series.getArray(), fastPeriod, slowPeriod,
                signalPeriod, &begIdx, &nbElement, &(macd.at(k)),
                &(macdSignal.at(k)), &at(k));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const unsigned int _slowPeriod;
  const unsigned int _signalPeriod;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              unsigned int slowPeriod,
                              unsigned int signalPeriod) {
    std::ostringstream o;
    o << series.getId() << " - MACD Hist - " << fastPeriod << ", " << slowPeriod
      << ", " << signalPeriod;

    return Id(o.str());
  }

 public:
  MakeMACDHistSeries(const SeriesImpl& series, unsigned int fastPeriod,
                     unsigned int slowPeriod, unsigned int signalPeriod)
      : MakeFromSeries(
            series, calculateId(series, fastPeriod, slowPeriod, signalPeriod)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod),
        _signalPeriod(signalPeriod) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDHistSeries(getSeries(), _fastPeriod, _slowPeriod, _signalPeriod,
                           id()),
        id()));
  }
};

class MakeMACDExtSeries : public MakeFromSeries {
 private:
  // MACDExt - moving average convergence/divergence with controllable MA type
  // TODO: use the cache to calculate these only once
  class MACDExtSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                  MAType fastMAType, unsigned int slowPeriod, MAType slowMAType,
                  unsigned int signalPeriod, MAType signalMAType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDEXT_Lookback(
          fastPeriod, (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
          signalPeriod, (TA_MAType)signalMAType);

      if (unsyncSize() > k)
        TA_MACDEXT(0, unsyncSize() - 1, series.getArray(), fastPeriod,
                   (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
                   signalPeriod, (TA_MAType)signalMAType, &begIdx, &nbElement,
                   &at(k), &(macdSignal.at(k)), &(macdHist.at(k)));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const MAType _fastMAType;
  const unsigned int _slowPeriod;
  const MAType _slowMAType;
  const unsigned int _signalPeriod;
  const MAType _signalMAType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              MAType fastMAType, unsigned int slowPeriod,
                              MAType slowMAType, unsigned int signalPeriod,
                              MAType signalMAType) {
    std::ostringstream o;
    o << series.getId() << " - MACD Ext- " << fastPeriod << ", " << fastMAType
      << ", " << slowPeriod << ", " << slowMAType << ", " << signalPeriod
      << ", " << signalMAType;

    return Id(o.str());
  }

 public:
  MakeMACDExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                    MAType fastMAType, unsigned int slowPeriod,
                    MAType slowMAType, unsigned int signalPeriod,
                    MAType signalMAType)
      : MakeFromSeries(series,
                       calculateId(series, fastPeriod, fastMAType, slowPeriod,
                                   slowMAType, signalPeriod, signalMAType)),
        _fastPeriod(fastPeriod),
        _fastMAType(fastMAType),
        _slowPeriod(slowPeriod),
        _slowMAType(slowMAType),
        _signalPeriod(signalPeriod),
        _signalMAType(signalMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDExtSeries(getSeries(), _fastPeriod, _fastMAType, _slowPeriod,
                          _slowMAType, _signalPeriod, _signalMAType, id()),
        id()));
  }
};

class MakeMACDSignalExtSeries : public MakeFromSeries {
 private:
  // MACDExt - moving average convergence/divergence with controllable MA type
  // TODO: use the cache to calculate these only once
  // MACDExt - moving average convergence/divergence with controllable MA type
  // TODO: use the cache to calculate these only once
  class MACDSignalExtSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDSignalExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                        MAType fastMAType, unsigned int slowPeriod,
                        MAType slowMAType, unsigned int signalPeriod,
                        MAType signalMAType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDEXT_Lookback(
          fastPeriod, (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
          signalPeriod, (TA_MAType)signalMAType);

      if (unsyncSize() > k)
        TA_MACDEXT(0, unsyncSize() - 1, series.getArray(), fastPeriod,
                   (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
                   signalPeriod, (TA_MAType)signalMAType, &begIdx, &nbElement,
                   &(macd.at(k)), &at(k), &(macdHist.at(k)));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const MAType _fastMAType;
  const unsigned int _slowPeriod;
  const MAType _slowMAType;
  const unsigned int _signalPeriod;
  const MAType _signalMAType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              MAType fastMAType, unsigned int slowPeriod,
                              MAType slowMAType, unsigned int signalPeriod,
                              MAType signalMAType) {
    std::ostringstream o;
    o << series.getId() << " - MACD Signal Ext - " << fastPeriod << ", "
      << fastMAType << ", " << slowPeriod << ", " << slowMAType << ", "
      << signalPeriod << ", " << signalMAType;

    return Id(o.str());
  }

 public:
  MakeMACDSignalExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                          MAType fastMAType, unsigned int slowPeriod,
                          MAType slowMAType, unsigned int signalPeriod,
                          MAType signalMAType)
      : MakeFromSeries(series,
                       calculateId(series, fastPeriod, fastMAType, slowPeriod,
                                   slowMAType, signalPeriod, signalMAType)),
        _fastPeriod(fastPeriod),
        _fastMAType(fastMAType),
        _slowPeriod(slowPeriod),
        _slowMAType(slowMAType),
        _signalPeriod(signalPeriod),
        _signalMAType(signalMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDSignalExtSeries(getSeries(), _fastPeriod, _fastMAType,
                                _slowPeriod, _slowMAType, _signalPeriod,
                                _signalMAType, id()),
        id()));
  }
};

class MakeMACDHistExtSeries : public MakeFromSeries {
 private:
  // MACDExt - moving average convergence/divergence with controllable MA type
  // TODO: use the cache to calculate these only once
  class MACDHistExtSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDHistExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                      MAType fastMAType, unsigned int slowPeriod,
                      MAType slowMAType, unsigned int signalPeriod,
                      MAType signalMAType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDEXT_Lookback(
          fastPeriod, (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
          signalPeriod, (TA_MAType)signalMAType);

      if (unsyncSize() > k)
        TA_MACDEXT(0, unsyncSize() - 1, series.getArray(), fastPeriod,
                   (TA_MAType)fastMAType, slowPeriod, (TA_MAType)slowMAType,
                   signalPeriod, (TA_MAType)signalMAType, &begIdx, &nbElement,
                   &(macd.at(k)), &(macdSignal.at(k)), &at(k));
    }
  };

 private:
  const unsigned int _fastPeriod;
  const MAType _fastMAType;
  const unsigned int _slowPeriod;
  const MAType _slowMAType;
  const unsigned int _signalPeriod;
  const MAType _signalMAType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              MAType fastMAType, unsigned int slowPeriod,
                              MAType slowMAType, unsigned int signalPeriod,
                              MAType signalMAType) {
    std::ostringstream o;
    o << series.getId() << " - MACD Hist Ext- " << fastPeriod << ", "
      << fastMAType << ", " << slowPeriod << ", " << slowMAType << ", "
      << signalPeriod << ", " << signalMAType;

    return Id(o.str());
  }

 public:
  MakeMACDHistExtSeries(const SeriesImpl& series, unsigned int fastPeriod,
                        MAType fastMAType, unsigned int slowPeriod,
                        MAType slowMAType, unsigned int signalPeriod,
                        MAType signalMAType)
      : MakeFromSeries(series,
                       calculateId(series, fastPeriod, fastMAType, slowPeriod,
                                   slowMAType, signalPeriod, signalMAType)),
        _fastPeriod(fastPeriod),
        _fastMAType(fastMAType),
        _slowPeriod(slowPeriod),
        _slowMAType(slowMAType),
        _signalPeriod(signalPeriod),
        _signalMAType(signalMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDHistExtSeries(getSeries(), _fastPeriod, _fastMAType,
                              _slowPeriod, _slowMAType, _signalPeriod,
                              _signalMAType, id()),
        id()));
  }
};

class MakeMACDFixSeries : public MakeFromSeriesWithOnePeriod {
 private:
  // MACD - moving average convergence/divergence Fix 12/26
  // TODO: use the cache to calculate these only once
  class MACDFixSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDFixSeries(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDFIX_Lookback(period);

      if (unsyncSize() > k)
        TA_MACDFIX(0, unsyncSize() - 1, series.getArray(), period, &begIdx,
                   &nbElement, &at(k), &(macdSignal.at(k)), &(macdHist.at(k)));
    }
  };

 public:
  MakeMACDFixSeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "MACD Fix") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDFixSeries(getSeries(), getPeriod(), id()), id()));
  }
};

class MakeMACDSignalFixSeries : public MakeFromSeriesWithOnePeriod {
 private:
  // MACD - moving average convergence/divergence signal Fix 12/26
  // TODO: what is this?
  // TODO: use the cache to calculate these only once
  class MACDSignalFixSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDSignalFixSeries(const SeriesImpl& series, unsigned int period,
                        const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdHist(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDFIX_Lookback(period);

      if (unsyncSize() > k)
        TA_MACDFIX(0, unsyncSize() - 1, series.getArray(), period, &begIdx,
                   &nbElement, &(macd.at(k)), &at(k), &(macdHist.at(k)));
    }
  };

 public:
  MakeMACDSignalFixSeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "MACD Signal Fix") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDSignalFixSeries(getSeries(), getPeriod(), id()), id()));
  }
};

class MakeMACDHistFixSeries : public MakeFromSeriesWithOnePeriod {
 private:
  // MACD hist - moving average convergence/divergence hist Fix 12/26
  // TODO: is this histogram?
  // TODO: use the cache to calculate these only once
  class MACDHistFixSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MACDHistFixSeries(const SeriesImpl& series, unsigned int period,
                      const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl macd(unsyncSize(), SynchronizerPtr());
      SeriesImpl macdSignal(unsyncSize(), SynchronizerPtr());

      unsigned int k = (unsigned int)TA_MACDFIX_Lookback(period);

      if (unsyncSize() > k)
        TA_MACDFIX(0, unsyncSize() - 1, series.getArray(), period, &begIdx,
                   &nbElement, &(macd.at(k)), &(macdSignal.at(k)), &at(k));
    }
  };

 public:
  MakeMACDHistFixSeries(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "MACD Hist Fix") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MACDHistFixSeries(getSeries(), getPeriod(), id()), id()));
  }
};

class MakeMFISeries : public MakeFromBarsWithOnePeriod {
 private:
  class MFISeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MFISeries(const BarsImpl& bars, unsigned int period, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int begIdx;
      int nbElement;
      const TA_Real* inHigh = bars.highSeries().getArray();
      const TA_Real* inLow = bars.lowSeries().getArray();
      const TA_Real* inClose = bars.closeSeries().getArray();
      const TA_Real* inVolume = bars.volumeSeries().getArray();

      if (unsyncSize() > (unsigned int)TA_MFI_Lookback(period))
        TA_MFI(0, unsyncSize() - 1, inHigh, inLow, inClose, inVolume, period,
               &begIdx, &nbElement, &at(TA_MFI_Lookback(period)));
    }
  };

 public:
  MakeMFISeries(const BarsImpl& bars, unsigned int period)
      : MakeFromBarsWithOnePeriod(bars, period, "MFI") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new MFISeries(getBars(), getPeriod(), id()), id()));
  }
};

class MakePPOSeries : public MakeFromSeries {
 private:
  // PPO - Percentage price oscillator
  class PPOSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    PPOSeries(const SeriesImpl& series, unsigned int fastPeriod,
              unsigned int slowPeriod, MAType maType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_PPO_Lookback(fastPeriod, slowPeriod,
                                                       (TA_MAType)maType))
        TA_PPO(0, unsyncSize() - 1, series.getArray(), fastPeriod, slowPeriod,
               (TA_MAType)maType, &begIdx, &nbElement,
               &at(TA_PPO_Lookback(fastPeriod, slowPeriod, (TA_MAType)maType)));
    }
  };

 private:
  const unsigned int _slowPeriod;
  const unsigned int _fastPeriod;
  const MAType _maType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int fastPeriod,
                              unsigned int slowPeriod, MAType maType) {
    std::ostringstream o;
    o << series.getId() << " - PPO - " << fastPeriod << ", " << slowPeriod
      << ", " << maType;

    return Id(o.str());
  }

 public:
  MakePPOSeries(const SeriesImpl& series, unsigned int fastPeriod,
                unsigned slowPeriod, MAType maType)
      : MakeFromSeries(series,
                       calculateId(series, fastPeriod, slowPeriod, maType)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod),
        _maType(maType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new PPOSeries(getSeries(), _fastPeriod, _slowPeriod, _maType, id()),
        id()));
  }
};

class MakeHTDCPeriodSeries : public MakeFromSeries {
 private:
  // TA_HT_DCPERIOD - Hilbert Transform - Dominant Cycle Period
  class HTDCPeriodSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTDCPeriodSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_HT_DCPERIOD_Lookback())
        TA_HT_DCPERIOD(0, unsyncSize() - 1, series.getArray(), &begIdx,
                       &nbElement, &at(TA_HT_DCPERIOD_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT DC Period");
  }

 public:
  MakeHTDCPeriodSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTDCPeriodSeries(getSeries(), id()), id()));
  }
};

class MakeHTDCPhaseSeries : public MakeFromSeries {
 private:
  // TA_HT_DCPHASE - Hilbert Transform - Dominant Cycle Phase
  class HTDCPhaseSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTDCPhaseSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      if (unsyncSize() > (unsigned int)TA_HT_DCPHASE_Lookback())
        TA_HT_DCPHASE(0, unsyncSize() - 1, series.getArray(), &begIdx,
                      &nbElement, &at(TA_HT_DCPHASE_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT DC Phase");
  }

 public:
  MakeHTDCPhaseSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTDCPhaseSeries(getSeries(), id()), id()));
  }
};

// TA_HT_PHASOR - Hilbert Transform - Phasor Component phase
class MakeHTPhasorPhaseSeries : public MakeFromSeries {
 private:
  class HTPhasorPhaseSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTPhasorPhaseSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl quadrature(unsyncSize(), SynchronizerPtr());
      if (unsyncSize() > (unsigned int)TA_HT_PHASOR_Lookback())
        TA_HT_PHASOR(0, unsyncSize() - 1, series.getArray(), &begIdx,
                     &nbElement, &at(TA_HT_PHASOR_Lookback()),
                     &quadrature.at(TA_HT_PHASOR_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT Phasor Phase");
  }

 public:
  MakeHTPhasorPhaseSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new HTPhasorPhaseSeries(getSeries(), id()), id()));
  }
};

class MakeHTPhasorQuadratureSeries : public MakeFromSeries {
 private:
  // TA_HT_PHASOR - Hilbert Transform - Phasor Component quadrature
  class HTPhasorQuadratureSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTPhasorQuadratureSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl phase(unsyncSize(), SynchronizerPtr());
      if (unsyncSize() > (unsigned int)TA_HT_PHASOR_Lookback())
        TA_HT_PHASOR(0, unsyncSize() - 1, series.getArray(), &begIdx,
                     &nbElement, &phase.at(TA_HT_PHASOR_Lookback()),
                     &at(TA_HT_PHASOR_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT Phasor quadrature");
  }

 public:
  MakeHTPhasorQuadratureSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new HTPhasorQuadratureSeries(getSeries(), id()), id()));
  }
};

class MakeHTSineSeries : public MakeFromSeries {
 private:
  // TA_HT_SINE - Hilbert Transform - SineWave
  class HTSineSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTSineSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl leadSine(unsyncSize(), SynchronizerPtr());
      if (unsyncSize() > (unsigned int)TA_HT_PHASOR_Lookback())
        TA_HT_SINE(0, unsyncSize() - 1, series.getArray(), &begIdx, &nbElement,
                   &at(TA_HT_SINE_Lookback()),
                   &leadSine.at(TA_HT_PHASOR_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT Sine");
  }

 public:
  MakeHTSineSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTSineSeries(getSeries(), id()), id()));
  }
};

class MakeHTLeadSineSeries : public MakeFromSeries {
 private:
  // TA_HT_PHASOR - Hilbert Transform - Phasor Component quadrature
  class HTLeadSineSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTLeadSineSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;
      SeriesImpl sine(unsyncSize(), SynchronizerPtr());
      if (unsyncSize() > (unsigned int)TA_HT_PHASOR_Lookback())
        TA_HT_SINE(0, unsyncSize() - 1, series.getArray(), &begIdx, &nbElement,
                   &sine.at(TA_HT_SINE_Lookback()),
                   &at(TA_HT_PHASOR_Lookback()));
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT Lead sine");
  }

 public:
  MakeHTLeadSineSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTLeadSineSeries(getSeries(), id()), id()));
  }
};

class MakeHTTrendModeSeries : public MakeFromSeries {
 private:
  // TA_HT_DCPHASE - Hilbert Transform - Dominant Cycle Phase
  class HTTrendModeSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    HTTrendModeSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int begIdx;
      int nbElement;

      if (unsyncSize() > (unsigned int)TA_HT_TRENDMODE_Lookback()) {
        vector<int> x(unsyncSize());

        // modified TA_... to take an array of doubles as output parameter
        TA_HT_TRENDMODE(0, unsyncSize() - 1, series.getArray(), &begIdx,
                        &nbElement, &x.at(TA_HT_TRENDMODE_Lookback()));

        std::copy(x.begin(), x.end(), _v.begin());
      }
    }
  };

 private:
  static const Id calculateId(const SeriesImpl& series) {
    return Id(series.getId() + " - HT Trend Mode");
  }

 public:
  MakeHTTrendModeSeries(const SeriesImpl& series)
      : MakeFromSeries(series, calculateId(series)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new HTTrendModeSeries(getSeries(), id()), id()));
  }
};

class MakeChaikinADSeries : public MakeFromBars {
 private:
  // TA_AD - Chaikin A/D Line
  class ChaikinADSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    ChaikinADSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_AD_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        const TA_Real* inVolume = bars.volumeSeries().getArray();
        TA_AD(0, unsyncSize() - 1, inHigh, inLow, inClose, inVolume, &begIdx,
              &nbElement, &at(TA_AD_Lookback()));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    return Id(bars.getId() + " - Chaikin A/D");
  }

 public:
  MakeChaikinADSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new ChaikinADSeries(getBars(), id()), id()));
  }
};

class MakeChaikinADOscillatorSeries : public MakeFromBars {
 private:
  // TA_AD - Chaikin A/D Line
  class ChaikinADOscillatorSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    ChaikinADOscillatorSeries(const BarsImpl& bars, unsigned int fastPeriod,
                              unsigned int slowPeriod, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() >
          (unsigned int)TA_ADOSC_Lookback(fastPeriod, slowPeriod)) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        const TA_Real* inVolume = bars.volumeSeries().getArray();
        TA_ADOSC(0, unsyncSize() - 1, inHigh, inLow, inClose, inVolume,
                 fastPeriod, slowPeriod, &begIdx, &nbElement,
                 &at(TA_ADOSC_Lookback(fastPeriod, slowPeriod)));
      }
    }
  };

 private:
  const unsigned int _fastPeriod;
  const unsigned int _slowPeriod;

 private:
  static const Id calculateId(const BarsImpl& bars, unsigned int fastPeriod,
                              unsigned int slowPeriod) {
    std::ostringstream o;

    o << bars.getId() << " - Chaikin A/D Oscillator - " << fastPeriod << ", "
      << slowPeriod;
    return Id(o.str());
  }

 public:
  MakeChaikinADOscillatorSeries(const BarsImpl& bars, unsigned int fastPeriod,
                                unsigned int slowPeriod)
      : MakeFromBars(bars, calculateId(bars, fastPeriod, slowPeriod)),
        _fastPeriod(fastPeriod),
        _slowPeriod(slowPeriod) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new ChaikinADOscillatorSeries(
                                   getBars(), _fastPeriod, _slowPeriod, id()),
                               id()));
  }
};

class MakeOBVSeries : public MakeFromBars {
 private:
  // TA_OBV - On Balance Volume
  class OBVSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    OBVSeries(const BarsImpl& bars, const Series& series, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_OBV_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inVolume = bars.volumeSeries().getArray();

        vector<double> y(unsyncSize());
        TA_OBV(0, unsyncSize() - 1, series.getArray(), inVolume, &begIdx,
               &nbElement, &y.at(TA_OBV_Lookback()));

        std::copy(y.begin(), y.end(), _v.begin());
      }
    }
  };

 private:
  const Series& _series;

 private:
  static const Id calculateId(const BarsImpl& bars, const SeriesImpl& series) {
    std::ostringstream o;

    o << bars.getId() << ", " << series.getId() << " - OBV";
    return Id(o.str());
  }

 public:
  MakeOBVSeries(const BarsImpl& bars, const Series& series)
      : MakeFromBars(bars, calculateId(bars, dynamic_cast<const SeriesImpl&>(
                                                 series.getSeries()))),
        _series(series) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new OBVSeries(getBars(), _series, id()), id()));
  }
};

class MakeStdDevSeries : public MakeFromSeries {
 private:
  class StdDevSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    StdDevSeries(const SeriesImpl& series, unsigned int period, double nbDev,
                 const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_STDDEV_Lookback(period, nbDev)) {
        int begIdx;
        int nbElement;
        TA_STDDEV(0, unsyncSize() - 1, series.getArray(), period, nbDev,
                  &begIdx, &nbElement, &at(TA_STDDEV_Lookback(period, nbDev)));
      }
    }
  };

 private:
  const unsigned int _period;
  const double _nbDev;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double nbDev) {
    std::ostringstream o;
    o << series.getId() << " - Standard deviation - " << period << ", "
      << nbDev;

    return Id(o.str());
  }

 public:
  MakeStdDevSeries(const SeriesImpl& series, unsigned int period, double nbDev)
      : MakeFromSeries(series, calculateId(series, period, nbDev)),
        _period(period),
        _nbDev(nbDev) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new StdDevSeries(getSeries(), _period, _nbDev, id()), id()));
  }
};

class MakeVarianceSeries : public MakeFromSeries {
 private:
  class VarianceSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    VarianceSeries(const SeriesImpl& series, unsigned int period, double nbDev,
                   const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_VAR_Lookback(period, nbDev)) {
        int begIdx;
        int nbElement;
        TA_VAR(0, unsyncSize() - 1, series.getArray(), period, nbDev, &begIdx,
               &nbElement, &at(TA_VAR_Lookback(period, nbDev)));
      }
    }
  };

 private:
  const unsigned int _period;
  const double _nbDev;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              double nbDev) {
    std::ostringstream o;
    o << series.getId() << " - Variance - " << period << ", " << nbDev;

    return Id(o.str());
  }

 public:
  MakeVarianceSeries(const SeriesImpl& series, unsigned int period,
                     double nbDev)
      : MakeFromSeries(series, calculateId(series, period, nbDev)),
        _period(period),
        _nbDev(nbDev) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new VarianceSeries(getSeries(), _period, _nbDev, id()), id()));
  }
};

class MakeAvgPriceSeries : public MakeFromBars {
 private:
  class AvgPriceSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    AvgPriceSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_AVGPRICE_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        const TA_Real* inOpen = bars.openSeries().getArray();
        TA_AVGPRICE(0, unsyncSize() - 1, inOpen, inHigh, inLow, inClose,
                    &begIdx, &nbElement, &at(TA_AVGPRICE_Lookback()));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    std::ostringstream o;

    o << bars.getId() << " - Average Price";
    return Id(o.str());
  }

 public:
  MakeAvgPriceSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new AvgPriceSeries(getBars(), id()), id()));
  }
};

class MakeMedPriceSeries : public MakeFromBars {
 private:
  class MedPriceSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    MedPriceSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_MEDPRICE_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        TA_MEDPRICE(0, unsyncSize() - 1, inHigh, inLow, &begIdx, &nbElement,
                    &at(TA_MEDPRICE_Lookback()));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    std::ostringstream o;

    o << bars.getId() << " - Median Price";
    return Id(o.str());
  }

 public:
  MakeMedPriceSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new MedPriceSeries(getBars(), id()), id()));
  }
};

class MakeTypPriceSeries : public MakeFromBars {
 private:
  class TypPriceSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    TypPriceSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_TYPPRICE_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        TA_TYPPRICE(0, unsyncSize() - 1, inHigh, inLow, inClose, &begIdx,
                    &nbElement, &at(TA_TYPPRICE_Lookback()));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    std::ostringstream o;

    o << bars.getId() << " - Typical Price";
    return Id(o.str());
  }

 public:
  MakeTypPriceSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new TypPriceSeries(getBars(), id()), id()));
  }
};

class MakeWclPriceSeries : public MakeFromBars {
 private:
  // Weighted close price
  class WclPriceSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    WclPriceSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_WCLPRICE_Lookback()) {
        int begIdx;
        int nbElement;
        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        TA_WCLPRICE(0, unsyncSize() - 1, inHigh, inLow, inClose, &begIdx,
                    &nbElement, &at(TA_WCLPRICE_Lookback()));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    std::ostringstream o;

    o << bars.getId() << " - Wighted Close Price";
    return Id(o.str());
  }

 public:
  MakeWclPriceSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new WclPriceSeries(getBars(), id()), id()));
  }
};

class MakeAccumDistSeries : public MakeFromBars {
 private:
  class AccumDistSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    AccumDistSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      // function_requires< SeriesImpl< T > >();

      // TODO: implement this with std::transform, but this requires
      // implementing Bars with iterators and stuff, so see if I can do this
      // with Bars
      //

      const std::vector<double>& close(bars.closeSeries().getVector());
      const std::vector<double>& low(bars.lowSeries().getVector());
      const std::vector<double>& high(bars.highSeries().getVector());
      const std::vector<double>& volume(bars.volumeSeries().getVector());
      double cum = 0;
      for (unsigned int n = 0; n < bars.unsyncSize(); n++) {
        cum +=
            (2 * close[n] - low[n] - high[n]) / (high[n] - low[n]) * volume[n];
        at(n) = cum;
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    return Id(bars.getId() + "- Accum/Dist");
  }

 public:
  MakeAccumDistSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new AccumDistSeries(getBars(), id()), id()));
  }
};

class MakeTrueRangeSeries : public MakeFromBars {
 private:
  class TrueRangeSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    TrueRangeSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      // function_requires< SeriesImpl< T > >();

      // TODO: implement this with std::transform, but this requires
      // implementing Bars with iterators and stuff, so see if I can do this
      // with Bars
      const std::vector<double>& close(bars.closeSeries().getVector());
      const std::vector<double>& low(bars.lowSeries().getVector());
      const std::vector<double>& high(bars.highSeries().getVector());

      at(0) = (double)(high[0] - low[0]);
      for (unsigned int n = 1; n < bars.unsyncSize(); n++) {
        at(n) = max3((double)(high[n] - low[n]),
                     (double)abs(close[n - 1] - high[n]),
                     (double)abs(close[n - 1] - low[n]));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    return Id(bars.getId() + "- True Range");
  }

 public:
  MakeTrueRangeSeries(const BarsImpl& bars)
      : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new TrueRangeSeries(getBars(), id()), id()));
  }
};

class MakeWSum : public MakeFromSeriesWithOnePeriod {
 private:
  // Welles summation Sum( n ) = Value( n ) + Sum( n - 1 )* ( period - 1
  // )/period; used by DIPlus and DIMinus
  class WSum : public SeriesImpl {
   public:
    WSum(const SeriesImpl& series, unsigned int period, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      const std::vector<double>& v(series.getVector());

      at(0) = v[0];

      for (unsigned int n = 1; n < series.unsyncSize(); n++)
        at(n) = v[n] + at(n - 1);  //*((double)( period - 1 ))/(double)period;
    }
  };

 public:
  /*
  : MakeFromBarsWithOnePeriod( bars, period, "Max" )
  */
  MakeWSum(const SeriesImpl& series, unsigned int period)
      : MakeFromSeriesWithOnePeriod(series, period, "WSum") {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new WSum(getSeries(), getPeriod(), id()), id()));
  }
};

/**
 * Group: Pattern Recognition
 */

typedef int (*TA_LOOKBACK)();
typedef int (*TA_LOOKBACK_PENETRATION)(double);
typedef TA_RetCode (*TA_FUNC)(int, int, const double[], const double[],
                              const double[], const double[], int*, int*,
                              int[]);
typedef TA_RetCode (*TA_FUNC_PENETRATION)(int, int, const double[],
                                          const double[], const double[],
                                          const double[], double, int*, int*,
                                          int[]);

template <TA_LOOKBACK T>
class CandleConstants {
 public:
  static const char* _id;
};

template <TA_LOOKBACK_PENETRATION T>
class CandleConstantsPenetration {
 public:
  static const char* _id;
};

template <>
const char* CandleConstants<TA_CDL3BLACKCROWS_Lookback>::_id =
    "Candle 3 Black Crows";
template <>
const char* CandleConstants<TA_CDLDOJISTAR_Lookback>::_id = "Candle Doji Star";
template <>
const char* CandleConstants<TA_CDLENGULFING_Lookback>::_id = "Candle Engulfing";
template <>
const char* CandleConstants<TA_CDLHAMMER_Lookback>::_id = "Candle Hammer";
template <>
const char* CandleConstants<TA_CDLHANGINGMAN_Lookback>::_id =
    "Candle Hanging Man";
template <>
const char* CandleConstants<TA_CDLHARAMI_Lookback>::_id = "Candle Harami";
template <>
const char* CandleConstants<TA_CDLHARAMICROSS_Lookback>::_id =
    "Candle Harami Cross";
template <>
const char* CandleConstants<TA_CDLHIGHWAVE_Lookback>::_id = "Candle High Wave";
template <>
const char* CandleConstants<TA_CDLIDENTICAL3CROWS_Lookback>::_id =
    "Candle Identical 3 Crows";
template <>
const char* CandleConstants<TA_CDLINVERTEDHAMMER_Lookback>::_id =
    "Candle Inverted Hammer";
template <>
const char* CandleConstants<TA_CDLLONGLINE_Lookback>::_id = "Candle Long Line";
template <>
const char* CandleConstants<TA_CDLSHOOTINGSTAR_Lookback>::_id =
    "Candle Shooting Star";
template <>
const char* CandleConstants<TA_CDLSHORTLINE_Lookback>::_id =
    "Candle Short Line";
template <>
const char* CandleConstants<TA_CDLSPINNINGTOP_Lookback>::_id =
    "Candle Spinning Top";
template <>
const char* CandleConstants<TA_CDLTRISTAR_Lookback>::_id = "Candle Tristar";
template <>
const char* CandleConstants<TA_CDLUPSIDEGAP2CROWS_Lookback>::_id =
    "Candle Upside Gap 2 Crows";
template <>
const char* CandleConstants<TA_CDLHIKKAKE_Lookback>::_id = "Candle Hikakke";

template <>
const char* CandleConstantsPenetration<TA_CDLABANDONEDBABY_Lookback>::_id =
    "Candle Abandoned Baby, penetration - ";
template <>
const char* CandleConstantsPenetration<TA_CDLEVENINGDOJISTAR_Lookback>::_id =
    "Candle Evening Doji Star, penetration: ";
template <>
const char* CandleConstantsPenetration<TA_CDLEVENINGSTAR_Lookback>::_id =
    "Candle Evening Star, penetration: ";
template <>
const char* CandleConstantsPenetration<TA_CDLMORNINGDOJISTAR_Lookback>::_id =
    "Candle Morning Doji Star, penetration - ";
template <>
const char* CandleConstantsPenetration<TA_CDLMORNINGSTAR_Lookback>::_id =
    "Candle Morning Star, penetration - ";

// introduced in ta-lib 0.1.3
template <>
const char* CandleConstants<TA_CDL2CROWS_Lookback>::_id = "Candle 2 Crows";
template <>
const char* CandleConstants<TA_CDL3INSIDE_Lookback>::_id = "Candle 3 Inside";
template <>
const char* CandleConstants<TA_CDL3LINESTRIKE_Lookback>::_id =
    "Candle 3 Line Strike";
template <>
const char* CandleConstants<TA_CDL3OUTSIDE_Lookback>::_id = "Candle 3 Outside";
template <>
const char* CandleConstants<TA_CDL3WHITESOLDIERS_Lookback>::_id =
    "Candle 3 Advancing White Soldiers";
template <>
const char* CandleConstants<TA_CDLADVANCEBLOCK_Lookback>::_id =
    "Candle 3 Advance Block";
template <>
const char* CandleConstants<TA_CDLINNECK_Lookback>::_id = "Candle In Neck";
template <>
const char* CandleConstants<TA_CDLONNECK_Lookback>::_id = "Candle On Neck";
template <>
const char* CandleConstants<TA_CDLPIERCING_Lookback>::_id = "Candle Piercing";
template <>
const char* CandleConstants<TA_CDLSTALLEDPATTERN_Lookback>::_id =
    "Candle Stalled Pattern";
template <>
const char* CandleConstants<TA_CDLTHRUSTING_Lookback>::_id = "Candle Thrusting";

template <>
const char* CandleConstantsPenetration<TA_CDLDARKCLOUDCOVER_Lookback>::_id =
    "Candle Dark Cloud Cover";

// introduced in ta-lib 0.1.4
template <>
const char* CandleConstants<TA_CDL3STARSINSOUTH_Lookback>::_id =
    "Candle Three Stars In The South";
template <>
const char* CandleConstants<TA_CDLBELTHOLD_Lookback>::_id = "Candle Belt-hold";
template <>
const char* CandleConstants<TA_CDLBREAKAWAY_Lookback>::_id = "Candle Breakaway";
template <>
const char* CandleConstants<TA_CDLCLOSINGMARUBOZU_Lookback>::_id =
    "Candle Closing Marubozu";
template <>
const char* CandleConstants<TA_CDLCONCEALBABYSWALL_Lookback>::_id =
    "Candle Concealing Baby Swallow";
template <>
const char* CandleConstants<TA_CDLCOUNTERATTACK_Lookback>::_id =
    "Candle Counterattack";
template <>
const char* CandleConstants<TA_CDLDOJI_Lookback>::_id = "Candle Doji";
template <>
const char* CandleConstants<TA_CDLDRAGONFLYDOJI_Lookback>::_id =
    "Candle Dragonfly Doji";
template <>
const char* CandleConstants<TA_CDLGAPSIDESIDEWHITE_Lookback>::_id =
    "Candle Up/Down-gap side-by-side white lines";
template <>
const char* CandleConstants<TA_CDLGRAVESTONEDOJI_Lookback>::_id =
    "Candle Gravestone Doji";
template <>
const char* CandleConstants<TA_CDLHOMINGPIGEON_Lookback>::_id =
    "Candle Homing Pigeon";
template <>
const char* CandleConstants<TA_CDLKICKING_Lookback>::_id = "Candle Kicking";
template <>
const char* CandleConstants<TA_CDLKICKINGBYLENGTH_Lookback>::_id =
    "Candle Kicking-bull/bear determined by the longer marubozu";
template <>
const char* CandleConstants<TA_CDLLADDERBOTTOM_Lookback>::_id =
    "Candle Ladder Bottom";
template <>
const char* CandleConstants<TA_CDLLONGLEGGEDDOJI_Lookback>::_id =
    "Candle Long Legged Doji";
template <>
const char* CandleConstants<TA_CDLMARUBOZU_Lookback>::_id = "Candle Marubozu";
template <>
const char* CandleConstants<TA_CDLMATCHINGLOW_Lookback>::_id =
    "Candle Matching Low";
template <>
const char* CandleConstants<TA_CDLRICKSHAWMAN_Lookback>::_id =
    "Candle Rickshaw Man";
template <>
const char* CandleConstants<TA_CDLRISEFALL3METHODS_Lookback>::_id =
    "Candle Rising/Falling Three Methods";
template <>
const char* CandleConstants<TA_CDLSEPARATINGLINES_Lookback>::_id =
    "Candle Separating Lines";
template <>
const char* CandleConstants<TA_CDLSTICKSANDWICH_Lookback>::_id =
    "Candle Stick Sandwich";
template <>
const char* CandleConstants<TA_CDLTAKURI_Lookback>::_id = "Candle Takuri";
template <>
const char* CandleConstants<TA_CDLTASUKIGAP_Lookback>::_id =
    "Candle Tasuki Gap";
template <>
const char* CandleConstants<TA_CDLUNIQUE3RIVER_Lookback>::_id =
    "Candle Unique 3 River";
template <>
const char* CandleConstants<TA_CDLXSIDEGAP3METHODS_Lookback>::_id =
    "Candle Upside/Downside Gap Three Methods";

template <>
const char* CandleConstantsPenetration<TA_CDLMATHOLD_Lookback>::_id =
    "- Candle Mat Hold";

template <TA_LOOKBACK T, TA_FUNC U>
class MakeCandle : public MakeFromBars {
 private:
  template <TA_LOOKBACK T, TA_FUNC U>
  class Candle : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    Candle(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)(*T)()) {
        int begIdx;
        int nbElement;
        vector<int> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inOpen = bars.openSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        (*U)(0, unsyncSize() - 1, inOpen, inHigh, inLow, inClose, &begIdx,
             &nbElement, &x.at((*T)()));

        std::copy(x.begin(), x.end(), _v.begin());
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    return Id(bars.getId() + " - ") + CandleConstants<T>::_id;
  }

 public:
  MakeCandle(const BarsImpl& bars) : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new Candle<T, U>(getBars(), id()), id()));
  }
};

template <TA_LOOKBACK_PENETRATION T, TA_FUNC_PENETRATION U>
class MakeCandlePenetration : public MakeFromBars {
 private:
  const double _penetration;

  template <TA_LOOKBACK_PENETRATION T, TA_FUNC_PENETRATION U>
  class CandlePenetration : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    CandlePenetration(const BarsImpl& bars, double penetration, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)(*T)(penetration)) {
        int begIdx;
        int nbElement;
        vector<int> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inOpen = bars.openSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();
        (*U)(0, unsyncSize() - 1, inOpen, inHigh, inLow, inClose, penetration,
             &begIdx, &nbElement,
             &x.at(TA_CDLABANDONEDBABY_Lookback(penetration)));

        std::copy(x.begin(), x.end(), _v.begin());
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars, double penetration) {
    std::ostringstream o;

    o << penetration;
    return Id(bars.getId() + CandleConstantsPenetration<T>::_id + o.str());
  }

 public:
  MakeCandlePenetration(const BarsImpl& bars, double penetration)
      : MakeFromBars(bars, calculateId(bars, penetration)),
        _penetration(penetration) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new CandlePenetration<T, U>(getBars(), _penetration, id()), id()));
  }
};

typedef MakeSeriesTAFunc1<TA_MAX_Lookback, TA_MAX> MakeMaxSeries;
typedef MakeSeriesTAFunc1<TA_MIN_Lookback, TA_MIN> MakeMinSeries;
typedef MakeSeriesTAFunc1<TA_DEMA_Lookback, TA_DEMA> MakeDEMASeries;
typedef MakeSeriesTAFunc1<TA_KAMA_Lookback, TA_KAMA> MakeKAMASeries;
typedef MakeSeriesTAFunc1<TA_MIDPOINT_Lookback, TA_MIDPOINT> MakeMidPointSeries;
typedef MakeSeriesTAFunc1<TA_TEMA_Lookback, TA_TEMA> MakeTEMASeries;
typedef MakeSeriesTAFunc1<TA_TRIMA_Lookback, TA_TRIMA> MakeTRIMASeries;
typedef MakeSeriesTAFunc1<TA_MOM_Lookback, TA_MOM> MakeMOMSeries;
typedef MakeSeriesTAFunc1<TA_ROC_Lookback, TA_ROC> MakeROCSeries;
typedef MakeSeriesTAFunc1<TA_ROCP_Lookback, TA_ROCP> MakeROCPSeries;
typedef MakeSeriesTAFunc1<TA_ROCR_Lookback, TA_ROCR> MakeROCRSeries;
typedef MakeSeriesTAFunc1<TA_ROCR100_Lookback, TA_ROCR100> MakeROCR100Series;
typedef MakeSeriesTAFunc1<TA_RSI_Lookback, TA_RSI> MakeRSISeries;
typedef MakeSeriesTAFunc1<TA_TRIX_Lookback, TA_TRIX> MakeTRIXSeries;
typedef MakeSeriesTAFunc1<TA_LINEARREG_Lookback, TA_LINEARREG>
    MakeLinearRegSeries;
typedef MakeSeriesTAFunc1<TA_LINEARREG_SLOPE_Lookback, TA_LINEARREG_SLOPE>
    MakeLinearRegSlopeSeries;
typedef MakeSeriesTAFunc1<TA_LINEARREG_ANGLE_Lookback, TA_LINEARREG_ANGLE>
    MakeLinearRegAngleSeries;
typedef MakeSeriesTAFunc1<TA_LINEARREG_INTERCEPT_Lookback,
                          TA_LINEARREG_INTERCEPT>
    MakeLinearRegInterceptSeries;
typedef MakeSeriesTAFunc1<TA_TSF_Lookback, TA_TSF> MakeTSFSeries;
typedef MakeSeriesTAFunc1<TA_CMO_Lookback, TA_CMO> MakeCMOSeries;

typedef MakeBarsHighLowSeries<TA_MIDPRICE_Lookback, TA_MIDPRICE>
    MakeMidPriceSeries;
typedef MakeBarsHighLowSeries<TA_MINUS_DM_Lookback, TA_MINUS_DM>
    MakeMinusDMSeries;
typedef MakeBarsHighLowSeries<TA_PLUS_DM_Lookback, TA_PLUS_DM> MakePlusDMSeries;

typedef MakeBarsHighLowCloseSeries<TA_ATR_Lookback, TA_ATR> MakeATRSeries;
typedef MakeBarsHighLowCloseSeries<TA_ADX_Lookback, TA_ADX> MakeADXSeries;
typedef MakeBarsHighLowCloseSeries<TA_ADXR_Lookback, TA_ADXR> MakeADXRSeries;
typedef MakeBarsHighLowCloseSeries<TA_CCI_Lookback, TA_CCI> MakeCCISeries;
typedef MakeBarsHighLowCloseSeries<TA_DX_Lookback, TA_DX> MakeDXSeries;
typedef MakeBarsHighLowCloseSeries<TA_MINUS_DI_Lookback, TA_MINUS_DI>
    MakeMinusDISeries;
typedef MakeBarsHighLowCloseSeries<TA_PLUS_DI_Lookback, TA_PLUS_DI>
    MakePlusDISeries;
typedef MakeBarsHighLowCloseSeries<TA_WILLR_Lookback, TA_WILLR> MakeWillRSeries;
typedef MakeBarsHighLowCloseSeries<TA_NATR_Lookback, TA_NATR> MakeNATRSeries;

typedef MakeCandle<TA_CDL3BLACKCROWS_Lookback, TA_CDL3BLACKCROWS>
    MakeCdl3BlackCrows;
typedef MakeCandle<TA_CDLDOJISTAR_Lookback, TA_CDLDOJISTAR> MakeCdlDojiStar;
typedef MakeCandle<TA_CDLENGULFING_Lookback, TA_CDLENGULFING> MakeCdlEngulfing;
typedef MakeCandle<TA_CDLHAMMER_Lookback, TA_CDLHAMMER> MakeCdlHammer;
typedef MakeCandle<TA_CDLHANGINGMAN_Lookback, TA_CDLHANGINGMAN>
    MakeCdlHangingMan;
typedef MakeCandle<TA_CDLHARAMI_Lookback, TA_CDLHARAMI> MakeCdlHarami;
typedef MakeCandle<TA_CDLHARAMICROSS_Lookback, TA_CDLHARAMICROSS>
    MakeCdlHaramiCross;
typedef MakeCandle<TA_CDLHIGHWAVE_Lookback, TA_CDLHIGHWAVE> MakeCdlHighWave;
typedef MakeCandle<TA_CDLIDENTICAL3CROWS_Lookback, TA_CDLIDENTICAL3CROWS>
    MakeCdlIdentical3Crows;
typedef MakeCandle<TA_CDLINVERTEDHAMMER_Lookback, TA_CDLINVERTEDHAMMER>
    MakeCdlInvertedHammer;
typedef MakeCandle<TA_CDLLONGLINE_Lookback, TA_CDLLONGLINE> MakeCdlLongLine;
typedef MakeCandle<TA_CDLSHOOTINGSTAR_Lookback, TA_CDLSHOOTINGSTAR>
    MakeCdlShootingStar;
typedef MakeCandle<TA_CDLSHORTLINE_Lookback, TA_CDLSHORTLINE> MakeCdlShortLine;
typedef MakeCandle<TA_CDLSPINNINGTOP_Lookback, TA_CDLSPINNINGTOP>
    MakeCdlSpinningTop;
typedef MakeCandle<TA_CDLTRISTAR_Lookback, TA_CDLTRISTAR> MakeCdlTristar;
typedef MakeCandle<TA_CDLUPSIDEGAP2CROWS_Lookback, TA_CDLUPSIDEGAP2CROWS>
    MakeCdlUpsideGap2Crows;
typedef MakeCandle<TA_CDLHIKKAKE_Lookback, TA_CDLHIKKAKE> MakeCdlHikkake;

typedef MakeCandlePenetration<TA_CDLABANDONEDBABY_Lookback, TA_CDLABANDONEDBABY>
    MakeCdlAbandonedBaby;
typedef MakeCandlePenetration<TA_CDLEVENINGDOJISTAR_Lookback,
                              TA_CDLEVENINGDOJISTAR>
    MakeCdlEveningDojiStar;
typedef MakeCandlePenetration<TA_CDLEVENINGSTAR_Lookback, TA_CDLEVENINGSTAR>
    MakeCdlEveningStar;
typedef MakeCandlePenetration<TA_CDLMORNINGDOJISTAR_Lookback,
                              TA_CDLMORNINGDOJISTAR>
    MakeCdlMorningDojiStar;
typedef MakeCandlePenetration<TA_CDLMORNINGSTAR_Lookback, TA_CDLMORNINGSTAR>
    MakeCdlMorningStar;

// TA 0.1.3
typedef MakeCandle<TA_CDL2CROWS_Lookback, TA_CDL2CROWS> MakeCdl2Crows;
typedef MakeCandle<TA_CDL3INSIDE_Lookback, TA_CDL3INSIDE> MakeCdl3Inside;
typedef MakeCandle<TA_CDL3LINESTRIKE_Lookback, TA_CDL3LINESTRIKE>
    MakeCdl3LineStrike;
typedef MakeCandle<TA_CDL3WHITESOLDIERS_Lookback, TA_CDL3WHITESOLDIERS>
    MakeCdl3WhiteSoldiers;
typedef MakeCandle<TA_CDLADVANCEBLOCK_Lookback, TA_CDLADVANCEBLOCK>
    MakeCdl3Outside;
typedef MakeCandle<TA_CDLINNECK_Lookback, TA_CDLINNECK> MakeCdlInNeck;
typedef MakeCandle<TA_CDLONNECK_Lookback, TA_CDLONNECK> MakeCdlOnNeck;
typedef MakeCandle<TA_CDLPIERCING_Lookback, TA_CDLPIERCING> MakeCdlPiercing;
typedef MakeCandle<TA_CDLSTALLEDPATTERN_Lookback, TA_CDLSTALLEDPATTERN>
    MakeCdlStalled;
typedef MakeCandle<TA_CDLTHRUSTING_Lookback, TA_CDLTHRUSTING> MakeCdlThrusting;

typedef MakeCandlePenetration<TA_CDLDARKCLOUDCOVER_Lookback,
                              TA_CDLDARKCLOUDCOVER>
    MakeCdlAdvanceBlock;

// TA 0.1.4
typedef MakeCandle<TA_CDL3STARSINSOUTH_Lookback, TA_CDL3STARSINSOUTH>
    MakeCdl3StarsInSouth;
typedef MakeCandle<TA_CDLBELTHOLD_Lookback, TA_CDLBELTHOLD> MakeCdlBeltHold;
typedef MakeCandle<TA_CDLBREAKAWAY_Lookback, TA_CDLBREAKAWAY> MakeCdlBreakaway;
typedef MakeCandle<TA_CDLCLOSINGMARUBOZU_Lookback, TA_CDLCLOSINGMARUBOZU>
    MakeCdlClosingMarubuzu;
typedef MakeCandle<TA_CDLCONCEALBABYSWALL_Lookback, TA_CDLCONCEALBABYSWALL>
    MakeCdlConcealingBabySwallow;
typedef MakeCandle<TA_CDLCOUNTERATTACK_Lookback, TA_CDLCOUNTERATTACK>
    MakeCdlCounterattack;
typedef MakeCandle<TA_CDLDOJI_Lookback, TA_CDLDOJI> MakeCdlDoji;
typedef MakeCandle<TA_CDLDRAGONFLYDOJI_Lookback, TA_CDLDRAGONFLYDOJI>
    MakeCdlDragonFlyDoji;
typedef MakeCandle<TA_CDLGAPSIDESIDEWHITE_Lookback, TA_CDLGAPSIDESIDEWHITE>
    MakeCdlGapSideBySideWhite;
typedef MakeCandle<TA_CDLGRAVESTONEDOJI_Lookback, TA_CDLGRAVESTONEDOJI>
    MakeCdlGravestoneDoji;
typedef MakeCandle<TA_CDLHOMINGPIGEON_Lookback, TA_CDLHOMINGPIGEON>
    MakeCdlHomingPigeon;
typedef MakeCandle<TA_CDLKICKING_Lookback, TA_CDLKICKING> MakeCdlKicking;
typedef MakeCandle<TA_CDLKICKINGBYLENGTH_Lookback, TA_CDLKICKINGBYLENGTH>
    MakeCdlKickingByLength;
typedef MakeCandle<TA_CDLLADDERBOTTOM_Lookback, TA_CDLLADDERBOTTOM>
    MakeCdlLadderBottom;
typedef MakeCandle<TA_CDLLONGLEGGEDDOJI_Lookback, TA_CDLLONGLEGGEDDOJI>
    MakeCdlLongLeggedDoji;
typedef MakeCandle<TA_CDLMARUBOZU_Lookback, TA_CDLMARUBOZU> MakeCdlMarubozu;
typedef MakeCandle<TA_CDLMATCHINGLOW_Lookback, TA_CDLMATCHINGLOW>
    MakeCdlMatchingLow;
typedef MakeCandle<TA_CDLRICKSHAWMAN_Lookback, TA_CDLRICKSHAWMAN>
    MakeCdlRickshawMan;
typedef MakeCandle<TA_CDLRISEFALL3METHODS_Lookback, TA_CDLRISEFALL3METHODS>
    MakeCdlRiseFall3Methods;
typedef MakeCandle<TA_CDLSEPARATINGLINES_Lookback, TA_CDLSEPARATINGLINES>
    MakeCdlSeparatingLines;
typedef MakeCandle<TA_CDLSTICKSANDWICH_Lookback, TA_CDLSTICKSANDWICH>
    MakeCdlStickSandwich;
typedef MakeCandle<TA_CDLTAKURI_Lookback, TA_CDLTAKURI> MakeCdlTakuri;
typedef MakeCandle<TA_CDLTASUKIGAP_Lookback, TA_CDLTASUKIGAP> MakeCdlTasukiGap;
typedef MakeCandle<TA_CDLUNIQUE3RIVER_Lookback, TA_CDLUNIQUE3RIVER>
    MakeCdlUnique3River;
typedef MakeCandle<TA_CDLXSIDEGAP3METHODS_Lookback, TA_CDLXSIDEGAP3METHODS>
    MakeCdlXSideGap3Methods;

typedef MakeCandlePenetration<TA_CDLMATHOLD_Lookback, TA_CDLMATHOLD>
    MakeCdlMatHold;

/*
// TODO: to test further - results don't match Wealth-Lab
class DIPlusSeries : public SeriesImpl
{
public:
  // TODO: enforce that T is of type series using function_requires
  DIPlusSeries( const BarsImpl& bars, unsigned int period )
  : SeriesImpl( *(init(bars, period ) ) )
  {

  }

private:
  SeriesPtrConst init( const BarsImpl& bars, unsigned int period ) const
  {

    const WSum trSum( TrueRangeSeries( bars ), period );

//    trSum.dump( std::cout, 20 );
        SeriesImpl dmiPlusSeries( bars.unsyncSize() );

    for( int n = 1; n < bars.unsyncSize(); n++ )
    {
      double hiDiff = bars.high( n ) - bars.high( n - 1 );
      double loDiff = bars.low( n - 1 ) - bars.low( n );

      if( ( ( hiDiff < 0 ) && ( loDiff < 0 ) ) || ( hiDiff <= loDiff ) )
        dmiPlusSeries[ n ] = 0;
      else
        dmiPlusSeries[ n ] = hiDiff;
    }
    WSum dmiPlusSumSeries( dmiPlusSeries, period );

    return dmiPlusSumSeries.multiplyValue( 100 ) -> divideSeries( trSum );
  }

  virtual ~DIPlusSeries()
  {
  }
};

// TODO: to test further - results don't match Wealth-Lab
// and numbers are negative, which they should not be
class DIMinusSeries : public SeriesImpl
{
public:
  // TODO: enforce that T is of type series using function_requires
  DIMinusSeries( const BarsImpl& bars, unsigned int period )
  : SeriesImpl( *(init(bars, period ) ) )
  {

  }

private:
  SeriesPtrConst init( const BarsImpl& bars, unsigned int period ) const
  {

    const WSum trSum( TrueRangeSeries( bars ), period );

//    trSum.dump( std::cout, 20 );
        SeriesImpl dmiMinusSeries( bars.unsyncSize() );

    for( int n = 1; n < bars.unsyncSize(); n++ )
    {
      double hiDiff = bars.high( n ) - bars.high( n - 1 );
      double loDiff = bars.low( n - 1 ) - bars.low( n );

      if( ( ( hiDiff < 0 ) && ( loDiff < 0 ) ) || ( hiDiff >= loDiff ) )
        dmiMinusSeries[ n ] = 0;
      else
        dmiMinusSeries[ n ] = hiDiff;
    }
    WSum dmiMinusSumSeries( dmiMinusSeries, period );

    return dmiMinusSumSeries.multiplyValue( 100 ) -> divideSeries( trSum );
  }

  virtual ~DIMinusSeries()
  {
  }
};

*/

class MakeStochSlowKSeries : public MakeFromBars {
 private:
  int _fastKPeriod;
  int _slowKPeriod;
  MAType _slowKMAType;
  int _slowDPeriod;
  MAType _slowDMAType;

  class STOCHSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHSeries(const BarsImpl& bars, int fastKPeriod, int slowKPeriod,
                MAType slowKMAType, int slowDPeriod, MAType slowDMAType,
                const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int lookback =
          TA_STOCH_Lookback(fastKPeriod, slowKPeriod, (TA_MAType)slowKMAType,
                            slowDPeriod, (TA_MAType)slowDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inOpen = bars.openSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();

        TA_STOCH(0, unsyncSize() - 1, inHigh, inLow, inClose, fastKPeriod,
                 slowKPeriod, (TA_MAType)slowKMAType, slowDPeriod,
                 (TA_MAType)slowDMAType, &begIdx, &nbElement, &at(lookback),
                 &x.at(lookback));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars, int fastKPeriod,
                              int slowKPeriod, MAType slowKMAType,
                              int slowDPeriod, MAType slowDMAType) {
    std::ostringstream o;

    o << bars.getId() << ", " << fastKPeriod << ", " << slowKPeriod << ", "
      << slowKMAType << ", " << slowDPeriod << ", " << slowDMAType
      << " - Stochastic Slow K";
    return Id(o.str());
  }

 public:
  MakeStochSlowKSeries(const BarsImpl& bars, int fastKPeriod, int slowKPeriod,
                       MAType slowKMAType, int slowDPeriod, MAType slowDMAType)
      : MakeFromBars(bars, calculateId(bars, fastKPeriod, slowKPeriod,
                                       slowKMAType, slowDPeriod, slowDMAType)),
        _fastKPeriod(fastKPeriod),
        _slowKPeriod(slowKPeriod),
        _slowKMAType(slowKMAType),
        _slowDPeriod(slowDPeriod),
        _slowDMAType(slowDMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHSeries(getBars(), _fastKPeriod, _slowKPeriod, _slowKMAType,
                        _slowDPeriod, _slowDMAType, id()),
        id()));
  }
};

class MakeBOPSeries : public MakeFromBars {
 private:
  class BOPSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    BOPSeries(const BarsImpl& bars, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int lookback = TA_BOP_Lookback();
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inOpen = bars.openSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();

        TA_BOP(0, unsyncSize() - 1, inOpen, inHigh, inLow, inClose, &begIdx,
               &nbElement, &at(lookback));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars) {
    std::ostringstream o;

    o << bars.getId() << ", "
      << " - BOP";
    return Id(o.str());
  }

 public:
  MakeBOPSeries(const BarsImpl& bars) : MakeFromBars(bars, calculateId(bars)) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new BOPSeries(getBars(), id()), id()));
  }
};

class MakeStochSlowDSeries : public MakeFromBars {
 private:
  int _fastKPeriod;
  int _slowKPeriod;
  MAType _slowKMAType;
  int _slowDPeriod;
  MAType _slowDMAType;

  class STOCHSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHSeries(const BarsImpl& bars, int fastKPeriod, int slowKPeriod,
                MAType slowKMAType, int slowDPeriod, MAType slowDMAType,
                const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int lookback =
          TA_STOCH_Lookback(fastKPeriod, slowKPeriod, (TA_MAType)slowKMAType,
                            slowDPeriod, (TA_MAType)slowDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();

        TA_STOCH(0, unsyncSize() - 1, inHigh, inLow, inClose, fastKPeriod,
                 slowKPeriod, (TA_MAType)slowKMAType, slowDPeriod,
                 (TA_MAType)slowDMAType, &begIdx, &nbElement, &x.at(lookback),
                 &at(lookback));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars, int fastKPeriod,
                              int slowKPeriod, MAType slowKMAType,
                              int slowDPeriod, MAType slowDMAType) {
    std::ostringstream o;

    o << bars.getId() << ", " << fastKPeriod << ", " << slowKPeriod << ", "
      << slowKMAType << ", " << slowDPeriod << ", " << slowDMAType
      << " - Stochastic Slow D";
    return Id(o.str());
  }

 public:
  MakeStochSlowDSeries(const BarsImpl& bars, int fastKPeriod, int slowKPeriod,
                       MAType slowKMAType, int slowDPeriod, MAType slowDMAType)
      : MakeFromBars(bars, calculateId(bars, fastKPeriod, slowKPeriod,
                                       slowKMAType, slowDPeriod, slowDMAType)),
        _fastKPeriod(fastKPeriod),
        _slowKPeriod(slowKPeriod),
        _slowKMAType(slowKMAType),
        _slowDPeriod(slowDPeriod),
        _slowDMAType(slowDMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHSeries(getBars(), _fastKPeriod, _slowKPeriod, _slowKMAType,
                        _slowDPeriod, _slowDMAType, id()),
        id()));
  }
};

class MakeStochFastDSeries : public MakeFromBars {
 private:
  int _fastKPeriod;
  int _fastDPeriod;
  MAType _fastDMAType;

  class STOCHSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHSeries(const BarsImpl& bars, int fastKPeriod, int fastDPeriod,
                MAType fastDMAType, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int lookback =
          TA_STOCHF_Lookback(fastKPeriod, fastDPeriod, (TA_MAType)fastDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();

        TA_STOCHF(0, unsyncSize() - 1, inHigh, inLow, inClose, fastKPeriod,
                  fastDPeriod, (TA_MAType)fastDMAType, &begIdx, &nbElement,
                  &x.at(lookback), &at(lookback));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars, int fastKPeriod,
                              int fastDPeriod, MAType fastDMAType) {
    std::ostringstream o;

    o << bars.getId() << ", " << fastKPeriod << ", " << fastDPeriod << ", "
      << fastDMAType << " - Stochastic Fast D";
    return Id(o.str());
  }

 public:
  MakeStochFastDSeries(const BarsImpl& bars, int fastKPeriod, int fastDPeriod,
                       MAType fastDMAType)
      : MakeFromBars(bars,
                     calculateId(bars, fastKPeriod, fastDPeriod, fastDMAType)),
        _fastKPeriod(fastKPeriod),
        _fastDPeriod(fastDPeriod),
        _fastDMAType(fastDMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHSeries(getBars(), _fastKPeriod, _fastDPeriod, _fastDMAType,
                        id()),
        id()));
  }
};

class MakeStochFastKSeries : public MakeFromBars {
 private:
  int _fastKPeriod;
  int _fastDPeriod;
  MAType _fastDMAType;

  class STOCHSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHSeries(const BarsImpl& bars, int fastKPeriod, int fastDPeriod,
                MAType fastDMAType, const Id& id)
        : SeriesImpl(bars.unsyncSize(), bars.synchronizer(), id) {
      int lookback =
          TA_STOCHF_Lookback(fastKPeriod, fastDPeriod, (TA_MAType)fastDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        const TA_Real* inHigh = bars.highSeries().getArray();
        const TA_Real* inLow = bars.lowSeries().getArray();
        const TA_Real* inClose = bars.closeSeries().getArray();

        TA_STOCHF(0, unsyncSize() - 1, inHigh, inLow, inClose, fastKPeriod,
                  fastDPeriod, (TA_MAType)fastDMAType, &begIdx, &nbElement,
                  &at(lookback), &x.at(lookback));
      }
    }
  };

 private:
  static const Id calculateId(const BarsImpl& bars, int fastKPeriod,
                              int fastDPeriod, MAType fastDMAType) {
    std::ostringstream o;

    o << bars.getId() << ", " << fastKPeriod << ", " << fastDPeriod << ", "
      << fastDMAType << " - Stochastic Fast K";
    return Id(o.str());
  }

 public:
  MakeStochFastKSeries(const BarsImpl& bars, int fastKPeriod, int fastDPeriod,
                       MAType fastDMAType)
      : MakeFromBars(bars,
                     calculateId(bars, fastKPeriod, fastDPeriod, fastDMAType)),
        _fastKPeriod(fastKPeriod),
        _fastDPeriod(fastDPeriod),
        _fastDMAType(fastDMAType) {}

  CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHSeries(getBars(), _fastKPeriod, _fastDPeriod, _fastDMAType,
                        id()),
        id()));
  }
};

class MakeStochRSIFastKSeries : public MakeFromSeries {
 private:
  class STOCHRSISeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHRSISeries(const SeriesImpl& series, int period, int fastKPeriod,
                   int fastDPeriod, MAType fastDMAType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int lookback = TA_STOCHRSI_Lookback(period, fastKPeriod, fastDPeriod,
                                          (TA_MAType)fastDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        TA_STOCHRSI(0, unsyncSize() - 1, series.getArray(), period, fastKPeriod,
                    fastDPeriod, (TA_MAType)fastDMAType, &begIdx, &nbElement,
                    &at(lookback), &x.at(lookback));
      }
    }
  };

 private:
  int _period;
  int _fastKPeriod;
  int _fastDPeriod;
  MAType _fastDMAType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              int fastKPeriod, int fastDPeriod,
                              MAType fastDMAType) {
    std::ostringstream o;
    o << series.getId() << ", " << period << ", " << fastKPeriod << ", "
      << fastDPeriod << ", " << fastDMAType << " - Stochastic RSI Fast K";

    return Id(o.str());
  }

 public:
  MakeStochRSIFastKSeries(const SeriesImpl& series, int period, int fastKPeriod,
                          int fastDPeriod, MAType fastDMAType)
      : MakeFromSeries(series, calculateId(series, period, fastKPeriod,
                                           fastDPeriod, fastDMAType)),
        _period(period),
        _fastKPeriod(fastKPeriod),
        _fastDPeriod(fastDPeriod),
        _fastDMAType(fastDMAType) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHRSISeries(getSeries(), _period, _fastKPeriod, _fastDPeriod,
                           _fastDMAType, id()),
        id()));
  }
};

class MakeStochRSIFastDSeries : public MakeFromSeries {
 private:
  class STOCHRSISeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    STOCHRSISeries(const SeriesImpl& series, int period, int fastKPeriod,
                   int fastDPeriod, MAType fastDMAType, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      int lookback = TA_STOCHRSI_Lookback(period, fastKPeriod, fastDPeriod,
                                          (TA_MAType)fastDMAType);
      if (unsyncSize() > (unsigned int)lookback) {
        int begIdx;
        int nbElement;

        vector<double> x(unsyncSize());

        TA_STOCHRSI(0, unsyncSize() - 1, series.getArray(), period, fastKPeriod,
                    fastDPeriod, (TA_MAType)fastDMAType, &begIdx, &nbElement,
                    &x.at(lookback), &at(lookback));
      }
    }
  };

 private:
  int _period;
  int _fastKPeriod;
  int _fastDPeriod;
  MAType _fastDMAType;

 private:
  static const Id calculateId(const SeriesImpl& series, unsigned int period,
                              int fastKPeriod, int fastDPeriod,
                              MAType fastDMAType) {
    std::ostringstream o;
    o << series.getId() << ", " << period << ", " << fastKPeriod << ", "
      << fastDPeriod << ", " << fastDMAType << " - Stochastic RSI Fast K";

    return Id(o.str());
  }

 public:
  MakeStochRSIFastDSeries(const SeriesImpl& series, int period, int fastKPeriod,
                          int fastDPeriod, MAType fastDMAType)
      : MakeFromSeries(series, calculateId(series, period, fastKPeriod,
                                           fastDPeriod, fastDMAType)),
        _period(period),
        _fastKPeriod(fastKPeriod),
        _fastDPeriod(fastDPeriod),
        _fastDMAType(fastDMAType) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(new IndicatorCacheable(
        new STOCHRSISeries(getSeries(), _period, _fastKPeriod, _fastDPeriod,
                           _fastDMAType, id()),
        id()));
  }
};

class Op2SeriesBase : public CacheableBuilderX {
 private:
  const SeriesAbstr& _series1;
  const SeriesAbstr& _series2;

 protected:
  virtual SeriesImpl* makeUnsyncSeries(const SeriesAbstr& series1,
                                       const SeriesAbstr& series2) const = 0;
  virtual SeriesImpl* makeSyncSeries(const SeriesAbstr& series1,
                                     const SeriesAbstr& series2) const = 0;

  static const Id calculateId(const SeriesAbstr& series1,
                              const SeriesAbstr& series2) {
    return Id();
  }

 public:
  Op2SeriesBase(const SeriesAbstr& series1,
                const SeriesAbstr& series2) throw(std::bad_cast)
      : _series1(series1),
        _series2(series2),
        CacheableBuilderX(calculateId(series1, series2)) {}

  virtual CacheableSeriesPtr make() const {
    if (_series1.size() != _series2.size())
      throw OperationOnUnequalSizeSeriesException(_series1.size(),
                                                  _series2.size());
    else {
      bool s1s = _series1.isSynchronized();
      bool s2s = _series2.isSynchronized();
      if (!s1s || !s2s)
        /* create a new unsync series of the common size, if at least one of
         * them is unsynchronized*/
        return CacheableSeriesPtr(
            new IndicatorCacheable(makeUnsyncSeries(_series1, _series2), id()));
      else {
        if (*(_series1.synchronizer()) != *(_series2.synchronizer())) {
          throw OperationOnSeriesSyncedToDifferentSynchronizers();
        } else
          return CacheableSeriesPtr(
              new IndicatorCacheable(makeSyncSeries(_series1, _series2), id()));
      }
    }
  }
};

#define MAKE_OP_2SERIES(CLASS_NAME, OPERATOR)                                  \
  class CLASS_NAME : public Op2SeriesBase \
{                                   \
    \
private : class Op : public SeriesImpl {                                       \
     public:                                                                   \
      /* generates an unsync series (at least on of series1 or 2 is */         \
      /* unsynchronized */                                                     \
      Op(const SeriesAbstr& series1, const SeriesAbstr& series2, const Id& id) \
          : SeriesImpl(series1.size(), SynchronizerPtr(), id) {                \
        for (size_t n = 0; n < series1.size(); n++)                            \
          _v.at(n) = series1[n] OPERATOR series2[n];                           \
      }                                                                        \
      /* generates a synced series*/                                           \
                                                                               \
      Op(const SeriesAbstr& series1, const SeriesAbstr& series2,               \
         SynchronizerPtr synchronizer, const Id& id)                           \
          : SeriesImpl(series1.unsyncSize(), synchronizer, id) {               \
        const double* s1 = series1.getArray();                                 \
        const double* s2 = series2.getArray();                                 \
        for (size_t n = 0; n < series1.unsyncSize(); n++)                      \
          _v.at(n) = s1[n] OPERATOR s2[n];                                     \
      }                                                                        \
    };                                                                         \
                                                                               \
    virtual SeriesImpl* makeUnsyncSeries(const SeriesAbstr& series1,           \
                                         const SeriesAbstr& series2) const {   \
      return new Op(series1, series2, id());                                   \
    }                                                                          \
    virtual SeriesImpl* makeSyncSeries(const SeriesAbstr& series1,             \
                                       const SeriesAbstr& series2) const {     \
      return new Op(series1, series2, series1.synchronizer(), id());           \
    }                                                                          \
    \
public : CLASS_NAME(const SeriesAbstr& series1,                                \
                    const SeriesAbstr& series2) throw(std::bad_cast)           \
        : Op2SeriesBase(series1, series2) {}                                   \
  \
};

MAKE_OP_2SERIES(MakeMultiplySeries, *)
MAKE_OP_2SERIES(MakeAddSeries, +)
MAKE_OP_2SERIES(MakeDivideSeries, /)
MAKE_OP_2SERIES(MakeSubtractSeries, -)

typedef TA_RetCode (*TA_FUNC2)(int, int, const double[], const double[], int,
                               int*, int*, double[]);

template <TA_FUNC2 T>
class TAFunc2Constants {
 public:
  static const LPCTSTR _id;
};

template <TA_LOOKBACK_INT T, TA_FUNC2 U>
class MakeOp2SeriesPeriod : public Op2SeriesBase {
 private:
  unsigned int _period;

 private:
  class X : public SeriesImpl {
   public:
    X(const SeriesAbstr& series1, const SeriesAbstr& series2,
      unsigned int period, const Id& id)
        : SeriesImpl(series1.size(), SynchronizerPtr(), id) {
      correlation(series1.getArray(), series2.getArray(), series1.size(),
                  period);
    }

    X(const SeriesAbstr& series1, const SeriesAbstr& series2,
      unsigned int period, SynchronizerPtr synchronizer, const Id& id)
        : SeriesImpl(series1.unsyncSize(), synchronizer, id) {
      correlation(series1.getArray(), series2.getArray(), series1.unsyncSize(),
                  period);
    }

   private:
    void correlation(const TA_Real* series1, const TA_Real* series2,
                     unsigned int size, unsigned int period) {
      int begIdx;
      int nbElement;
      (*U)(0, size - 1, series1, series2, period, &begIdx, &nbElement,
           &at((*T)(period)));
    }
  };

 protected:
  virtual SeriesImpl* makeUnsyncSeries(const SeriesAbstr& series1,
                                       const SeriesAbstr& series2) const {
    return new X(series1, series2, _period, id());
  }
  virtual SeriesImpl* makeSyncSeries(const SeriesAbstr& series1,
                                     const SeriesAbstr& series2) const {
    return new X(series1, series2, _period, series1.synchronizer(), id());
  }

 public:
  MakeOp2SeriesPeriod(const SeriesAbstr& series1, const SeriesAbstr& series2,
                      unsigned int period)
      : Op2SeriesBase(series1, series2), _period(period) {}
};

typedef MakeOp2SeriesPeriod<TA_CORREL_Lookback, TA_CORREL>
    MakeCorrelationSeries;
typedef MakeOp2SeriesPeriod<TA_BETA_Lookback, TA_BETA> MakeBetaSeries;

class MakeShiftRightSeries : public CacheableBuilderX {
 private:
  unsigned int _n;
  const SeriesAbstr& _series;

  class ShiftRight : public SeriesImpl {
   public:
    ShiftRight(const SeriesAbstr& series, unsigned int n, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      const std::vector<double>& v(series.getVector());

      //        std::cout << "shifting Right " ) << n << _T( " elements, size: "
      //        ) << _v.size( << std::endl;

      if (n < series.unsyncSize())
        std::copy_backward(v.begin(), v.end() - n, _v.end());
    }
  };

 public:
  static const Id calculateId(const SeriesAbstr& series, unsigned int n) {
    return Id();
  }

  MakeShiftRightSeries(const SeriesAbstr& series, unsigned int n)
      : _series(series), _n(n), CacheableBuilderX(calculateId(series, n)) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new ShiftRight(_series, _n, id()), id()));
  }
};

class MakeShiftLeftSeries : public CacheableBuilderX {
 private:
  unsigned int _n;
  const SeriesAbstr& _series;

  class ShiftLeft : public SeriesImpl {
   public:
    ShiftLeft(const SeriesAbstr& series, unsigned int n, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      const std::vector<double>& v(series.getVector());

      //        std::cout << "shifting left " ) << n << _T( " elements, size: "
      //        ) << _v.size( << std::endl;
      if (n < series.unsyncSize())
        std::copy(v.begin() + n, v.end(), _v.begin());
    }
  };

 public:
  static const Id calculateId(const SeriesAbstr& series, unsigned int n) {
    return Id();
  }

  MakeShiftLeftSeries(const SeriesAbstr& series, unsigned int n)
      : _series(series), _n(n), CacheableBuilderX(calculateId(series, n)) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new ShiftLeft(_series, _n, id()), id()));
  }
};

#define MAKE_OP_SERIES_TO_VALUE(CLASS_NAME, BIND_FUNCTION, OP, ID_STRING)    \
  class CLASS_NAME : public CacheableBuilderX \
{                             \
    \
private : const SeriesAbstr& _series;                                        \
    const double _value;                                                     \
    \
\
private : class OpValue : public SeriesImpl {                                \
     public:                                                                 \
      OpValue(const SeriesAbstr& series, double value, const Id& id)         \
          : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {     \
        std::transform(series.getVector().begin(), series.getVector().end(), \
                       _v.begin(), BIND_FUNCTION(OP<double>(), (value)));    \
      }                                                                      \
    };                                                                       \
    static const Id calculateId(const SeriesImpl& series, double value) {    \
      std::ostringstream o;                                                  \
      o << value;                                                            \
      return Id(series.getId() + ID_STRING + o.str());                       \
    }                                                                        \
    \
\
\
public : CLASS_NAME(const SeriesImpl& series,                                \
                    double value) throw(std::bad_cast)                       \
        : _series(series),                                                   \
          _value(value),                                                     \
          CacheableBuilderX(calculateId(series, value)) {}                   \
                                                                             \
    virtual CacheableSeriesPtr make() const {                                \
      return CacheableSeriesPtr(                                             \
          new IndicatorCacheable(new OpValue(_series, _value, id()), id())); \
    }                                                                        \
  \
};

MAKE_OP_SERIES_TO_VALUE(MakeAddSeriesToValue, std::bind2nd, std::plus,
                        " - add value - ")
MAKE_OP_SERIES_TO_VALUE(MakeSubtractValueFromSeries, std::bind2nd, std::minus,
                        " - subtract value - ")
MAKE_OP_SERIES_TO_VALUE(MakeDivideSeriesByValue, std::bind2nd, std::divides,
                        " - divide by value - ")
MAKE_OP_SERIES_TO_VALUE(MakeDivideValueBySeries, std::bind1st, std::divides,
                        " - divide value by - ")
MAKE_OP_SERIES_TO_VALUE(MakeSubtractSeriesFromValue, std::bind1st, std::minus,
                        " - subtract from value - ")
MAKE_OP_SERIES_TO_VALUE(MakeMultiplySeriesByValue, std::bind2nd,
                        std::multiplies, " - multiply by value - ")

// TODO: mark this as not cacheable, and mark the series created from them, so
// they could be removed from the
// cache.
class EmptySeries : public SeriesImpl {
 private:
  static unsigned long _l;
  static Mutex _mutex;

  static const Id calculateId() {
    Lock lock(_mutex);
    std::ostringstream o;

    o << _l++;

    return o.str();
  }

 public:
  EmptySeries() : SeriesImpl(calculateId()) {}

  EmptySeries(size_t size)
      : SeriesImpl(size, SynchronizerPtr(), calculateId()) {}
};

typedef TA_RetCode (*TA_FUNC0)(int, int, const double[], int*, int*, double[]);

template <TA_FUNC0 T>
class TAFunc0Constants {
 public:
  static const char* _id;
};

template <>
const char* TAFunc0Constants<TA_SIN>::_id = "- Sin";
template <>
const char* TAFunc0Constants<TA_COS>::_id = "- Cos";
template <>
const char* TAFunc0Constants<TA_TAN>::_id = "- Tan";
template <>
const char* TAFunc0Constants<TA_COSH>::_id = "- Cosh";
template <>
const char* TAFunc0Constants<TA_SINH>::_id = "- Sinh";
template <>
const char* TAFunc0Constants<TA_TANH>::_id = "- Tanh";
template <>
const char* TAFunc0Constants<TA_ACOS>::_id = "- Acos";
template <>
const char* TAFunc0Constants<TA_ASIN>::_id = "- Asin";
template <>
const char* TAFunc0Constants<TA_ATAN>::_id = "- Atan";
template <>
const char* TAFunc0Constants<TA_CEIL>::_id = "- Ceil";
template <>
const char* TAFunc0Constants<TA_FLOOR>::_id = "- Floor";
template <>
const char* TAFunc0Constants<TA_EXP>::_id = "- Exp";
template <>
const char* TAFunc0Constants<TA_SQRT>::_id = "- Sqrt";
template <>
const char* TAFunc0Constants<TA_LN>::_id = "- Ln";
template <>
const char* TAFunc0Constants<TA_LOG10>::_id = "- Log10";

template <TA_LOOKBACK TA_LB, TA_FUNC0 TA_FUNC>
class MakeSeriesTAFunc0 : public MakeFromSeries {
 private:
  /**
   * Simple Moving Average - calculated iteratively
   */
  // MAX value over a period
  class XSeries : public SeriesImpl {
   public:
    // TODO: enforce that T is of type series using function_requires
    XSeries(const SeriesImpl& series, const Id& id)
        : SeriesImpl(series.unsyncSize(), series.synchronizer(), id) {
      if (unsyncSize() > (unsigned int)TA_LB()) {
        int begIdx;
        int nbElement;
        TA_FUNC(0, unsyncSize() - 1, series.getArray(), &begIdx, &nbElement,
                &at(TA_LB()));
      }
    }
  };

 public:
  MakeSeriesTAFunc0(const SeriesImpl& series)
      : MakeFromSeries(series, TAFunc0Constants<TA_FUNC>::_id) {}

  virtual CacheableSeriesPtr make() const {
    return CacheableSeriesPtr(
        new IndicatorCacheable(new XSeries(getSeries(), id()), id()));
  }
};

typedef MakeSeriesTAFunc0<TA_SIN_Lookback, TA_SIN> MakeSinSeries;
typedef MakeSeriesTAFunc0<TA_COS_Lookback, TA_COS> MakeCosSeries;
typedef MakeSeriesTAFunc0<TA_TAN_Lookback, TA_TAN> MakeTanSeries;
typedef MakeSeriesTAFunc0<TA_COSH_Lookback, TA_COSH> MakeCoshSeries;
typedef MakeSeriesTAFunc0<TA_SINH_Lookback, TA_SINH> MakeSinhSeries;
typedef MakeSeriesTAFunc0<TA_TANH_Lookback, TA_TANH> MakeTanhSeries;
typedef MakeSeriesTAFunc0<TA_ACOS_Lookback, TA_ACOS> MakeAcosSeries;
typedef MakeSeriesTAFunc0<TA_ASIN_Lookback, TA_ASIN> MakeAsinSeries;
typedef MakeSeriesTAFunc0<TA_ATAN_Lookback, TA_ATAN> MakeAtanSeries;
typedef MakeSeriesTAFunc0<TA_CEIL_Lookback, TA_CEIL> MakeCeilSeries;
typedef MakeSeriesTAFunc0<TA_FLOOR_Lookback, TA_FLOOR> MakeFloorSeries;
typedef MakeSeriesTAFunc0<TA_EXP_Lookback, TA_EXP> MakeExpSeries;
typedef MakeSeriesTAFunc0<TA_SQRT_Lookback, TA_SQRT> MakeSqrtSeries;
typedef MakeSeriesTAFunc0<TA_LN_Lookback, TA_LN> MakeLnSeries;
typedef MakeSeriesTAFunc0<TA_LOG10_Lookback, TA_LOG10> MakeLog10Series;
