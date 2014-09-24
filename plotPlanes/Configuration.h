//
//  Configuration.h
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/14/2014
//

#ifndef plotPlanes_Configuration_h
#define plotPlanes_Configuration_h

#include <array>
#include <cmath>
#include <string>

/// numerical constants describing relevent hardware configuration parameters during data taking
const static float          kgChannelTimeWindow     = 4.8; // ms
const static unsigned       kgMaxAllowedSample      = 20000; // ADC units
const static unsigned short kgNEventsPerRun         = 1,
                            kgNoisyThreshold        = 3, // ADC units
                            kgNSamplesPerChannel    = 9595,
                            kgNChannelsPerPort      = 32,
                            kgNChannelsPerASIC      = 16,
                            kgNPortsPerRun          = 24,
                            kgNPortsPerPlane        = 12,
                            kgNPlanes               = 3,
                            kgNMotherboardsPerPlane = 2,

                            kgNChannelsPerRun = kgNChannelsPerPort * kgNPortsPerRun,
                            kgNPlanesPerRun = kgNPortsPerRun % kgNPortsPerPlane == 0 ?
                                              kgNPortsPerRun / kgNPortsPerPlane :
                                              kgNPortsPerRun / kgNPortsPerPlane + 1,
                            kgNWiresPerPlane = kgNChannelsPerPort * kgNPortsPerPlane,
                            kgNPorts = kgNPortsPerPlane * kgNPlanes,
                            kgNASICsPerMotherboard = kgNWiresPerPlane / kgNMotherboardsPerPlane
                                                     / kgNChannelsPerASIC,
                            kgNChannelsPerMotherboard = kgNWiresPerPlane / kgNMotherboardsPerPlane,
                            kgNASICSPerPlane = kgNASICsPerMotherboard * kgNMotherboardsPerPlane;


const static std::array<std::string, kgNPlanes> kgPlaneNames{"u","v","anode"};

#endif
