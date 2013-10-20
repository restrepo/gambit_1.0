//  GAMBIT: Global and Modular BSM Inference Tool
//  //  ********************************************
//  //
//  //  For now, this is a main script for testing
//  //  the Pythia8Backend and Delphes3Backend codes.
//  //
//  //  Later on, it should be converted to a module
//  //  with a proper rollcall.
//  //
//  //  ********************************************
//  //
//  //  Authors
//  //  =======
//  //
//  //  (add name and date if you modify)
//  //
//  //  Abram Krislock
//  //  2013 Apr 23, Aug 30
//  //  Aldo Saavedra
//  //  2013 June 14
//  //  Andy Buckley
//  //  2013 July 20, Oct 04
//  //
//  //  ********************************************
//
//
#include "Pythia8Backend.hpp"
#include "Delphes3Backend.hpp"
#include "Analysis.hpp"
#include "Event.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <numeric>

#ifdef ARCHIVE
#include "boost/archive/text_oarchive.hpp"
#include "boost/lexical_cast.hpp"
#endif

#include "omp.h"
#define MAIN_SHARED slhaFileName,delphesConfigFile,myDelphes
#define MAIN_PRIVATE genEvent,recoEvent,myPythia

using namespace std;


namespace Gambit {
  namespace HEColliderBit {

    struct SubprocessGroup {
      SubprocessGroup()
        : xsec(-1), nevts(-1) { }
      /// @todo pair<vector<int>> instead of two vector<int> inputs?
      SubprocessGroup(double xs, const vector<int>& parts1, const vector<int>& parts2)
        : xsec(xs), nevts(-1), particlesInProcess1(parts1), particlesInProcess2(parts2) { }
      void add_analysis(Analysis* a) { analyses.push_back(shared_ptr<Analysis>(a)); }
      double xsec;
      // string name; // or int id = 1000*sp_type + sp_instance
      int nevts;
      // double kFactor;
      /// @todo Calc effective lumi?
      /// @todo Add some metric of CPU cost per event for this process type?
      /// The processes are selected by the IDs of the particles which
      /// must be in the process.
      vector<int> particlesInProcess1;
      vector<int> particlesInProcess2;
      vector<shared_ptr<Analysis>> analyses;
    };


    class KFactorHooks : public Pythia8::UserHooks {
      /// @todo once KFactorContainer is created, need to pass it in here.
      KFactorHooks() {}
      ~KFactorHooks() {}

      // To apply the KFactors directly within Pythia, we need to modify the cross sections:
      virtual bool canModifySigma() {return true;}
      virtual double multiplySigmaBy(const Pythia8::SigmaProcess* sigmaProcessPtr,
                                     const Pythia8::PhaseSpace* phaseSpacePtr, bool inEvent)
      {
        // Get the process code:
        //int processCode = sigmaProcessPtr->code();

        // Each process has a different KFactor... does something like this work?
        /// @todo this function has not been created yet....:
        // return magicKFactor->getKFactorMagically(processCode);
        return 1.;
      }

    private:
      /// @todo this class has not been created yet....:
      // KFactorContainer *magicKFactor;
      /// @todo should it instead get this info from SubprocessGroup?
      SubprocessGroup* subprocessGroup;
    };

  }
}


int main()
{
  // Basic setup
  /// @todo Model info including SLHA will need to come from ModelBit
  const string slhaFileName = "sps1aWithDecays.spc"; //"mhmodBenchmark.slha";
  /// @todo We'll eventually need more than just ATLAS, so Delphes/FastSim handling will need to be bound to analyses (and cached)
  /// @note That means that the class loaders had better be working by then...
  const string delphesConfigFile = "delphes_card_ATLAS.tcl";
  const int NEVENTS = 10000;

  // For event generation
  Gambit::HEColliderBit::Pythia8Backend* myPythia;
  Gambit::HEColliderBit::Delphes3Backend* myDelphes = new Gambit::HEColliderBit::Delphes3Backend(delphesConfigFile);

  // For event storage
  Pythia8::Event genEvent;
  Gambit::Event recoEvent;

  cout << endl << "Running parallelized HECollider simulation" << endl << endl;

  // Subprocess group setup
  // Decide how many events of each subprocess group to use, split according to number of threads
  /// @todo Use fast lookup of interpolated NLO subprocess cross-sections to do this
  /// @note Hard-coded for now, so I can do *something*
  /// @todo Hard coding of nthread assignment calc: yuck, yuck, yuck! Generalise to containers of subprocesses
  map<string, Gambit::HEColliderBit::SubprocessGroup> sp_groups;

  /// @note The old SubprocessGroup constructor calls are commented out below the new inputs
  /// @note Also, the new constructor calls exclude third generation squarks
  sp_groups["g~"] = Gambit::HEColliderBit::SubprocessGroup(0.4,
                    {{1000021}},
                    {{1000021, 1000001, 1000002, 1000003, 1000004,
                      2000001, 2000002, 2000003, 2000004}});
  //sp_groups["g~"] = Gambit::HEColliderBit::SubprocessGroup(0.4, {{"SUSY:gg2gluinogluino", "SUSY:qqbar2gluinogluino", "SUSY:qg2squarkgluino"}});

  sp_groups["q~"] = Gambit::HEColliderBit::SubprocessGroup(0.2,
                    {{1000001, 1000002, 1000003, 1000004,
                      2000001, 2000002, 2000003, 2000004}},
                    {{1000001, 1000002, 1000003, 1000004,
                      2000001, 2000002, 2000003, 2000004}});
  //sp_groups["q~"] = Gambit::HEColliderBit::SubprocessGroup(0.2, {{"SUSY:gg2squarkantisquark", "SUSY:qqbar2squarkantisquark", "SUSY:qq2squarksquark"}});

  sp_groups["X~"] = Gambit::HEColliderBit::SubprocessGroup(0.02,
                    {{1000021, 1000001, 1000002, 1000003, 1000004,
                      2000001, 2000002, 2000003, 2000004}},
                    {{1000022, 1000023, 1000024, 1000025, 1000035, 1000037}});
  //sp_groups["X~"] = Gambit::HEColliderBit::SubprocessGroup(0.02, {{"SUSY:qg2chi0squark", "SUSY:qg2chi+-squark", "SUSY:qqbar2chi0gluino", "SUSY:qqbar2chi+-gluino"}});

  // Bind subprocesses to analysis pointers
  for (auto& sp_group : sp_groups) {
    sp_group.second.add_analysis( Gambit::mkAnalysis("ATLAS_0LEP") );
  }

  /// @note Needs more complexity when we want to run different analyses with
  /// high acceptances for low-xsec subprocesses.
  //
  // Abram's thoughts and brainstorming about these issues:
  // 1) Negligible processes: A process is negligible when the cross-section times acceptance
  //    is so low that it will automatically be dwarfed by the background. I guess that makes the
  //    likelihood for that particular analysis 1.
  // 2) Acceptances can never be > 1, so there ought to be a fairly simple formula to apply even
  //    for low-xsecs. For instance:
  //      if (xsec * ideal_acceptance)[worst subprocess] < 0.01 (xsec * crappy_acceptance)[next-to-worst subprocess]
  // 3) The question then becomes: can we somehow estimate a priori ideal_acceptance and crappy_acceptance?
  //
  //
  const int NUM_CORES = omp_get_max_threads();
  const int num_events_per_thread = (int) ceil(NEVENTS / (double) NUM_CORES);
  vector<Gambit::HEColliderBit::SubprocessGroup> thread_cfgs;
  thread_cfgs.reserve(NUM_CORES);
  double total_xsec = 0;
  for (const auto& sp_group : sp_groups) total_xsec += sp_group.second.xsec;
  for (auto& sp_group : sp_groups) {
    /// @todo Need to be able to discard negligible processes... defined how? (see above)
    const int sp_group_num_threads = (int) ceil(NUM_CORES * sp_group.second.xsec / total_xsec);
    cout << sp_group.first << " #threads = " << sp_group_num_threads << endl;
    for (int i = 0; i < sp_group_num_threads; ++i) {
      thread_cfgs.push_back(sp_group.second);
      thread_cfgs.back().nevts = num_events_per_thread;
    }
  }
  const int NUM_THREADS = thread_cfgs.size();
  cout << "Total #cores = " << NUM_CORES << ", #threads = " << NUM_THREADS << endl;
  if (NUM_THREADS > NUM_CORES)
    cerr << "WARNING: more Pythia instances are needed than there are CPUs. HEColliderBit will run SLOW." << endl;
  omp_set_num_threads(NUM_THREADS);
  cout << " Total target number of events = " << NEVENTS << endl;

  #pragma omp parallel shared(MAIN_SHARED) private(MAIN_PRIVATE) num_threads(NUM_THREADS)
  {
    // Py8 backend process configuration
    const int NTHREAD = omp_get_thread_num();
    myPythia = new Gambit::HEColliderBit::Pythia8Backend(NTHREAD);
    myPythia->set("SLHA:file", slhaFileName);
    myPythia->set("SUSY:idVecA", thread_cfgs[NTHREAD].particlesInProcess1);
    myPythia->set("SUSY:idVecB", thread_cfgs[NTHREAD].particlesInProcess2);

    // Run persistency configuration
    #ifdef ARCHIVE
    ofstream outFile("tester_thread" + boost::lexical_cast<string>(NTHREAD) + ".dat");
    boost::archive::text_oarchive outArchive(outFile);
    #endif

    // Print out some run details
    /// @note We might want more CPU-heavy processes to run fewer events.
    #pragma omp critical
    {
      cout << " Target number of events for thread #" << NTHREAD << " = " << thread_cfgs[NTHREAD].nevts << endl;
    }

    // Run the event loop
    int counter = 0;
    assert(thread_cfgs[NTHREAD].nevts > 0);
    for (counter = 0; counter < thread_cfgs[NTHREAD].nevts; counter++) {
      genEvent.clear();
      recoEvent.clear();

      // Run Pythia8
      myPythia->nextEvent(genEvent);

      // Run Delphes (not thread safe)
      #pragma omp critical
      {
        myDelphes->processEvent(genEvent, recoEvent);
      }

      // Run all analyses attached to the thread
      for (shared_ptr<Gambit::Analysis> ana : thread_cfgs[NTHREAD].analyses)
        ana->analyze(recoEvent);

      // Archive the recoEvent to file if persistency is enabled
      #ifdef ARCHIVE
      outArchive << recoEvent;
      #endif
    } // end event loop

    // Print out final run details
    #pragma omp critical
    {
      cout << "Thread #" << NTHREAD << " has run " << counter << " events" << endl;
    }

    #ifdef ARCHIVE
    outFile.close();
    #endif
    delete myPythia;
  } // end omp parallel block


  // Finalize each process group
  /// @todo Parallelize... but OpenMP and vector iteration don't play well
  // omp_set_num_threads(sp_groups.size());
  // #pragma omp parallel for
  for (auto& spg : sp_groups) {
    cout << "Finalizing " << spg.first << endl;
    spg.second.analyses[0]->finalize();
  }


  /// @todo Combine results from each subprocess + K-factors -> likelihood measure
  //cout << "LIKELIHOOD = " << ana->likelihood() << endl;

  delete myDelphes;
  return 0;
}