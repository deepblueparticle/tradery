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

typedef std::string Id;

/**
 * Abstract based class for classes that can have an id. The id is currently
 * implemented as a string
 *
 * The cache uses the id to identify the cached objects
 */
class Ideable {
 private:
  Id _id;

 public:
  /**
   * Constructor - takes the Id as a parameter
   *
   * @param id     the id
   */
  Ideable(const Id& id) : _id(id) {}

  virtual ~Ideable() {}

  /**
   * Retrieves the Id
   *
   * @return the id
   */
  const Id& getId() const { return _id; }
};

template <class T>
class Cacheable : public ManagedPtr<T> {
 private:
  Id _id;

 public:
  Cacheable(T* t, const Id& id) : ManagedPtr<T>(t), _id(id) {}

  Cacheable(const Id& id) : ManagedPtr<T>(), _id(id) {}

  Cacheable(const Cacheable& cacheable)
      : _id(cacheable.id()), ManagedPtr<T>(cacheable) {}

  virtual ~Cacheable() {}

  const Cacheable& operator=(const Cacheable& cacheable) {
    if (this != &cacheable) {
      ManagedPtr<T>::operator=(cacheable);
      _id = cacheable.id();
    }
    return *this;
  }

  const Id& id() const { return _id; }
};

template <class T>
class CacheableBuilder {
 public:
  virtual std::auto_ptr<Cacheable<T> > make() const = 0;
  virtual const Id& id() const = 0;
  // TODO: by default all is consistent, but it should be pure virtual and each
  // class make its own decision
  virtual bool isConsistent(const Cacheable<T>& cacheable) const = 0;

  virtual ~CacheableBuilder() {}
};

/**
 * abstract base class for a cache thread class
 *
 * This class defines the method run, which will be called by the cache thread
 * running in the background and doing garbage collection or other cache
 * optimization tasks
 *
 * @see Cache
 */
class CacheThread {
 public:
  virtual ~CacheThread() {}

  /**
   * The thread method - called when the background thread starts
   */
  virtual void run() = 0;
};

/**
 * function called to start the background cache management thread
 *
 * @param param  Pointer to a CacheThread object
 */
inline void cache_thread_func(void* param) {
  CacheThread* p = reinterpret_cast<CacheThread*>(param);

  p->run();
}

/**
 * The way the cache works: each time a cache user needs data that may be in the
 * cache, it calls findAndAdd with an instance of a CacheableBuilder derived
 * class. This class has two methods that are known to the cache: id, which
 * returns the id of the object to be looked up in the cache, and make, which is
 * to be used in case the object is not in the cache, to make the object to be
 * inserted in the cache and returned.
 *
 * The cache first sees if the id is in the cache, and if yes, it returns the
 * object corresponding to the id. If not, it calls make, inserts the object in
 * the cache and returns it.
 *
 * The cache always returns a ManagedPtr< T >, where T is the type of the object
 * stored in the cache. This managed pointer is a type of smart pointer which
 * also does reference counting, so the user code does not need to keep track
 * and delete these pointers. Internally it stores a pointer to T, and a pointer
 * to a reference countable object, which is normally stored in the cache along
 * with the pointer.
 *
 * There can be many instances of the managed pointer, in any number of threads,
 * all pointing to the same object and reference countable. in which case the
 * count is > 0. When they all go out of their respective scope, the count
 * decreases to 0. At this moment the cache knows that the object is no longer
 * in use and may choose to remove it, or keep it (this is still to be
 * determined). This is done in the background thread running in the Cache - it
 * keep looking for objects with reference 0, and it may choose to remove some
 * of them based on how long they have been there, how much memory is being used
 * and other criteria. The two extremes would be to either remove all objects
 * with reference count 0 immediately, or to keep everthing for the life time of
 * the cache object and only delete all of them in the destructor of the cache.
 *
 * Regarding the Id's, they are calculated by the cached objects themselves -
 * the only requirement is that they be unique for unique objects
 *
 * The currently cached classes, such as Series or DataCollection calculate the
 * id in a recursive fashion. A series can be created as a result of an
 * operation on other series, and all the information regarding its ancestors
 * and how it was calculated is stored in the id.
 *
 *
 * Empty Series, which are created by the user, are given an unique initial id,
 * although they are not stored in the cache (see the SeriesManager class). The
 * reason for this unique id is to insure that series constructed based on them
 * have unique ids as well, or conflicts may arise. These initially empty series
 * are returned as non-const managed pointers, wich delete the series upon going
 * out of scope, as they are not stored in the cache.
 *
 * The method to be used by the cache "consumers" is findAndAdd, which does the
 * finding and adding operation in one move, and in a thread safe manner. If
 * find and add were exposed as separate methods, multiple threads may call find
 * on the same id at the same time and not find it, and they would all try to
 * add the new item, which could be a problem.
 *
 * The cache is implemented as a block allocated map
 *
 * The cache is derived from CacheThread - its method run is the background
 * thread
 *
 * The cache is thread safe
 */
template <class T>
class Cache : public CacheThread {
  typedef Cacheable<T> CacheableT;
  typedef std::vector<CacheableT> ParkVector;

  typedef ConstPtrMap<Id, CacheableT> CacheableMap;

 private:
  bool _enable;
  mutable Mutex _mutex;
  // mutex for background processing
  mutable Mutex _bpmutex;
  CacheableMap _cache;
  ParkVector _park;
  bool _run;
  bool _running;
  bool _first;
  bool _doBackgroundProcessing;
  unsigned int _size;

 public:
  /**
   * Default constructor
   *
   * Initializes the cache
   *
   * @param T
   */
  Cache(unsigned int size, bool enable)
      : _enable(enable),
        _run(true),
        _running(false),
        _first(true),
        _size(size),
        _doBackgroundProcessing(false) {}

  virtual ~Cache() {
    // tell the background thread to stop
    _run = false;
    // wait for the background thread to stop
    while (_running) ::Sleep(1);
  }

  void startBackgroundThread() { _beginthread(cache_thread_func, 0, this); }

  void doBackgroundProcessing() {
    // lock for background processing
    //		XLock l( _bpmutex );
    Lock l(_mutex);
    // signal that we have done the background processing
    _doBackgroundProcessing = false;

    // remove the parked elements that have refcount == 1 (which means that they
    // are only referenced in the current object, but nowhere else and they can
    // be safely deleted
    while (!_park.empty()) {
      // TODO: erase from the end - don't want to move the vector around too
      // much when there are many elements
      const CacheableT& p = _park.front();
      if (!p.isReferred()) _park.erase(_park.begin());
    }

    // if cache size > _size (currently number of itmes)
    // it will erase one random element
    // TODO: implement some smart scheme, which looks at frequency of access as
    // well as how recent an element has been accessed
    static unsigned int crt = 0;
    if (_cache.size() > _size) {
      if (crt >= _cache.size()) crt = 0;

      int n = 0;
      for (CacheableMap::iterator i = _cache.begin(); i != _cache.end();
           i++, n++) {
        if (n == crt) {
          const CacheableT* p = i->second;
          if (!p->isReferred()) {
            delete p;
            _cache.erase(i);
            break;
          }
        }
      }
      crt++;
    }
  }

  /**
   * called by Cache users to find a cache item.
   *
   * If the item is not found, it will be created and returned.
   *
   * This method is thread safe and does both in one move, so thre won't be
   * consistency problems if 2 threads are trying to access the same object at
   * the same time and possibly create duplicates
   *
   * The CacheableBuilder has information about the object to be looked up - its
   * id, and if not found, how to create a new instance
   *
   * @param T      The CacheableBuilder object
   * @param mc
   * @param mc
   * @return A managed pointer to the retrieved or newly created object
   */
  ManagedPtr<T> findAndAdd(const CacheableBuilder<T>& mc) {
    Lock lock(_mutex);

    if (_first) {
      // start the background thread on the first access
      // this thread does garbage collection and in general cache management
      startBackgroundThread();
      _first = false;
    }

    if (_enable) {
      // if enabled, do the find and add if not found
      const Id& id = mc.id();
      CacheableMap::iterator i = _cache.find(id);
      if (i != _cache.end()) {
        // cache hit
        // check data consistency and if not, get the new data
        if (mc.isConsistent(*i->second))
          return *i->second;
        else {
          // move the element that's inconsistent to the park vector
          // where it will be removed at some point by the background cleanup
          if ((i->second)->isReferred()) {
            // lock temporarily on the background processing mutex, as we don't
            // want to end up with an invalid ParkVector
            //						XLock l( _bpmutex );
            _park.push_back(*i->second);
          }
          delete i->second;
          _cache.erase(i);
          // recursively call findAndAdd to get the new element and add it to
          // the cache
          return findAndAdd(mc);
        }
      } else {
        // cache miss, create the requested item
        // insert in the cache
        std::pair<CacheableMap::iterator, bool> p =
            _cache.insert(CacheableMap::value_type(id, mc.make().release()));
        // return a copy of the managed ptr in the cache (which increments the
        // reference)
        if (p.second)
          return *((*p.first).second);
        else {
          assert(false);
          return ManagedPtr<T>();
        }
      }
    } else {
      // if the cache is disabled, just return return a ManagedPtr that will
      // auto destroy the RefCounter and the payload when the reference count
      // goes to 0 in this case, the ID doesn't count as nothing is stored in
      // the cache, but calculated on the fly every time
      return *mc.make();
    }
  }

 public:
  /**
   * Enables or disables the cache. If disabled, the cache will always create
   * the requested object
   *
   * @param T
   * @param enable enable if true, disable if false
   * @param enable
   */
  void enable(bool enable) {
    Lock lock(_mutex);
    _enable = enable;
  }

  void setSize(unsigned int size) {
    Lock lock(_mutex);
    _size = size;
  }

  /**
   * background cache management thread,
   *
   * Defined in the CacheThread abstract base class
   *
   * @param T
   * @see CacheThread
   */
  void run() {
    // signal it's running
    _running = true;
    // run while allowed
    while (_run) {
      // every 10 seconds signal that background processing must be done
      ::Sleep(10);
      _doBackgroundProcessing = true;
      doBackgroundProcessing();
    }
    // signal that it has stopped running
    _running = false;
  }
};

// class SeriesAbstr;
typedef Cache<const tradery::DataCollection> DataCache;
typedef Cache<SeriesAbstr> SeriesCache;
