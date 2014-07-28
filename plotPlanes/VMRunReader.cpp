//
//  VMRunReader.cpp
//  plotPlanes
//
//  Daine L. Danielson
//  CAPTAIN
//  Los Alamos National Laboratory
//  07/27/2014
//

#include "VMRunReader.h"


std::string VMRunReader::POSIXExpand(const std::string& kWord)
{
   wordexp_t expandedWord;
   wordexp(kWord.c_str(), &expandedWord, 0);
   return std::string(*expandedWord.we_wordv);
}