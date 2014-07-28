//
//  Configuration.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//

#ifndef TPlaneROOTBuilder_Configuration_h
#define TPlaneROOTBuilder_Configuration_h

#include <array>
#include <string>

/// numerical constants describing relevent hardware configuration parameters during data taking
const static float    kgChannelTimeWindow = 4.8; // ms
const static unsigned kgMaxAllowedSample = 20000; // ADC units
const static unsigned short kgNSamplesPerChannel = 9595, kgNChannelsPerPort = 32,
                            kgNChannelsPerASIC = 16, kgNPortsPerRun = 2,
                            kgNPortsPerPlane = 12, kgNPlanes = 3,
                            kgNMotherboardsPerPlane = 2,

                            kgNChannelsPerRun = kgNChannelsPerPort * kgNPortsPerRun,
                            kgNWiresPerPlane = kgNChannelsPerPort * kgNPortsPerPlane,
                            kgNPorts = kgNPortsPerPlane * kgNPlanes,
                            kgNASICsPerMotherboard = kgNWiresPerPlane / kgNMotherboardsPerPlane
                                                     / kgNChannelsPerASIC,
                            kgNChannelsPerMotherboard = kgNWiresPerPlane / kgNMotherboardsPerPlane,
                            kgNASICSPerPlane = kgNASICsPerMotherboard * kgNMotherboardsPerPlane;


const static std::array<std::string, kgNPlanes> kgPlaneNames{"u","v","anode"};



/// set C_MACRO_MODE to 1 if you want to read from ReadDataByChannel ROOT *.C
/// macro run directories instead of dat files.
#define C_MACRO_MODE 0
const static unsigned short kgNCollectionsPerRun = 5; // only used in C Macro Mode


#endif
