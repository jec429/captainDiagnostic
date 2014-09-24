//
//  TTPCDataHandler.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//


#ifndef __plotPlanes__TTPCDataHandler__
#define __plotPlanes__TTPCDataHandler__

#include "MSingleEventRunReader.h"
#include <TH1.h>
#include <TH2.h>
#include <TLine.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <map>
#include <set>
#include <memory>
#include <cmath>


/// class to assemble TPC run data, calculate noise RMS, and map onto the wire planes
class TTPCDataHandler: protected MSingleEventRunReader {


   /// the first and last run numbers from the log to be included in the mapping
   const unsigned fkRun;

   /// RunsData: run <--> (*[event])[plane][channel][sample]
   typedef std::map<unsigned, RunData> RunsData;

   /// RunsWiresMap: run <--> [port]{plane, [wire]}
   typedef std::map<unsigned, std::array<PlaneWires, kgNPortsPerRun>> RunsWiresMap;

   /// PlanesData: (*[plane][event])[wire][sample]
   /// for now, PlaneData groups channels by event number,
   /// an arbitrary association when planes are split across multiple runs.
   typedef std::array<std::array<int, kgNSamplesPerChannel>, kgNWiresPerPlane> PlaneEvent;
   typedef std::vector<std::unique_ptr<PlaneEvent>> PlaneData;
   typedef std::vector<PlaneData> PlanesData;

   /// WiresRMS: [plane][wire]
   typedef std::array<std::array<float, kgNWiresPerPlane>, kgNPlanes> WiresRMS;

   /// ASICsMeanRMS: [plane][motherboard][ASIC]{fRMS, fWires}
   struct RMSWires {
      float fRMS;
      std::array<unsigned short, kgNChannelsPerASIC> fWires;
   };
   typedef std::array<std::array<std::array<RMSWires, kgNASICsPerMotherboard>,
                                 kgNMotherboardsPerPlane>, kgNPlanes> ASICsMeanRMS;



   // data members
   PlanesData fPlanesData;
   WiresRMS fRMS;


   /// given a log file logFile,
   /// generate a RunsWiresMap mapping from run numbers onto the
   /// wire planes and sets of wires monitored during that run,
   RunsWiresMap MapRunsToPlanes(std::ifstream& logFile) const;

   /// given the path of the directory containing Nevis dat files and/or C macro
   // /run directories and a RunsWiresMap from run numbers onto the wire planes
   /// and wires, assemble a RunsData mapping from run numbers onto the data,
   /// indexed by Event, channel, and sample.
   RunsData AssembleRunsData(const std::string& kRunPath,
                             const RunsWiresMap& kRunsToPlanes) const;

   /// given a RunsData mapping and RunsWiresMap mapping, assemble data into a
   /// PlanesData vector indexed by plane, event, wire, and sample.
   PlanesData AssemblePlanesData(const RunsData& kRunsData, const RunsWiresMap& kRunsWiresMap) const;

   /// compute the mean RMS for each wire
   WiresRMS ComputeWiresRMS() const;

public:
   /// given the path to a log file kLogPath and the run number
   /// to be processed, kRun, construct a TTPCDataHandler.
   ///
   /// log files should be plaintext formatted as follows:
   ///
   /// runNumber; lineDriverPortNumber1, lineDriverPortNumber2, ... # comment
   /// # comment
   ///
   TTPCDataHandler(const std::string& kLogPath, const std::string& kRunsDirectory,
                   const unsigned kRun);

   /// compute the mean sample value on the wire iWire
   float ComputeWireMeanVoltage(const unsigned short kiPlane, const unsigned short kiWire) const;

   /// compute the mean sample value of the entire plane kiPlane for Event kiEvent
   float ComputePlaneEventMeanVoltage(const unsigned short kiPlane,
                                      const unsigned short kiEvent) const;

   /// Calculate the voltage noise RMS for all wires.
   /// This makes two passes: one pass calculates an initial voltage pedestal (mean0)
   /// and noise RMS (RMS0).
   /// The second pass recalculates a new pedestal (mean1) and RMS (RMS1) excluding
   /// bins deviating by more than 3*RMS0 from mean0.
   /// RMS1 is the final result.
   WiresRMS GetWiresRMS() const;

   /// Get the mean RMS for each ASIC
   ASICsMeanRMS GetASICsMeanRMS() const;

   /// Compute the percentge of channels with RMS > 3 sigmaRMS
   float ComputeFractionNoisy(const WiresRMS& kRMS, const unsigned short kiPlane) const;
   float ComputeFractionNoisy(const ASICsMeanRMS& kRMS, const unsigned short kiPlane) const;

   /// get all wire plane voltage data
   PlanesData GetPlanesData() const;
   /// write out all data for all wire planes to a ROOT file, grouped by event number,
   /// an arbitrary association when planes are split across multiple runs.
   std::string WritePlanesData(const std::string& kROOTFilename) const;

   /// constructs a voltage map for each wire plane
   TCanvas GetVoltageHistogram() const;

   /// constructs a histogram plotting noise RMS voltage by wire
   TH1D GetWiresRMSHistogram() const;

   /// constructs a histogram plotting mean noise RMS voltage by ASIC
   TH1D GetASICsMeanRMSHistogram() const;
};

#endif /* defined(__plotPlanes__TTPCDataHandler__) */
