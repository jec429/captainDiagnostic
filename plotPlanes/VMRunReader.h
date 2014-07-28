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
#include <fstream>
#include <wordexp.h>


/// abstract base class for mixins that read various run data files.
class VMRunReader {
   
protected:
   /// RunData: [collection][channel][sample]
   /// stores one run's data, indexed by collection, channel, and sample.
   typedef std::array<std::array<int, kgNSamplesPerChannel>, kgNChannelsPerRun> RunCollection;
   typedef std::vector<RunCollection> RunData;

    /// expands a POSIX expression, i.e., a path containing environmental variables
   static std::string POSIXExpand(const std::string& kWord);

public:

   /// Reads one run.
   virtual RunData ReadRunData(const std::string& kPath) const = 0;
};

#endif /* defined(__TTPCDataHandler__VMRunReader__) */
