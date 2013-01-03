#include "PrimingSite.h"
#include <string>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2007 Affymetrix, Inc.                                         //
//                                                                             //
// This program is free software; you can redistribute it and/or modify        //
// it under the terms of the GNU General Public License (version 2) as         //
// published by the Free Software Foundation.                                  //
//                                                                             //
// This program is distributed in the hope that it will be useful,             //
// but WITHOUT ANY WARRANTY; without even the implied warranty of              // 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            //
// General Public License for more details.                                    //
//                                                                             //
// You should have received a copy of the GNU General Public License           //
// along with this program;if not, write to the                                //
//                                                                             //
// Free Software Foundation, Inc.,                                             //
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                       // 
/////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PrimingSite (constructor)                                                 //
//  inputs: seqName <char*>, for primer pos <int>, rev primer pos <int>,     //
//          strand <int>, for match score <int>, rev match score <int>       //
//    desc: initializes the priming site                                     //                                                   
///////////////////////////////////////////////////////////////////////////////  

PrimingSite::PrimingSite(char *cSeqName, unsigned int cForPos, unsigned int cRevPos,
                         PrimerDirection cForDir, PrimerDirection cRevDir,
                         int cStrand, int cForScore, int cRevScore) {
  strcpy(seqName,cSeqName);
  forPos = cForPos;
  revPos = cRevPos;
  forScore = cForScore;
  revScore = cRevScore;
  strand = cStrand;
  forDir = cForDir;
  revDir = cRevDir;
}

///////////////////////////////////////////////////////////////////////////////
// compare                                                                   //
//  inputs: another priming site <PrimingSite*>                              //
//  output: 0 if sites are equivalent, otherwise non-0                       //
//    desc: compares two priming sites to see if they're equivalent          //                                                   
///////////////////////////////////////////////////////////////////////////////  

int PrimingSite::compare(PrimingSite *ps) {
  return strcmp(seqName,ps->seqName) || (forPos - ps->forPos) ||
         (revPos - ps->revPos);
}

///////////////////////////////////////////////////////////////////////////////
// accessors                                                                 //                                                  
///////////////////////////////////////////////////////////////////////////////  

unsigned int PrimingSite::getForPos() {
  return forPos;
}

unsigned int PrimingSite::getRevPos() {
  return revPos;
}

PrimerDirection PrimingSite::getForDir() {
  return forDir;
}

PrimerDirection PrimingSite::getRevDir() {
  return revDir;
}

int PrimingSite::getStrand() {
  return strand;
}

char *PrimingSite::getSeqName() {
  return seqName;
}
