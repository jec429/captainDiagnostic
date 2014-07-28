//
//  MDatRunReader.cpp
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//

#include "MDatRunReader.h"



MDatRunReader::RunData MDatRunReader::ReadRunData(const std::string& kDatPath) const
{
   std::ifstream datFile(POSIXExpand(kDatPath), std::ios_base::binary);
   if (datFile.good()) {
      RunData data;
      uint16_t word; // one 16-bit binary word
      bool pause = true;



      unsigned short iChannel = 0;
      for (unsigned short iSample = kgNSamplesPerChannel + 1; datFile.good();) {

         datFile.read(reinterpret_cast<char*>(&word), sizeof word);

         if ((word & 0xf000) >> 12 == 4) {

            // start new channel
            iSample = 0;
            iChannel = word & 0xfff;
            if (iChannel == 0 || (iChannel == 1 && pause)) {

               // start new collection
               data.push_back(RunCollection{{0}});

               pause = false;
            } else if (iChannel > 1) {
               pause = true;
            }
         } else if ((word & 0xf000) >> 12 == 5) {
            iSample = 0;
         } else {

            /// Sometimes, dat files report erroniously large channel or sample
            /// indices. These are ignored.
            if (iChannel < kgNChannelsPerRun && iSample < kgNSamplesPerChannel) {
               // store sample.
               /// |sample| > kgMaxAllowedSample and sample == 0 are symptoms of soft errors in dat.
               /// Cause to be determined.
               /// This sets these samples to zero.
               data.back()[iChannel][iSample] = (std::abs(word) > kgMaxAllowedSample ? 0 : word);
            }


            ++iSample;
         }
      }

      // discard channels with too few samples
      // Typically these arise when the dat file contains an erronious collection
      // with a channel containing just one sample.
      if (iChannel < kgNChannelsPerRun / 2) {
         data.pop_back();
      }



      datFile.close();
      return data;

   } else {
      std::cout << "plotPlanes MDatRunReader::ReadRunData() error: could not read "
                << kDatPath << std::endl;
      exit(EXIT_FAILURE);
   }
}
