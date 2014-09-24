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
   unsigned short run;
   std::string logFilePath, datDirectoryPath;


   // process command line arguements
   unsigned short iArg = argc;


   if (argc == 4) {
      run = std::atoi(argv[--iArg]);
      logFilePath = argv[--iArg];
      datDirectoryPath = argv[--iArg];
   } else {
      // if command line arguments are missing or invalid, prompt for run parameters
      std::cout << "Enter dat directory path: ";
      std::cin >> datDirectoryPath;

      std::cout << "Enter log file: ";
      std::cin >> logFilePath;

      std::cout << "Enter run number to process: ";
      std::cin >> run;

   }


   std::cout << "..." << std::endl;


   std::string ROOTFilename;
   ROOTFilename = "run" + std::to_string(run);

   TTPCDataHandler handler(logFilePath, datDirectoryPath, run);
   handler.WritePlanesData(ROOTFilename);



   return EXIT_SUCCESS;
}

