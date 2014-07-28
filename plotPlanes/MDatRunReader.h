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

#include "VMRunReader.h"


/// mixin class to read a single run in the Nevis dat format
class MDatRunReader : protected virtual VMRunReader {

protected:
   
   /// reads one run from the Nevis dat file located at kDatPath.
   RunData ReadRunData(const std::string& kDatPath) const;

};

#endif /* defined(__TTPCDataHandler__MDatRunReader__) */
