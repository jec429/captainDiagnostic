//
//  MSingleEventRunReader.cpp
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//

#include "MSingleEventRunReader.h"



MSingleEventRunReader::RunData MSingleEventRunReader::ReadRunData(const std::string& kDatPath,
                                                                  const std::array<PlaneWires, kgNPortsPerRun>& kPlaneWires) const
{

   std::ifstream datFile(POSIXExpand(kDatPath), std::ios_base::binary);
   if (datFile.good()) {
      RunData data;
      uint16_t word; // one 16-bit binary word
      bool first = true;



      unsigned short iPlane = 0, iChannel = 0;
      data.push_back(std::unique_ptr<RunEvent>(new RunEvent{0}));


      for (unsigned short iSample = 0; datFile.good();) {

         datFile.read(reinterpret_cast<char*>(&word), sizeof word);

         if ((word & 0xf000) >> 12 == 4) {
            if (!first) {

               // start new channel
               iSample = 0;
               if (iChannel < kgNWiresPerPlane) {
                  ++iChannel;
               } else {
                  ++iPlane;
                  iChannel = 1;
               }
            } else {
               first = false;
            }
         } else {

            /// Sometimes, dat files report erroniously large channel or sample
            /// indices. These are ignored.
            if (iSample < kgNSamplesPerChannel) {
               // store sample.
               /// |sample| > kgMaxAllowedSample and sample == 0 are symptoms of soft errors in dat.
               /// Cause to be determined.
               /// This sets these samples to zero.
              // std::cout << "plane"<<iPlane << " channel" << iChannel << " iSample" << iSample<<" word" << word << "\n";
               (*data.back())[iPlane][iChannel][iSample] = (std::abs(word) > kgMaxAllowedSample ? 0 : word);
            }


            ++iSample;
         }
      }



      datFile.close();
      return data;

   } else {
      std::cout << "plotPlanes MSingleEventRunReader::ReadRunData() error: could not read "
                << kDatPath << std::endl;
      exit(EXIT_FAILURE);
   }
}



