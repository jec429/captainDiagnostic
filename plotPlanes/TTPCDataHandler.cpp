//
//  TTPCDataHandler.cpp
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//


#include "TTPCDataHandler.h"



TTPCDataHandler::TTPCDataHandler(const std::string& kLogPath,
                                 const std::string& kDatDirectory,
                                 const unsigned kFirstRun,
                                 const unsigned kLastRun)
: fkFirstRun(kFirstRun), fkLastRun(kLastRun)
{
   std::ifstream logFile(POSIXExpand(kLogPath));
   if (logFile.good()) {

      const RunsWiresMap kRunsWiresMap = MapRunsToPlanes(logFile);
      const RunsData kRunsData = AssembleRunsData(kDatDirectory, kRunsWiresMap);

      fPlanesData = AssemblePlanesData(kRunsData, kRunsWiresMap);
      fRMS = ComputeWiresRMS();

   } else {
      std::cout << "plotPlanes TTPCDataHandler() error: could not open log \""
                << kLogPath << "\"" << std::endl;
      exit(EXIT_FAILURE);
   }
   logFile.close();
}



TTPCDataHandler::RunsWiresMap TTPCDataHandler::MapRunsToPlanes(std::ifstream& logFile) const
{
   // run <--> [port]
   // RunPortsMap maps run numbers onto the set of line driver port numbers included in that run.
   // It's basically just a data structure representing exactly the log file.
   typedef std::map<unsigned, std::array<unsigned short, kgNPortsPerRun>> RunPortsMap;

   // [port](plane, [wire])
   // PortWiresMap associates each line driver port with the plane and wire indices it reads out.
   typedef std::array<PlaneWires, kgNPorts> PortWiresMap;


   RunPortsMap runsToPorts;

   while (logFile.good()) {

      // ignore groups of commented lines when they appear
      while (logFile.peek() == '#') {
         logFile.ignore(256, '\n');
      }

      // run number
      unsigned run;


      // lambda function checking whether there are any ports listed for this run in the log
      auto CheckLogForPorts = [&]()
      {
         if (   logFile.peek() == ';' || logFile.peek() == '\n'
             || logFile.peek() == '#' || logFile.peek() == EOF ) {
            std::cout << "plotPlanes TTPCDataHandler::MapRunsToPlanes() error: missing ports for run "
                      << run << std::endl;
            exit(EXIT_FAILURE);
         }
      };



      // read in the run number and port numbers in this log entry
      logFile >> run;
      if (run >= fkFirstRun && run <= fkLastRun) {
         // the line driver port numbers corresponding to this run
         std::array<unsigned short, kgNPortsPerRun> runPorts;

         logFile.ignore(256, ';');
         CheckLogForPorts();
         logFile >> runPorts[0];
         --runPorts[0]; // port numbers logged starting with 1; convert to index by subtracting 1

         for (unsigned short iPortCount = 1; iPortCount < runPorts.size(); ++iPortCount) {
            logFile.ignore(256, ',');
            CheckLogForPorts();
            logFile >> runPorts[iPortCount];
            --runPorts[iPortCount]; // convert logged port number to index
         }

         runsToPorts.emplace(run, runPorts);
      }
   }



   // map line driver port numbers onto their connected plane and wires indices
   PortWiresMap portToWires;
   for (unsigned short port = 0; port < portToWires.size(); ++port) {
      const unsigned short kPlane = port / kgNPortsPerPlane;

      std::array<unsigned short, kgNChannelsPerPort> wires;
      for (unsigned short iWire = 0; iWire < kgNChannelsPerPort; ++iWire) {
         const unsigned short kWire = port * kgNChannelsPerPort -
                                      kPlane * kgNWiresPerPlane + iWire;
         wires[iWire] = kWire;
      }

      const PlaneWires kPortWires {kPlane, wires};
      portToWires[port] = kPortWires;
   }



   // finally, assemble mapping from run numbers onto wire plane and wires indices
   RunsWiresMap runsToPlanes;
   for (auto& run : runsToPorts) {

      std::array<PlaneWires, kgNPortsPerRun> runWires;
      unsigned short iRunPort = 0;
      for (unsigned short iPort : runsToPorts[run.first]) {
         runWires[iRunPort] = portToWires[iPort];
         ++iRunPort;
      }

      runsToPlanes.emplace(run.first, runWires);
   }

   return runsToPlanes;
}



TTPCDataHandler::RunsData TTPCDataHandler::AssembleRunsData(const std::string& kDatDirectory,
                                                            const RunsWiresMap& kRunsToPlanes) const
{
   RunsData runsData;
   for (auto& run : kRunsToPlanes) {
      runsData.emplace(run.first, ReadRunData(POSIXExpand(kDatDirectory) + "/xmit_exttrig_bin_"
                                              + std::to_string(run.first) + ".dat"));
   }

   return runsData;
}



TTPCDataHandler::PlanesData TTPCDataHandler::AssemblePlanesData(const RunsData& kRunsData,
                                                                const RunsWiresMap& kRunsWiresMap) const
{
   // kRunsData: run <--> [collection][channel][sample]
   // kRunsWiresMap: run <--> [(plane, [wire])]



   // planesData: [plane][collection][wire][sample]
   PlanesData planesData;
   for (auto& run : kRunsWiresMap) {

      for (unsigned short iCollection = 0; iCollection < kRunsData.at(run.first).size(); ++iCollection) {

         for (unsigned short iRunPort = 0; iRunPort < kRunsWiresMap.at(run.first).size(); ++iRunPort) {

            const unsigned short kiPlane = kRunsWiresMap.at(run.first)[iRunPort].fPlane;


            while (planesData.size() <= kiPlane) {
               planesData.push_back(PlaneData{});
               
            }
            while (planesData[kiPlane].size() <= iCollection) {
               std::unique_ptr<PlaneCollection> collection(new PlaneCollection{});
               planesData[kiPlane].push_back(std::move(collection));
            }


            for (unsigned short portChannel = 0; portChannel < kgNChannelsPerPort; ++portChannel) {

               const unsigned short kRunChannel = iRunPort * kgNChannelsPerPort + portChannel,
                                    kWire = kRunsWiresMap.at(run.first)[iRunPort].fWires[portChannel];

               for (unsigned short iSample = 0; iSample < kRunsData.at(run.first)[iCollection][kRunChannel].size(); ++iSample) {
                  (*planesData[kiPlane][iCollection])[kWire][iSample] = kRunsData.at(run.first)[iCollection][kRunChannel][iSample];
               }

            }
         }
      }
   }

   return planesData;
}



float TTPCDataHandler::ComputeWireMeanVoltage(const unsigned short kiPlane,
                                              const unsigned short kiWire) const
{
   float mean = 0;
   unsigned nSamples = 0;

   for (unsigned short collection = 0; collection < fPlanesData[kiPlane].size(); ++collection) {
      for (unsigned short iSample = 0; iSample < (*fPlanesData[kiPlane][collection])[kiWire].size(); ++iSample) {

         // ignoring zero-valued samples
         if ((*fPlanesData[kiPlane][collection])[kiWire][iSample] != 0) {
            mean += (*fPlanesData[kiPlane][collection])[kiWire][iSample];
            ++nSamples;
         }

      }
   }

   return mean / nSamples;
}



float TTPCDataHandler::ComputePlaneCollectionMeanVoltage(const unsigned short kiPlane,
                                                         const unsigned short kiCollection) const
{
   float mean = 0;
   unsigned nSamples = 0;

   for (auto& wire : *fPlanesData[kiPlane][kiCollection]) {
      for (int sample : wire) {

         // ignoring zero-valued samples
         if (sample != 0) {
            mean += sample;
            ++nSamples;
         }

      }
   }

   return mean / nSamples;
}



TTPCDataHandler::WiresRMS TTPCDataHandler::ComputeWiresRMS() const
{
   WiresRMS RMS{{0}};

   for (unsigned short iPlane = 0; iPlane < fPlanesData.size(); ++iPlane) {
      for (unsigned short wire = 0; wire < kgNWiresPerPlane; ++wire) {
         unsigned nSamples = 0;

         const float kWireMean = ComputeWireMeanVoltage(iPlane, wire);

         for (unsigned short collection = 0; collection < fPlanesData[iPlane].size(); ++collection) {
            for (unsigned short iSample = 0; iSample < (*fPlanesData[iPlane][collection])[wire].size(); ++iSample) {

               // ignoring zero-valued samples
               if ((*fPlanesData[iPlane][collection])[wire][iSample] != 0) {
                  RMS[iPlane][wire] += std::pow((*fPlanesData[iPlane][collection])[wire][iSample] - kWireMean, 2);
                  ++nSamples;
               }

            }
         }

        RMS[iPlane][wire] /= nSamples;
        RMS[iPlane][wire] = std::sqrt(RMS[iPlane][wire]);
      }
   }

   return RMS;
}



std::string TTPCDataHandler::WritePlanesData(const std::string& kROOTFilename) const
{
   std::string runs("runs" + std::to_string(fkFirstRun) + "through" + std::to_string(fkLastRun)),
               allDataFilename(runs + ".root");
   // if the file already exists, try appending successive numbers __2, __3, ...
   for (unsigned short i = 2; std::ifstream(allDataFilename).good(); ++i) {
      allDataFilename = runs + "__" + std::to_string(i) + ".root";
   }

   TFile ROOTFile(allDataFilename.c_str(), "RECREATE");



   for (unsigned short iPlane = 0; iPlane < fPlanesData.size(); ++iPlane) {

      gStyle->SetOptStat(false);

      { // construct voltage histograms
         for (unsigned short iCollection = 0; iCollection < fPlanesData[iPlane].size(); ++iCollection) {

            // histograms have voltage pedestal subtracted
            const int kVoltagePedestal = std::round(ComputePlaneCollectionMeanVoltage(iPlane, iCollection));
            TH2S voltageHistogram(std::string("volts_mb" + std::to_string(iPlane * 2 + 1)
                                              + "and" + std::to_string(iPlane * 2 + 2)
                                              + "col" + std::to_string(iCollection)).c_str(),
                                  std::string("motherboards " + std::to_string(iPlane * 2 + 1)
                                              + " and " + std::to_string(iPlane * 2 + 2) + ", collection "
                                              + std::to_string(iCollection) + ", subtracted pedestal "
                                              + std::to_string(kVoltagePedestal) + " ADC units").c_str(),
                                  kgNSamplesPerChannel, 0, kgChannelTimeWindow,
                                  kgNWiresPerPlane, 0, kgNWiresPerPlane);
            voltageHistogram.SetXTitle("time (ms)");
            voltageHistogram.SetYTitle("wire");
            voltageHistogram.SetZTitle("voltage (ADC units)");



            for (unsigned short wire = 0; wire < kgNWiresPerPlane; ++wire) {
               for (unsigned short iSample = 0; iSample < kgNSamplesPerChannel; ++iSample) {

                  // ignoring zero-valued samples
                  if ((*fPlanesData[iPlane][iCollection])[wire][iSample] != 0) {
                     voltageHistogram.SetBinContent(iSample + 1, wire + 1,
                                                    (*fPlanesData[iPlane][iCollection])[wire][iSample]
                                                    - kVoltagePedestal);
                  }
                  
               }
            }
            

            
            TCanvas canvas(voltageHistogram.GetName(), voltageHistogram.GetTitle());
            voltageHistogram.Draw("COLZ");
            canvas.Write();
         }
      }



      { // construct RMS histogram

         TH1F RMSHistogram(std::string("RMS_mb" + std::to_string(iPlane * 2 + 1)
                                       + "and" + std::to_string(iPlane * 2 + 2)).c_str(),
                           std::string("motherboards " + std::to_string(iPlane * 2 + 1)
                                       + " and " + std::to_string(iPlane * 2 + 2)).c_str(),
                           kgNWiresPerPlane, 0, kgNWiresPerPlane);

         for (unsigned short wire = 0; wire < fRMS[iPlane].size(); ++wire) {
            RMSHistogram.SetBinContent(wire + 1, fRMS[iPlane][wire]);
         }

         RMSHistogram.SetXTitle("wire");
         RMSHistogram.SetYTitle("voltage RMS (ADC units)");
         RMSHistogram.Write();
      }



   }
   
   ROOTFile.Close();
   std::cout << "plotPlanes: saved " << allDataFilename << std::endl;
   return allDataFilename;
}