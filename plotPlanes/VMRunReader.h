//
//  VMRunReader.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/27/2014
//


#ifndef __TTPCDataHandler__VMRunReader__
#define __TTPCDataHandler__VMRunReader__

#include "Configuration.h"
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <wordexp.h>


/// abstract base class for mixins that read various run data files.
class VMRunReader {

protected:
   /// RunData: (*[event])[plane][channel][sample]
   /// stores one run's data, indexed by event, channel, and sample.
   typedef std::array<std::array<std::array<int, kgNSamplesPerChannel>,
                                 kgNWiresPerPlane>, kgNPlanesPerRun> RunEvent;
   typedef std::vector<std::unique_ptr<RunEvent>> RunData;

   struct PlaneWires {
      unsigned short fPlane;
      std::array<unsigned short, kgNChannelsPerPort> fWires;
   };


   /// expands a POSIX expression, i.e., a path containing environmental variables
   static std::string POSIXExpand(const std::string& kWord);

   
   /// Reads one run.
   RunData ReadRunData(const std::string& kPath,
                       const std::array<PlaneWires, kgNPortsPerRun> = {0}) const;


   // pure virtual destrutor to make this class abstract
   //virtual ~VMRunReader() = 0;
};

#endif /* defined(__TTPCDataHandler__VMRunReader__) */
