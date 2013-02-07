//  GAMBIT: Global and Modular BSM Inference Tool
//  *********************************************
//
//  Generic observable and likelihood object 
//  definitions.
//
//  Initially inspired by Abram  
//  Krislock's Objects.hpp.
//
//  *********************************************
//
//  Authors
//  =======
//
//  (add name and date if you modify)
//
//  Pat Scott
//  2012  Nov 15++  
//  2013  Jan 18, 29-31, Feb 04
//
//  Abram Krislock
//  2013 Jan 31, Feb 05
//  *********************************************

#ifndef __observable_hpp__
#define __observable_hpp__

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <boost/any.hpp>
#include <dictionary.hpp>

namespace GAMBIT {

  // Observable/likelihood/dependency function wrapper (functor) class,
  // with result type TYPE
  template <typename TYPE>
  class functor {

    public:

      // Constructor
      functor(void (*inputFunction)(TYPE &)) {
        myFunction = inputFunction;
        needs_recalculating = true;
      }

      // Operation (return value) 
      TYPE operator()() { return myValue; }

      // Add pointer to pointer to dependent functor
      template <typename DEP>
      void addToDepList(functor<DEP>* &dep_functor) { 
        dependency_list.push_back (&dep_functor);
      }

      // It may be safer to have the following three things made accessible 
      // only to the likelihood wrapper class and/or dependency resolver, i.e. so they cannot be used 
      // from within module functions

      // Calculate method
      void calculate() { if(needs_recalculating) { myFunction(myValue); } }

      // Needs recalculating or not?  (Externally modifiable)
      bool needs_recalculating;

      // Internal list of pointers to pointers to dependent functors
      std::list<boost::any> dependency_list;

    private:

      // Internal storage of function value
      TYPE myValue;

      // Internal storage of function pointer
      void (*myFunction)(TYPE &);

  };


  // A container for a function that needs to be constructed at compile
  // and executed as initialisation code at startup.
  struct ini_code {
    ini_code(void (*unroll)()) { (*unroll)(); }
  };

}

#define STRINGIFY(X) STRINGIFY2(X)
#define STRINGIFY2(X) #X
#define CAT(X,Y) CAT2(X,Y)
#define CAT2(X,Y) X##Y
#define DUMMY
#define DUMMYARG(...)

#ifdef IN_CORE
  #define START_MODULE    CORE_START_MODULE
  #define START_FUNCTION  CORE_START_FUNCTION
  #define DEPENDENCY      CORE_DEPENDENCY
#else
  #define START_MODULE    DUMMY
  #define START_FUNCTION  DUMMYARG
  #define DEPENDENCY      MODULE_DEPENDENCY
#endif


#define CORE_START_MODULE                                                      \
                                                                               \
  namespace GAMBIT {                                                           \
                                                                               \
    namespace MODULE {                                                         \
                                                                               \
      /* A way to fetch a trait of an observable or likelihood                 \
      (like its type), based on its tag.*/                                     \
      template <typename TAG>                                                  \
      struct function_traits {                                                 \
        typedef double type;  /* Scalar numerical value by default. */         \
      };                                                                       \
                                                                               \
      /* Equivalent class for dependencies, where TAG is dependent upon        \
      DEP_TAG */                                                               \
      template <typename DEP_TAG, typename TAG>                                \
      struct dep_traits {                                                      \
        typedef double type;  /* Scalar numerical value by default. */         \
      };                                                                       \
                                                                               \
      /* module name */                                                        \
      std::string name() { return STRINGIFY(MODULE); }                         \
                                                                               \
      /* maps from tag strings to tag-specialisted functions */                \
      std::map<std::string, bool(*)()> map_bools;                              \
      std::map<std::string, void(*)()> map_voids;                              \
      GAMBIT::dict moduleDict;                                                 \
                                                                               \
      /* all module observables/likelihoods, their dependencies and            \
      their types, as strings */                                               \
      static std::map<std::string,std::string> iCanDo;                         \
      static std::map<std::string,std::string> iMayNeed;                       \
                                                                               \
      /* module provides observable/likelihood TAG? */                         \
      template <typename TAG> bool provides() { return false; }                \
                                                                               \
      /* overloaded, non-templated version */                                  \
      bool provides(std::string obs) {                                         \
        if (map_bools.find(obs) == map_bools.end()) { return false; }          \
        return (*map_bools[obs])();                                            \
      }                                                                        \
                                                                               \
      /* module requires observable/likelihood DEP_TAG to compute TAG */       \
      template <typename DEP_TAG, typename TAG>                                \
        bool requires() { return false; }                                      \
                                                                               \
      /* overloaded, non-templated version */                                  \
      bool requires(std::string dep, std::string obs) {                        \
        if (map_bools.find(dep+obs) == map_bools.end()) { return false; }      \
        return (*map_bools[dep+obs])();                                        \
      }                                                                        \
                                                                               \
      /* report on observable/likelihood TAG */                                \
      template <typename TAG> void report() {                                  \
        std::cout<<"This tag is not supported by ";                            \
        std::cout<<STRINGIFY(MODULE)<<"."<<std::endl;                          \
      }                                                                        \
                                                                               \
      /* overloaded, non-templated version */                                  \
      void report(std::string obs) {                                           \
        if (map_voids.find(obs) == map_voids.end()) {                          \
          std::cout<<"This tag is not supported by ";                          \
          std::cout<<STRINGIFY(MODULE)<<"."<<std::endl;                        \
        }                                                                      \
        else { (*map_voids[obs])(); }                                          \
      }                                                                        \
                                                                               \
      /* alias for observable/likelihood function TAG */                       \
      template <typename TAG> typename function_traits<TAG>::type result() {   \
        std::cout<<"This tag is not supported by ";                            \
        std::cout<<STRINGIFY(MODULE)<<"."<<std::endl;                          \
        return 0;                                                              \
      }                                                                        \
                                                                               \
      /* overloaded, 'stringy' version */                                      \
      /* A templated function that uses an input string obs to pull a pointer  \
      to the zero-parameter alias fuction above out of the module's private    \
      dictionary.  It then dereferences that pointer, calls the function and   \
      returns the result. */                                                   \
      template <typename TYPE> TYPE result(std::string obs) {                  \
        return ( *moduleDict.get<TYPE(*)()>(obs) )();                          \
      }                                                                        \
                                                                               \
      /* runtime registration function for observable/likelihood function TAG*/\
      template <typename TAG>                                                  \
      void rt_register_function () {                                           \
        std::cout<<"This tag is not supported by ";                            \
        std::cout<<STRINGIFY(MODULE)<<"."<<std::endl;                          \
      }                                                                        \
                                                                               \
      /* runtime registration function for dependency DEP_TAG of function TAG*/\
      template <typename DEP_TAG, typename TAG>                                \
      void rt_register_dependency () {                                         \
        std::cout<<"This dependency does not exist in ";                       \
        std::cout<<STRINGIFY(MODULE)<<"."<<std::endl;                          \
      }                                                                        \
    }                                                                          \
                                                                               \
  }                                                                            \
                                                                               

#define CORE_START_FUNCTION(TYPE)                                              \
                                                                               \
  namespace GAMBIT {                                                           \
                                                                               \
    /* Add FUNCTION to the global set of tags of recognised functions */       \
    ADD_TAG_IN_CURRENT_NAMESPACE(FUNCTION)                                     \
                                                                               \
    namespace MODULE {                                                         \
                                                                               \
      /* Indicate that this module can provide function FUNCTION */            \
      template <> bool provides<Tags::FUNCTION>() {                            \
        return true;                                                           \
      }                                                                        \
                                                                               \
      /* Set up an auxilary method to report stuff to the core about the       \
      function.  Not actually sure what this would                             \
      be used for at this stage. */                                            \
      template <>                                                              \
      void report<Tags::FUNCTION>() {                                          \
        std::cout<<"Dear Core, I provide the function with tag: "<<            \
        STRINGIFY(FUNCTION)<<std::endl;                                        \
      }                                                                        \
                                                                               \
      /* Register (prototype) the function */                                  \
      void FUNCTION (TYPE &);                                                  \
                                                                               \
      /* Register the FUNCTION's result TYPE */                                \
      template<> struct function_traits<Tags::FUNCTION> {                      \
        typedef TYPE type;                                                     \
      };                                                                       \
                                                                               \
      /* Create the function wrapper object (functor) */                       \
      namespace Functown {                                                     \
        functor<TYPE> FUNCTION (&MODULE::FUNCTION);                            \
      }                                                                        \
                                                                               \
      /* Set up an alias function to call the function */                      \
      template <> function_traits<Tags::FUNCTION>::type                        \
       result<Tags::FUNCTION>() {                                              \
         Functown::FUNCTION.calculate();                                       \
         return Functown::FUNCTION();                                          \
      }                                                                        \
                                                                               \
      /* Set up the commands to be called at runtime to register the function*/\
      template <> void rt_register_function<Tags::FUNCTION> () {               \
        map_bools[STRINGIFY(FUNCTION)] = &provides<Tags::FUNCTION>;            \
        map_voids[STRINGIFY(FUNCTION)] = &report<Tags::FUNCTION>;              \
        iCanDo[STRINGIFY(FUNCTION)] = STRINGIFY(TYPE);                         \
        moduleDict.set<TYPE(*)()>(STRINGIFY(FUNCTION),&result<Tags::FUNCTION>);\
      }                                                                        \
                                                                               \
      /* Create the function initialisation object */                          \
      namespace Ini {                                                          \
        ini_code FUNCTION (&rt_register_function<Tags::FUNCTION>);             \
      }                                                                        \
                                                                               \
    }                                                                          \
                                                                               \
  }                                                                            \
                                                                          

#define CORE_DEPENDENCY(DEP, TYPE)                                             \
                                                                               \
  namespace GAMBIT {                                                           \
                                                                               \
    /* Add DEP to the global set of tags of recognised functions */            \
    ADD_TAG_IN_CURRENT_NAMESPACE(DEP)                                          \
                                                                               \
    namespace MODULE {                                                         \
                                                                               \
      /* Register the required TYPE of the required observable or likelihood   \
      function DEP */                                                          \
      template<> struct dep_traits<Tags::DEP, Tags::FUNCTION> {                \
        typedef TYPE type;                                                     \
      };                                                                       \
                                                                               \
      /* Create a pointer to the dependency functor. To be filled by the       \
      dependency resolver during runtime. */                                   \
      namespace Dependencies {                                                 \
        namespace FUNCTION {                                                   \
          functor<TYPE> *DEP;                                                  \
        }                                                                      \
      }                                                                        \
                                                                               \
      /* Indicate that FUNCTION requires DEP to have been computed previously*/\
      template <> bool requires<Tags::DEP, Tags::FUNCTION>() {                 \
        return true;                                                           \
      }                                                                        \
                                                                               \
      /* Set up the commands to be called at runtime to register dependency*/  \
      template <> void rt_register_dependency<Tags::DEP, Tags::FUNCTION> () {  \
        map_bools[STRINGIFY(CAT(DEP,FUNCTION))] =                              \
         &requires<Tags::DEP, Tags::FUNCTION>;                                 \
        iMayNeed[STRINGIFY(DEP)] = STRINGIFY(TYPE);                            \
        Functown::FUNCTION.addToDepList<TYPE>(Dependencies::FUNCTION::DEP);    \
      }                                                                        \
                                                                               \
      /* Create the dependency initialisation object */                        \
      namespace Ini {                                                          \
        ini_code DEP##_for_##FUNCTION                                          \
         (&rt_register_dependency<Tags::DEP, Tags::FUNCTION>);                 \
      }                                                                        \
                                                                               \
   }                                                                           \
                                                                               \
  }                                                                            \


#define MODULE_DEPENDENCY(DEP, TYPE)                                           \
                                                                               \
  namespace GAMBIT {                                                           \
                                                                               \
    namespace MODULE {                                                         \
                                                                               \
      /* Create a pointer to the dependency functor. To be filled by the       \
      dependency resolver during runtime. */                                   \
      namespace Dependencies {                                                 \
        namespace FUNCTION {                                                   \
          extern functor<TYPE> *DEP;                                           \
        }                                                                      \
      }                                                                        \
                                                                               \
   }                                                                           \
                                                                               \
  }                                                                            \


#define ADD_TAG_IN_CURRENT_NAMESPACE(TAG) namespace Tags { struct TAG; }


#endif // defined(__observable_hpp__) 
