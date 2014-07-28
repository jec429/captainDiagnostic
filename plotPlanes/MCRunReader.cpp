//
//  MCRunReader.cpp
//  TPC Data Handler
//
//  Created by Daine Danielson on 7/26/14.
//  Copyright (c) 2014 CAPTAIN. All rights reserved.
//

#include "MCRunReader.h"



MCRunReader::RunData MCRunReader::ReadRunData(const std::string& kCRunDirectory) const
{
   std::ifstream inputFile;

   // look for a POSIX run directory
   inputFile.open(kCRunDirectory);
   if (!inputFile.good()) {
      std::cout << "plotPlanes MCRunReader::ReadRunData() error: could not read "
                << kCRunDirectory << std::endl;
      exit(EXIT_FAILURE);
   }
   inputFile.close();

   RunData data;

   for (unsigned short collection = 0; collection < kgNCollectionsPerRun; ++collection) {
      unsigned short nSamples; // time samples per channel

      data.push_back(RunCollection{{0}});

      for (unsigned short channel = 0; channel < data[collection].size(); ++channel) {

         // open channel file to parse
         inputFile.open(kCRunDirectory+ "/histo_Collection" + std::to_string(collection + 1)
                        + "_Channel" + std::to_string(channel) + ".C");

         if (inputFile.good()) {

            for (unsigned short iComma = 0; iComma < 4; ++iComma) {
               inputFile.ignore(256, ',');
            }


            // construct channel histogram
            inputFile >> nSamples;


            char temp[25];
            inputFile.ignore(256, '\n');
            inputFile.get(temp, 25);



            // fill channel histogram
            while (strcmp(temp, "   histo->SetBinContent(") == 0) {

               unsigned short iSample;
               int voltage;

               inputFile >> iSample;
               inputFile.ignore();
               inputFile >> voltage;


               data[collection][channel][iSample] = voltage;


               inputFile.ignore(256, '\n');
               inputFile.get(temp, 25);

            }

         }
         inputFile.close();


      } // channel done
   } // collection done

   return data;
}