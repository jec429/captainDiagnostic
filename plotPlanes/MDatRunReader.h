//
//  MDatRunReader.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//


#ifndef __TTPCDataHandler__MDatRunReader__
#define __TTPCDataHandler__MDatRunReader__

#include "Configuration.h"
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <wordexp.h>


/// class to read a single run in the Nevis dat format
class MDatRunReader {

protected:
   /// RunData: [collection][channel][sample]
   /// stores one run's data, indexed by collection, channel, and sample.
   typedef std::array<std::array<int, kgNSamplesPerChannel>, kgNChannelsPerRun> RunCollection;
   typedef std::vector<RunCollection> RunData;

   /// expands a POSIX expression, i.e., a path containing environmental variables
   std::string POSIXExpand(const std::string& kWord) const;

public:
   /// reads one run from the Nevis dat file located at kDatPath.
   RunData ReadRunData(const std::string& kDatPath) const;
};

#endif /* defined(__TTPCDataHandler__MDatRunReader__) */
