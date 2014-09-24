//
//  MSingleEventRunReader.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//


#ifndef __TTPCDataHandler__MSingleEventRunReader__
#define __TTPCDataHandler__MSingleEventRunReader__

#include "VMRunReader.h"


/// mixin class to read a single run in the Nevis dat format
class MSingleEventRunReader : protected virtual VMRunReader {

protected:
   RunData ReadRunData(const std::string& kDatPath,
                       const std::array<PlaneWires, kgNPortsPerRun>& kPlaneWires) const;


};

#endif /* defined(__TTPCDataHandler__MSingleEventRunReader__) */
