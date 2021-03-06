#include "PrimerSearch.h"
//#include "Properties.h"
#include <string>
#include <string.h>

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
// PrimerSearch (constructor)                                                //
//  inputs: primer pair <PrimerPair*>, weight matrix <WeightMatrix*>,        //
//          don't rev comp flag <int>, min amplicon length factor <double>,  //
//          max amplicon length factor <double>, min amplicon size <int>,    //
//          max amplicon size <int>, max allowed mismatch score <int>        //
//    desc: initializes a primer search for a given primer pair              //                                                   
///////////////////////////////////////////////////////////////////////////////  

PrimerSearch::PrimerSearch(PrimerPair *pp, WeightMatrix *wm, bool cNoRevC, bool cNoSelfSelf, double minF, double maxF, int minA, int maxA, int ckmm) {
  primerPair = pp;
  noRevC = cNoRevC;
  noSelfSelf = cNoSelfSelf;
  kmm = ckmm;
  weights = wm;
  
  // select number of bits needed for this level of mismatches
  if (kmm == 0) {
    kbits = 0;
  } 
  else {
    if (kmm == 1) {
      kbits = 1;
    }
    else {
      if (kmm < 4) {
        kbits = 2;
      }
      else {
        if (kmm < 8) {
          kbits = 3;
        }
        else {
          kbits = 4;
        }
      }  
    }
  }     
  
  // set up mismatch bit vectors
  setupMismatchVectors();

  // record lengths to speed up search
  forPrimerLength = pp->getForPrimer().length();
  revPrimerLength = pp->getRevPrimer().length();
  
  // now create pattern arrays
  if (revPrimerLength > 0) {
    if (noRevC) {
	  createPatternArray(FOR, REV, P5, false, true, pp->getForPrimer(), reverseComplement(pp->getRevPrimer()), forParray);
      createPatternArray(FOR, REV, P3, true, false, pp->getRevPrimer(), reverseComplement(pp->getForPrimer()), revParray);
    }
    else {
	  createPatternArray(FOR, REV, P5, false, false, pp->getForPrimer(), pp->getRevPrimer(),forParray);
	  createPatternArray(FOR, REV, P3, true, true, reverseComplement(pp->getRevPrimer()), reverseComplement(pp->getForPrimer()), revParray);
    }

    // set up "hit buffers"...
    // these are the bits to watch to see if we have a hit
    forHitBuffer = 15LLU << (forPrimerLength-1);
    forHitBuffer |= 15LLU << (revPrimerLength-1 + 32);
    revHitBuffer = 15LLU << (revPrimerLength-1);
    revHitBuffer |= 15LLU << (forPrimerLength-1 + 32);
  }
  else {
	createPatternArray(FOR, REV, P5, false, false, pp->getForPrimer(), reverseComplement(pp->getForPrimer()),forParray);

	// set up "hit buffers"...
	// these are the bits to watch to see if we have a hit
	forHitBuffer = 15LLU << (forPrimerLength-1);
	forHitBuffer |= 15LLU << (forPrimerLength-1 + 32);
  }
  
 
  // set up min amp size (don't let it go below size of max primer length)
  if (minA > 0) {
    minAmpSize = minA;
  }
  else {
    minAmpSize = int(pp->getAmpSize()/minF);
  }
  if (minAmpSize < forPrimerLength) {
    minAmpSize = forPrimerLength;
  } 
  if (minAmpSize < revPrimerLength) {
    minAmpSize = revPrimerLength;
  } 
  if (maxA > 0) {
    maxAmpSize = maxA;
  }
  else {
    maxAmpSize = int(pp->getAmpSize()*maxF);
  }

  // init match count
  matchCount = 0;
}

///////////////////////////////////////////////////////////////////////////////
// reset                                                                     //
//  inputs: seq names <char*>                                                //
//  output: none                                                             //
//    desc: records the sequence name and resets the search                  //                                                   
///////////////////////////////////////////////////////////////////////////////  

void PrimerSearch::reset(char* cSeqName) {
  
  // record the name of the seq we are searching

  strcpy(seqName,cSeqName);
  
  rightIndex = 0;
  for (unsigned int j=0; j <= kbits; j++) {
    forR[j] = ~((unsigned long long int) 0);
    revR[j] = ~((unsigned long long int) 0);
  }

}

///////////////////////////////////////////////////////////////////////////////
// accessors                                                                 //
///////////////////////////////////////////////////////////////////////////////  

int PrimerSearch::getMatchCount() {
  return matchCount;
}

PrimingSite *PrimerSearch::getPrimingSite(int mc) {
  return matchLocations[mc];
}            

PrimerPair *PrimerSearch::getPrimerPair() {
  return primerPair;
}

int PrimerSearch::getNoRevC() {
  return noRevC;
}

///////////////////////////////////////////////////////////////////////////////
// complement                                                                //
//  inputs: base (char)                                                      //
//  output: complemented base (char)                                         //
//    desc: complements a base                                               //                                                   
///////////////////////////////////////////////////////////////////////////////  

char PrimerSearch::complement(char base) {
  char cbase;
  
  switch (base) {
    case 'A' : 
      cbase = 'T';
      break;
    case 'C' :
      cbase = 'G';
      break;
    case 'G' :
      cbase = 'C';
      break;
    case 'T' :
      cbase = 'A';
      break;
    default:
      // do nothing to strange bases
      cbase = base;
      break;  
  }
    
  return cbase;
}

///////////////////////////////////////////////////////////////////////////////
// complement                                                                //
//  inputs: primer seq <std::string>                                         //
//  output: complemented primer seq <std::string>                            //
//    desc: complements an entire primer                                     //                                                   
///////////////////////////////////////////////////////////////////////////////  

std::string PrimerSearch::complement(std::string primer) {
  int plen = primer.length();
  std::string compPrimer = primer;
  for (int i=0; i < plen; i++) {
    compPrimer[i] = complement(primer[i]);
  }
  return compPrimer;
} 

///////////////////////////////////////////////////////////////////////////////
// reverseComplement                                                         //
//  inputs: primer seq <std::string>                                         //
//  output: rev complemented primer seq <std::string>                        //
//    desc: reverse complements an entire primer                             //                                                   
///////////////////////////////////////////////////////////////////////////////  

std::string PrimerSearch::reverseComplement(std::string primer) {
  std::string compPrimer = complement(primer);
  std::reverse(compPrimer.begin(), compPrimer.end());
  return compPrimer;
}

///////////////////////////////////////////////////////////////////////////////
// covertToSequence                                                          //
//  inputs: seq buffer <char*>, buffer size <int>, buffer loc start <int>    //
//          size of seq to return <int>                                      //
//  output: sequence <char*>                                                 //
//    desc: decodes a portion of the sequence buffer                         //                                                   
///////////////////////////////////////////////////////////////////////////////  

char* PrimerSearch::convertToSequence(char *cBuffer, int sBufferSize, int bStart, int size) {  
  
  // create char array to hold results
  char* sequence = new char[size*4+1];
  
  // copy seq data into our sequence being careful to avoid buffer boundaries
  char* fourmer;  
  for (int i=0; i < size; i++) {
    int bufPos = (bStart + i) % sBufferSize;
    if (bufPos <  0) {
      bufPos += sBufferSize;
    }
    fourmer = bits2chars((unsigned int) cBuffer[bufPos]);
    for (int k=0; k < 4; k++) {
      sequence[i*4+k] = fourmer[k];
    }
  }
  sequence[size*4] = '\0';
  
  return sequence;
}

///////////////////////////////////////////////////////////////////////////////
// setupMismatchVectors                                                      //
//  inputs: none                                                             //
//  output: none                                                             //
//    desc: creates mismatch vectors for the search alg                      //                                                   
///////////////////////////////////////////////////////////////////////////////  

void PrimerSearch::setupMismatchVectors() {
  // creates vectors of 1's and 0's depending on k
  unsigned int k = kmm;
  for (int i=0; i < MAX_BITS; i++) {
    if (k & 1 == 1) {
      mismBits[i] = ~((unsigned long long int) 0LLU);
    } 
    else {
      mismBits[i] = 0LLU;
    }
    k >>= 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
// getWeight                                                                 //
//  inputs: primer - F/R (0,1) <int>, location rel to 5' <int>, location     //
//          rel to 3' <int>, template base <char>, primer base <char>        // 
//  output: weight <int>                                                     //
//    desc: returns the weight for a match/mismatch at a given location      //                                                   
///////////////////////////////////////////////////////////////////////////////  

int PrimerSearch::getWeight(PrimerDirection primer, int p5, int p3, char b1, char b2) {
  return weights->getWeight(primer, p5, p3, b1, b2);
} 

///////////////////////////////////////////////////////////////////////////////
// getBasecode                                                               //
//  inputs: base <char>                                                      // 
//  output: 2-bit encoded base <int>                                         //
//    desc: returns the weight for a match/mismatch at a given location      //                                                   
///////////////////////////////////////////////////////////////////////////////  

inline int PrimerSearch::getBasecode(char base) {
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
// countMismatches                                                           //
//  inputs: primer F/R (0,1) <int>, 5' or 3' dir <int>, comp flag <int>,     //
//          primer seq <string>, template four-mer <char>, start <int>,      //
//          stop <int>                                                       // 
//  output: mismatch penalty score <int>                                     //
//    desc: given a primer sequence and a possible template 4-mer (4 bases)  //
//          returns the weighted mismatch penalty score                      //                                                   
///////////////////////////////////////////////////////////////////////////////  

unsigned int PrimerSearch::countMismatches(PrimerDirection primer, PrimerEnd dir, bool isComp, std::string pattern, char* fourmer, int start, int stop) { 
  int patLength = pattern.length();
  int mmc = 0;
  
  for (int i=start; i<=stop; ++i) {
    if ((i >= 0) && (i < patLength)) {
      char pbase = pattern[i];
	  char fbase = fourmer[i-start];
	  if (isComp) {
        pbase = complement(pbase);
		fbase = complement(fbase);
	  }
      int wt = weights->getWeight(primer, ((patLength - 1) - i)*dir + (1-dir)*i,
                                  ((patLength - 1) - i)*(1-dir) + dir*i,
                                  pbase, fbase); 
      mmc += wt;
    }
  }
  
  return mmc;
}

///////////////////////////////////////////////////////////////////////////////
// bits2chars                                                                //
//  inputs: bit encrypted 4-mer <unsigned int>                               //
//  output: 4-mer <char*>                                                    //
//    desc: converts back a bit encoded 4-mer into 4 bases                   //                                                 
///////////////////////////////////////////////////////////////////////////////  

char* PrimerSearch::bits2chars(unsigned int pat) {
  char *fourmer = new char[4];
  
  //for (int i=strlen(pattern)-1; i >= 0; i--) {
  for (int i=3; i >= 0; --i) {
    unsigned int base = pat & 3; 
    switch (base) {
      case 0: 
        fourmer[i] = 'A';
        break;
      case 1:
        fourmer[i] = 'C';
        break;
      case 2:
        fourmer[i] = 'G';
        break;
      case 3:
        fourmer[i] = 'T';
        break;
    };
    pat >>= 2;
  }
  return fourmer;
}

///////////////////////////////////////////////////////////////////////////////
// createPatternArray                                                        //
//  inputs: primer direction <PrimerDirection>, primer end (5' or 3')        //
//          <PrimerEnd>, is complented flag <bool>, primer seq <string>,     //
//          pattern array <long[][]>                                         //
//  output: none                                                             //
//    desc: sets up a pattern array for every possible 4-mer and one of our  //
//          primers                                                          //                                                 
///////////////////////////////////////////////////////////////////////////////  

void PrimerSearch::createPatternArray(PrimerDirection primer1, PrimerDirection primer2, PrimerEnd dir, bool isComp1, bool isComp2, std::string pattern1, std::string pattern2, unsigned long long patternArray[][NFOURMERS]) {
     
  // do primers in reverse order since bits get shifted left
  // primer2
  int patLength = pattern2.length();
  
  for (unsigned long i=0; i < NFOURMERS; i++) {
    
    char* fourmer = bits2chars(i);
    
    // clear pattern bits
    for (int k=0; k < MAX_BITS; k++) {
      patternArray[k][i] = 0LLU;
    }
    
    for (int j = patLength+2; j >= 0; --j) {
      unsigned int mmc = countMismatches(primer2, dir, isComp2, pattern2, fourmer, j-3, j);
     
      for (int nmm = 0; nmm < MAX_BITS; ++nmm) {
        patternArray[nmm][i] <<= 1;
        if (mmc & 1 || (mmc > kmm)) {
          patternArray[nmm][i] |= 1LLU;
        }
        mmc >>= 1;
      }
    }
  }

  // space to 32 bits
  patLength = pattern1.length();
  for (unsigned long i=0; i < NFOURMERS; i++) {
	for (int j = 32 - (patLength+4); j >= 0; --j) {
      for (int nmm = 0; nmm < MAX_BITS; ++nmm) {
		patternArray[nmm][i] <<= 1;
      }
	}
  }

  // primer1

  for (unsigned long i=0; i < NFOURMERS; i++) {

    char* fourmer = bits2chars(i);

    for (int j = patLength+2; j >= 0; --j) {
      unsigned int mmc = countMismatches(primer1, dir, isComp1, pattern1, fourmer, j-3, j);

      for (int nmm = 0; nmm < MAX_BITS; ++nmm) {
        patternArray[nmm][i] <<= 1;
        if (mmc & 1 || (mmc > kmm)) {
          patternArray[nmm][i] |= 1LLU;
        }
        mmc >>= 1;
      }
    }
  }

}

///////////////////////////////////////////////////////////////////////////////
// forMatch                                                                  //
//  inputs: position of for match <int>                                      //
//  output: none                                                             //
//    desc: records a forward match by adding position to stack              //
///////////////////////////////////////////////////////////////////////////////  

inline void PrimerSearch::forMatch(unsigned int position) {

  unsigned long long int offbit =   (forHitBuffer & (
         ((~forR[4] & mismBits[4]) |
         ((~forR[4] | mismBits[4]) & (~forR[3] & mismBits[3])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] & mismBits[2])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] | mismBits[2]) & (~forR[1] & mismBits[1])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] | mismBits[2]) & (~forR[1] | mismBits[1])) & (~forR[0] | mismBits[0]))));
  unsigned long long int offbitPS = offbit >> (forPrimerLength-1);
  unsigned long long int offbitMS = offbit >> (revPrimerLength-1 + 32);

  if (offbitPS & 15) {
    // for hits

	if (offbitPS & 1) {
      forMatches[rightIndex] = position -(forPrimerLength-4)+1;
      forMatchDir[rightIndex] = FOR;
      rightIndex++;
    }

    if (offbitPS & 2) {
      forMatches[rightIndex] = position -(forPrimerLength-4);
      forMatchDir[rightIndex] = FOR;
      rightIndex++;
    }

    if (offbitPS & 4) {
      forMatches[rightIndex] = position -(forPrimerLength-4) - 1;
      forMatchDir[rightIndex] = FOR;
      rightIndex++;
    }
  
    if (offbitPS & 8) {
      forMatches[rightIndex] = position -(forPrimerLength-4) - 2;
      forMatchDir[rightIndex] = FOR;
      rightIndex++;
    }

  }
  if (offbitMS & 15) {
    // rev hits

    if (offbitMS & 1) {
      forMatches[rightIndex] = position -(revPrimerLength-4)+1;
      forMatchDir[rightIndex] = REV;
      rightIndex++;
    }

    if (offbitMS & 2) {
      forMatches[rightIndex] = position -(revPrimerLength-4);
      forMatchDir[rightIndex] = REV;
      rightIndex++;
    }

    if (offbitMS & 4) {
      forMatches[rightIndex] = position -(revPrimerLength-4) - 1;
      forMatchDir[rightIndex] = REV;
      rightIndex++;
    }
  
    if (offbitMS & 8) {
      forMatches[rightIndex] = position -(revPrimerLength-4) - 2;
      forMatchDir[rightIndex] = REV;
      rightIndex++;
    }

  }
  if (rightIndex >= MATCH_BUFFER) {
    std::cerr << "ERROR:  Exceeded max forward matches.  Please reduce allowed number of mismatches or increase weight matrix.\n";
    exit(0);
  } 
} 

///////////////////////////////////////////////////////////////////////////////
// forOneMatch                                                               //
//  inputs: position of for match <int>                                      //
//  output: none                                                             //
//    desc: records a forward match by adding position to stack              //
//          records any match since single primer search                     //
///////////////////////////////////////////////////////////////////////////////

inline bool PrimerSearch::forOneMatch(unsigned int position, int maxHits) {

  unsigned long long int offbit =   (forHitBuffer & (
         ((~forR[4] & mismBits[4]) |
         ((~forR[4] | mismBits[4]) & (~forR[3] & mismBits[3])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] & mismBits[2])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] | mismBits[2]) & (~forR[1] & mismBits[1])) |
         ((~forR[4] | mismBits[4]) & (~forR[3] | mismBits[3]) & (~forR[2] | mismBits[2]) & (~forR[1] | mismBits[1])) & (~forR[0] | mismBits[0]))));
  unsigned long long int offbitPS = offbit >> (forPrimerLength-1);
  unsigned long long int offbitMS = offbit >> (forPrimerLength-1 + 32);

  if (offbitPS & 15) {
    // for hits

	if (offbitPS & 1) {
      addPrimingSite(position -(forPrimerLength-4)+1, position + 4, FOR, REV);
    }

    if (offbitPS & 2) {
      addPrimingSite(position -(forPrimerLength-4), position + 3, FOR, REV);
    }

    if (offbitPS & 4) {
      addPrimingSite(position -(forPrimerLength-4) - 1, position + 2, FOR, REV);
    }

    if (offbitPS & 8) {
      addPrimingSite(position -(forPrimerLength-4) - 2, position + 1, FOR, REV);
    }

  }
  if (offbitMS & 15) {
    // rev hits

	forMatchDir[rightIndex] = REV;

    if (offbitMS & 1) {
      addPrimingSite(position -(forPrimerLength-4) + 1, position + 4, REV, FOR);
    }

    if (offbitMS & 2) {
      addPrimingSite(position -(forPrimerLength-4), position + 3, REV, FOR);
    }

    if (offbitMS & 4) {
      addPrimingSite(position -(forPrimerLength-4) - 1, position + 2, REV, FOR);
    }

    if (offbitMS & 8) {
      addPrimingSite(position -(forPrimerLength-4) - 2, position + 1, REV, FOR);
    }

  }
  if (matchCount > maxHits) {
    return true;
  }
  return false;

}


///////////////////////////////////////////////////////////////////////////////
// addPrimingSite                                                            //
//  inputs: for position <int>, rev position <int>, 5' match primer,         //
//          3' match primer                                                  //
//  output: none                                                             //
//    desc: adds priming site to list while making sure site is unique       //                                                 
///////////////////////////////////////////////////////////////////////////////  

inline void PrimerSearch::addPrimingSite(unsigned int forPos, unsigned int revPos, PrimerDirection forDir, PrimerDirection revDir) {

  // skip if we're not doing F-F or R-R hits and that's what we have
  if (forDir == revDir && noSelfSelf) {
	  return;
  }

  // create a new priming site, ignore scores for now
  // always use Plus Strand unless (Rev, For) match
  DNAStrand strand = PS;
  if (forDir == REV && revDir == FOR) {
	  strand = MS;
  }

  PrimingSite *ps = new PrimingSite(seqName, forPos, revPos, forDir, revDir, strand, 0, 0);
  
  // search all previous sites to make sure we don't already have this
  // - for now, simple loop search will do.  If we plan on keeping many
  //   hits we should move this to a hash
  for (int i=0; i < matchCount; i++) {
    if (ps->compare(matchLocations[i]) == 0) {
      // already seen
      delete ps;
      return; 
    }
  }
  
  // a new one!
  matchLocations[matchCount] = ps;
  matchCount++;
}

///////////////////////////////////////////////////////////////////////////////
// revMatch                                                                  //
//  inputs: position of rev match <int>,                                     //
//          maxHits <int>                                                    //
//  output: true if exceed max hits <bool>                                   //
//    desc: handles a reverse primer match by checking if the amp length is  //
//          ok and tallies the matches                                       //                                                 
///////////////////////////////////////////////////////////////////////////////  

inline bool PrimerSearch::revMatch(unsigned int position, int maxHits) {

  unsigned int matchPosition = position + 3;
  //bool foundGoodAmp = false;
  unsigned int forPos = 0;
  unsigned int firstHit = 0;
  unsigned int lastHit = 0;
    
  for (int i=rightIndex-1; i >= 0; i--) {
    // the constants I +/- here are because we jump 4 bps at a time
    if (matchPosition - 2 - forMatches[i] > maxAmpSize) {
      // any remaining hits will also be too far out
      lastHit = i + 1;
      break;
    }
    if (firstHit == 0 && matchPosition + 2 - forMatches[i] >= minAmpSize) {
      // stop with first good amp
      firstHit = i;
    }
  }
  
  if (lastHit > firstHit) {
    // no amps of correct size
    return false;
  } 
  for (unsigned int i = lastHit; i <= firstHit; ++i) { 
    forPos = forMatches[i];
    PrimerDirection forDir = forMatchDir[i];

    // find match sites
    unsigned long long int offbit =   (revHitBuffer & (
         ((~revR[4] & mismBits[4]) |
         ((~revR[4] | mismBits[4]) & (~revR[3] & mismBits[3])) |
         ((~revR[4] | mismBits[4]) & (~revR[3] | mismBits[3]) & (~revR[2] & mismBits[2])) |
         ((~revR[4] | mismBits[4]) & (~revR[3] | mismBits[3]) & (~revR[2] | mismBits[2]) & (~revR[1] & mismBits[1])) |
         ((~revR[4] | mismBits[4]) & (~revR[3] | mismBits[3]) & (~revR[2] | mismBits[2]) & (~revR[1] | mismBits[1])) & (~revR[0] | mismBits[0]))));

    unsigned long long int offbitPS = offbit >> (revPrimerLength-1);
    unsigned long long int offbitMS = offbit >> (forPrimerLength-1 + 32);

    if (offbitPS & 15) {
      // REV hits

	  if (offbitPS & 1 && (matchPosition + 2 - forPos <= maxAmpSize && matchPosition + 2 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition +1) << " - " << (matchPosition + 2 - forPos) << " - " << minAmpSize << "\n";
        addPrimingSite(forPos,(matchPosition + 1), forDir, REV);
      }
  
      if (offbitPS & 2 && (matchPosition + 1 - forPos <= maxAmpSize && matchPosition + 1 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition) << " - " << (matchPosition + 1 - forPos) << " - " << minAmpSize<<"\n";
        addPrimingSite(forPos,(matchPosition), forDir, REV);
      }
    
      if (offbitPS & 4 && (matchPosition - forPos <= maxAmpSize && matchPosition - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition - 1) << " - " << (matchPosition - forPos) << " - " << minAmpSize<<"\n";
        addPrimingSite(forPos,(matchPosition - 1), forDir, REV);
      }
  
      if (offbitPS & 8 && (matchPosition - 1 - forPos <= maxAmpSize && matchPosition - 1 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition - 2) << " - " << (matchPosition -1 - forPos) << " - " << minAmpSize<< "\n";
        addPrimingSite(forPos,(matchPosition - 2), forDir, REV);
      }
    }

    if (offbitMS & 15) {
      // FOR hits

	  if (offbitMS & 1 && (matchPosition + 2 - forPos <= maxAmpSize && matchPosition + 2 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition +1) << " - " << (matchPosition + 2 - forPos) << " - " << minAmpSize<< "\n";
        addPrimingSite(forPos,(matchPosition + 1), forDir, FOR);
      }

      if (offbitMS & 2 && (matchPosition + 1 - forPos <= maxAmpSize && matchPosition + 1 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition) << " - " << (matchPosition + 1 - forPos) << " - " << minAmpSize<< "\n";
        addPrimingSite(forPos,(matchPosition), forDir, FOR);
      }

      if (offbitMS & 4 && (matchPosition - forPos <= maxAmpSize && matchPosition - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition - 1) << " - " << (matchPosition - forPos) << " - " << minAmpSize<<"\n";
        addPrimingSite(forPos,(matchPosition - 1), forDir, FOR);
      }

      if (offbitMS & 8 && (matchPosition - 1 - forPos <= maxAmpSize && matchPosition - 1 - forPos >= minAmpSize)) {
        //std::cerr << "R Hit, PP at: " << seqName << " : " << forPos << " - " << (matchPosition - 2) << " - " << (matchPosition -1 - forPos) << " - " << minAmpSize<<"\n";
        addPrimingSite(forPos,(matchPosition - 2), forDir, FOR);
      }
    }

    if (matchCount > maxHits) {
      return true;
    }
  }
  return false;
} 

///////////////////////////////////////////////////////////////////////////////
// findPattern                                                               //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of maxHits or less and //
//          returns true if max number of hits is exceeded                   //
//          note: for max speed, loops have been unwound and different k vals//
//          are split into separate methods.  It's ugly to look at and harder//
//          to maintain, but runs much faster than leaving in conditionals.  //
//          Searches are done on sense/antisense strands simultaneously.     //
//          Fork if only a one primer search.                                //
///////////////////////////////////////////////////////////////////////////////  

bool PrimerSearch::findPattern(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {
                 	
  if (revPrimerLength > 0) {
	switch (kbits) {
      case 0: return findPattern0(buffer, bufferStart, bufferEnd, startPosition, maxHits);
      case 1: return findPattern1(buffer, bufferStart, bufferEnd, startPosition, maxHits);
      case 2: return findPattern2(buffer, bufferStart, bufferEnd, startPosition, maxHits);
      case 3: return findPattern3(buffer, bufferStart, bufferEnd, startPosition, maxHits);
      //case 4: return findPattern4(buffer, bufferStart, bufferEnd, startPosition, maxHits);
    }
  }
  else {
	switch (kbits) {
	  case 0: return findOnePattern0(buffer, bufferStart, bufferEnd, startPosition, maxHits);
	  case 1: return findOnePattern1(buffer, bufferStart, bufferEnd, startPosition, maxHits);
	  case 2: return findOnePattern2(buffer, bufferStart, bufferEnd, startPosition, maxHits);
	  //case 3: return findPattern3(buffer, bufferStart, bufferEnd, startPosition, maxHits);
	  //case 4: return findPattern4(buffer, bufferStart, bufferEnd, startPosition, maxHits);
	}
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// findPattern0                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 0 or less and       //
//          returns true if max number of hits is exceeded                   //                                                   
///////////////////////////////////////////////////////////////////////////////  

bool PrimerSearch::findPattern0(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {
  
  unsigned char fourmer;

  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];
 
    // shift all vectors // P64 - remove strand
    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40
    forR[0] |= forParray[0][fourmer];
    
 
    // check forward primers for a match
    if (forHitBuffer & ~forR[0]) {
      // have a match, add to stack
      forMatch(startPosition + (i-bufferStart)*4);
    }
    
    // look for open searches
    if (rightIndex > 0) {

      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[rightIndex-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex = 0;
        revR[0] = ~(0LLU);
      }
      else {
        
        // reverse screen
        revR[0] <<= 4;
        revR[0] &= CLR_MASK;
        revR[0] |= revParray[0][fourmer];
                
        // check reverse primers for a match
        if (revHitBuffer & ~revR[0]) {
          
          // have a match, add to matches
          if (revMatch(startPosition + (i-bufferStart)*4, maxHits)) { // P64 - TODO - update function
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex > 0
        

  } // for (int i...
  
  // not enough hits
  return false;
} // findPattern0

///////////////////////////////////////////////////////////////////////////////
// findOnePattern0                                                           //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  //
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 0 or less and       //
//          returns true if max number of hits is exceeded                   //
//          For single primer                                                //
///////////////////////////////////////////////////////////////////////////////

bool PrimerSearch::findOnePattern0(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {

  unsigned char fourmer;

  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];

    // shift all vectors // P64 - remove strand
    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40
    forR[0] |= forParray[0][fourmer];


    // check forward primers for a match
    if (forHitBuffer & ~forR[0]) {
      // have a match, add to stack
      if (forOneMatch(startPosition + (i-bufferStart)*4, maxHits)) {
    	return true; // max hits reached
      }
    }
  }

  // not enough hits
  return false;
} // findOnePattern0


///////////////////////////////////////////////////////////////////////////////
// findPattern1                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 1 or less and       //
//          returns true if max number of hits is exceeded                   //                                                   
///////////////////////////////////////////////////////////////////////////////  

bool PrimerSearch::findPattern1(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {
  
  unsigned char fourmer;
  unsigned long long int carry1;
  
  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];
 
    // shift all vectors 
    forR[1] <<= 4;
    forR[1] &= CLR_MASK; // clear bits 32-40

    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40
    
    carry1  = forR[0] & forParray[0][fourmer];
    forR[0] = forR[0] ^ forParray[0][fourmer];

    forR[1] = carry1 | forR[1] | forParray[1][fourmer];
    
    // check forward primers for a match
    if (!(!(forHitBuffer & (~forR[1] | mismBits[1])) ||
          !(forHitBuffer & (~forR[1] | mismBits[1]) &
                              (~forR[0] | mismBits[0] | ~forR[1] & mismBits[1]))
                              )) {
      
      // have a match, add to stack
      forMatch(startPosition + (i-bufferStart)*4);
      
    }
    
    // look for open searches
    if (rightIndex > 0) {

      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[rightIndex-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex = 0;
        revR[0] = ~(0LLU);
        revR[1] = ~(0LLU);
       
      }
      else {
        
        // reverse screen
        revR[1] <<= 4;
        revR[1] &= CLR_MASK;
        revR[0] <<= 4;
        revR[0] &= CLR_MASK;
    
        carry1  = revR[0] & revParray[0][fourmer];
        revR[0] = revR[0] ^ revParray[0][fourmer];
    
        revR[1] = carry1 | revR[1] | revParray[1][fourmer];
            
        // check reverse primers for a match
        if (!(!(revHitBuffer & (~revR[1] | mismBits[1])) ||
          !(revHitBuffer & (~revR[1] | mismBits[1]) &
                              (~revR[0] | mismBits[0] | ~revR[1] & mismBits[1]))
                              )) {
        
          // have a match, add to matches
          if (revMatch(startPosition + (i-bufferStart)*4, maxHits)) {
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex > 0

  } // for (int i...
  
  // not enough hits
  return false;
} // findPattern1


///////////////////////////////////////////////////////////////////////////////
// findOnePattern1                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  //
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 1 or less and       //
//          returns true if max number of hits is exceeded                   //
///////////////////////////////////////////////////////////////////////////////

bool PrimerSearch::findOnePattern1(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {

  unsigned char fourmer;
  unsigned long long int carry1;

  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];

    // shift all vectors
    forR[1] <<= 4;
    forR[1] &= CLR_MASK; // clear bits 32-40

    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40

    carry1  = forR[0] & forParray[0][fourmer];
    forR[0] = forR[0] ^ forParray[0][fourmer];

    forR[1] = carry1 | forR[1] | forParray[1][fourmer];

    // check forward primers for a match
    if (!(!(forHitBuffer & (~forR[1] | mismBits[1])) ||
          !(forHitBuffer & (~forR[1] | mismBits[1]) &
                              (~forR[0] | mismBits[0] | ~forR[1] & mismBits[1]))
                              )) {

      // have a match, add to stack
      if (forOneMatch(startPosition + (i-bufferStart)*4, maxHits)) {
    	  return true; // maxHits reached
      }

    }

  } // for (int i...

  // not enough hits
  return false;
} // findOnePattern1


///////////////////////////////////////////////////////////////////////////////
// findPattern2                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 3 or less and       //
//          returns true if max number of hits is exceeded                   //                                                   
///////////////////////////////////////////////////////////////////////////////  

bool PrimerSearch::findPattern2(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {

  unsigned char fourmer;
  unsigned long long int carry1;
  unsigned long long int carry2;
  
  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];
 
    // shift all vectors 
    forR[2] <<= 4;
    forR[2] &= CLR_MASK; // clear bits 32-40
    forR[1] <<= 4;
    forR[1] &= CLR_MASK; // clear bits 32-40
    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40
    
    carry1     = forR[0] & forParray[0][fourmer];
    forR[0] = forR[0] ^ forParray[0][fourmer];
    
    carry2     = carry1 & forR[1] | carry1 & forParray[1][fourmer] | forR[1] & forParray[1][fourmer];
    forR[1] = carry1 ^ forR[1] ^ forParray[1][fourmer];
    
    forR[2] = carry2 | forR[2] | forParray[2][fourmer];
    
    // check forward primers for a match
    if (!(!(forHitBuffer & (~forR[2] | mismBits[2])) ||
          !(forHitBuffer & (~forR[2] | mismBits[2]) &
                              (~forR[1] | mismBits[1] | ~forR[2] & mismBits[2])) ||
          !(forHitBuffer & (~forR[2] | mismBits[2]) &
                              (~forR[1] | mismBits[1] | ~forR[2] & mismBits[2]) &
                              (~forR[0] | mismBits[0] | ~forR[2] & mismBits[2] | ~forR[1] & mismBits[1]))
                              )) {
      
      // have a match, add to stack
      forMatch(startPosition + (i-bufferStart)*4);
      
    }
    
    // look for open searches
    if (rightIndex > 0) {
      
      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[rightIndex-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex = 0;
        revR[0] = ~(0LLU);
        revR[1] = ~(0LLU);
        revR[2] = ~(0LLU);
        
      }
      else {
        
        // reverse screen
        revR[2] <<= 4;
        revR[2] &= CLR_MASK;

        revR[1] <<= 4;
        revR[1] &= CLR_MASK;

        revR[0] <<= 4;
        revR[0] &= CLR_MASK;

        carry1     = revR[0] & revParray[0][fourmer];
        revR[0] = revR[0] ^ revParray[0][fourmer];
    
        carry2     = carry1 & revR[1] | carry1 & revParray[1][fourmer] | revR[1] & revParray[1][fourmer];
        revR[1] = carry1 ^ revR[1] ^ revParray[1][fourmer];
    
        revR[2] = carry2 | revR[2] | revParray[2][fourmer];

            
        // check reverse primers for a match
        if (!(!(revHitBuffer & (~revR[2] | mismBits[2])) ||
          !(revHitBuffer & (~revR[2] | mismBits[2]) &
                              (~revR[1] | mismBits[1] | ~revR[2] & mismBits[2])) ||
          !(revHitBuffer & (~revR[2] | mismBits[2]) &
                              (~revR[1] | mismBits[1] | ~revR[2] & mismBits[2]) &
                              (~revR[0] | mismBits[0] | ~revR[2] & mismBits[2] | ~revR[1] & mismBits[1]))
                              )) {
        
          // have a match, add to matches
          if (revMatch(startPosition + (i-bufferStart)*4, maxHits)) {
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex[PS] > 0
  }

  // not enough hits
  return false;
} // findPattern2


///////////////////////////////////////////////////////////////////////////////
// findOnePattern2                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  //
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 3 or less and       //
//          returns true if max number of hits is exceeded                   //
///////////////////////////////////////////////////////////////////////////////

bool PrimerSearch::findOnePattern2(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {

  unsigned char fourmer;
  unsigned long long int carry1;
  unsigned long long int carry2;

  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];

    // shift all vectors
    forR[2] <<= 4;
    forR[2] &= CLR_MASK; // clear bits 32-40
    forR[1] <<= 4;
    forR[1] &= CLR_MASK; // clear bits 32-40
    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40

    carry1     = forR[0] & forParray[0][fourmer];
    forR[0] = forR[0] ^ forParray[0][fourmer];

    carry2     = carry1 & forR[1] | carry1 & forParray[1][fourmer] | forR[1] & forParray[1][fourmer];
    forR[1] = carry1 ^ forR[1] ^ forParray[1][fourmer];

    forR[2] = carry2 | forR[2] | forParray[2][fourmer];

    // check forward primers for a match
    if (!(!(forHitBuffer & (~forR[2] | mismBits[2])) ||
          !(forHitBuffer & (~forR[2] | mismBits[2]) &
                              (~forR[1] | mismBits[1] | ~forR[2] & mismBits[2])) ||
          !(forHitBuffer & (~forR[2] | mismBits[2]) &
                              (~forR[1] | mismBits[1] | ~forR[2] & mismBits[2]) &
                              (~forR[0] | mismBits[0] | ~forR[2] & mismBits[2] | ~forR[1] & mismBits[1]))
                              )) {

      // have a match, add to stack
      if (forOneMatch(startPosition + (i-bufferStart)*4, maxHits)) {
    	return true;
      }

    }

  }

  // not enough hits
  return false;
} // findOnePattern2


///////////////////////////////////////////////////////////////////////////////
// findPattern3                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 7 or less and       //
//          returns true if max number of hits is exceeded                   //                                                   
///////////////////////////////////////////////////////////////////////////////  

bool PrimerSearch::findPattern3(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {
  
  unsigned char fourmer;
  unsigned long long int carry1;
  unsigned long long int carry2;
  unsigned long long int carry3;
  
  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];
 
    // shift all vectors 
    forR[3] <<= 4;
    forR[3] &= CLR_MASK; // clear bits 32-40
    forR[2] <<= 4;
    forR[2] &= CLR_MASK; // clear bits 32-40
    forR[1] <<= 4;
    forR[1] &= CLR_MASK; // clear bits 32-40
    forR[0] <<= 4;
    forR[0] &= CLR_MASK; // clear bits 32-40
    
    carry1     = forR[0] & forParray[0][fourmer];
    forR[0] = forR[0] ^ forParray[0][fourmer];
    
    carry2     = carry1 & forR[1] | carry1 & forParray[1][fourmer] | forR[1] & forParray[1][fourmer];
    forR[1] = carry1 ^ forR[1] ^ forParray[1][fourmer];
    
    carry3     = carry2 & forR[2] | carry2 & forParray[2][fourmer] | forR[2] & forParray[2][fourmer];
    forR[2] = carry2 ^ forR[2] ^ forParray[2][fourmer];

    forR[3] = forR[3] | carry3 | forParray[3][fourmer];
    
    // check forward primers for a match
    if (!(!(forHitBuffer & (~forR[3] | mismBits[3])) ||
          !(forHitBuffer & (~forR[3] | mismBits[3]) &
                              (~forR[2] | mismBits[2] | ~forR[3] & mismBits[3])) ||
          !(forHitBuffer & (~forR[3] | mismBits[3]) &
                              (~forR[2] | mismBits[2] | ~forR[3] & mismBits[3]) &
                              (~forR[1] | mismBits[1] | ~forR[3] & mismBits[3] | ~forR[2] & mismBits[2])) ||
          !(forHitBuffer & (~forR[3] | mismBits[3]) &
                              (~forR[2] | mismBits[2] | ~forR[3] & mismBits[3]) &
                              (~forR[1] | mismBits[1] | ~forR[3] & mismBits[3] | ~forR[2] & mismBits[2]) &
                              (~forR[0] | mismBits[0] | ~forR[3] & mismBits[3] | ~forR[2] & mismBits[2] | ~forR[1] & mismBits[1]))
                              
                              )) {
      
      // have a match, add to stack
      forMatch(startPosition + (i-bufferStart)*4);
      
    }
    
    // look for open searches
    if (rightIndex > 0) {
      
      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[rightIndex-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex = 0;
        revR[0] = ~(0LLU);
        revR[1] = ~(0LLU);
        revR[2] = ~(0LLU);
        revR[3] = ~(0LLU);
      }
      else {
        
        // reverse screen
        revR[3] <<= 4;
        revR[3] &= CLR_MASK; // clear bits 32-40
        revR[2] <<= 4;
        revR[2] &= CLR_MASK; // clear bits 32-40
        revR[1] <<= 4;
        revR[1] &= CLR_MASK; // clear bits 32-40
        revR[0] <<= 4;
        revR[0] &= CLR_MASK; // clear bits 32-40

        carry1     = revR[0] & revParray[0][fourmer];
        revR[0] = revR[0] ^ revParray[0][fourmer];
    
        carry2     = carry1 & revR[1] | carry1 & revParray[1][fourmer] | revR[1] & revParray[1][fourmer];
        revR[1] = carry1 ^ revR[1] ^ revParray[1][fourmer];
    
        carry3     = carry2 & revR[2] | carry2 & revParray[2][fourmer] | revR[2] & revParray[2][fourmer];
        revR[2] = carry2 ^ revR[2] ^ revParray[2][fourmer];

        revR[3] = revR[3] | carry3 | revParray[3][fourmer];
        
            
        // check reverse primers for a match
        if (!(!(revHitBuffer & (~revR[3] | mismBits[3])) ||
          !(revHitBuffer & (~revR[3] | mismBits[3]) &
                              (~revR[2] | mismBits[2] | ~revR[3] & mismBits[3])) ||
          !(revHitBuffer & (~revR[3] | mismBits[3]) &
                              (~revR[2] | mismBits[2] | ~revR[3] & mismBits[3]) &
                              (~revR[1] | mismBits[1] | ~revR[3] & mismBits[3] | ~revR[2] & mismBits[2])) ||
          !(revHitBuffer & (~revR[3] | mismBits[3]) &
                              (~revR[2] | mismBits[2] | ~revR[3] & mismBits[3]) &
                              (~revR[1] | mismBits[1] | ~revR[3] & mismBits[3] | ~revR[2] & mismBits[2]) &
                              (~revR[0] | mismBits[0] | ~revR[3] & mismBits[3] | ~revR[2] & mismBits[2] | ~revR[1] & mismBits[1]))
                              
                              )) {
        
          // have a match, add to matches
          if (revMatch(startPosition + (i-bufferStart)*4, maxHits)) {
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex[PS] > 0
      
  } // for (int i...
  
  // not enough hits
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// findPattern4                                                              //
//  inputs: sequence buffer <char*>, buffer start loc <int>, buffer end <int>//
//          overall start loc <int>, max hits allowed <int>                  // 
//  output: true if exceed max hits <bool>                                   //
//    desc: searches seq for hits with mismatch score of 15 or less and      //
//          returns true if max number of hits is exceeded                   //                                                   
///////////////////////////////////////////////////////////////////////////////  
/*
bool PrimerSearch::findPattern4(char* buffer, int bufferStart, int bufferEnd,
                 unsigned int startPosition, int maxHits) {
  
  unsigned char fourmer;
  unsigned int carry1;
  unsigned int carry2;
  unsigned int carry3;
  unsigned int carry4;
  
  for (int i=bufferStart; i < bufferEnd; ++i) {
    fourmer = buffer[i];
 
    // shift all vectors 
    forR[PS][4] <<= 4; 
    forR[PS][3] <<= 4; 
    forR[PS][2] <<= 4; 
    forR[PS][1] <<= 4;
    forR[PS][0] <<= 4;
    
    carry1     = forR[PS][0] & forParray[PS][0][fourmer];
    forR[PS][0] = forR[PS][0] ^ forParray[PS][0][fourmer];
    
    carry2     = carry1 & forR[PS][1] | carry1 & forParray[PS][1][fourmer] | forR[PS][1] & forParray[PS][1][fourmer];
    forR[PS][1] = carry1 ^ forR[PS][1] ^ forParray[PS][1][fourmer];
    
    carry3     = carry2 & forR[PS][2] | carry2 & forParray[PS][2][fourmer] | forR[PS][2] & forParray[PS][2][fourmer];
    forR[PS][2] = carry2 ^ forR[PS][2] ^ forParray[PS][2][fourmer];

    carry4     = carry3 & forR[PS][3] | carry3 & forParray[PS][3][fourmer] | forR[PS][3] & forParray[PS][3][fourmer];
    forR[PS][3] = carry3 ^ forR[PS][3] ^ forParray[PS][3][fourmer];

    forR[PS][4] = forR[PS][4] | carry4 | forParray[PS][4][fourmer];
    
    // check forward primers for a match
    if (!(!(forHitBuffer[PS] & (~forR[PS][4] | mismBits[4])) ||
          !(forHitBuffer[PS] & (~forR[PS][4] | mismBits[4]) & 
                              (~forR[PS][3] | mismBits[3] | ~forR[PS][4] & mismBits[4])) ||
          !(forHitBuffer[PS] & (~forR[PS][4] | mismBits[4]) & 
                              (~forR[PS][3] | mismBits[3] | ~forR[PS][4] & mismBits[4]) &
                              (~forR[PS][2] | mismBits[2] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3])) ||
          !(forHitBuffer[PS] & (~forR[PS][4] | mismBits[4]) & 
                              (~forR[PS][3] | mismBits[3] | ~forR[PS][4] & mismBits[4]) &
                              (~forR[PS][2] | mismBits[2] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3]) &
                              (~forR[PS][1] | mismBits[1] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3] | ~forR[PS][2] & mismBits[2])) ||
          !(forHitBuffer[PS] & (~forR[PS][4] | mismBits[4]) & 
                              (~forR[PS][3] | mismBits[3] | ~forR[PS][4] & mismBits[4]) &
                              (~forR[PS][2] | mismBits[2] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3]) &
                              (~forR[PS][1] | mismBits[1] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3] | ~forR[PS][2] & mismBits[2]) &
                              (~forR[PS][0] | mismBits[0] | ~forR[PS][4] & mismBits[4] | ~forR[PS][3] & mismBits[3] | ~forR[PS][2] & mismBits[2] | ~forR[PS][1] & mismBits[1])) 
                              
                              )) {
      
      // have a match, add to stack
      forMatch(PS, startPosition + (i-bufferStart)*4);
      
    }
    
    // look for open searches
    if (rightIndex[PS] > 0) {
      
      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[PS][rightIndex[PS]-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex[PS] = 0;
        revR[PS][0] = ~((unsigned int) 0);
        revR[PS][1] = ~((unsigned int) 0);
        revR[PS][2] = ~((unsigned int) 0);
        revR[PS][3] = ~((unsigned int) 0);
        revR[PS][4] = ~((unsigned int) 0);
      }
      else {
        
        // reverse screen
        revR[PS][4] <<= 4; 
        revR[PS][3] <<= 4; 
        revR[PS][2] <<= 4; 
        revR[PS][1] <<= 4;
        revR[PS][0] <<= 4;
    
        carry1     = revR[PS][0] & revParray[PS][0][fourmer];
        revR[PS][0] = revR[PS][0] ^ revParray[PS][0][fourmer];
    
        carry2     = carry1 & revR[PS][1] | carry1 & revParray[PS][1][fourmer] | revR[PS][1] & revParray[PS][1][fourmer];
        revR[PS][1] = carry1 ^ revR[PS][1] ^ revParray[PS][1][fourmer];
    
        carry3     = carry2 & revR[PS][2] | carry2 & revParray[PS][2][fourmer] | revR[PS][2] & revParray[PS][2][fourmer];
        revR[PS][2] = carry2 ^ revR[PS][2] ^ revParray[PS][2][fourmer];

        carry4     = carry3 & revR[PS][3] | carry3 & revParray[PS][3][fourmer] | revR[PS][3] & revParray[PS][3][fourmer];
        revR[PS][3] = carry3 ^ revR[PS][3] ^ revParray[PS][3][fourmer];

        revR[PS][4] = revR[PS][4] | carry4 | revParray[PS][4][fourmer];
        
            
        // check reverse primers for a match
        if (!(!(revHitBuffer[PS] & (~revR[PS][4] | mismBits[4])) ||
          !(revHitBuffer[PS] & (~revR[PS][4] | mismBits[4]) & 
                              (~revR[PS][3] | mismBits[3] | ~revR[PS][4] & mismBits[4])) ||
          !(revHitBuffer[PS] & (~revR[PS][4] | mismBits[4]) & 
                              (~revR[PS][3] | mismBits[3] | ~revR[PS][4] & mismBits[4]) &
                              (~revR[PS][2] | mismBits[2] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3])) ||
          !(revHitBuffer[PS] & (~revR[PS][4] | mismBits[4]) & 
                              (~revR[PS][3] | mismBits[3] | ~revR[PS][4] & mismBits[4]) &
                              (~revR[PS][2] | mismBits[2] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3]) &
                              (~revR[PS][1] | mismBits[1] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3] | ~revR[PS][2] & mismBits[2])) ||
           !(revHitBuffer[PS] & (~revR[PS][4] | mismBits[4]) & 
                              (~revR[PS][3] | mismBits[3] | ~revR[PS][4] & mismBits[4]) &
                              (~revR[PS][2] | mismBits[2] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3]) &
                              (~revR[PS][1] | mismBits[1] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3] | ~revR[PS][2] & mismBits[2]) &
                              (~revR[PS][0] | mismBits[0] | ~revR[PS][4] & mismBits[4] | ~revR[PS][3] & mismBits[3] | ~revR[PS][2] & mismBits[2] | ~revR[PS][1] & mismBits[1]))
                              
                              
                              )) {
        
          // have a match, add to matches
          if (revMatch(PS, startPosition + (i-bufferStart)*4, maxHits)) {
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex[PS] > 0
        
    // shift all vectors 
    forR[MS][4] <<= 4; 
    forR[MS][3] <<= 4; 
    forR[MS][2] <<= 4; 
    forR[MS][1] <<= 4;
    forR[MS][0] <<= 4;
    
    carry1     = forR[MS][0] & forParray[MS][0][fourmer];
    forR[MS][0] = forR[MS][0] ^ forParray[MS][0][fourmer];
    
    carry2     = carry1 & forR[MS][1] | carry1 & forParray[MS][1][fourmer] | forR[MS][1] & forParray[MS][1][fourmer];
    forR[MS][1] = carry1 ^ forR[MS][1] ^ forParray[MS][1][fourmer];
    
    carry3     = carry2 & forR[MS][2] | carry2 & forParray[MS][2][fourmer] | forR[MS][2] & forParray[MS][2][fourmer];
    forR[MS][2] = carry2 ^ forR[MS][2] ^ forParray[MS][2][fourmer];

    carry4     = carry3 & forR[MS][3] | carry3 & forParray[MS][3][fourmer] | forR[MS][3] & forParray[MS][3][fourmer];
    forR[MS][3] = carry3 ^ forR[MS][3] ^ forParray[MS][3][fourmer];

    forR[MS][4] = forR[MS][4] | carry4 | forParray[MS][4][fourmer];
    
    // check forward primers for a match
    if (!(!(forHitBuffer[MS] & (~forR[MS][4] | mismBits[4])) ||
          !(forHitBuffer[MS] & (~forR[MS][4] | mismBits[4]) & 
                              (~forR[MS][3] | mismBits[3] | ~forR[MS][4] & mismBits[4])) ||
          !(forHitBuffer[MS] & (~forR[MS][4] | mismBits[4]) & 
                              (~forR[MS][3] | mismBits[3] | ~forR[MS][4] & mismBits[4]) &
                              (~forR[MS][2] | mismBits[2] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3])) ||
          !(forHitBuffer[MS] & (~forR[MS][4] | mismBits[4]) & 
                              (~forR[MS][3] | mismBits[3] | ~forR[MS][4] & mismBits[4]) &
                              (~forR[MS][2] | mismBits[2] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3]) &
                              (~forR[MS][1] | mismBits[1] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3] | ~forR[MS][2] & mismBits[2])) ||
          !(forHitBuffer[MS] & (~forR[MS][4] | mismBits[4]) & 
                              (~forR[MS][3] | mismBits[3] | ~forR[MS][4] & mismBits[4]) &
                              (~forR[MS][2] | mismBits[2] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3]) &
                              (~forR[MS][1] | mismBits[1] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3] | ~forR[MS][2] & mismBits[2]) &
                              (~forR[MS][0] | mismBits[0] | ~forR[MS][4] & mismBits[4] | ~forR[MS][3] & mismBits[3] | ~forR[MS][2] & mismBits[2] | ~forR[MS][1] & mismBits[1])) 
                                                            
                              )) {
 
      // have a match, add to stack
      forMatch(MS, startPosition + (i-bufferStart)*4);    
     
    }
    
    // look for open searches
    if (rightIndex[MS] > 0) {
      
      // make sure we are still within range 
      int curPosition = startPosition + (i-bufferStart)*4;
      if (curPosition - forMatches[MS][rightIndex[MS]-1] > maxAmpSize) {
        // out of range... reset!
        rightIndex[MS] = 0;
        revR[MS][0] = ~((unsigned int) 0);
        revR[MS][1] = ~((unsigned int) 0);
        revR[MS][2] = ~((unsigned int) 0);
        revR[MS][3] = ~((unsigned int) 0);
        revR[MS][4] = ~((unsigned int) 0);
      }
      else {
        
        // reverse search
        revR[MS][4] <<= 4; 
        revR[MS][3] <<= 4; 
        revR[MS][2] <<= 4; 
        revR[MS][1] <<= 4;
        revR[MS][0] <<= 4;
    
        carry1     = revR[MS][0] & revParray[MS][0][fourmer];
        revR[MS][0] = revR[MS][0] ^ revParray[MS][0][fourmer];
    
        carry2     = carry1 & revR[MS][1] | carry1 & revParray[MS][1][fourmer] | revR[MS][1] & revParray[MS][1][fourmer];
        revR[MS][1] = carry1 ^ revR[MS][1] ^ revParray[MS][1][fourmer];
    
        carry3     = carry2 & revR[MS][2] | carry2 & revParray[MS][2][fourmer] | revR[MS][2] & revParray[MS][2][fourmer];
        revR[MS][2] = carry2 ^ revR[MS][2] ^ revParray[MS][2][fourmer];

        carry4     = carry3 & revR[MS][3] | carry3 & revParray[MS][3][fourmer] | revR[MS][3] & revParray[MS][3][fourmer];
        revR[MS][3] = carry3 ^ revR[MS][3] ^ revParray[MS][3][fourmer];

        revR[MS][4] = revR[MS][4] | carry4 | revParray[MS][4][fourmer];
        
        // check reverse primers for a match
        if (!(!(revHitBuffer[MS] & (~revR[MS][4] | mismBits[4])) ||
          !(revHitBuffer[MS] & (~revR[MS][4] | mismBits[4]) & 
                              (~revR[MS][3] | mismBits[3] | ~revR[MS][4] & mismBits[4])) ||
          !(revHitBuffer[MS] & (~revR[MS][4] | mismBits[4]) & 
                              (~revR[MS][3] | mismBits[3] | ~revR[MS][4] & mismBits[4]) &
                              (~revR[MS][2] | mismBits[2] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3])) ||
          !(revHitBuffer[MS] & (~revR[MS][4] | mismBits[4]) & 
                              (~revR[MS][3] | mismBits[3] | ~revR[MS][4] & mismBits[4]) &
                              (~revR[MS][2] | mismBits[2] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3]) &
                              (~revR[MS][1] | mismBits[1] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3] | ~revR[MS][2] & mismBits[2])) ||
          !(revHitBuffer[MS] & (~revR[MS][4] | mismBits[4]) & 
                              (~revR[MS][3] | mismBits[3] | ~revR[MS][4] & mismBits[4]) &
                              (~revR[MS][2] | mismBits[2] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3]) &
                              (~revR[MS][1] | mismBits[1] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3] | ~revR[MS][2] & mismBits[2]) &
                              (~revR[MS][0] | mismBits[0] | ~revR[MS][4] & mismBits[4] | ~revR[MS][3] & mismBits[3] | ~revR[MS][2] & mismBits[2] | ~revR[MS][1] & mismBits[1]))                              
                              
                              )) {

          // have a match, add to matches
          if (revMatch(MS, startPosition + (i-bufferStart)*4, maxHits)) {
            // max hits has been reached
            return true;
          }
        } 
      }
    } // if rightIndex[MS] > 0
      
  } // for (int i...
  
  // not enough hits
  return false;
}
*/
