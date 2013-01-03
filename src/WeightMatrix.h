#ifndef WEIGHTMATRIX_H__
#define WEIGHTMATRIX_H__

#include <string>
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
// WeightMatrix                                                              //
// - Represents a matrix of weights for position-specific, primer-template   //
// pairings                                                                  //                                               
///////////////////////////////////////////////////////////////////////////////  

class WeightMatrix {
  public:
    WeightMatrix();
    void loadWeightsFile(std::string filename);
    int getWeight(PrimerDirection primer, int p5, int p3, char b1, char b2);
    void toString();
    
  private:
    
    static const int WT_VALS_PER_LINE = 7;
    static const int WT_MAX_PRIMER_SZ = 32; 
    
    int weight5[NDIRS][WT_MAX_PRIMER_SZ][NBASES][NBASES];
    int weight3[NDIRS][WT_MAX_PRIMER_SZ][NBASES][NBASES];
        
    inline int getBasecode(char base);
    void clearMatrix();
    void updateMatrix(); 
    void updateWeightPos(int pos, int weightMatrix[][WT_MAX_PRIMER_SZ][NBASES][NBASES], PrimerDirection primer, int wt, char pbase, char tbase);
    bool isPatternMatch(unsigned int pl, unsigned int tl, char pbase, char tbase);
    unsigned int iupac2bits(char base);
    void updateWeights(std::string line);
};

#endif
