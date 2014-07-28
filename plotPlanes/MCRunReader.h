//
//  MCRunReader.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/27/2014
//


#ifndef __TTPCDataHandler__MCRunReader__
#define __TTPCDataHandler__MCRunReader__

#include "VMRunReader.h"
#include <cstring>


/// mixin class to read a single run from a ROOT single-histogram C macro
class MCRunReader : protected virtual VMRunReader {

protected:
   
   /// reads one run from the Nevis dat file located at kDatPath.
   RunData ReadRunData(const std::string& kCRunDirectory) const;
};

#endif /* defined(__TTPCDataHandler__MCRunReader__) */
