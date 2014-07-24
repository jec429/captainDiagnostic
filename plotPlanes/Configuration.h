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

#endif
