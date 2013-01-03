#ifndef PRIMINGSITE_H__
#define PRIMINGSITE_H__

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
// PrimingSite                                                               //
// - Holds a priming site for a primer pair                                  //                                                 
///////////////////////////////////////////////////////////////////////////////  

class PrimingSite{
  public:
    PrimingSite(char* cSeqName, unsigned int cForPos, unsigned int cRevPos, 
				PrimerDirection cForDir, PrimerDirection cRevDir,
                int cStrand, int cForScore, int cRevScore);
    int compare(PrimingSite *ps);
    unsigned int getForPos();
    unsigned int getRevPos();
    PrimerDirection getForDir();
    PrimerDirection getRevDir();
    int getStrand();
    char *getSeqName();
    
  private:
    static const int MAX_SEQ_NAME = 256;
    char seqName[MAX_SEQ_NAME];
    unsigned int forPos;
    unsigned int revPos;
    PrimerDirection forDir;
    PrimerDirection revDir;
    int strand;
    int forScore;
    int revScore;
   
};

#endif // PRIMINGSITE_H

