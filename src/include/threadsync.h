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

#ifdef MISC_EXPORTS
#define MISC_API __declspec(dllexport)
#else
#define MISC_API __declspec(dllimport)
#endif

namespace tradery {

class MutexAbstr {
 public:
  virtual ~MutexAbstr() {}

  MISC_API static MutexAbstr* make();
};

class LockAbstr {
 public:
  virtual ~LockAbstr() {}

  MISC_API static LockAbstr* make(MutexAbstr& mutex);
};

/**
 * Mutex class
 *
 * Together with Lock, used to synchronize code that runs in multiple threads
 *
 * @see Lock
 */
class Mutex {
  friend class Lock;

 private:
  typedef std::auto_ptr<MutexAbstr> MutexAbstrPtr;
  MutexAbstrPtr _mutex;

 public:
  /**
   * Creates a Mutex object
   */
  Mutex() : _mutex(MutexAbstrPtr(MutexAbstr::make())) {}
};

/**
 * Lock class
 *
 * Together with Mutex, used to synchronize code running in multiple threads
 *
 * @see Mutex
 */
class Lock {
  friend class Condition;

 private:
  typedef std::auto_ptr<LockAbstr> LockAbstrPtr;
  LockAbstrPtr _lock;

 public:
  /**
   * Constructor that takes a Mutex object as parameter
   *
   * The object is created in the lock state.
   *
   * The destructor automatically unlocks the mutex.
   *
   * Usage:
   * - crate a Mutex object, usually as a data member of a class that needs to
   * be syncronized
   * - in the method that is to have serialized access to some resource, create
   * an automatic instance of Lock, passing the mutex as parameter.
   * - when the Lock object goes out of scope, it automatically unlocks the
   * mutex, so another thread can continue execution.
   *
   * @param mutex  The mutex to be locked
   */
  Lock(Mutex& mutex) : _lock(LockAbstrPtr(LockAbstr::make(*(mutex._mutex)))) {}
};

class NonRecursiveMutexAbstr {
 public:
  virtual ~NonRecursiveMutexAbstr() {}

  MISC_API static NonRecursiveMutexAbstr* make();
};

class NonRecursiveLockAbstr {
 public:
  virtual ~NonRecursiveLockAbstr() {}

  MISC_API static NonRecursiveLockAbstr* make(NonRecursiveMutexAbstr& mutex);

  virtual void lock() = 0;
  virtual void unlock() = 0;
};

class NonRecursiveMutex {
  friend class NonRecursiveLock;

 private:
  typedef std::auto_ptr<NonRecursiveMutexAbstr> NonRecursiveMutexAbstrPtr;
  NonRecursiveMutexAbstrPtr _mutex;

 public:
  NonRecursiveMutex()
      : _mutex(NonRecursiveMutexAbstrPtr(NonRecursiveMutexAbstr::make())) {}
};

class NonRecursiveLock {
  friend class Condition;

 private:
  typedef std::auto_ptr<NonRecursiveLockAbstr> NonRecursiveLockAbstrPtr;
  NonRecursiveLockAbstrPtr _lock;

 public:
  NonRecursiveLock(NonRecursiveMutex& mutex)
      : _lock(NonRecursiveLockAbstrPtr(
            NonRecursiveLockAbstr::make(*(mutex._mutex)))) {}

  void lock() { _lock->lock(); }
  void unlock() { _lock->unlock(); }
};

class ConditionAbstr {
 public:
  virtual ~ConditionAbstr() {}
  virtual void wait(NonRecursiveLockAbstr& lock) = 0;
  virtual void notify_one() = 0;

  MISC_API static ConditionAbstr* make();
};

class Condition {
 private:
  typedef std::auto_ptr<ConditionAbstr> ConditionAbstrPtr;
  ConditionAbstrPtr _condition;

 public:
  Condition() : _condition(ConditionAbstrPtr(ConditionAbstr::make())) {}

  void wait(NonRecursiveLock& lock) { _condition->wait(*(lock._lock)); }

  void notify_one() { _condition->notify_one(); }
};

}  // namespace tradery
