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
#pragma warning(disable : 4290)

#ifdef DATASOURCE_EXPORTS
#define DATASOURCE_API __declspec(dllexport)
#else
#define DATASOURCE_API __declspec(dllimport)
#endif

/**
 * @file
 *  \brief Contains all data source and symbols info source adapters related
 * definitions
 *
 * A data source adapter is a module that sits between the simlib framework and
 * a specific data source, such as the file system, an Internet based real time
 * quote provider, a historical quotes data base etc.
 *
 * Data sources can connect to arbitrary types of data. However, currently the
 * Framework has support for bar and tick data. Other types, although handled by
 * the framework due to polymorphic implementtion of data types, will have to be
 * understood by user code.
 *
 * This file defines classes that constitute the common protocols that all data
 * source adapters need to support in order to communicate with the SimLib
 * framework.
 *
 * A symbols info source adapter is a module that can iterate through
 * collections of symbols. More specifically, a symbols info source adapter
 * needs to be able to read lists of symbols stored on some medium or even
 * generate them on the fly, and be prepared to iterate through the collection
 * of symbols and generate all the information needed by the framework to
 * retrieve the data associated with each symbol and run various systems using
 * this data.
 *
 */
#include <boost/shared_ptr.hpp>
#include "exceptions.h"
#include "series.h"
#include "filesymbols.h"
#include "synchronizer.h"
#include "macros.h"
#include "dataunit.h"
#include "datetimerange.h"
#include "pluginconfig.h"
#include "objcounter.h"

/** @cond */
namespace tradery {
/** @endcond */

class DataInfo;

/**
 * Abstract class - base for all data source concrete classes.
 *
 * A data source is an interface module between a specific source of data, such
 * a real time quotes library, a database etc, and the framework.
 *
 * DataSource is the base class for static (which do not update data)
 * as well as dynamic data sources (which dynamically update data).
 *
 * It is essential that a data source have an unique name among all other data
 * sources - the framework cashing mechanism as well as data source resolution
 * rely on the uniqueness of this name.
 *
 * @see DataSourceListener
 * @see DataSourceEvent
 */
class DATASOURCE_API DataSource : public PluginConfiguration {
  OBJ_COUNTER(DataSource)
 public:
  /**
   * Constructor - takes a data source name
   *
   * The name must be unique among all data source names
   *
   * @param info   data source info (id, name, description )
   * @see DataSourceID
   */
  DataSource(const Info& info) : PluginConfiguration(info) {}
  virtual ~DataSource() {}

  class DataX {
   public:
    DataX(BarsPtr dataCollection, const std::string& stamp)
        : _dataCollection(dataCollection), _stamp(stamp) {}

    /*		DataX( DataX& datax )
                            : _dataCollection( datax.releaseDataCollection() ),
       _stamp( datax.getStamp() )
                    {
                    }
    */
    virtual ~DataX() {}

    BarsPtr getDataCollection() const { return _dataCollection; }

    const std::string& getStamp() const { return _stamp; }

    size_t getSize() const { return _dataCollection->size(); }

   private:
    BarsPtr _dataCollection;
    const std::string _stamp;
  };

  typedef boost::shared_ptr<DataX> DataXPtr;

  /**
   * Requests data from the data source.
   *
   * If the range parameter is 0 or not used, it will load all available data,
   * otherwise it will load the specified range.
   *
   * Pure virtual method - in derived concrete classes will return a pointer to
   * a DataCollection object corresponding to a SymbolsInfo
   *
   * <!-- \htmlonly <FONT COLOR="RED">TODO: see what exceptions are thrown
   * here</FONT> \endhtmlonly -->
   *
   * The method can take any type of range which is derived from Range, thus
   * allowing time ranges,  or bar ranges, or any other type of ranges.
   *
   * @param dataInfo     A pointer to a DataInfo object
   * @param range  A range (time or bar index, or other). Can be 0, in which
   * case all the data is considered
   * @return a pointer to a DataCollection object, containing the data
   * @exception DataSourceException
   *                   thrown if error in getData
   * @see SymbolInfo
   * @see Bars
   * @see Range
   */
  virtual DataXPtr getData(const DataInfo* dataInfo, DateTimeRangePtr = 0) const
      throw(DataSourceException) = 0;
  virtual bool isConsistent(const std::string& stamp, const Symbol& si,
                            DateTimeRangePtr range = 0) const
      throw(DataSourceException) = 0;
};

/**
 * A "smart" pointer to a data source
 */
typedef std::auto_ptr<DataSource> DataSourcePtr;

/**
 * A symbols list iterator
 *
 * Used to iterate through a list of symbols.
 *
 * Iterators are created only by instances of the class SymbolsListSource or
 * derived, never by the user code directly.
 *
 * @see SymbolsListSource
 */
class DATASOURCE_API SymbolsIterator {
 public:
  virtual ~SymbolsIterator() {}
  /**
   * Gets the next symbol
   *
   * This method is thread safe - can be used from more than one thread at a
   * time.
   *
   * @return A pointer to a SymbolInfo object if next is available, or 0 if no
   * more symbols
   */
  virtual SymbolConstPtr getNext() = 0;
  virtual void reset() = 0;
  virtual SymbolConstPtr getFirst() = 0;
  virtual SymbolConstPtr getCurrent() = 0;
  virtual bool hasMore() = 0;
};

typedef std::auto_ptr<SymbolsIterator> SymbolsIteratorPtr;

/**
 * Abstract class - base for symbols list sources
 *
 * SymbolsListSource allows a developer to define lists of symbols that are
 * specific to the data source that is being implemented and still provide a
 * common interface that is accepted by the SimLib framework for running trading
 * systems on multiple symbols.
 *
 * The most important feature of a symbols list is an iterator, which provides a
 * common way of traversing a list of symbols for all types of concrete lists of
 * symbols.
 *
 * A symobls list source can be shared by multiple trading systems, so multiple
 * trading systems can iterate over the same symbol list simultaneously. The
 * extreme case is when multiple instances of the same trading system share the
 * same iterator, thus effectively allowing this one system to be run
 * simulaneously on multiple symbols, in multiple threads, for increased
 * performance.
 *
 * The pointers to these iterators are owned by the SymbolsListSource object, so
 * when this object is destroyed, it will delete all the iterators attached to
 * it. They do not need to and should not be deleted outside of this class.
 *
 * Concrete classes derived from SymbolsListSource must call addIterator
 * whenever a new iterator is created, to add the iterator to the list of
 * iterators associated with the SymbolsListSource object.
 *
 * @see SymbolsListIterator
 */
class DATASOURCE_API SymbolsSource : public StrVector,
                                     public PluginConfiguration {
  friend SymbolsIterator;

 private:
  // holds pointers to all iterators attached to this symbols list
  PtrVector<SymbolsIterator> _iterators;

 private:
  // copy constructor and assignment operators are made private to prevent
  // copying the semantics of this class does not allow for copying
  SymbolsSource(const SymbolsSource& s) : PluginConfiguration(s) {}

  void operator=(const SymbolsSource& s) {}

 public:
  /**
   * Constructor - takes the name of the default data source as parameter
   *
   */
  SymbolsSource(const Info& info) : PluginConfiguration(info) {}

 public:
  virtual ~SymbolsSource() {}

  /**
   * Creates a symbols list iterator
   *
   * All the iterators are owned by the SymbolsListSourceI object from which
   * they were created, so they don't need and should not be deleted elsewhere.
   * They will be deleted in the destructor of this class
   *
   * @return A pointer to the created iterator
   * @see SymbolsListIteratorI
   */
  SymbolsIterator* makeIterator();

  /**
   * creates the SymbolInfo instance for the symbol pointed to by the passed as
   * parameter
   *
   * @param i      Reference to the iterator pointing to a symbol
   * @return A pointer to a SymbolInfo instance
   */
  virtual SymbolConstPtr makeSymbol(const_iterator& i) const
      throw(SymbolsSourceException) = 0;

  /**
   * Returns the number of symbols
   *
   * @return the number of symbols
   */
  size_t size() const { return StrVector::size(); }

  /**
   * Sends the contents to an output stream
   *
   * @param os     the output stream to send the contents to
   * @return reference to the input stream
   */
  std::ostream& dump(std::ostream& os) const;
};

/**
 * Smart pointer to a SymbolslListSource
 *
 * @see SymbolsListSource
 */
typedef ManagedPtr<SymbolsSource> SymbolsSourcePtr;

// the class that presents a combination of datasources and symbols list and
// implements an iterator to get the next DataInfo this is a very important
// class - it makes the connection between one or more symbols sources and one
// or more data sources The framework will provide a default implementation that
// takes one datasource and one symbols source User defined DataInfoIterators
// can be used to implement highly customizable input data models
class DataInfoIterator {
 public:
  virtual ~DataInfoIterator() {}
  virtual DataInfoConstPtr getNext() throw(DataInfoException) = 0;
};

class SimpleDataInfoIterator : public DataInfoIterator {
  DataSource* _dataSource;
  SymbolsIterator* _symbolsIterator;

 public:
  SimpleDataInfoIterator(DataSource* dataSource,
                         SymbolsIterator* symbolsIterator)
      : _dataSource(dataSource), _symbolsIterator(symbolsIterator) {}

  virtual DataInfoConstPtr getNext() throw(DataInfoException) {
    SymbolConstPtr sym(_symbolsIterator->getNext());
    if (sym.get() != 0)
      return DataInfoConstPtr(new DataInfo(_dataSource, sym));
    else
      return DataInfoConstPtr(0);
  }
};

// a combination datasource/symbol that is passed to the framework to find the
// data

/**
 * Used by a symbols list iterator in case the call to getNext is blocking.
 *
 * Here is a scenario: in real time trading, calls are made to getNext to
 * receive the next available symbol or data. I there is no data, getNext will
 * block until there is something. In the meantime, the system may need to exit,
 * so it will signal that it is not running anymore by returning false to
 * isRunning. getNext then will test this and will return whether it has data or
 * not. This is somewhat similar to how blocking sockets work for example. If
 * the socket is blockig and there is no data, a call to closesocket will make
 * unblock the call to listen.
 */
/*class RunningStatus
{
public:
  ~RunningStatus()
  {
  }

  virtual bool isRunning() const = 0;
};
*/

/**
 * Creates an empty Bars object
 *
 * This will be populated by a datasource
 *
 * @param dataSourceName
 *                   Name of the data source
 * @param symbol     symbol
 * @param type       type - stock or symbol
 * @param resolution resolution in seconds
 * @param range      range for which data is to be retrieved
 * @return an empty bars collection object
 */
CORE_API BarsPtr createBars(const std::string& dataSourceName,
                            const std::string& symbol, BarsAbstr::Type type,
                            unsigned int resolution, DateTimeRangePtr range,
                            ErrorHandlingMode errorHandlingMode);
/**
 * Creates an empty Ticks object
 *
 * This will be populated by a datasource
 *
 * @param dataSourceName
 *                   Name of the data source
 * @param symbol     symbol
 * @param range      range for which data is to be retrieved
 * @return an empty ticks collection object
 */
CORE_API Ticks* createTicks(const std::string& dataSourceName,
                            const std::string& symbol, const Range* range);

}  // namespace tradery
