//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  GAMBIT file locking functions
///  Use these to block access to sensitive parts
///  of the code by other processes when we are
///  using them. For example, some backends
///  do stupid things like initialise themselves
///  by writing and reading files; use these
///  routines to force only one process at a time
///  to perform these routines.
///
///  Usage:
///
///   {
///     Utils::FileLock mylock("some_unique_name");
///     mylock.get_lock();
///     /* Do sensitive stuff. No other process will
///        be allowed into this code region while we
///        are here. */
///     mylock.release_lock();
///   }
///   /* If not already done, lock is automatically 
///      released when 'mylock' is destructed */
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///   
///  \author Ben Farmer
///          (ben.farmer@gmail.com)
///  \date 2016 Feb
///
///  *********************************************

#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h> // I think this should work on osx as well...
#include <string>

#include "gambit/Utils/file_lock.hpp"
#include "gambit/Utils/util_functions.hpp"
#include "gambit/Utils/standalone_error_handlers.hpp"
#include "gambit/Utils/local_info.hpp"

namespace Gambit {
   namespace Utils {

      /// @{ Members of FileLock class

      /// Initialise prefix path name to lock files, and extension
      const std::string FileLock::lock_prefix("scratch/locks/");
      const std::string FileLock::lock_suffix(".lock");

      const std::string hardmsg("Now calling abort (will produce a core file for analysis if this is enabled on your system; if so please include this with the bug report)");

      /// Constructor
      FileLock::FileLock(const std::string& fname, const bool harderrs)
       : my_lock_fname(ensure_path_exists(lock_prefix + fname + lock_suffix))
       , fd(open(my_lock_fname.c_str(), O_RDWR | O_CREAT, 0666)) // last argument is permissions, in case file has to be created.
       , have_lock(false)
       , hard_errors(harderrs)
      {
        /// Should check for errors opening the file. List of error codes is kind of long though, let people look it up themselves for now...
        if(fd<0)
        {
          /// Error opening file!
          std::ostringstream msg;
          msg << "Error getting file descriptor for lock file '"<<my_lock_fname<<"'! Error was: "<< std::strerror(errno);
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }
      }

      /// Destructor
      /// Closing the file descriptor will automatically release any lock we might have
      FileLock::~FileLock()
      {
        int return_code = close(fd);

        if(return_code!=0)
        {
          /// Error closing file!
          std::ostringstream msg;
          msg << "Error closing file descriptor for lock file '"<<my_lock_fname<<"'! Error was: "<< std::strerror(errno);
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }
      }

      /// Obtain lock (or wait if lock cannot be obtained, and then obtain lock)
      void FileLock::get_lock()
      {
        if(have_lock)
        {
          /// Already have the lock!
          std::ostringstream msg;
          msg << "Tried to obtain lock for file '"<<my_lock_fname<<"', but we already have it! This indicates a logic error in whatever code tried to obtain the lock, please file a bug report.";
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }

        // Attempt to gain the lock. If the lock cannot be obtained, will block until it can.
        // This operation is atomic and so should be safe.
        int return_code = lockf(fd, F_LOCK, 0);

        if(return_code!=0)
        {
          // Uh oh, error occurred. Return error message
          std::ostringstream msg;
          msg << "Error obtaining lock on \""<<my_lock_fname<<"\"! Error was: "<< std::strerror(errno);
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }
        // Else the lock is ours!
        have_lock = true;
      }

      /// Release a lock (error if no lock held)
      void FileLock::release_lock()
      {
        if(not have_lock)
        {
          /// Don't have the lock!
          std::ostringstream msg;
          msg << "Tried to release lock for file '"<<my_lock_fname<<"', but it is not ours to release (i.e. get_lock() was not called, or the lock has already been released)! This indicates a logic error in whatever code tried to obtain the lock, please file a bug report.";
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }

        /// Release the lock
        int return_code = lockf(fd, F_ULOCK, 0);

        if(return_code!=0)
        {
          // Uh oh, error occurred. Return error message
          std::ostringstream msg;
          msg << "Error releasing lock on \""<<my_lock_fname<<"\"! Error was: "<< std::strerror(errno);
          if(hard_errors) { std::cerr<<"Error! ("<<LOCAL_INFO<<"): "<<msg.str()<<hardmsg<<std::endl; std::cerr.flush(); abort(); }
          else { utils_error().raise(LOCAL_INFO,msg.str()); }
        }
        have_lock = false; 
      }

      /// @} 
   }
}