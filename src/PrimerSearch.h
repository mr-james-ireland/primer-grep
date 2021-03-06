#ifndef PRIMERSEARCH5_H__
#define PRIMERSEARCH5_H__

#include <iostream>
#include <algorithm>
#include <string>
#include "Properties.h"
#include "PrimerPair.h"
#include "PrimingSite.h"
#include "WeightMatrix.h"

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
// PrimerSearch                                                              //
// - The main workhorse which searches with one primer pair against a seq db //                                                  
///////////////////////////////////////////////////////////////////////////////  

class PrimerSearch {
  public:

    PrimerSearch(PrimerPair *pp, WeightMatrix *wm, bool cNoRevC, bool noSelfSelf, double minF, double maxF, int minA, int maxA, int ckmm);
    static char* bits2chars(unsigned int pat);
    unsigned long char2bits(char *pattern);
    void reset(char* cSeqName);
    void setupMismatchVectors();
    bool findPattern(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits);
    int getMatchCount();
    int getNoRevC();
    int getWeight(PrimerDirection primer, int p5, int p3, char b1, char b2);
    
    PrimingSite *getPrimingSite(int mc);             
    PrimerPair* getPrimerPair();
    
    static std::string reverseComplement(std::string primer);
    static std::string complement(std::string);
    static char complement(char base);
    
    static char* convertToSequence(char *cBuffer, int sBufferSize, int start, int size);
    
  private:
    void createPatternArray(PrimerDirection primer1, PrimerDirection primer2, PrimerEnd dir, bool isComp1, bool isComp2, std::string pattern1, std::string pattern2, unsigned long long patternArray[][NFOURMERS]);
    unsigned int countMismatches(PrimerDirection primer, PrimerEnd dir, bool isComp, std::string pattern, char* fourmer, int start, int stop); 
  
    PrimerPair *primerPair;
    WeightMatrix *weights;
    unsigned long forMatches[MATCH_BUFFER]; // P64 - Remove strand
    PrimerDirection forMatchDir[MATCH_BUFFER]; // was 5' match the FOR or REV primer?
    PrimingSite *matchLocations[MAX_MATCHES]; 
    int rightIndex; // P64 - remove strand
    int matchCount; // number of matches so far
    int noRevC;
    bool noSelfSelf;
   
    unsigned long long int forR[MAX_BITS]; //primer pair, state, mm // P64 - Remove strand
    unsigned long long int revR[MAX_BITS];
    unsigned long long int forHitBuffer; // P64 - Remove strand
    unsigned long long int revHitBuffer;
    unsigned long long int mismBits[MAX_BITS];
    unsigned int forPrimerLength;
    unsigned int revPrimerLength;

    // 64-bit mask for seq shifts
    static const unsigned long long int CLR_MASK = ~(15LLU << 32);

    // amplicon characteristics
    unsigned int minAmpSize;
    unsigned int maxAmpSize;
     
    // number mismatches allowed
    unsigned int kmm; 
    
    // number of bits used for the search
    unsigned int kbits;
     
    // current seq being searched
    char seqName[MAX_SEQNAME_SIZE];     
  
    // pattern arrays
    unsigned long long int forParray[MAX_BITS][NFOURMERS];
    unsigned long long int revParray[MAX_BITS][NFOURMERS];
    
    inline void forMatch(unsigned int position);
    inline bool forOneMatch(unsigned int position, int maxHits);
    inline bool revMatch(unsigned int position, int maxHits);
    inline void addPrimingSite(unsigned int forPos, unsigned int revPos, PrimerDirection forDir, PrimerDirection revDir);
    inline int getBasecode(char base);
    
    // pattern finding for different k
    bool findPattern0(char* buffer, int bufferStart, int bufferEnd,
                      unsigned int startPosition, int maxHits);
    bool findOnePattern0(char* buffer, int bufferStart, int bufferEnd,
                         unsigned int startPosition, int maxHits);
    bool findPattern1(char* buffer, int bufferStart, int bufferEnd,
                      unsigned int startPosition, int maxHits);
    bool findOnePattern1(char* buffer, int bufferStart, int bufferEnd,
                         unsigned int startPosition, int maxHits);
    bool findPattern2(char* buffer, int bufferStart, int bufferEnd,
                      unsigned int startPosition, int maxHits);
    bool findOnePattern2(char* buffer, int bufferStart, int bufferEnd,
                         unsigned int startPosition, int maxHits);
    bool findPattern3(char* buffer, int bufferStart, int bufferEnd,
                      unsigned int startPosition, int maxHits);
    /* bool findPattern4(char* buffer, int bufferStart, int bufferEnd,
                      unsigned int startPosition, int maxHits);*/
    
    
};

#endif // PRIMERSEARCH5_H__ 
