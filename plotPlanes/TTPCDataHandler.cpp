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
                                 const unsigned kRun)
: fkRun(kRun)
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
      char temp;

      while ((logFile.get(temp), temp) == '#' && logFile.good()) {
         logFile.ignore(256, '\n');
      }
      logFile.unget();

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

      if (run >= fkRun && logFile.good()) {
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

      logFile.ignore(256, '\n');
   }



   // map line driver port numbers onto their connected plane and wires indices
   PortWiresMap portToWires;
   for (unsigned short port = 0; port < portToWires.size(); ++port) {
      const unsigned short kPlane = port / kgNPortsPerPlane;

      std::array<unsigned short, kgNChannelsPerPort> wires;
      for (unsigned short iWire = 0; iWire < kgNChannelsPerPort; ++iWire) {

         unsigned wire;
         if (port % 2 == 0) {
            wire = port * kgNChannelsPerPort - kPlane * kgNWiresPerPlane
                   + 2 * iWire;
         } else {
            wire = (port - 1) * kgNChannelsPerPort - kPlane * kgNWiresPerPlane
                   + 2 * iWire + 1;
         }

         wires[iWire] = wire;
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

   runsData.emplace(fkRun, MSingleEventRunReader::ReadRunData(POSIXExpand(kRunsDirectory)
                                                                   + "/xmit_exttrig_bin_"
                                                                   + std::to_string(fkRun)
                                                                   + ".dat", kRunsToPlanes.at(fkRun)));



   return runsData;
}


TTPCDataHandler::PlanesData TTPCDataHandler::AssemblePlanesData(const RunsData& kRunsData,
                                                                const RunsWiresMap& kRunsWiresMap) const
{

   // kRunsData: run <--> [event][plane][channel][sample]
   // kRunsWiresMap: run <--> [(plane, [wire])]


   // planesData: [plane][event][wire][sample]
   PlanesData planesData;
   for (auto& run : kRunsWiresMap) {

       for (unsigned short iEvent = 0; iEvent < kRunsData.at(run.first).size(); ++iEvent) {
          unsigned short runPlane = 0, iPlane = 0;


         for (unsigned short iRunPort = 0; iRunPort < kgNPortsPerRun; ++iRunPort) {

            if (iPlane != kRunsWiresMap.at(run.first)[iRunPort].fPlane) {
               iPlane = kRunsWiresMap.at(run.first)[iRunPort].fPlane;
               ++runPlane;
            }


            while (planesData.size() <= iPlane) {
               planesData.push_back(PlaneData{});
            }
            while (planesData[iPlane].size() <= iEvent) {
               std::unique_ptr<PlaneEvent> event(new PlaneEvent{});
               planesData[iPlane].push_back(std::move(event));
            }


            for (unsigned short portChannel = 0; portChannel < kgNChannelsPerPort; ++portChannel) {

               const unsigned short kRunPlaneChannel = iRunPort * kgNChannelsPerPort - runPlane * kgNWiresPerPlane + portChannel,
                                    kWire = kRunsWiresMap.at(run.first)[iRunPort].fWires[portChannel];
              // std::cout << "plane" << iPlane << "event" << iEvent << "wire" << kWire << "run" << run.first << "runplanechannel" << kRunPlaneChannel << "portchannel" << portChannel << "iRunPort" << iRunPort<<"\n";

               for (unsigned short iSample = 0; iSample < kgNSamplesPerChannel; ++iSample) {
                  (*planesData[iPlane][iEvent])[kWire][iSample] = (*kRunsData.at(run.first)[iEvent])[iPlane][kRunPlaneChannel][iSample];
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

   for (unsigned short event = 0; event < fPlanesData[kiPlane].size(); ++event) {
      for (unsigned short iSample = 0; iSample < (*fPlanesData[kiPlane][event])[kiWire].size(); ++iSample) {

         // ignoring zero-valued samples
         if ((*fPlanesData[kiPlane][event])[kiWire][iSample] != 0) {
            mean += (*fPlanesData[kiPlane][event])[kiWire][iSample];
            ++nSamples;
         }

      }
   }

   if (nSamples > 0) {
      mean /= nSamples;
   }
   return mean;
}



float TTPCDataHandler::ComputePlaneEventMeanVoltage(const unsigned short kiPlane,
                                                         const unsigned short kiEvent) const
{
   float mean = 0;
   unsigned nSamples = 0;

   for (auto& wire : *fPlanesData[kiPlane][kiEvent]) {
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
   WiresRMS RMS{};

   for (unsigned short iPlane = 0; iPlane < fPlanesData.size(); ++iPlane) {
      for (unsigned short wire = 0; wire < kgNWiresPerPlane; ++wire) {
         float mean0 = 0;
         unsigned nSamples = 0;
         for (auto& event : fPlanesData[iPlane]) {
            for (int sample : (*event)[wire]) {
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
         for (auto& event : fPlanesData[iPlane]) {
            for (int sample : (*event)[wire]) {
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
         for (auto& event : fPlanesData[iPlane]) {
            for (int sample : (*event)[wire]) {
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
         for (auto& event : fPlanesData[iPlane]) {
            for (int sample : (*event)[wire]) {
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
      unsigned short wire = 0, ASICChannel = 0, moboASIC = 0, motherboard = 0, nRMS = 0;
      for (; wire < fRMS[iPlane].size(); ++wire, ++ASICChannel) {


         if (ASICChannel < kgNChannelsPerASIC) {

            ASICMeanRMS += fRMS[iPlane][wire];
            ++nRMS;

         } else { // this ASIC is done

            // store its RMS
            allRMS[iPlane][motherboard][moboASIC].fRMS = ASICMeanRMS / nRMS;

            // begin the next ASIC
            if (wire - motherboard * kgNChannelsPerMotherboard < kgNChannelsPerMotherboard) {
               ++moboASIC;
            } else {
               ++motherboard;
               moboASIC = 0;
            }
            ASICChannel = 0;
            ASICMeanRMS = 0;
            nRMS = 0;

         }
         allRMS[iPlane][motherboard][moboASIC].fWires[ASICChannel] = wire;
      }
      // catch the last ASIC in the plane
      allRMS[iPlane][motherboard][moboASIC].fRMS = ASICMeanRMS / nRMS;
   }

   return allRMS;
}



float TTPCDataHandler::ComputeFractionNoisy(const WiresRMS& kRMS,
                                            const unsigned short kiPlane) const {

   // compute mean RMS
   float meanRMS = 0;
   for (float RMS : kRMS[kiPlane]) {
      meanRMS += RMS;

   }
   meanRMS /= kgNWiresPerPlane;

   // compute standard deviation of RMS
   float sigmaRMS = 0;
   for (float RMS : kRMS[kiPlane]) {
      sigmaRMS += std::pow(RMS - meanRMS, 2);
   }
   sigmaRMS /= kgNWiresPerPlane;
   sigmaRMS = std::sqrt(sigmaRMS);

   // count noisy channels
   unsigned short nNoisy = 0;
   for (float RMS : kRMS[kiPlane]) {
      if (RMS - meanRMS > kgNoisyThreshold) {
         ++nNoisy;
      }
   }


   return static_cast<float>(nNoisy) / kgNWiresPerPlane;
}



float TTPCDataHandler::ComputeFractionNoisy(const ASICsMeanRMS& kRMS,
                                            const unsigned short kiPlane) const {

   // compute mean RMS
   float meanRMS = 0;
   for (auto motherboard : kRMS[kiPlane]) {
      for (auto ASIC : motherboard) {
         meanRMS += ASIC.fRMS;
      }
   }
   meanRMS /= kgNASICSPerPlane;

   // compute standard deviation of RMS
   float sigmaRMS = 0;
   for (auto motherboard : kRMS[kiPlane]) {
      for (auto ASIC : motherboard) {
         sigmaRMS += std::pow(ASIC.fRMS - meanRMS, 2);
      }
   }
   sigmaRMS /= kgNASICSPerPlane;
   sigmaRMS = std::sqrt(sigmaRMS);

   // count noisy channels
   unsigned short nNoisy = 0;
   for (auto motherboard : kRMS[kiPlane]) {
      for (auto ASIC : motherboard) {
         if (ASIC.fRMS - meanRMS > kgNoisyThreshold) {
            ++nNoisy;
         }
      }
   }

   return static_cast<float>(nNoisy) / kgNASICSPerPlane;
}



std::string TTPCDataHandler::WritePlanesData(const std::string& kROOTFilename) const
{
   std::string runs = kROOTFilename,
               allDataFilename = runs + ".root";
   // if the file already exists, try appending successive numbers __2, __3, ...
   unsigned short iDuplicate = 0;
   if (std::ifstream(allDataFilename).good()) {
      for (iDuplicate = 2; std::ifstream(allDataFilename).good(); ++iDuplicate) {
         allDataFilename = runs + "__" + std::to_string(iDuplicate) + ".root";
      }
      --iDuplicate; // because for loops overshoot before the break
   }
   TFile ROOTFile(allDataFilename.c_str(), "RECREATE");



   for (unsigned short iPlane = 0; iPlane < fPlanesData.size(); ++iPlane) {

      gStyle->SetOptStat(false);



      { // construct voltage histograms
         for (unsigned short iEvent = 0; iEvent < fPlanesData[iPlane].size(); ++iEvent) {

            // histograms have voltage pedestal subtracted
            const int kVoltagePedestal = std::round(ComputePlaneEventMeanVoltage(iPlane, iEvent));
            TH2S voltageHistogram(std::string("volts_" + kgPlaneNames[iPlane] + "PlaneCol" + std::to_string(iEvent)).c_str(),
                                  std::string(kgPlaneNames[iPlane] + " plane, event "
                                              + std::to_string(iEvent) + ", subtracted pedestal "
                                              + std::to_string(kVoltagePedestal) + " ADC units").c_str(),
                                  kgNSamplesPerChannel, 0, kgChannelTimeWindow,
                                  kgNWiresPerPlane, 0, kgNWiresPerPlane);
            voltageHistogram.SetXTitle("time (ms)");
            voltageHistogram.SetYTitle("wire");
            voltageHistogram.SetZTitle("voltage (ADC units)");



            for (unsigned short wire = 0; wire < kgNWiresPerPlane; ++wire) {
               for (unsigned short iSample = 0; iSample < kgNSamplesPerChannel; ++iSample) {

                  // ignoring zero-valued samples
                  if ((*fPlanesData[iPlane][iEvent])[wire][iSample] != 0) {
                     voltageHistogram.SetBinContent(iSample + 1, wire + 1,
                                                    (*fPlanesData[iPlane][iEvent])[wire][iSample]
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
                           std::string(kgPlaneNames[iPlane] + " plane, "
                                       + std::to_string(ComputeFractionNoisy(fRMS, iPlane) * 100)
                                       + "% are >" + std::to_string(kgNoisyThreshold)).c_str(),
                           kgNWiresPerPlane, 0, kgNWiresPerPlane);

         for (unsigned short wire = 0; wire < fRMS[iPlane].size(); ++wire) {
            RMSHistogram.SetBinContent(wire + 1, fRMS[iPlane][wire]);
         }

         RMSHistogram.SetXTitle("wire");
         RMSHistogram.SetYTitle("noise RMS voltage (ADC units)");
         RMSHistogram.GetXaxis()->SetNdivisions(406, kFALSE);
         RMSHistogram.Write();

         TCanvas cRMS;
         RMSHistogram.Draw();
         cRMS.SaveAs(std::string(RMSHistogram.GetName()
                                 + (iDuplicate == 0 ? ""
                                    : "__" + std::to_string(iDuplicate))
                                 + ".pdf").c_str());
      }


      { // contruct ASIC RMS histogram
         ASICsMeanRMS RMS = GetASICsMeanRMS();
         TH1F ASICMeanRMSHistogram(std::string("ASICMeanRMS_" + kgPlaneNames[iPlane] + "Plane").c_str(),
                                   std::string(kgPlaneNames[iPlane] + " plane,"
                                               + std::to_string(ComputeFractionNoisy(RMS, iPlane) * 100)
                                               + "% are >" + std::to_string(kgNoisyThreshold)).c_str(),
                                   kgNASICSPerPlane, 0, kgNASICSPerPlane);


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
         cASICMeanRMS.SaveAs(std::string(ASICMeanRMSHistogram.GetName()
                                         + (iDuplicate == 0 ? ""
                                            : "__" + std::to_string(iDuplicate))
                                         + ".pdf").c_str());
         
      }
      
      
      
   }
   
   ROOTFile.Close();
   std::cout << "plotPlanes: saved " << allDataFilename << std::endl;
   return allDataFilename;
}