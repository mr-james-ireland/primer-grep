
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "WeightMatrix.h"
//#include "Properties.h"

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
// WeightMatrix (constructor)                                                //
//  inputs: none                                                             //
//    desc: creates the default weight matrix where matches are given a      //
//          penalty of 0 and mismatches are given a penalty of 1             //
//          Weights are recorded relative to 5' and relative to 3' end and   //
//          the final weight is the sum of the 3' and 5' weights             //
///////////////////////////////////////////////////////////////////////////////  

WeightMatrix::WeightMatrix() {
  for (int pr=0; pr < NDIRS; ++pr) {
	for (int i=0; i < WT_MAX_PRIMER_SZ; ++i) {
      for (int p=0; p < NBASES; ++p) {
        for (int t=0; t < NBASES; ++t) {
          if (p == t) {
            weight5[pr][i][p][t] = 0;
          } else {
            weight5[pr][i][p][t] = 1;
          }
          weight3[pr][i][p][t] = 0;
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// clearMatrix                                                               //
//  inputs: none                                                             //
//  output: none                                                             //
//    desc: sets the weight matrix to 0                                      //
///////////////////////////////////////////////////////////////////////////////  

void WeightMatrix::clearMatrix() {
  for (int pr=0; pr < NDIRS; ++pr) {
	for (int i=0; i < WT_MAX_PRIMER_SZ; ++i) {
      for (int p=0; p < NBASES; ++p) {
        for (int t=0; t < NBASES; ++t) {
          weight3[pr][i][p][t] = 0;
          weight5[pr][i][p][t] = 0;
		}
      }
    }
  } 
}


///////////////////////////////////////////////////////////////////////////////
// loadWeightsFile                                                           //
//  inputs: weights filename                                                 //
//  output: none                                                             //
//    desc: reads in a tab-delimited weight matrix file.  Format of the      //
//          file is:                                                         //
// header line                                                               //
// primer(l,r)<tab>position ref(5,3)<tab>start pos<tab>stop pos<tab>         //
// weight(integer)<tab>p base<tab>t base                                     //
///////////////////////////////////////////////////////////////////////////////
  
void WeightMatrix::loadWeightsFile(std::string filename) {
  std::string line;

  clearMatrix();

  std::ifstream weightsFile(filename.c_str());
  if (!weightsFile.is_open()) { 
    std::cerr << "Error opening weight matrix file: " << filename << "\n";
    exit(1);
  }
  
  // first line is header
  std::getline(weightsFile, line);
  
  while (!weightsFile.eof()) {
    // read in line of weights file
    std::getline(weightsFile, line);
    
    // adjust weights  
    updateWeights(line);
  }
}  

///////////////////////////////////////////////////////////////////////////////
// updateWeights                                                             //
//  inputs: line from the weights file <string>                              //
//  output: none                                                             //
//    desc: parses a line from the weight matrix file and updates the weight //
//          matrix                                                           //
//          NOTE: ill-formed lines are currently skipped without warning     //
///////////////////////////////////////////////////////////////////////////////  

void WeightMatrix::updateWeights(std::string line) {
  
  std::vector<std::string> tabLine(WT_VALS_PER_LINE);
  int count = 0;
  unsigned int matchPos;
  unsigned int lastPos = 0;
  while (count < WT_VALS_PER_LINE) {
    matchPos = line.find('\t',lastPos);
    
    if (matchPos == (const unsigned int) std::string::npos) {
      if (count == WT_VALS_PER_LINE - 1) {	
        matchPos = line.length()+1;
      }
      else {
      	return; // ill-formed line
      }
    }
    
    tabLine[count++].assign(line, lastPos, matchPos - lastPos );
    lastPos = matchPos + 1;	
  }
  
  if (count != WT_VALS_PER_LINE || tabLine[0] == "") {
  	return; // ill-formed or blank line
  }
  	
  int pstart = 0;
  int pend = WT_MAX_PRIMER_SZ - 1;
  int startPos; 
  int stopPos;
  if (tabLine[2] == "*") {
  	startPos = pstart;
  	stopPos = pend;
  }
  else {
  	startPos = atoi(tabLine[2].c_str())-1;
  	stopPos = atoi(tabLine[3].c_str())-1;
  }
  
  if (startPos < pstart || startPos > pend || stopPos < pstart || stopPos > pend ||
      stopPos < startPos) {
    std::cerr << "ERROR:  Position must be between " << (pstart+1) << " and " << (pend+1) <<
                   " in weights file and start <= stop position.\nLine: " << line << "\n";
    exit(1);             
  }
  else {
    pstart = startPos;
    pend = stopPos;
  }
  
  int weight = atoi(tabLine[4].c_str());
  if (weight < 0) {
    std::cerr << "ERROR:  Weight must be a positive integer.  Line:\n" << line << "\n";
    exit(1);
  }
  for (int i=pstart; i <= pend; i++) {
    if (tabLine[1] == "5") {
      if (tabLine[0] == "F" || tabLine[0] == "f" ||
        tabLine[0] == "L" || tabLine[0] == "l" || tabLine[0] == "*") {
        updateWeightPos(i, weight5, FOR, weight, tabLine[5][0], tabLine[6][0]); 
	  } 
      if (tabLine[0] == "R" || tabLine[0] == "r" || tabLine[0] == "*") {
        updateWeightPos(i, weight5, REV, weight, tabLine[5][0], tabLine[6][0]); 
	  } 	  
    } 
    else {
      if (tabLine[0] == "F" || tabLine[0] == "f" ||
        tabLine[0] == "L" || tabLine[0] == "l" || tabLine[0] == "*") {
        updateWeightPos(i, weight3, FOR, weight, tabLine[5][0], tabLine[6][0]); 
	  } 
      if (tabLine[0] == "R" || tabLine[0] == "r" || tabLine[0] == "*") {
        updateWeightPos(i, weight3, REV, weight, tabLine[5][0], tabLine[6][0]); 
	  } 	  
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// iupac2bits                                                                //
//  inputs: base <char>                                                      //
//  output: 4-bit rep of the base <unsigned int>                             //
//    desc: converts an iupac code into a 4-bit representation.  Multiple    //
//          bits may be set to represent ambiguity                           //
///////////////////////////////////////////////////////////////////////////////  

unsigned int WeightMatrix::iupac2bits(char base) {
  unsigned int bits = 0; 
  switch (base) {
    case 'A':
      bits = 1;
      break;
    case 'C':
      bits = 2;
      break;
    case 'G':
      bits = 4;
      break;
    case 'T': case 'U':
      bits = 8;
      break;
    case 'R':
      bits = 5;
      break;
    case 'Y':
      bits = 10;
      break;
    case 'S':
      bits = 6;
      break;
    case 'W':
      bits = 9;
      break;
    case 'K':
      bits = 12;
      break;
    case 'M':
      bits = 3;
      break;
    case 'B':
      bits = 14;
      break;
    case 'D':
      bits = 13;
      break;
    case 'H':
      bits = 11;
      break;
    case 'V':
      bits = 7;
      break;
    case 'N': case 'X': case '!': case '=':
      bits = 15;
      break;
  }
  return bits;
}                           

///////////////////////////////////////////////////////////////////////////////
// isPatternMatch                                                            //
//  inputs: proposed primer base <uns int>, proposed template base <uns int>,//
//          pattern primer base <char>, pattern template base <char>         //
//  output: true if pattern matches proposed pairing                         //
//    desc: this method determines whether a proposed primer-template base   //
//          pairing matches one of the patterns of the weighting matrix.     //
//          The weighting matrix uses IUPAC codes to describe bases and      //
//          ambiguities.  It also uses '*' = any base (like 'N'), '!'        //
//          means different base (e.g. template different base than primer   //
//          base and '=' means same base.                                    //
///////////////////////////////////////////////////////////////////////////////  

bool WeightMatrix::isPatternMatch(unsigned int pl, unsigned int tl, char pbase, char tbase) {
  unsigned int pmatch = iupac2bits(pbase) & pl;
  unsigned int tmatch = iupac2bits(tbase) & tl;
  if (pbase == '!' || tbase == '!') {
    if (pl != tl && pmatch && tmatch) {
      return true;
    }
    return false;
  }
  if (pbase == '=' || tbase == '=') {
    if (pl == tl && pmatch && tmatch) {
      return true;
    }
    return false;
  } 
  return (pmatch && tmatch);
}

///////////////////////////////////////////////////////////////////////////////
// updateWeightPos                                                           //
//  inputs: relative position in primer <int>, 5' or 3' wt matrix <int>,     //
//          for/rev primer (0/1) <int>, weight <int>, primer base patt       //
//          <char>, template base patt <char>                                //
//  output: none                                                             //
//    desc: given a position and pattern, adjusts the weights of the 3' or   //
//          5' weight matrix                                                 //                                                   
///////////////////////////////////////////////////////////////////////////////  

void WeightMatrix::updateWeightPos(int pos, int weightMatrix[][WT_MAX_PRIMER_SZ][NBASES][NBASES], PrimerDirection primer, int wt, char pbase, char tbase) {
  for (int i=0; i < NBASES; ++i) {
    unsigned int pl = 1 << i;
    for (int j=0; j < NBASES; ++j) {
      unsigned int tl = 1 << j;
      if (isPatternMatch(pl, tl, pbase, tbase)) {
        weightMatrix[primer][pos][i][j] += wt;
      }
    }
  }
}  
  
///////////////////////////////////////////////////////////////////////////////
// getBasecode                                                               //
//  inputs: base <char>                                                      //
//  output: int rep of base <int>                                                              //
//    desc: converts base to int for matrix lookup                           //                                                   
///////////////////////////////////////////////////////////////////////////////  
  
inline int WeightMatrix::getBasecode(char base) {
  switch (base) {
    case 'A': 
      return 0;
    case 'C':
      return 1;
    case 'G':
      return 2;
    case 'T':
      return 3;
  };   
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// getWeight                                                                 //
//  inputs: primer F/R (0,1) <int>, positition rel 5' <int>, position rel    //
//          3' <int>, template base <char>, primer base <char>               //
//  output: weight <int>                                                     //
//    desc: computes the weight at a given pos for given pairing             //                                                   
///////////////////////////////////////////////////////////////////////////////  

int WeightMatrix::getWeight(PrimerDirection primer, int p5, int p3, char b1, char b2) {
  int bi1 = getBasecode(b1);
  int bi2 = getBasecode(b2);
  if (b1 >= 0 && b2 >= 0) {
    return weight5[primer][p5][bi1][bi2] + weight3[primer][p3][bi1][bi2];
  }
  return -1;
} 

///////////////////////////////////////////////////////////////////////////////
// toString                                                                  //
//  inputs: none                                                             //
//  output: none                                                             //
//    desc: displays the contents of the weight matrix (for debugging)       //                                                   
///////////////////////////////////////////////////////////////////////////////  

void WeightMatrix::toString() {
  std::cout << "3' Weight Matrix\n";
  for (int pr=0; pr < NDIRS; ++pr) {
	std::cout << ((pr ==0) ? "For Primer:\n" : "Rev Primer:\n");
    for (int i=0; i < WT_MAX_PRIMER_SZ; ++i) {
      std::cout << "Pos: " << i << " : ";
      for (int j=0; j < NBASES; ++j) {
        for (int k=0; k < NBASES; ++k) {
          std::cout << weight3[pr][i][j][k] << ", ";
        }
      }
      std::cout << "\n";
    }
  }
  std::cout << "\n";
  std::cout << "5' Weight Matrix\n";
  for (int pr=0; pr < NDIRS; ++pr) {
	std::cout << ((pr ==0) ? "For Primer:\n" : "Rev Primer:\n");
    for (int i=0; i < WT_MAX_PRIMER_SZ; ++i) {
      std::cout << "Pos: " << i << " : ";
      for (int j=0; j < NBASES; ++j) {
        for (int k=0; k < NBASES; ++k) {
          std::cout << weight5[pr][i][j][k] << ", ";
        }
      }
      std::cout << "\n";
    }
  }
}
