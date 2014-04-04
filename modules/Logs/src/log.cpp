//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Master source file for Gambit logging
///
///  The LogMaster object is statically declared
///  here, so this should be in its own compilation
///  unit. The function defined here lets you
///  retrieve this static log object in the other
///  compilation units. To use this function,
///  include the header "log.hpp" in your file.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///   
///  \author Ben Farmer
///          (benjamin.farmer@monash.edu.au)
///  \date 2014 Mar
///
///  \author Pat Scott
///          (patscott@physics.mcgill.ca)
///  \date 2014 Mar
///
///  *********************************************

#include "log.hpp"

namespace Gambit
{

  using namespace Logging;

  // This is an empty struct that I use simple to ensure that the logger LogTag "catalogues" are constructed before the logger itself. This ensures that they are destructed *after* the logger, which is important because we need them to exist during destruction of the logger.
  struct ensure_construction_order
  {
    ensure_construction_order()
    {
      // call the functions which retrieve the catalogues/maps, thus ensuring those objects are constructed.
      msgtypes();
      flags();
      components();
      tag2str();
    }
  };

  LogMaster& logger()
  {
      // I assume that these being static, the constructor for x will only get called once.
      // With the LogMaster constructed last out of these object, it should be destructed first, thus the other objects should be available for use in the LogMaster destructor.
      static ensure_construction_order x;
      static LogMaster global_log;
      return global_log;
  }

} // end namespace Gambit