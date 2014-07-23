//
//  main.cpp
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//


#include "TTPCDataHandler.h"

int main(int argc, const char* argv[])
{
   unsigned short firstRun, lastRun;
   std::string logFilePath, datDirectoryPath;

   unsigned short iArg = 0;
   if (argc == 5) {


      datDirectoryPath = argv[++iArg];
      firstRun = std::atoi(argv[++iArg]);
      lastRun = std::atoi(argv[++iArg]);
      logFilePath = argv[++iArg];


   } else {


      if (argc == 2) {
         datDirectoryPath = argv[++iArg];
      } else {
         std::cout << "Enter dat directory path: ";
         std::cin >> datDirectoryPath;
      }

      std::cout << "Enter lowest run number to process: ";
      std::cin >> firstRun;
      std::cout << "Enter highest run number to process: ";
      std::cin >> lastRun;

      std::cout << "Enter log file: ";
      std::cin >> logFilePath;
   }


   
   std::cout << "..." << std::endl;
   std::string ROOTFilename("runs" + std::to_string(firstRun)
                            + "through" + std::to_string(lastRun) + ".root");
   TTPCDataHandler handler(logFilePath, datDirectoryPath, firstRun, lastRun);
   ROOTFilename = handler.WritePlanesData(ROOTFilename);



   return EXIT_SUCCESS;
}

