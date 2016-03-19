//   GAMBIT: Global and Modular BSM Inference Tool
//   *********************************************
///  \file
///
///  Example of GAMBIT DarkBit standalone
///  main program.
///
///  *********************************************
///
///  Authors (add name and date if you modify):
///   
///  \author Christoph Weniger
///  \date 2016 Feb
///
///  *********************************************

// Always required in any standalone module main file
#include "gambit/Utils/standalone_module.hpp"
#include "gambit/DarkBit/DarkBit_rollcall.hpp"
#include "gambit/Elements/spectrum_factories.hpp"
#include "gambit/Elements/mssm_slhahelp.hpp"
#include "gambit/Models/SimpleSpectra/MSSMSimpleSpec.hpp"

// Only needed here
#include "gambit/Utils/util_functions.hpp"

using namespace DarkBit::Functown;     // Functors wrapping the module's actual module functions
using namespace DarkBit::Accessors;    // Helper functions that provide some info about the module
using namespace BackendIniBit::Functown;    // Functors wrapping the backend initialisation functions

QUICK_FUNCTION(DarkBit, decay_rates, NEW_CAPABILITY, createDecays, DecayTable, ())
QUICK_FUNCTION(DarkBit, SingletDM_spectrum, OLD_CAPABILITY, createSpectrum, const Spectrum*, ())
QUICK_FUNCTION(DarkBit, cascadeMC_gammaSpectra, OLD_CAPABILITY, CMC_dummy, DarkBit::stringFunkMap, ())

namespace Gambit
{
  namespace DarkBit
  {
    // Dummy functor for returning empty cascadeMC result spectra
    void CMC_dummy(DarkBit::stringFunkMap& result)
    {
      DarkBit::stringFunkMap sfm;
      result = sfm;
    }

    // Create spectrum object from SLHA file input.slha
    void createSpectrum(const Spectrum *& outSpec){
      static Spectrum mySpec;
      std::string inputFileName = "input.slha";

      Models::SingletDMModel singletmodel;
      singletmodel.HiggsPoleMass   = 125.; // *myPipe::Param.at("mH");
      singletmodel.HiggsVEV        = 246.; // 1. / sqrt(sqrt(2.)*sminputs.GF);
      singletmodel.SingletPoleMass = 100.; // *myPipe::Param.at("mS");
      singletmodel.SingletLambda   = 0.05; // *myPipe::Param.at("lambda_hS");

      SLHAstruct slhaea = read_SLHA(inputFileName);      
      mySpec = spectrum_from_SLHAea<Models::ScalarSingletDMSimpleSpec, Models::SingletDMModel>(singletmodel, slhaea);
      outSpec = &mySpec;
    }

    // Create decay object from SLHA file input.slha
    // FIXME: Include invisible Higgs contribution
    void createDecays(DecayTable& outDecays)
    {
      std::string inputFileName = "input.slha";
      outDecays = DecayTable(inputFileName);
    }
  }
}

int main()
{
  std::cout << std::endl 
            << "Start DarkBit standalone example" << std::endl;
  std::cout << "--------------------------------" << std::endl;
  std::cout << std::endl;
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  std::cout << "This program reads and needs a file 'input.slha', or crashes otherwise." << std::endl;
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  std::cout << std::endl;


  // ---- Initialise (or disable) logging ----

  std::map<std::string, std::string> loggerinfo;
  std::string prefix("runs/DarkBit_standalone_SingletDM/logs/");
  Utils::ensure_path_exists(prefix);

  loggerinfo["Core, Error"] = prefix+"core_errors.log";
  loggerinfo["Default"]     = prefix+"default.log";
  loggerinfo["Debug"]       = prefix+"debug.log";
  loggerinfo["Warning"]     = prefix+"warnings.log";
  loggerinfo["DarkBit, Info"] = prefix+"DarkBit_info.log";

  logger().initialise(loggerinfo);

  model_warning().set_fatal(true);
  DarkBit::DarkBit_error().set_fatal(true);

  logger()<<"Running DarkBit standalone example"<<LogTags::info<<EOM;


  // ---- Initialize models ----

  // Initialize SingletDM model
  ModelParameters* SingletDM_primary_parameters = Models::SingletDM::Functown::primary_parameters.getcontentsPtr();
  SingletDM_primary_parameters->setValue("mS", 100.);
  SingletDM_primary_parameters->setValue("lambda_hS", 0.05);

  // Initialize LocalHalo model
  ModelParameters* LocalHalo_primary_parameters = Models::LocalHalo::Functown::primary_parameters.getcontentsPtr();
  LocalHalo_primary_parameters->setValue("rho0", 0.4);
  LocalHalo_primary_parameters->setValue("vrot", 235.);
  LocalHalo_primary_parameters->setValue("v0", 235.);
  LocalHalo_primary_parameters->setValue("vesc", 550.);
  LocalHalo_primary_parameters->setValue("vearth", 29.78);

  // Initialize nuclear_params_fnq model
  ModelParameters* nuclear_params_fnq = Models::nuclear_params_fnq::Functown::primary_parameters.getcontentsPtr();
  nuclear_params_fnq->setValue("fpd", 0.034);
  nuclear_params_fnq->setValue("fpu", 0.023);
  nuclear_params_fnq->setValue("fps", 0.14);
  nuclear_params_fnq->setValue("fnd", 0.041);
  nuclear_params_fnq->setValue("fnu", 0.019);
  nuclear_params_fnq->setValue("fns", 0.14);
  nuclear_params_fnq->setValue("deltad", -0.40);
  nuclear_params_fnq->setValue("deltau", 0.74);
  nuclear_params_fnq->setValue("deltas", -0.12);


  // ---- Initialize spectrum and decays from SLHA file ----

  createSpectrum.reset_and_calculate();
  createDecays.reset_and_calculate();


  // ---- Initialize backends ----

//  // Initialize nulike backend
//  Backends::nulike_1_0_2::Functown::nulike_bounds.setStatus(2);
//  nulike_1_0_2_init.reset_and_calculate();
  
  // Initialize gamLike backend
  gamLike_1_0_0_init.reset_and_calculate();

  // Initialize MicrOmegas backend (specific for SingletDM)
  //MicrOmegasSingletDM_3_6_9_2_init.resolveDependency(&createSpectrum);
  MicrOmegasSingletDM_3_6_9_2_init.notifyOfModel("SingletDM");
  MicrOmegasSingletDM_3_6_9_2_init.resolveDependency(&Models::SingletDM::Functown::primary_parameters);
  MicrOmegasSingletDM_3_6_9_2_init.reset_and_calculate();

  // Initialize DarkSUSY backend
  DarkSUSY_5_1_3_init.reset_and_calculate();
//  DarkSUSY_PointInit_MSSM.notifyOfModel("MSSM30atQ");
//  DarkSUSY_PointInit_MSSM.resolveDependency(&createSpectrum);
//  DarkSUSY_PointInit_MSSM.resolveDependency(&createDecays);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::init_diskless);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsgive_model_isasugra);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dssusy_isasugra);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dssusy);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsSLHAread);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsprep);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dswwidth);
//  DarkSUSY_PointInit_MSSM.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::mssmpar);
//  DarkSUSY_PointInit_MSSM.setOption<bool>("use_dsSLHAread", true);
//  DarkSUSY_PointInit_MSSM.reset_and_calculate();

  // Initialize DarkSUSY Local Halo Model
  DarkSUSY_PointInit_LocalHalo_func.notifyOfModel("LocalHalo");
  DarkSUSY_PointInit_LocalHalo_func.resolveDependency(&Models::LocalHalo::Functown::primary_parameters);
  DarkSUSY_PointInit_LocalHalo_func.resolveDependency(&RD_fraction_fixed);
  DarkSUSY_PointInit_LocalHalo_func.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dshmcom);
  DarkSUSY_PointInit_LocalHalo_func.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dshmisodf);
  DarkSUSY_PointInit_LocalHalo_func.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dshmframevelcom);
  DarkSUSY_PointInit_LocalHalo_func.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dshmnoclue);
  DarkSUSY_PointInit_LocalHalo_func.reset_and_calculate();

  // Initialize DDCalc0 backend
  Backends::DDCalc0_0_0::Functown::DDCalc0_LUX_2013_CalcRates.setStatus(2);
  DDCalc0_0_0_init.notifyOfModel("LocalHalo");
  DDCalc0_0_0_init.resolveDependency(&Models::LocalHalo::Functown::primary_parameters);
  DDCalc0_0_0_init.resolveDependency(&RD_fraction_fixed);
  DDCalc0_0_0_init.reset_and_calculate();


  // ---- Relic density ----

  // Relic density calculation with MicrOmegas
  RD_oh2_MicrOmegas.resolveBackendReq(&Backends::MicrOmegasSingletDM_3_6_9_2::Functown::darkOmega);
  RD_oh2_MicrOmegas.reset_and_calculate();

//  // Relic density calculation with DarkSUSY (the sloppy version)
//  RD_oh2_DarkSUSY.resolveDependency(&DarkSUSY_PointInit_MSSM);
//  RD_oh2_DarkSUSY.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsrdomega);
//  RD_oh2_DarkSUSY.setOption<int>("fast", 1);  // 0: normal; 1: fast; 2: dirty
//  RD_oh2_DarkSUSY.reset_and_calculate();
//  // FIXME: Use "general" version instead

  // Calculate WMAP likelihoods, based on MicrOmegas result
  lnL_oh2_Simple.resolveDependency(&RD_oh2_MicrOmegas);
  lnL_oh2_Simple.reset_and_calculate();


  // ---- Set up basic internal structures for direct & indirect detection ----

  // Set identifier for DM particle
  DarkMatter_ID_SingletDM.notifyOfModel("SingletDM");
  DarkMatter_ID_SingletDM.resolveDependency(&Models::SingletDM::Functown::primary_parameters);
  DarkMatter_ID_SingletDM.reset_and_calculate();

  // Set up process catalog based on DarkSUSY annihilation rates
  TH_ProcessCatalog_SingletDM.notifyOfModel("SingletDM");
  TH_ProcessCatalog_SingletDM.resolveDependency(&Models::SingletDM::Functown::primary_parameters);
  TH_ProcessCatalog_SingletDM.resolveDependency(&createSpectrum);
  TH_ProcessCatalog_SingletDM.resolveDependency(&createDecays);
  TH_ProcessCatalog_SingletDM.reset_and_calculate();

  // Assume for direct and indirect detection likelihoods that dark matter
  // density is always the measured one (despite relic density results)
  RD_fraction_fixed.reset_and_calculate();

  // Set generic WIMP mass object
  mwimp_generic.resolveDependency(&TH_ProcessCatalog_SingletDM);
  mwimp_generic.resolveDependency(&DarkMatter_ID_SingletDM);
  mwimp_generic.reset_and_calculate();

  // Set generic annihilation rate in late universe (v->0 limit)  // FIXME: Check limit
  sigmav_late_universe.resolveDependency(&TH_ProcessCatalog_SingletDM);
  sigmav_late_universe.resolveDependency(&DarkMatter_ID_SingletDM);
  sigmav_late_universe.reset_and_calculate();


  // ---- Direct detection -----

  // Calculate DD couplings with Micromegas
  /*
  DD_couplings_MicrOmegas.notifyOfModel("SingletDM");
  DD_couplings_MicrOmegas.notifyOfModel("nuclear_params_fnq");
  DD_couplings_MicrOmegas.resolveDependency(&Models::nuclear_params_fnq::Functown::primary_parameters);
  DD_couplings_MicrOmegas.resolveBackendReq(&Backends::MicrOmegas_3_6_9_2::Functown::nucleonAmplitudes);
  DD_couplings_MicrOmegas.resolveBackendReq(&Backends::MicrOmegas_3_6_9_2::Functown::FeScLoop);
  DD_couplings_MicrOmegas.resolveBackendReq(&Backends::MicrOmegas_3_6_9_2::Functown::mocommon_);
  DD_couplings_MicrOmegas.reset_and_calculate();
  */

  DD_couplings_SingletDM.notifyOfModel("nuclear_params_fnq");
  DD_couplings_SingletDM.notifyOfModel("SingletDM");
  // NOTE: Should also resolve SingletDM parameters, but not relevant here
  DD_couplings_SingletDM.resolveDependency(&Models::nuclear_params_fnq::Functown::primary_parameters);
  DD_couplings_SingletDM.resolveDependency(&createSpectrum);
  DD_couplings_SingletDM.reset_and_calculate();

//  // Calculate DD couplings with DarkSUSY
//  DD_couplings_DarkSUSY.notifyOfModel("nuclear_params_fnq");
//  DD_couplings_DarkSUSY.resolveDependency(&Models::nuclear_params_fnq::Functown::primary_parameters);
//  DD_couplings_DarkSUSY.resolveDependency(&DarkSUSY_PointInit_MSSM);
//  DD_couplings_DarkSUSY.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsddgpgn);
//  DD_couplings_DarkSUSY.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::mspctm);
//  DD_couplings_DarkSUSY.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::ddcom);
//  DD_couplings_DarkSUSY.reset_and_calculate();

  // Push WIMP paramters to DDCalc0 backend
  SetWIMP_DDCalc0.resolveDependency(&DD_couplings_SingletDM);  // Use DarkSUSY parameters
  SetWIMP_DDCalc0.resolveDependency(&TH_ProcessCatalog_SingletDM);
  SetWIMP_DDCalc0.resolveDependency(&DarkMatter_ID_SingletDM);
  SetWIMP_DDCalc0.resolveBackendReq(&Backends::DDCalc0_0_0::Functown::DDCalc0_SetWIMP_mG);
  SetWIMP_DDCalc0.resolveBackendReq(&Backends::DDCalc0_0_0::Functown::DDCalc0_GetWIMP_msigma);
  SetWIMP_DDCalc0.reset_and_calculate();

  // Calculate direct detection rates for LUX 2013
  CalcRates_LUX_2013_DDCalc0.resolveDependency(&SetWIMP_DDCalc0);
  CalcRates_LUX_2013_DDCalc0.resolveBackendReq(&Backends::DDCalc0_0_0::Functown::DDCalc0_LUX_2013_CalcRates);
  CalcRates_LUX_2013_DDCalc0.reset_and_calculate();

  // Calculate direct detection likelihood for LUX 2013
  LUX_2013_LogLikelihood_DDCalc0.resolveDependency(&CalcRates_LUX_2013_DDCalc0);
  LUX_2013_LogLikelihood_DDCalc0.resolveBackendReq(&Backends::DDCalc0_0_0::Functown::DDCalc0_LUX_2013_LogLikelihood);
  LUX_2013_LogLikelihood_DDCalc0.reset_and_calculate();

  // Set generic scattering cross-section for later use
  sigma_SI_p_simple.resolveDependency(&DD_couplings_SingletDM);
  sigma_SI_p_simple.resolveDependency(&mwimp_generic);
  sigma_SI_p_simple.reset_and_calculate();

  // Set generic scattering cross-section for later use
  sigma_SD_p_simple.resolveDependency(&DD_couplings_SingletDM);
  sigma_SD_p_simple.resolveDependency(&mwimp_generic);
  sigma_SD_p_simple.reset_and_calculate();


  // ---- Gamma-ray yields ----

  // Initialize tabulated gamma-ray yields
  SimYieldTable_DarkSUSY.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dshayield);
  SimYieldTable_DarkSUSY.reset_and_calculate();

  // Collect missing final states for simulation in cascade MC
  GA_missingFinalStates.resolveDependency(&TH_ProcessCatalog_SingletDM);
  GA_missingFinalStates.resolveDependency(&SimYieldTable_DarkSUSY);
  GA_missingFinalStates.resolveDependency(&DarkMatter_ID_SingletDM);
  GA_missingFinalStates.reset_and_calculate();


  // Infer for which type of final states particles MC should be performed
  cascadeMC_FinalStates.reset_and_calculate();

  // Collect decay information for cascade MC
  cascadeMC_DecayTable.resolveDependency(&TH_ProcessCatalog_SingletDM);
  cascadeMC_DecayTable.resolveDependency(&SimYieldTable_DarkSUSY);
  cascadeMC_DecayTable.reset_and_calculate();

  // Set up MC loop manager for cascade MC
  cascadeMC_LoopManager.resolveDependency(&GA_missingFinalStates);
  cascadeMC_LoopManager.resolveDependency(&cascadeMC_DecayTable);
  cascadeMC_LoopManager.resolveDependency(&SimYieldTable_DarkSUSY);
  cascadeMC_LoopManager.resolveDependency(&TH_ProcessCatalog_SingletDM);
  std::vector<functor*> nested_functions = initVector<functor*>(
      &cascadeMC_InitialState, &cascadeMC_GenerateChain, &cascadeMC_Histograms, &cascadeMC_EventCount);
  cascadeMC_LoopManager.setNestedList(nested_functions);

  // Set up initial state for cascade MC step
  cascadeMC_InitialState.resolveDependency(&GA_missingFinalStates);
  cascadeMC_InitialState.resolveLoopManager(&cascadeMC_LoopManager);
  //cascadeMC_InitialState.reset_and_calculate();

  // Perform MC step for cascade MC
  cascadeMC_GenerateChain.resolveDependency(&cascadeMC_InitialState);
  cascadeMC_GenerateChain.resolveDependency(&cascadeMC_DecayTable);
  cascadeMC_GenerateChain.resolveLoopManager(&cascadeMC_LoopManager);
  //cascadeMC_GenerateChain.reset_and_calculate();

  // Generate histogram for cascade MC
  cascadeMC_Histograms.resolveDependency(&cascadeMC_InitialState);
  cascadeMC_Histograms.resolveDependency(&cascadeMC_GenerateChain);
  cascadeMC_Histograms.resolveDependency(&TH_ProcessCatalog_SingletDM);
  cascadeMC_Histograms.resolveDependency(&SimYieldTable_DarkSUSY);
  cascadeMC_Histograms.resolveDependency(&cascadeMC_FinalStates);
  cascadeMC_Histograms.resolveLoopManager(&cascadeMC_LoopManager);
  //cascadeMC_Histograms.reset_and_calculate();

  // Check convergence of cascade MC
  cascadeMC_EventCount.resolveDependency(&cascadeMC_InitialState);
  cascadeMC_EventCount.resolveLoopManager(&cascadeMC_LoopManager);
  //cascadeMC_EventCount.reset_and_calculate();

  // Start cascade MC loop
  cascadeMC_LoopManager.reset_and_calculate();

  // Infer gamma-ray spectra for recorded MC results
  cascadeMC_gammaSpectra.resolveDependency(&GA_missingFinalStates);
  cascadeMC_gammaSpectra.resolveDependency(&cascadeMC_FinalStates);
  cascadeMC_gammaSpectra.resolveDependency(&cascadeMC_Histograms);
  cascadeMC_gammaSpectra.resolveDependency(&cascadeMC_EventCount);
  cascadeMC_gammaSpectra.reset_and_calculate();

  // Calculate total gamma-ray yield (cascade MC + tabulated results)
  GA_AnnYield_General.resolveDependency(&TH_ProcessCatalog_SingletDM);
  GA_AnnYield_General.resolveDependency(&SimYieldTable_DarkSUSY);
  GA_AnnYield_General.resolveDependency(&DarkMatter_ID_SingletDM);
  GA_AnnYield_General.resolveDependency(&cascadeMC_gammaSpectra);
  GA_AnnYield_General.reset_and_calculate();

  // Calculate Fermi LAT dwarf likelihood
  lnL_FermiLATdwarfs_gamLike.resolveDependency(&GA_AnnYield_General);
  lnL_FermiLATdwarfs_gamLike.resolveDependency(&RD_fraction_fixed);
  lnL_FermiLATdwarfs_gamLike.resolveBackendReq(&Backends::gamLike_1_0_0::Functown::lnL);
  lnL_FermiLATdwarfs_gamLike.reset_and_calculate();


//  // ---- IceCube limits ----
//
//  // Infer WIMP capture rate in Sun
//  capture_rate_Sun_constant_xsec.resolveDependency(&mwimp_generic);
//  capture_rate_Sun_constant_xsec.resolveDependency(&sigma_SI_p_simple);
//  capture_rate_Sun_constant_xsec.resolveDependency(&sigma_SD_p_simple);
//  capture_rate_Sun_constant_xsec.resolveDependency(&DarkSUSY_PointInit_LocalHalo_func);
//  capture_rate_Sun_constant_xsec.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsntcapsuntab);
//  capture_rate_Sun_constant_xsec.reset_and_calculate();
//
//  // Infer WIMP equilibration time in Sun
//  equilibration_time_Sun.resolveDependency(&sigmav_late_universe);
//  equilibration_time_Sun.resolveDependency(&mwimp_generic);
//  equilibration_time_Sun.resolveDependency(&capture_rate_Sun_constant_xsec);
//  equilibration_time_Sun.reset_and_calculate();
//
//  // Infer WIMP annihilation rate in Sun
//  annihilation_rate_Sun.resolveDependency(&equilibration_time_Sun);
//  annihilation_rate_Sun.resolveDependency(&capture_rate_Sun_constant_xsec);
//  annihilation_rate_Sun.reset_and_calculate();
//
//  // Infer neutrino yield from Sun
//  nuyield_from_DS.resolveDependency(&TH_ProcessCatalog_SingletDM);
//  nuyield_from_DS.resolveDependency(&mwimp_generic);
//  nuyield_from_DS.resolveDependency(&sigmav_late_universe);
//  nuyield_from_DS.resolveDependency(&sigma_SI_p_simple);
//  nuyield_from_DS.resolveDependency(&sigma_SD_p_simple);
//  nuyield_from_DS.resolveDependency(&DarkMatter_ID_SingletDM);
//  nuyield_from_DS.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::dsgenericwimp_nusetup);
//  nuyield_from_DS.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::neutrino_yield);
//  nuyield_from_DS.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::DS_neutral_h_decay_channels);
//  nuyield_from_DS.resolveBackendReq(&Backends::DarkSUSY_5_1_3::Functown::DS_charged_h_decay_channels);
//  nuyield_from_DS.reset_and_calculate();
//
//  // Calculate number of events at IceCube
//  IC79WH_full.resolveDependency(&mwimp_generic);
//  IC79WH_full.resolveDependency(&annihilation_rate_Sun);
//  IC79WH_full.resolveDependency(&nuyield_from_DS);
//  IC79WH_full.resolveBackendReq(&Backends::nulike_1_0_2::Functown::nulike_bounds);
//  IC79WH_full.reset_and_calculate();
//  // FIXME: Code up other analyses
//
//  // Calculate IceCube likelihood
//  IC79WH_loglike.resolveDependency(&IC79WH_full);
//  IC79WH_loglike.reset_and_calculate();
//  // FIXME: Code up other analyses
  

  // ---- Dump results on screen ----

  // Retrieve and print MicrOmegas result
  double oh2 = RD_oh2_MicrOmegas(0);
  logger() << "Relic density from MicrOmegas: " << oh2 << LogTags::info << EOM;

  // Retrieve and print DarkSUSY result
  oh2 = RD_oh2_DarkSUSY(0);
  logger() << "Relic density from DarkSUSY: " << oh2 << LogTags::info << EOM;

  std::cout << "Done!" << std::endl;

  return 0;
}