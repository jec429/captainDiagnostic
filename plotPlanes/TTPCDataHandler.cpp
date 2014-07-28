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
                                 const std::string& kRunsDirectory,
                                 const unsigned kFirstRun,
                                 const unsigned kLastRun)
: fkFirstRun(kFirstRun), fkLastRun(kLastRun)
{
   std::ifstream logFile(POSIXExpand(kLogPath));
   if (logFile.good()) {

      const RunsWiresMap kRunsWiresMap = MapRunsToPlanes(logFile);
      const RunsData kRunsData = AssembleRunsData(kRunsDirectory, kRunsWiresMap);

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



TTPCDataHandler::RunsData TTPCDataHandler::AssembleRunsData(const std::string& kRunsDirectory,
                                                            const RunsWiresMap& kRunsToPlanes) const
{
   RunsData runsData;

   for (auto& run : kRunsToPlanes) {
      if (!kgCMacroMode) {
         runsData.emplace(run.first, ReadRunData(POSIXExpand(kRunsDirectory) + "/xmit_exttrig_bin_"
                                                 + std::to_string(run.first) + ".dat"));
      } else {
         runsData.emplace(run.first, ReadRunData(POSIXExpand(kRunsDirectory)
                                                 + "/outputdir_run_" + std::to_string(run.first)));
      }
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

   if (nSamples > 0) {
      mean /= nSamples;
   }
   return mean;
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

   if (nSamples > 0) {
      mean /= nSamples;
   }
   return mean;
}



TTPCDataHandler::WiresRMS TTPCDataHandler::ComputeWiresRMS() const
{
   WiresRMS RMS{{0}};

   for (unsigned short iPlane = 0; iPlane < fPlanesData.size(); ++iPlane) {
      for (unsigned short wire = 0; wire < kgNWiresPerPlane; ++wire) {
         float mean0 = 0;
         unsigned nSamples = 0;
         for (auto& collection : fPlanesData[iPlane]) {
            for (int sample : (*collection)[wire]) {
               // ignoring zero-valued samples
               if (sample != 0) {
                  mean0 += sample;
                  ++nSamples;
               }
            }
         }
         if (nSamples != 0) {
            mean0 /= nSamples;
         }

         float RMS0 = 0.;
         for (auto& collection : fPlanesData[iPlane]) {
            for (int sample : (*collection)[wire]) {
               // ignoring zero-valued samples
               if (sample != 0) {
                  RMS0 += std::pow(sample, 2);
               }
            }
         }
         if (nSamples != 0) {
            RMS0 =  std::sqrt(RMS0 / nSamples);
         }

         float mean1 = 0;
         nSamples = 0;
         for (auto& collection : fPlanesData[iPlane]) {
            for (int sample : (*collection)[wire]) {
               // ignoring zero-valued samples
               if (std::abs(sample - mean0) < 3 * RMS0 && sample != 0) {
                  mean1 += sample;
                  ++nSamples;
               }
            }
         }
         if (nSamples != 0) {
            mean1 /= nSamples;
         }

         float RMS1 = 0;
         nSamples = 0;
         for (auto& collection : fPlanesData[iPlane]) {
            for (int sample : (*collection)[wire]) {
               // ignoring zero-valued samples
               if (std::abs(sample - mean1) < 3 * RMS0 && sample != 0) {
                  RMS1 += std::pow(sample - mean1, 2.);
                  ++nSamples;
               }
            }
         }

         if (nSamples != 0) {
            RMS[iPlane][wire] = std::sqrt(RMS1 / nSamples);
         }
      }
   }
   
   return RMS;
}



TTPCDataHandler::WiresRMS TTPCDataHandler::GetWiresRMS() const
{
   return fRMS;
}



TTPCDataHandler::ASICsMeanRMS TTPCDataHandler::GetASICsMeanRMS() const
{
   ASICsMeanRMS allRMS;

   for (unsigned short iPlane = 0; iPlane < fRMS.size(); ++iPlane) {
      float ASICMeanRMS = 0;

      for (unsigned short wire = 0, ASICChannel = 0, moboASIC = 0, motherboard = 0, nRMS = 0;
           wire < fRMS[iPlane].size(); ++wire, ++ASICChannel) {



         if (ASICChannel < kgNChannelsPerASIC) {

            ASICMeanRMS += fRMS[iPlane][wire];
            ++nRMS;

         } else {

            allRMS[iPlane][motherboard][moboASIC].fRMS = ASICMeanRMS / nRMS;

            if (wire - motherboard * kgNChannelsPerMotherboard == kgNChannelsPerMotherboard) {
               ++motherboard;
               moboASIC = 0;
            } else {
               ++moboASIC;
            }

            ASICChannel = 0;
            ASICMeanRMS = 0;

            nRMS = 0;

         }
         allRMS[iPlane][motherboard][moboASIC].fWires[ASICChannel] = wire;


      }
   }

   return allRMS;
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
            TH2S voltageHistogram(std::string("volts_" + kgPlaneNames[iPlane] + "PlaneCol" + std::to_string(iCollection)).c_str(),
                                  std::string(kgPlaneNames[iPlane] + " plane, collection "
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



      { // construct wire RMS histogram

         TH1F RMSHistogram(std::string("RMS_" + kgPlaneNames[iPlane] + "Plane").c_str(),
                           std::string(kgPlaneNames[iPlane] + " plane").c_str(),
                           kgNWiresPerPlane, 0, kgNWiresPerPlane);

         for (unsigned short wire = 0; wire < fRMS[iPlane].size(); ++wire) {
            RMSHistogram.SetBinContent(wire + 1, fRMS[iPlane][wire]);
         }

         RMSHistogram.SetXTitle("wire");
         RMSHistogram.SetYTitle("noise RMS voltage (ADC units)");
         RMSHistogram.Write();

         TCanvas cRMS;
         RMSHistogram.Draw();
         cRMS.SaveAs((std::string(RMSHistogram.GetName()) + ".pdf").c_str());
      }


      { // contruct ASIC RMS histogram
         TH1F ASICMeanRMSHistogram(std::string("ASICMeanRMS_" + kgPlaneNames[iPlane] + "Plane").c_str(),
                                   std::string(kgPlaneNames[iPlane] + " plane").c_str(),
                                   kgNASICSPerPlane, 0, kgNASICSPerPlane);

         ASICsMeanRMS RMS = GetASICsMeanRMS();

         unsigned short bin = 1;
         for (unsigned short motherboard = 0; motherboard < RMS[iPlane].size(); ++motherboard) {
            for (unsigned short ASIC = 0; ASIC < RMS[iPlane][motherboard].size(); ++ASIC, ++bin) {
               ASICMeanRMSHistogram.SetBinContent(bin, RMS[iPlane][motherboard][ASIC].fRMS);
            }
         }

         ASICMeanRMSHistogram.SetXTitle("ASIC");
         ASICMeanRMSHistogram.SetYTitle("mean noise RMS voltage (ADC units)");
         ASICMeanRMSHistogram.Write();

         TCanvas cASICMeanRMS;
         ASICMeanRMSHistogram.Draw();
         cASICMeanRMS.SaveAs((std::string(ASICMeanRMSHistogram.GetName()) + ".pdf").c_str());

      }



   }
   
   ROOTFile.Close();
   std::cout << "plotPlanes: saved " << allDataFilename << std::endl;
   return allDataFilename;
}