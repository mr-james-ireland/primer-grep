#include "TargetCache.h"
//#include "PrimerSearch.h"
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
// TargetCache (constructor)                                                 //
//  inputs: none                                                             //
//    desc: initializes cache as empty                                       //                                                   
///////////////////////////////////////////////////////////////////////////////  

TargetCache::TargetCache() {
  empty = true;
}

///////////////////////////////////////////////////////////////////////////////
// copyBuffer                                                                //
//  inputs: seq name <char*>, contig start <int>, contig end <int>, seq      //
//          buffer <char*>, buffer size <int>, buffer start <int>, size <int>//                                               //
//  output: none                                                             //
//    desc: copies seq from a seq buffer                                     //                                                   
///////////////////////////////////////////////////////////////////////////////  

void TargetCache::copyBuffer(char* cSeqName, unsigned int cStart, unsigned int cEnd,
                             char *cBuffer, int sBufferSize, int bStart, int size) {  
  // remember name and positions
  strcpy(seqName,cSeqName);
  start = cStart;
  end = cEnd;
  
  // copy seq data into our buffer being careful to avoid buffer boundaries
  for (int i=0; i < size; ++i) {
    int bufPos = (bStart + i) % sBufferSize; // TODO: is this correct?
    seqBuffer[i] = cBuffer[bufPos];
  }
  bufferSize = size;
  empty = false;
}

///////////////////////////////////////////////////////////////////////////////
// accessors                                                                 //                                                  
///////////////////////////////////////////////////////////////////////////////  

bool TargetCache::isEmpty() {
  return empty;
}

char *TargetCache::getBuffer() {
  return seqBuffer;
}

char *TargetCache::getSeqName() {
  return seqName;
}

int TargetCache::getBufferSize() {
  return bufferSize;
}

unsigned int TargetCache::getStart() {
  return start;
}

