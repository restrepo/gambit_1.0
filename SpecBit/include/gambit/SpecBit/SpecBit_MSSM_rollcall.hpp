//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Rollcall declarations for module functions
///  contained in SpecBit_MSSM.cpp
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///
///  \author Ben Farmer
///          (benjamin.farmer@fysik.su.se)
///    \date 2014 Sep - Dec, 2015 Jan - Mar
///
///  \author Christopher Rogan
///          (christophersrogan@gmail.com)
///  \date 2015 Apr
///
///  *********************************************

#ifndef __SpecBit_MSSM_hpp__
#define __SpecBit_MSSM_hpp__

// Include this here so that typedef for SLHAstruct gets passed on to standalone codes which use these module functions
#include "gambit/Elements/slhaea_helpers.hpp"

  /// @{ Functions to supply particle spectra in various forms

  // This capability supplies the physical mass spectrum of the MSSM plus running
  // parameters in the DRbar scheme. This can be generated by many different
  // constrained models with various boundary conditions, or defined directly.
  #define CAPABILITY unimproved_MSSM_spectrum
  START_CAPABILITY

    // ==========================
    // GUT MSSM parameterisations
    // (CMSSM and its various non-universal generalisations)

    /// Get MSSM spectrum from CMSSM boundary conditions
    //  The type, (const) Spectrum, is a class containing two SubSpectrum* members and an SMInputs
    //  member. The SubSpectrum* members point to a "UV" Spectrum object (the MSSM) and an
    //  "LE" (low energy) Spectrum object (an effective Standard Model description), while SMInputs
    //  contains the information in the SMINPUTS block defined by SLHA2.
    #define FUNCTION get_CMSSM_spectrum
    START_FUNCTION(/*TAG*/ Spectrum)
    ALLOW_MODELS(CMSSM)
    DEPENDENCY(SMINPUTS, SMInputs) // Need SLHA2 SMINPUTS to set up spectrum generator
    #undef FUNCTION

    // FlexibleSUSY compatible maximal CMSSM generalisation (MSSM with GUT boundary conditions)
    #define FUNCTION get_MSSMatMGUT_spectrum
    START_FUNCTION(/*TAG*/ Spectrum)
    ALLOW_MODELS(MSSM63atMGUT)
    DEPENDENCY(SMINPUTS, SMInputs) // Need SLHA2 SMINPUTS to set up spectrum generator
    #undef FUNCTION

    // ==============================
    // MSSM parameterised with input at (user-defined) scale Q
    #define FUNCTION get_MSSMatQ_spectrum
    START_FUNCTION(/*TAG*/ Spectrum)
    ALLOW_MODELS(MSSM63atQ)
    DEPENDENCY(SMINPUTS, SMInputs) // Need SLHA2 SMINPUTS to set up spectrum generator
    #undef FUNCTION

    // ==============================
    // MSSM spectrum retrieved from an SLHA file
    // Wraps it up in MSSMskeleton; i.e. no RGE running possible.
    // This is mainly for testing against benchmark points, but may be a useful last
    // resort for interacting with "difficult" spectrum generators.
    #define FUNCTION get_MSSM_spectrum_from_SLHAfile
    START_FUNCTION(/*TAG*/ Spectrum)
    // Technically doesn't need a Model to work...
    // Could add some kind of dependency here, like on the input filename, to allow dependency
    // resolver to ignore it most of the time.
    #undef FUNCTION

    // MSSM spectrum constructed from an SLHAstruct (SLHAea::Coll).
    // Can use as an improvement upon creating a Spectrum object from an SLHA file (avoids
    // disk access), but without going to a full SubSpectrum wrapper interface.
    #define FUNCTION get_MSSM_spectrum_from_SLHAstruct
    START_FUNCTION(Spectrum)
    DEPENDENCY(unimproved_MSSM_spectrum, SLHAstruct)
    #undef FUNCTION

    // ==============================
    // Get unimproved MSSM spectrum as an SLHAea object
    #define FUNCTION get_MSSM_spectrum_as_SLHAea
    START_FUNCTION(SLHAstruct)
    DEPENDENCY(unimproved_MSSM_spectrum, /*TAG*/ Spectrum) // Takes a (pointer to a) Spectrum object and returns an SLHAstruct
    #undef FUNCTION

  #undef CAPABILITY


  #define CAPABILITY MSSM_spectrum
    // ==============================
    // Convert an MSSM_spectrum into a standard map so that it can be printed
    #define FUNCTION get_MSSM_spectrum_as_map
    START_FUNCTION(map_str_dbl) // Just a string to double map. Can't have commas in macro input
    DEPENDENCY(MSSM_spectrum, /*TAG*/ Spectrum)
    #undef FUNCTION
  #undef CAPABILITY

  #define CAPABILITY unimproved_MSSM_spectrum
   // Same as above, but works with unimproved version of spectrum
    #define FUNCTION get_unimproved_MSSM_spectrum_as_map
    START_FUNCTION(map_str_dbl) // Just a string to double map. Can't have commas in macro input
    DEPENDENCY(unimproved_MSSM_spectrum, /*TAG*/ Spectrum)
    #undef FUNCTION
  #undef CAPABILITY


  #define CAPABILITY SMlike_Higgs_PDG_code
  START_CAPABILITY
    #define FUNCTION most_SMlike_Higgs_MSSM
    START_FUNCTION(int) // just returns pdg code of most SM-like CP even Higgs
    DEPENDENCY(MSSM_spectrum, Spectrum)
    #undef FUNCTION
  #undef CAPABILITY

  #define CAPABILITY SM_subspectrum
  START_CAPABILITY

    // TODO: NOTE! I removed this because currently the string names don't quite match correctly and the MSSM version doesn't provide all the Standard Model pole masses.

    //  #define FUNCTION convert_MSSM_to_SM
    //  START_FUNCTION(/*TAG*/ Spectrum)
    //  DEPENDENCY(MSSM_spectrum, /*TAG*/ Spectrum)
    //  #undef FUNCTION

    //  // etc. for other functions

    // Extract appropriate SubSpectrum* from Spectrum struct, starting from MSSM_spectrum
    #define FUNCTION get_SM_SubSpectrum_from_MSSM_Spectrum
    START_FUNCTION(const SubSpectrum*)
    DEPENDENCY(unimproved_MSSM_spectrum, /*TAG*/ Spectrum)
    #undef FUNCTION

  #undef CAPABILITY


  // FeynHiggs SUSY masses and mixings
  #define CAPABILITY FH_MSSMMasses
  START_CAPABILITY
    #define FUNCTION FH_MSSMMasses
    START_FUNCTION(fh_MSSMMassObs)
    BACKEND_REQ(FHGetPara, (libfeynhiggs), void, (int&,int&,
                Farray<fh_real, 1,2, 1,5, 1,3>&, Farray<fh_complex, 1,2, 1,2, 1,5, 1,3>&,
                Farray<fh_real, 1,6, 1,5>&, Farray<fh_complex, 1,36, 1,5>&,
                Farray< fh_real,1,2>&, Farray< fh_complex,1,4>&,
                Farray< fh_complex,1,4>&, Farray< fh_real,1,4>&,
                Farray< fh_complex,1,16>&, fh_complex&, fh_real&,
                Farray< fh_real,1,4>&, fh_real&))
    BACKEND_OPTION( (FeynHiggs), (libfeynhiggs) )
    ALLOW_MODELS(MSSM63atQ, MSSM63atMGUT)
    #undef FUNCTION
  #undef CAPABILITY

  // Higgs masses and mixings with theoretical uncertainties
  #define CAPABILITY prec_HiggsMasses
  START_CAPABILITY
    #define FUNCTION FH_HiggsMasses
    START_FUNCTION(fh_HiggsMassObs)
    DEPENDENCY(FH_MSSMMasses, fh_MSSMMassObs)
    BACKEND_REQ(FHHiggsCorr, (libfeynhiggs), void, (int&, Farray< fh_real,1,4>&, fh_complex&,
                Farray<fh_complex, 1,3, 1,3>&,
                Farray<fh_complex, 1,3, 1,3>&))
    BACKEND_REQ(FHUncertainties, (libfeynhiggs), void, (int&, Farray< fh_real,1,4>&, fh_complex&,
                Farray<fh_complex, 1,3, 1,3>&,
                Farray<fh_complex, 1,3, 1,3>&))
    BACKEND_OPTION( (FeynHiggs), (libfeynhiggs) )
    ALLOW_MODELS(MSSM63atQ, MSSM63atMGUT)
    #undef FUNCTION
  #undef CAPABILITY

  // Higgs couplings information directly computed by FeynHiggs
  #define CAPABILITY FH_Couplings_ouput
  START_CAPABILITY
    #define FUNCTION FH_Couplings
    START_FUNCTION(fh_Couplings)
    BACKEND_REQ(FHSelectUZ, (libfeynhiggs), void, (int&,int&,int&,int&))
    BACKEND_REQ(FHCouplings, (libfeynhiggs), void, (int&, Farray< fh_complex,1,681>&,
                                                    Farray< fh_complex,1,231>&,
                                                    Farray< fh_real,1,978>&,
                                                    Farray< fh_real,1,250>&, int&))
    BACKEND_OPTION( (FeynHiggs), (libfeynhiggs) )
    ALLOW_MODELS(MSSM63atQ, MSSM63atMGUT)
    #undef FUNCTION
  #undef CAPABILITY

  // Generalised Higgs couplings
  #define CAPABILITY Higgs_Couplings

    #define FUNCTION MSSM_higgs_couplings_pwid
    START_FUNCTION(HiggsCouplingsTable)
    DEPENDENCY(MSSM_spectrum, Spectrum)
    DEPENDENCY(Reference_SM_Higgs_decay_rates, DecayTable::Entry)
    DEPENDENCY(Reference_SM_h0_2_decay_rates, DecayTable::Entry)
    DEPENDENCY(Reference_SM_A0_decay_rates, DecayTable::Entry)
    DEPENDENCY(Higgs_decay_rates, DecayTable::Entry)
    DEPENDENCY(h0_2_decay_rates, DecayTable::Entry)
    DEPENDENCY(A0_decay_rates, DecayTable::Entry)
    DEPENDENCY(H_plus_decay_rates, DecayTable::Entry)
    DEPENDENCY(t_decay_rates, DecayTable::Entry)
    #undef FUNCTION

    #define FUNCTION MSSM_higgs_couplings_FH
    START_FUNCTION(HiggsCouplingsTable)
    DEPENDENCY(MSSM_spectrum, Spectrum)
    DEPENDENCY(Reference_SM_Higgs_decay_rates, DecayTable::Entry)
    DEPENDENCY(Reference_SM_h0_2_decay_rates, DecayTable::Entry)
    DEPENDENCY(Reference_SM_A0_decay_rates, DecayTable::Entry)
    DEPENDENCY(Higgs_decay_rates, DecayTable::Entry)
    DEPENDENCY(h0_2_decay_rates, DecayTable::Entry)
    DEPENDENCY(A0_decay_rates, DecayTable::Entry)
    DEPENDENCY(H_plus_decay_rates, DecayTable::Entry)
    DEPENDENCY(t_decay_rates, DecayTable::Entry)
    DEPENDENCY(FH_Couplings_output, fh_Couplings)
    #undef FUNCTION

  #undef CAPABILITY


  /// @}

#endif

