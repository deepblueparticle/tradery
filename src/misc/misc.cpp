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

#include "stdafx.h"
#include <misc.h>
#include <versionno.h>
#pragma comment(lib, "rpcrt4")
#include "rpc.h"
#include "rpcdce.h"
#include <math.h>
#include "stringformat.h"

using namespace tradery;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

MISC_API tradery::Mutex tradery::m;
MISC_API tradery::Mutex tradery::m_debug;
MISC_API int idcount = 0;
MISC_API int ObjCount::_objCount;
MISC_API int ObjCount::_totalObjects;
MISC_API std::wostream& ObjCount::_os = std::wcout;
MISC_API int RefCountable::_totalCount = 0;

static boost::posix_time::ptime EPOCH(boost::gregorian::date(1970, 1, 1));

MISC_API std::ostream& tradery::sprint(const std::string& str,
                                       std::ostream& os) {
  Lock lock(m);
  os << str;
  return os;
}

MISC_API std::ostream& tradery::sprint(const std::ostringstream& o,
                                       std::ostream& os) {
  Lock lock(m);
  os << o.str();
  return os;
}

#ifdef _DEBUG
void tradery::tsprint_debug(const std::ostringstream& o) {
  tsprint_debug(o.str());
}

void tradery::tsprint_debug(const std::string& str) {
  Lock lock(m_debug);
  OutputDebugString(s2ws(str).c_str());
}
#endif

class DateImpl;
class DateTimeImpl;

class DateDurationImpl : public DateDurationAbstr {
  friend class DateImpl;
  friend class DateTimeImpl;

 private:
  boost::gregorian::date_duration _duration;

 public:
  DateDurationImpl(boost::gregorian::date_duration duration)
      : _duration(duration) {}

  DateDurationImpl(long days) : _duration(days) {}

  DateDurationImpl() : _duration(boost::gregorian::date_duration::unit()) {}

  virtual ~DateDurationImpl() {}

  virtual long days() const { return _duration.days(); }
  virtual bool is_negative() const { return _duration.is_negative(); }

  virtual bool operator==(const DateDurationAbstr& duration) const {
    try {
      return _duration ==
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator!=(const DateDurationAbstr& duration) const {
    try {
      return _duration !=
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator>(const DateDurationAbstr& duration) const {
    try {
      return _duration >
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator<(const DateDurationAbstr& duration) const {
    try {
      return _duration <
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator>=(const DateDurationAbstr& duration) const {
    try {
      return _duration >=
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator<=(const DateDurationAbstr& duration) const {
    try {
      return _duration <=
             dynamic_cast<const DateDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateDurationAbstrPtr operator+(
      const DateDurationAbstr& duration) const {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(
          _duration +
          dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual DateDurationAbstrPtr operator-(
      const DateDurationAbstr& duration) const {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(
          _duration -
          dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual DateDurationAbstrPtr operator/(int divisor) {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(_duration / divisor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  /*  virtual DateDurationAbstrPtr operator*( int factor )
    {
      try
      {
        return DateDurationAbstrPtr( new DateDurationImpl( _duration * factor )
    );
      }
                  catch( const std::bad_cast& )
      {
        assert( false );
        throw;
      }
    }
  */
  virtual DateDurationAbstrPtr operator-=(const DateDurationAbstr& duration) {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(
          _duration -=
          dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual DateDurationAbstrPtr operator+=(const DateDurationAbstr& duration) {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(
          _duration +=
          dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual DateDurationAbstrPtr operator/=(int factor) {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(_duration /= factor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  /*  virtual DateDurationAbstrPtr operator*=( int factor )
    {
      try
      {
        return DateDurationAbstrPtr( new DateDurationImpl( _duration *= factor )
    );
      }
                  catch( const std::bad_cast& )
      {
        assert( false );
        throw;
      }
    }
  */
};

class DateImpl : public DateAbstr {
  friend class DateTimeImpl;

 private:
  boost::gregorian::date _date;

 public:
  DateImpl(unsigned int year, unsigned int month, unsigned int day)
      : _date(year, month, day) {}

  DateImpl(const boost::gregorian::date date) : _date(date) {}

  DateImpl() {}

  virtual std::string to_simple_string() const {
    return boost::gregorian::to_simple_string_type<char>(_date);
  }
  virtual std::string to_iso_string() const {
    return boost::gregorian::to_iso_string_type<char>(_date);
  }
  virtual std::string to_iso_extended_string() const {
    return boost::gregorian::to_iso_extended_string_type<char>(_date);
  }

  virtual unsigned short year() const { return _date.year(); }
  virtual unsigned short month() const { return _date.month(); }
  virtual unsigned short day() const { return _date.day(); }
  virtual bool is_infinity() const { return _date.is_infinity(); }
  virtual bool is_neg_infinity() const { return _date.is_neg_infinity(); }
  virtual bool is_pos_infinity() const { return _date.is_pos_infinity(); }
  virtual bool is_not_a_date() const { return _date.is_not_a_date(); }
  virtual bool is_special() const { return _date.is_special(); }

  int week_number() const { return _date.week_number(); }
  virtual bool operator==(const DateAbstr& date) const {
    try {
      return _date == dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator!=(const DateAbstr& date) const {
    try {
      return _date != dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator>(const DateAbstr& date) const {
    try {
      return _date > dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator<(const DateAbstr& date) const {
    try {
      return _date < dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator>=(const DateAbstr& date) const {
    try {
      return _date >= dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator<=(const DateAbstr& date) const {
    try {
      return _date <= dynamic_cast<const DateImpl&>(date)._date;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateAbstrPtr operator-(const DateDurationAbstr& duration) const {
    try {
      return DateAbstrPtr(new DateImpl(
          _date - dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateAbstrPtr operator+(const DateDurationAbstr& duration) const {
    try {
      return DateAbstrPtr(new DateImpl(
          _date + dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateDurationAbstrPtr operator-(const DateAbstr& date) const {
    try {
      return DateDurationAbstrPtr(new DateDurationImpl(
          _date - dynamic_cast<const DateImpl&>(date)._date));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateAbstrPtr operator-=(const DateDurationAbstr& duration) {
    try {
      return DateAbstrPtr(new DateImpl(
          _date -= dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateAbstrPtr operator+=(const DateDurationAbstr& duration) {
    try {
      return DateAbstrPtr(new DateImpl(
          _date += dynamic_cast<const DateDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

/*
class DateFormat
{
private:
  const TCHAR _sep;
  const std::wstring _format;
public:
  DateFormat( const std::wstring& format )
  : _format( format )
  {

  }

private:
  searchSep()
  {
    std::wstring::size_type pos = _format.find_first_not_of( _T( "mdy" ) );

    if( pos != std::wstring::npos)
      _sep = _format.at( pos );

    Tokenizer tok( _format, _sep );

    if( tok.size() != 3 )
      throw( DateFormatException() );

  }
};

MISC_API Date::Date( const std::wstring& date, const std::wstring& format )
throw( DateException )
{
  Tokenizer(
}
*/

/*MISC_API Date::Date( const std::wstring& xdate, DateFormat format, bool sep )
throw( DateException )
{
        parse( xdate, format, std::wstring( sep ? _T( "/-" ) : _T( "" ) ) );
}
*/

MISC_API Date::Date(const std::string& xdate, DateFormat format,
                    const std::string& sep) throw(DateException) {
  parse(xdate, format, sep);
#if _DEBUG
  _str = to_simple_string();
#endif
}

void Date::parse(const std::string& xdate, DateFormat format,
                 const std::string& sep) throw(DateException) {
  std::string date = tradery::trim(xdate);

  unsigned int year;
  unsigned int month;
  unsigned int day;

  if (format != xyz) {
    unsigned int first;
    ;
    unsigned int second;
    unsigned int third;

    if (!sep.empty()) {
      Tokenizer tokens(date, sep);

      if (tokens.size() != 3)
        throw DateException(date,
                            tradery::format("Invalid date: \"%1%\"", date));

      first = atoi(tokens[0].c_str());
      second = atoi(tokens[1].c_str());
      third = atoi(tokens[2].c_str());

    } else {
      if (date.length() == 6) {
        first = atoi(date.substr(0, 2).c_str());
        second = atoi(date.substr(2, 2).c_str());
        third = atoi(date.substr(4, 2).c_str());
      } else if (date.length() == 8) {
        switch (format) {
          case us:
          case european:
            first = atoi(date.substr(0, 2).c_str());
            second = atoi(date.substr(2, 2).c_str());
            third = atoi(date.substr(4, 4).c_str());
            break;
          case iso:
            first = atoi(date.substr(0, 4).c_str());
            second = atoi(date.substr(4, 2).c_str());
            third = atoi(date.substr(6, 2).c_str());
            break;
          default:
            throw DateException(date, "Unknown format type");
            break;
        }
      } else
        throw DateException(date,
                            tradery::format("Invalid date: \"%1%\"", date));
    }

    switch (format) {
      case us:
        year = third;
        month = first;
        day = second;
        break;
      case european:
        year = third;
        month = second;
        day = first;
        break;
      case iso:
        year = first;
        month = second;
        day = third;
        break;
      default:
        throw DateException(date, "Unknown format type");
        break;
    }

    year = year < 50 ? year + 2000 : (year < 100 ? year + 1900 : year);

    if (year < 1800 || year > 2100)
      throw DateException(
          date, "Year must be an integer value between 1800 and 2100");

    if (month < 1 || month > 12)
      throw DateException(date,
                          "Month must be an integer value between 1 and 12");

    // make sure day is valid for month and year
    if (day < 1 || day > 31)
      throw DateException(date,
                          "Day must be an integer value between 1 and 31");

  } else {
    static std::string months[] = {"",    "Jan", "Feb", "Mar", "Apr",
                                   "May", "Jun", "Jul", "Aug", "Sep",
                                   "Oct", "Nov", "Dec"};

    Tokenizer tokens(xdate, sep);

    if (tokens.size() != 3) throw DateException(date, "Wrong date format");

    unsigned int i = 0;
    for (; i <= 12; i++) {
      if (tokens[1] == months[i]) {
        month = i;
        break;
      }
    }

    if (i > 12) throw DateException(date, "Wrong date format");

    year = atoi(tokens[2].c_str());

    if (year < 30) year += 2000;
    if (year < 100 && year >= 30) year += 1900;

    day = atoi(tokens[0].c_str());
  }
  _date = DateAbstr::make(year, month, day);
}

class TimeDurationImpl : public TimeDurationAbstr {
  friend class DateTimeImpl;

 private:
  boost::posix_time::time_duration _duration;

 public:
  TimeDurationImpl(__int64 hours, __int64 mins, __int64 secs, __int64 frac_secs)
      : _duration(hours, mins, secs, frac_secs) {}

  TimeDurationImpl(boost::posix_time::time_duration duration)
      : _duration(duration) {}

  virtual long hours() const { return _duration.hours(); }
  virtual long minutes() const { return _duration.minutes(); }
  virtual long seconds() const { return _duration.seconds(); }
  virtual long total_seconds() const { return _duration.total_seconds(); }
  virtual long fractional_seconds() const {
    return _duration.fractional_seconds();
  }
  virtual bool is_negative() const { return _duration.is_negative(); }
  //	virtual time_duration invert_sign() { return _duration.hours() ;}

  virtual bool operator==(const TimeDurationAbstr& duration) const {
    try {
      return _duration ==
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator!=(const TimeDurationAbstr& duration) const {
    try {
      return _duration !=
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator>(const TimeDurationAbstr& duration) const {
    try {
      return _duration >
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator<(const TimeDurationAbstr& duration) const {
    try {
      return _duration <
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator>=(const TimeDurationAbstr& duration) const {
    try {
      return _duration >=
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator<=(const TimeDurationAbstr& duration) const {
    try {
      return _duration <=
             dynamic_cast<const TimeDurationImpl&>(duration)._duration;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual TimeDurationAbstrPtr operator+(
      const TimeDurationAbstr& duration) const {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(
          _duration +
          dynamic_cast<const TimeDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual TimeDurationAbstrPtr operator-(
      const TimeDurationAbstr& duration) const {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(
          _duration -
          dynamic_cast<const TimeDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual TimeDurationAbstrPtr operator/(int divisor) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(_duration / divisor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual TimeDurationAbstrPtr operator*(int factor) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(_duration * factor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual TimeDurationAbstrPtr operator-=(const TimeDurationAbstr& duration) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(
          _duration -=
          dynamic_cast<const TimeDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual TimeDurationAbstrPtr operator+=(const TimeDurationAbstr& duration) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(
          _duration +=
          dynamic_cast<const TimeDurationImpl&>(duration)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual TimeDurationAbstrPtr operator/=(int factor) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(_duration /= factor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual TimeDurationAbstrPtr operator*=(int factor) {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(_duration *= factor));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

class DateTimeImpl : public DateTimeAbstr {
 private:
  boost::posix_time::ptime _time;

 public:
  DateTimeImpl() : _time() {}

  DateTimeImpl(boost::posix_time::ptime time) : _time(time) {}

  DateTimeImpl(const DateImpl& date) : _time(date._date) {}

  DateTimeImpl(const DateImpl& date, const TimeDurationImpl& duration)
      : _time(date._date, duration._duration) {}

  DateTimeImpl(const DateTimeImpl& time) : _time(time._time) {}

  bool operator<(const DateTimeAbstr& xtime) const {
    try {
      return _time < dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  bool operator>(const DateTimeAbstr& xtime) const {
    try {
      return _time > dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator>=(const DateTimeAbstr& xtime) const {
    try {
      return _time >= dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator<=(const DateTimeAbstr& xtime) const {
    try {
      return _time <= dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual bool operator==(const DateTimeAbstr& xtime) const {
    try {
      return _time == dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
  virtual bool operator!=(const DateTimeAbstr& xtime) const {
    try {
      return _time != dynamic_cast<const DateTimeImpl&>(xtime)._time;
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  std::string to_simple_string() const {
    return boost::posix_time::to_simple_string_type<char>(_time);
  }

  std::string to_iso_string() const {
    return boost::posix_time::to_iso_string_type<char>(_time);
  }

  __int64 to_epoch_time() const {
    boost::posix_time::time_duration diff = _time - EPOCH;

    return diff.total_seconds();
  }

  virtual DateAbstrPtr date() const {
    return DateAbstrPtr(new DateImpl(_time.date()));
  }

  virtual TimeDurationAbstrPtr time_of_day() const {
    return TimeDurationAbstrPtr(new TimeDurationImpl(_time.time_of_day()));
  }

  virtual bool is_not_a_date_time() const { return _time.is_not_a_date_time(); }

  virtual bool is_infinity() const { return _time.is_infinity(); }

  virtual bool is_pos_infinity() const { return _time.is_pos_infinity(); }

  virtual bool is_neg_infinity() const { return _time.is_neg_infinity(); }

  virtual bool is_special() const { return _time.is_special(); }

  virtual TimeDurationAbstrPtr operator-(const DateTimeAbstr& time) const {
    try {
      return TimeDurationAbstrPtr(new TimeDurationImpl(
          _time - dynamic_cast<const DateTimeImpl&>(time)._time));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator+(const DateDurationAbstr& dd) const {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time + dynamic_cast<const DateDurationImpl&>(dd)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator+=(const DateDurationAbstr& dd) {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time += dynamic_cast<const DateDurationImpl&>(dd)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator-(const DateDurationAbstr& dd) const {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time - dynamic_cast<const DateDurationImpl&>(dd)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator-=(const DateDurationAbstr& dd) {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time -= dynamic_cast<const DateDurationImpl&>(dd)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator+(const TimeDurationAbstr& td) const {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time + dynamic_cast<const TimeDurationImpl&>(td)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator+=(const TimeDurationAbstr& td) {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time += dynamic_cast<const TimeDurationImpl&>(td)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator-(const TimeDurationAbstr& td) const {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time - dynamic_cast<const TimeDurationImpl&>(td)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }

  virtual DateTimeAbstrPtr operator-=(const TimeDurationAbstr& td) {
    try {
      return DateTimeAbstrPtr(new DateTimeImpl(
          _time -= dynamic_cast<const TimeDurationImpl&>(td)._duration));
    } catch (const std::bad_cast&) {
      assert(false);
      throw;
    }
  }
};

MISC_API DateTimeAbstrPtr DateTimeAbstr::localTimeSubSec() {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::date_time::microsec_clock<
                       boost::posix_time::ptime>::local_time()));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::localTimeSec() {
  return DateTimeAbstrPtr(new DateTimeImpl(
      boost::date_time::second_clock<boost::posix_time::ptime>::local_time()));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::universalTime() {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::date_time::second_clock<
                       boost::posix_time::ptime>::universal_time()));
}

DateDurationAbstrPtr DateDurationAbstr::make(long days) {
  return DateDurationAbstrPtr(new DateDurationImpl(days));
}

DateDurationAbstrPtr DateDurationAbstr::make() {
  return DateDurationAbstrPtr(new DateDurationImpl());
}

DateTimeAbstrPtr DateTimeAbstr::makeFromIsoString(
    const std::string& iso_string) {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::posix_time::from_iso_string(iso_string)));
}

DateTimeAbstrPtr DateTimeAbstr::makeFromDelimitedString(
    const std::string& delimitedString) {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::posix_time::time_from_string(delimitedString)));
}

// YYYYMMDDHHMMSS
DateTimeAbstrPtr DateTimeAbstr::makeFromNonDelimitedString(
    const std::string& nonDelimitedString) {
  if (nonDelimitedString.length() != std::string("20091007233000").length())
    return DateTimeAbstrPtr(new DateTimeImpl());
  else {
    std::string str(nonDelimitedString);
    str.insert(8, "T");
    return makeFromIsoString(str);
  }
}

TimeDurationAbstrPtr TimeDurationAbstr::make(__int64 hours, __int64 mins,
                                             __int64 secs, __int64 frac_secs) {
  return TimeDurationAbstrPtr(
      new TimeDurationImpl(hours, mins, secs, frac_secs));
}

DateAbstrPtr DateAbstr::make(unsigned int year, unsigned int month,
                             unsigned int day) {
  return DateAbstrPtr(new DateImpl(year, month, day));
}

DateAbstrPtr DateAbstr::make() { return DateAbstrPtr(new DateImpl()); }

DateTimeAbstrPtr DateTimeAbstr::make(const DateAbstr& date,
                                     const TimeDurationAbstr& duration) {
  try {
    return DateTimeAbstrPtr(
        new DateTimeImpl(dynamic_cast<const DateImpl&>(date),
                         dynamic_cast<const TimeDurationImpl&>(duration)));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make() {
  return DateTimeAbstrPtr(new DateTimeImpl());
}
DateTimeAbstrPtr DateTimeAbstr::make(const DateAbstr& date) {
  try {
    return DateTimeAbstrPtr(
        new DateTimeImpl(dynamic_cast<const DateImpl&>(date)));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make(const DateTimeAbstr& time) {
  try {
    return DateTimeAbstrPtr(
        new DateTimeImpl(dynamic_cast<const DateTimeImpl&>(time)));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

DateTimeAbstrPtr DateTimeAbstr::make(__int64 time) {
  boost::posix_time::ptime t(
      EPOCH + boost::posix_time::time_duration(time / 3600, (time % 3600) / 60,
                                               time % 60, 0));
  return DateTimeAbstrPtr(new DateTimeImpl(t));
}

// make special values
MISC_API DateAbstrPtr DateAbstr::makePosInfinity() {
  return DateAbstrPtr(
      new DateImpl(boost::gregorian::date(boost::date_time::pos_infin)));
}
MISC_API DateAbstrPtr DateAbstr::makeNegInfinity() {
  return DateAbstrPtr(
      new DateImpl(boost::gregorian::date(boost::date_time::neg_infin)));
}
MISC_API DateAbstrPtr DateAbstr::makeMaxDate() {
  return DateAbstrPtr(
      new DateImpl(boost::gregorian::date(boost::date_time::max_date_time)));
}
MISC_API DateAbstrPtr DateAbstr::makeMinDate() {
  return DateAbstrPtr(
      new DateImpl(boost::gregorian::date(boost::date_time::min_date_time)));
}
MISC_API DateAbstrPtr DateAbstr::makeNotADate() {
  return DateAbstrPtr(
      new DateImpl(boost::gregorian::date(boost::date_time::not_a_date_time)));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::makePosInfinity() {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::posix_time::ptime(boost::date_time::pos_infin)));
}

MISC_API DateTimeAbstrPtr DateTimeAbstr::makeNegInfinity() {
  return DateTimeAbstrPtr(
      new DateTimeImpl(boost::posix_time::ptime(boost::date_time::neg_infin)));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeMaxDateTime() {
  return DateTimeAbstrPtr(new DateTimeImpl(
      boost::posix_time::ptime(boost::date_time::max_date_time)));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeMinDateTime() {
  return DateTimeAbstrPtr(new DateTimeImpl(
      boost::posix_time::ptime(boost::date_time::min_date_time)));
}
MISC_API DateTimeAbstrPtr DateTimeAbstr::makeNotADateTime() {
  return DateTimeAbstrPtr(new DateTimeImpl(
      boost::posix_time::ptime(boost::date_time::not_a_date_time)));
}

DateAbstrPtr DateAbstr::make(const DateAbstr& date) {
  try {
    return DateAbstrPtr(new DateImpl(dynamic_cast<const DateImpl&>(date)));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

TimeDurationAbstrPtr TimeDurationAbstr::make(
    const TimeDurationAbstr& duration) {
  try {
    return TimeDurationAbstrPtr(
        new TimeDurationImpl(dynamic_cast<const TimeDurationImpl&>(duration)));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

class MutexImpl : public MutexAbstr {
  friend class LockImpl;

 private:
  boost::recursive_mutex _mutex;
};

class NonRecursiveMutexImpl : public NonRecursiveMutexAbstr {
  friend class NonRecursiveLockImpl;

 private:
  boost::mutex _mutex;
};

class LockImpl : public LockAbstr {
  friend class ConditionImpl;

 private:
  boost::lock_guard<boost::recursive_mutex> _lock;

 public:
  LockImpl(MutexImpl& mutex) : _lock(mutex._mutex) {}
};

class NonRecursiveLockImpl : public NonRecursiveLockAbstr {
  friend class ConditionImpl;

 private:
  boost::unique_lock<boost::mutex> _lock;

 public:
  NonRecursiveLockImpl(NonRecursiveMutexImpl& mutex) : _lock(mutex._mutex) {}

  virtual void lock() { _lock.lock(); }
  virtual void unlock() { _lock.unlock(); }
};

MutexAbstr* MutexAbstr::make() { return new MutexImpl(); }

NonRecursiveMutexAbstr* NonRecursiveMutexAbstr::make() {
  return new NonRecursiveMutexImpl();
}

LockAbstr* LockAbstr::make(MutexAbstr& mutex) {
  try {
    return new LockImpl(dynamic_cast<MutexImpl&>(mutex));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

NonRecursiveLockAbstr* NonRecursiveLockAbstr::make(
    NonRecursiveMutexAbstr& mutex) {
  try {
    return new NonRecursiveLockImpl(
        dynamic_cast<NonRecursiveMutexImpl&>(mutex));
  } catch (const std::bad_cast&) {
    assert(false);
    return 0;
  }
}

class ConditionImpl : public ConditionAbstr {
 private:
  boost::condition_variable _condition;

 public:
  void wait(NonRecursiveLockAbstr& lock) {
    try {
      _condition.wait(dynamic_cast<NonRecursiveLockImpl&>(lock)._lock);
    } catch (const std::bad_cast&) {
      assert(false);
    }
  }

  void notify_one() { _condition.notify_one(); }
};

ConditionAbstr* ConditionAbstr::make() { return new ConditionImpl(); }

class TimerImpl : public TimerAbstr {
 private:
  boost::timer _timer;
  double _lastValue;
  bool _running;

 public:
  TimerImpl() : _running(true), _lastValue(0.0) {}

  void restart() {
    _timer.restart();
    _running = true;
  }

  double elapsed() const { return _running ? _timer.elapsed() : _lastValue; }

  void stop() {
    if (_running) {
      _lastValue = _timer.elapsed();
      _running = false;
    }
  }

  bool isStopped() const { return !_running; }
};

TimerAbstr* TimerAbstr::make() { return new TimerImpl(); }

MISC_API Version::Version() throw(VersionException) : Version(0, 0, 0, 0) {}

MISC_API Version::Version(const char* version) throw(VersionException)
    : m_major(0), m_minor(0), m_build(0), m_revision(0) {
  parse(version);
}

Version::Version(unsigned int major, unsigned int minor, unsigned int revision,
                 unsigned int build)
    : m_major(major), m_minor(minor), m_revision(revision), m_build(build) {}

MISC_API void Version::parse(const char* version) throw(VersionException) {
  if (version != nullptr && strlen(version) > 0) {
    Tokenizer tokens(version, VERSION_SEPARATORS);

    try {
      for (size_t n = 0; n < tokens.size(); ++n) {
        unsigned int value = boost::lexical_cast<unsigned int>(tokens[n]);
        switch (n) {
          case 0:
            m_major = value;
            break;
          case 1:
            m_minor = value;
            break;
          case 2:
            m_revision = value;
            break;
          case 3:
            m_build = value;
            break;
          default:
            throw VersionException(std::string(version) +
                                   " - too many version elements");
        }
      }
    } catch (const boost::bad_lexical_cast& e) {
      const char* what = e.what();
      throw VersionException(std::string(version) + ", " + e.what());
    }
  }
}

MISC_API Seconds::Seconds(double seconds)
    : TimeDuration(0, 0, (long)seconds, fmod(seconds, 1.0) * 1000000) {}

MISC_API std::vector<std::string> tradery::cmdLineSplitter(
    const std::string& line) {
  std::vector<std::string> v;

  bool inQuotedString = false;
  bool inUnquotedString = false;
  bool escape = false;

  std::string sep(" \t");
  std::string str;
  unsigned int state = 0;

  for (std::string::size_type n = 0; n < line.size(); ++n) {
    char c(line[n]);

    switch (state) {
      case 0:
        assert(str.empty());
        switch (c) {
          case '"':
            state = 1;
            break;
          case ' ':
          case '\t':
            break;
          default:
            str += c;
            state = 2;
            break;
        }
        break;
      case 1:
        switch (c) {
          case '"':
            state = 2;
            break;
          case '\\':
            str += c;
            state = 3;
            break;
          default:
            str += c;
            break;
        }
        break;
      case 2:
        switch (c) {
          case ' ':
          case '\t':
            // assert( !str.empty() );
            if (!str.empty()) {
              v.push_back(str);
              str.clear();
            }
            state = 0;
            break;
          default:
            str += c;
            break;
        }
        break;
      case 3:
        switch (c) {
          case '"':
            state = 4;
            break;
          default:
            state = 1;
            break;
        }
        str += c;
        break;
      case 4:
        switch (c) {
          case ' ':
          case '\t':
            // assert( !str.empty() );
            if (!str.empty()) {
              v.push_back(str);
              str.clear();
            }
            state = 0;
            break;
          default:
            str += c;
            state = 1;
            break;
        }
        break;
      default:
        assert(false);
        break;
    }
  }

  if (!str.empty()) v.push_back(str);
  return v;
}

MISC_API std::string tradery::Date::toString(
    DateFormat format, const std::string& separator) const {
  if (is_special())
    return "";
  else {
    unsigned int first;
    unsigned int second;
    unsigned int third;

    switch (format) {
      case us:
        first = month();
        second = day();
        third = year();
        break;
      case european:
        first = day();
        second = month();
        third = year();
        break;
      case iso:
        first = year();
        second = month();
        third = day();
        break;
      case xyz:
        throw DateException("", "Format xyz not supported at this time");
      default:
        throw DateException("", "Unknown format type");
    }

    return tradery::format("%1%%4%%2%%4%%3%", first, second, third, separator);
  }
}
