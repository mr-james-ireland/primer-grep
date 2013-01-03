#ifndef TARGETCACHE_H__
#define TARGETCACHE_H__

#include "Properties.h"

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
// TargetCache                                                               //
// - Records sequence surrounding a previous primer pair hit.                //                                                 
///////////////////////////////////////////////////////////////////////////////  

class TargetCache{
  public:
    TargetCache();
    void copyBuffer(char* cSeqName, unsigned int cStart, unsigned int cEnd,
                    char *cBuffer, int sBufferSize, int start, int size);
    bool isEmpty();
    char *getBuffer();
    char *getSeqName();
    int getBufferSize();
    unsigned int getStart();
    
  private:
    static const int MAX_SEQ_BUFFER = MAX_AMP_SIZE + 3*BUFFER_FLANK; // needs to be synched with max allowed amp size
    static const int MAX_SEQ_NAME = MAX_SEQNAME_SIZE;
    char seqBuffer[MAX_SEQ_BUFFER];
    int start;
    int end;
    int bufferSize;
    char seqName[MAX_SEQ_NAME];
    bool empty;
};

#endif // TARGETCACHE_H__
