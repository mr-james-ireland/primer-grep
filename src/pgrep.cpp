
/////////////////////////////////////////////////////////////////////////////////
// PGREP  v2.10                                                                //
//  -An Application to Rapidly Filter PCR Primers Based on Genomic Specificity //
//                                                                             //
// Author: James Ireland                                                       //
//                                                                             //
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

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>

#include "Properties.h"
#include "TargetCache.h"
#include "PrimerSearch.h"
#include "BlastInfo.h"
           
// set up target cache
TargetCache SEQ_CACHE[CACHE_SIZE];
int cache_ptr = 0;

 // flag for full output mode
int fullOutput = 0;

///////////////////////////////////////////////////////////////////////////////
// uppercase                                                                 //
//  inputs: primer sequence <std::string>                                    //
//  output: none                                                             //
//    desc: replaces any lower-case chars in primer seq with upper-case      //
///////////////////////////////////////////////////////////////////////////////  

void uppercase(std::string &primer) {
  int primerLength = primer.length();
  for (int i=0; i < primerLength; i++) {
    if (primer[i] >= 97 && primer[i] <= 122) {
      primer[i] = primer[i] - 32;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// longSwap                                                                  //
//  inputs: 32-bit integer to be reversed (from big endian to little end.)   //
//  output: swapped 32-bits                                                  //
//    desc: blast info files are written big-endian.  Need to convert.       //
///////////////////////////////////////////////////////////////////////////////

int longSwap (int i)
{
  unsigned char b1, b2, b3, b4;

  b1 = i & 255;
  b2 = ( i >> 8 ) & 255;
  b3 = ( i>>16 ) & 255;
  b4 = ( i>>24 ) & 255;

  return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

///////////////////////////////////////////////////////////////////////////////
// shortenPrimerSequence                                                     //
//  inputs: primer sequence <char*>, offset <int>                            //
//  output: none                                                             //
//    desc: shifts primer sequence over (in 5' direction).  Used to shorten  //
//          primers that are over the max length handled by PGREP            //
///////////////////////////////////////////////////////////////////////////////  

void shortenPrimerSequence(char* primSeq, int offset) {
  int psLen = strlen(primSeq);
  int i=0;
  while (offset + i <= psLen) {
    primSeq[i] = primSeq[i+offset];
    ++i;
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadBlastInfo                                                             //
//  inputs: blast db name <std::string>, ptr to blast info array <BlastInfo*>//
//  output: none                                                             //
//    desc: reads in .nin file to figure out seq names and offsets           //
//          .nin format is hard-coded here so if the format ever changes     //
//          this will need to be changed                                     //
//          at the same time, reads in .nhr file for the header names        //
///////////////////////////////////////////////////////////////////////////////  

void loadBlastInfo(std::string filename, BlastInfo* biseqs[]) {
  std::string ninFilename;
  std::string nhrFilename;
  ninFilename = filename + ".nin";
  nhrFilename = filename + ".nhr";
  
  std::ifstream blastNinFile;
  blastNinFile.open (ninFilename.c_str(), std::ios::in | std::ios::binary);
  if (!blastNinFile.is_open()) { 
    std::cerr << "Error opening blast .nin file: " << ninFilename << "\n";
    exit(1);
  }
  
  std::ifstream blastNhrFile;
  blastNhrFile.open (nhrFilename.c_str(), std::ios::in | std::ios::binary);
  if (!blastNhrFile.is_open()) { 
    std::cerr << "Error opening blast .nhr file: " << nhrFilename << "\n";
    exit(1);
  }
  
  // read in formatdb version
  int formatdb;
  blastNinFile.read((char*)&formatdb, BLAST_INT);
   
  // read in blast db type
  int blastType; 
  blastNinFile.read((char*)&blastType, BLAST_INT);
  if (blastType != 0) {
  	// not a nucleotide db!
  	std::cerr << "Error: blast file " << ninFilename << " is not a nucleotide blast db.\n";
    exit(1);
  }
  
  // read through title
  int titleSize;
  blastNinFile.read((char*)&titleSize, BLAST_INT);
  titleSize = longSwap(titleSize);
  char ignore;
  
  for (int i = 0; i < titleSize; ++i) {
  	blastNinFile.read((char*)&ignore, 1);
  }
  
  // read through date
  int dateSize;
  blastNinFile.read((char*)&dateSize, BLAST_INT);
  dateSize = longSwap(dateSize);
  for (int i = 0; i < dateSize; ++i) {
  	blastNinFile.read((char*)&ignore, 1);
  }
  
  // now get to the good stuff
  int nseqs;
  blastNinFile.read((char*)&nseqs, BLAST_INT);
  nseqs = longSwap(nseqs);
  if (nseqs > MAX_BLAST_ENTRIES) {
  	std::cerr << "Error:  Blast db has too many seq entries.  Right now PGREP is limited to " << MAX_BLAST_ENTRIES << ".\n";
  	exit(1);
  }
  
  // total seq length appears to be little-endian, 64 bit
  long long int tseq;
  blastNinFile.read((char*)&tseq, BLAST_INT*2);
  
  int maxseq;
  blastNinFile.read((char*)&maxseq, BLAST_INT);
  maxseq = longSwap(maxseq);
    
  // loop through the seq names
  int lastOffset;
  int nextOffset;
  blastNinFile.read((char*)&lastOffset, BLAST_INT);
  lastOffset = longSwap(lastOffset);
  char ignoreText[8];
  char seqName[256];
 
  for (int i = 0; i < nseqs; ++i) {
    blastNinFile.read((char*)&nextOffset, BLAST_INT);
  	nextOffset = longSwap(nextOffset);
  	
  	// read text from the last to next offset
  	// - the first 8 chars get tossed - not sure what this is
  	// - also toss last 32 chars... 
  	blastNhrFile.read((char*)&ignoreText, 8);
  	blastNhrFile.read((char*)&seqName, (nextOffset - (lastOffset + 8)));
  	seqName[nextOffset - (lastOffset + 40)] = '\0';
  	biseqs[i] = new BlastInfo(seqName);
  	lastOffset = nextOffset;
  }
  
  blastNinFile.read((char*)&lastOffset, BLAST_INT);
  lastOffset = longSwap(lastOffset);
  
  for (int i = 0; i < nseqs; ++i) {
    blastNinFile.read((char*)&nextOffset, BLAST_INT);
  	nextOffset = longSwap(nextOffset);

    biseqs[i]->setStartOffset(lastOffset);
    biseqs[i]->setSeqLength(nextOffset-lastOffset); 
  	lastOffset = nextOffset;
  }
  biseqs[nseqs] = NULL;  
}
  
///////////////////////////////////////////////////////////////////////////////
// createPrimerPair                                                          //
//  inputs:  left primer, right primer, target size, noRevC                  //
//                                                                           //
//  output: PrimerPair object                                                //
//    desc: creates primer pair with truncation if need be                   //
///////////////////////////////////////////////////////////////////////////////

PrimerPair* createPrimerPair(std::string primerId, std::string leftPrimer, std::string rightPrimer, int productSize, bool noRevC) {
	// make a new primer pair object
	if (leftPrimer.length() > MAXPRIMERSIZE) {
	  std::cerr << "WARNING: " << primerId << " left primer/probe too long.  Shortening!.\n";
	  productSize -= (leftPrimer.length() - MAXPRIMERSIZE);
	  leftPrimer = leftPrimer.substr(leftPrimer.length() - MAXPRIMERSIZE, MAXPRIMERSIZE);
	}
	if (rightPrimer.length() > MAXPRIMERSIZE) {
	  std::cerr << "WARNING: " << primerId << " right primer/probe too long.  Shortening!.\n";
	  productSize -= (rightPrimer.length() - MAXPRIMERSIZE);
	  if (noRevC) {
	    rightPrimer = rightPrimer.substr(0, MAXPRIMERSIZE);
	  } else {
	    rightPrimer = rightPrimer.substr(rightPrimer.length() - MAXPRIMERSIZE, MAXPRIMERSIZE);
	  }
	}

	uppercase(leftPrimer);
	uppercase(rightPrimer);
	return new PrimerPair(primerId, leftPrimer, rightPrimer, productSize);
}

///////////////////////////////////////////////////////////////////////////////
// loadAllBlastInfo                                                          //
//  inputs: blast db file array <vector std::string>, blast info pointer     //
//          <BlastInfo[][]>, blast db file cnt <int>                         //
//  output: none                                                             //
//    desc: loads blast info for the entire set of blast dbs                 //
///////////////////////////////////////////////////////////////////////////////  

int loadPrimerFile(std::string filename, PrimerPair* ppairs[], bool noRevC) {

  // open the primer pair file
  std::ifstream primerFile(filename.c_str());
  if (!primerFile.is_open()) { 
    std::cerr << "Error opening primer file: " << filename << "\n";
    exit(1);
  }
  
  // first line is header
  std::string line;
  std::getline(primerFile, line);
  int ppcount = 0;

  while (!primerFile.eof()) {
  	// check that we don't have too many primer pairs
  	if (ppcount >= MAX_PRIMER_PAIRS) {
  	  std::cerr << "Error:  Too many primer pairs in input file.  The limit is [" << MAX_PRIMER_PAIRS << "].\n";
  	  exit(1);
  	}	
  		
    // read in line of primers
    std::getline(primerFile, line);
    
    // read id first
    std::string primerId;
    unsigned int nid = line.find('\t',0);
    if (nid == (const unsigned int) std::string::npos) {
      continue; // ill-formed line - skip (error message instead?)
    }
    primerId = line.substr(0, nid);
    //std::cout << "ID: " << primerId << "\n";
    
    // read left primer
    std::string leftPrimer;
    unsigned int nlp = line.find('\t', nid + 1);
    if (nlp == (const unsigned int)std::string::npos) {
      continue; // ill-formed line - skip (error message instead?)
    }
    leftPrimer = line.substr(nid + 1, nlp - nid - 1);
    //std::cerr << "Left: " << leftPrimer << "\n";

    // read right primer
    std::string rightPrimer;
    int unsigned nrp = line.find('\t', nlp + 1);
    if (nrp == (const unsigned int) std::string::npos) {
      continue; // ill-formed line - skip (error message instead?)
    }
    rightPrimer = line.substr(nlp + 1, nrp - nlp - 1);

    // read amp size
    int productSize;
    std::string productSizeText;
    unsigned int nps = line.find('\t', nrp + 1);
    if (nps == (const unsigned int) std::string::npos) {
      productSizeText = line.substr(nrp+1, line.length() - nrp);
    }
    else {
      productSizeText = line.substr(nrp + 1, nps - nrp - 1);
    }
    productSize = atoi(productSizeText.c_str());
    
    ppairs[ppcount] = createPrimerPair(primerId, leftPrimer, rightPrimer, productSize, noRevC);
    ppcount++;
    
  }
  
  return ppcount;
}  

void loadAllBlastInfo(std::vector<std::string> blastFiles, BlastInfo *biseqs[][MAX_BLAST_ENTRIES], int blastIndex) {
  for (int i=0; i < blastIndex; ++i) {
  	loadBlastInfo(blastFiles[i], biseqs[i]);
  }
}

///////////////////////////////////////////////////////////////////////////////
// readPrimersFromStdin                                                      //
//  inputs: ptr preamble <std::string>, ptr bldrText <std::string>, noRevC   //
//  output: PrimerPair <PrimerPair*>                                         //
//    desc: reads in primer pairs in boulder format from STDIN               //
//          assumes primer pair starts with PRIMER_PAIR_PENALTY and ends     //
//          with PRIMER_PRODUCT_SIZE                                         //
/////////////////////////////////////////////////////////////////////////////// 

PrimerPair* readPrimersFromStdin(std::string &preamble, std::string &bldrText, bool noRevC) {
  
  // read stdin until we reach the next end of boulder or end of stream
  std::string leftPrimer = "";
  std::string rightPrimer = "";
  std::string primerPairId = "";
  int prodSize = -1;
  bool recordStart = false;
  std::string line;
  
  while(std::getline(std::cin, line)) {
  	
  	if (!recordStart) {
  	  if (line.find("PRIMER_PAIR_PENALTY") == std::string::npos) {
  	  	preamble = preamble + line + "\n";
  	  	continue;
  	  }
  	  recordStart = true;
  	}
  	  		 
  	bldrText = bldrText + line + "\n";
  	
  	if (line.find("_SEQUENCE=",0) != std::string::npos) {
      int sstart = line.find("=",0);
  	  std::string primerSeq = line.substr(sstart + 1, line.length()- sstart);
  	  if (line.find("PRIMER_LEFT") != std::string::npos) {
  	  	leftPrimer = primerSeq;
  	  }
  	  else {
  	  	if (line.find("PRIMER_RIGHT") != std::string::npos) {
  	  	  rightPrimer = primerSeq;
  	    }
  	  }  	
  	} 
  	if (line.find("PRIMER_PRODUCT_SIZE",0) != std::string::npos) {
  	  int sstart = line.find("=",0);
  	  if (sstart == 19) {
  	  	primerPairId = "PRIMER_0";
  	  } 
  	  else {
  	    primerPairId = "PRIMER" + line.substr(19, sstart - 19); 
  	  }
  	  prodSize = atoi(line.substr(sstart + 1, line.length() - sstart).c_str());
  	  break;
  	}
  }
  
  if (leftPrimer == "" || rightPrimer == "" || prodSize == -1) {
  	return (NULL);
  }    
  
  return createPrimerPair(primerPairId,leftPrimer,rightPrimer,prodSize,noRevC);
   
}  
    
///////////////////////////////////////////////////////////////////////////////
// addHitsToCache                                                            //
//  inputs: primer search <PrimerSearch*>, current number matches <int>,     //
//          sequence name <char*>, seq buffer <char*>, start pos<uns int>,   //
//          buffer start pos <int>, buffer end pos <int>, not at end flg<int>//
//  output: none                                                             //
//    desc: records where previous primers hit to a cache so that subsequent //
//          primers may be search against these locations first              //
/////////////////////////////////////////////////////////////////////////////// 
  
void addHitsToCache (PrimerSearch *ps, int &curMatchCount, char *seqName, char *buffer, 
                     unsigned int startPos, int bufferStart, int bufferEnd, int notAtEnd) {
  int matchCount = ps->getMatchCount();
  for (int i = curMatchCount; i < matchCount; i++) {
    PrimingSite *match = ps->getPrimingSite(i);
    unsigned int start;
    if (BUFFER_FLANK > match->getForPos()) {
      start = 0;
    } 
    else {
      start = int((match->getForPos() - BUFFER_FLANK)/4)*4;
    }
    
    int bStart = int(start - startPos); // start and startPos should both be div by 4
    bStart = int(bStart/4) + bufferStart; 
    int end = int((match->getRevPos() + BUFFER_FLANK)/4)*4;
    int bEnd = int((int(end - startPos))/4) + bufferStart;
    if (bEnd >= bufferEnd) {
      if (notAtEnd) {
        // too close to edge... skip remaining hits and move on to next
        // scan
        curMatchCount = i;
        return;
      }
      else {
        bEnd = bufferEnd - 1;
      }
    }
    
    SEQ_CACHE[cache_ptr].copyBuffer(seqName, start, end,
                                    buffer, BUFFERSIZE*2,
                                    bStart, (bEnd - bStart + 1));  
    cache_ptr = (cache_ptr + 1) % CACHE_SIZE;
  }  
  curMatchCount = matchCount; // all hits accounted for
}  

// does simple bp by bp comparison and returns " || "
// style alignment + weighted alignment score

///////////////////////////////////////////////////////////////////////////////
// makeAlignment                                                             //
//  inputs: primer direction <PrimerDirection>, whether no rev compl req<int>//
//          primer search <PrimerSearch>, alignment string <std::string>,    //
//          primer sequence <std::string>, template seq <std::string>        //
//  output: mismatch score <int>                                             //
//    desc: accepts a primer and template sequence and produces and alignment//
//          string by placing an '|' at positions where the two strings agree//
//          At the same time, a mismatch score is computed.                  //
/////////////////////////////////////////////////////////////////////////////// 

int makeAlignment(PrimerDirection primer, int dir, PrimerSearch* ps, std::string &align, std::string seq1, std::string seq2) {
  int seqLen = seq1.length();
  align = seq1;
  int kmm = 0;
  for (int i=0; i < seqLen; i++) {
    if (seq1[i] == seq2[i]) {
      align[i] = '|';
    }
    else {
      align[i] = ' ';
      
    }
    kmm += ps->getWeight(primer, (1-dir)*i +dir*(seqLen - i - 1), (1-dir)*(seqLen - i - 1) + dir*i, seq1[i], seq2[i]);

  }
  return kmm;  
}


void substr(char *newSt, char *st, int start, int ln) {
  for (int i=0; i < ln; i++) {
    newSt[i] = st[i + start];
  }
  newSt[ln] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
// displayHits                                                               //
//  inputs: primer search <PrimerSearch*>, number of matches found<int>,     //
//          name of seq being searched <char*>, seq buffer <char*>, start    //
//          pos in seq <int>, buffer start pos <int>, buffer stop pos <int>, //
//          flag whether we're at end of seq <int>                           //
//  output: none                                                             //
//    desc: produces tab-delimited full-output results                       //
/////////////////////////////////////////////////////////////////////////////// 
  
void displayHits (PrimerSearch *ps, int &disMatchCount, char *seqName, char *buffer, 
                  unsigned int startPos, int bufferStart, int bufferEnd, int notAtEnd) {
  int matchCount = ps->getMatchCount();
  
  for (int i = disMatchCount; i < matchCount; i++) {
    PrimingSite *match = ps->getPrimingSite(i);
    unsigned int start = match->getForPos();
    
    int bStart = int(start - startPos - 1);
    // todo - put in a real floor function
    if (bStart < 0) {
      bStart -= 3;
    }
    bStart = int(bStart/4) + bufferStart; 
    
    unsigned int end = match->getRevPos();
    int bEnd = int((int(end - startPos - 1))/4) + bufferStart;
    
    if (bEnd >= bufferEnd) {
      if (notAtEnd) {
        // too close to edge... skip remaining hits and move on to next
        // scan
        disMatchCount = i;
        return;
      }
      else {
        std::cerr << "FATAL ERROR:  Match fell outside of buffer area." << "\n";
        exit(1);
      }
    }
    char* sequence = PrimerSearch::convertToSequence(buffer, BUFFERSIZE*2, bStart, (bEnd - bStart + 1));  
                                    
    PrimerPair *pp = ps->getPrimerPair();

    std::cout << pp->getId() << "\t" << match->getSeqName() << "\t" << match->getForPos() << "\t";
    std::cout << match->getRevPos() << "\t" << match->getStrand() << "\t" << (match->getRevPos() - match->getForPos() + 1);
    std::cout << "\t";

    // amp seq
    int ampStart = ((start-startPos-1) % 4);
    //int ampStop = ((bEnd-bStart)*4 + ((end - startPos) % 4));
    int ampLength = end-start+1;
     
    char amp[MAX_AMP_SIZE];
    substr(amp,sequence,ampStart,ampLength);
    
    // primer info
    std::string forSeq;
    std::string revSeq;
    std::string forPrimerSeq;
    if (match->getForDir() == FOR || match->getStrand()) {
    	forPrimerSeq = pp->getForPrimer();
    }
    else {
    	forPrimerSeq = pp->getRevPrimer();
    }

    std::string revPrimerSeq;
    if (match->getRevDir() == REV || match->getStrand()) {
        	revPrimerSeq = pp->getRevPrimer();
    }
    else {
        	revPrimerSeq = pp->getForPrimer();
    }

    std::string align;
    int forLength = forPrimerSeq.length();
    int revLength = revPrimerSeq.length();
    int kmm;
    if (match->getStrand()) {
      // reverse strand, so use end of amplicon
      forSeq.assign(amp,ampLength-forLength,forLength);
      forSeq = PrimerSearch::reverseComplement(forSeq);      
      revSeq.assign(amp,0,revLength);
 
      if (ps->getNoRevC()) {
        revSeq = PrimerSearch::reverseComplement(revSeq);
      }
    }
    else {
      forSeq.assign(amp,0,forLength);
      revSeq.assign(amp,ampLength-revLength,revLength);
      if (!ps->getNoRevC()) {
        revSeq = PrimerSearch::reverseComplement(revSeq);
      }
    }
    std::cout << forPrimerSeq << "\t";      
    kmm = makeAlignment(FOR, 0, ps, align, forPrimerSeq, forSeq);
    std::cout << align << "\t";
    std::cout << forSeq << "\t" << kmm << "\t";
    
    std::cout << revPrimerSeq << "\t";
    int dir = (ps->getNoRevC()) ? 1 : 0;
    
    kmm = makeAlignment(REV, dir, ps, align, revPrimerSeq, revSeq);
    std::cout << align << "\t";
    std::cout << revSeq << "\t" << kmm << "\t";
    
    // dump amplicon as well
    std::cout << amp << "\n";
   
  }  
   
  // everything displayed
  disMatchCount = matchCount;
 
} 

///////////////////////////////////////////////////////////////////////////////
// searchFile                                                                //
//  inputs: pgrep db filename <std::string>, max allowed mm <int>,           //
//          max number hits <int>, primer search <PrimerSearch*>             //
//  output: true if too many priming sites                                   //
//    desc: runs the core search alg on a single pgrep db file               //
/////////////////////////////////////////////////////////////////////////////// 
  
bool searchFile(std::string filename, int kmm, int maxHits, PrimerSearch *ps) {

  // open file 
  //  create a buffer that is twice as big as we need so
  //  that we can always pull previous flanking sequence 
  //  load each half in turn
  std::ifstream targetFile(filename.c_str());
  
  char buffer[BUFFERSIZE*2];
  int curBuffer;
  int curMatchCount;
  int disMatchCount;
  char seqName[MAX_SEQNAME_SIZE];
  bool tooManyHits;
  //targetFile.open (filename, std::ios::in | std::ios::binary);

  while (targetFile.read(seqName,MAX_SEQNAME_SIZE)) {
    // first read in name and size of seq
    
    int bufferHalf = 0;
    unsigned int position = 0;

    unsigned int seqSize;
    targetFile.read((char*)&seqSize,sizeof(unsigned int));
    
    // reset search for start of new sequence
    ps->reset(seqName);
       
    // record current number of matches so we can cache new hits
    curMatchCount = ps->getMatchCount();
    disMatchCount = ps->getMatchCount();

    while (seqSize) {      
    
 
      if (seqSize < BUFFERSIZE) {
        curBuffer = seqSize;
      } 
      else {
        curBuffer = BUFFERSIZE;
      }
      targetFile.read(buffer + (bufferHalf*BUFFERSIZE),curBuffer);
      if (!targetFile) {
        // error!!! did not have enough seq!
        std::cerr << "Error: Not enough sequence found when reading from file: " << filename << "\n";
        exit(0);
      } 
      tooManyHits = ps->findPattern(buffer, (bufferHalf*BUFFERSIZE), (bufferHalf*BUFFERSIZE)+ curBuffer, position, maxHits);
       
      // any caching to do?
      if (ps->getMatchCount() > curMatchCount) {
        // produce output? 
        if (fullOutput) { 
          displayHits(ps,disMatchCount,seqName,buffer,position,bufferHalf*BUFFERSIZE,
                       (bufferHalf*BUFFERSIZE)+ curBuffer,(seqSize-curBuffer)); 
        }
        // add to cache        
        addHitsToCache(ps,curMatchCount,seqName,buffer,position,bufferHalf*BUFFERSIZE,
                       (bufferHalf*BUFFERSIZE)+ curBuffer,(seqSize-curBuffer)); 
      }
      
      if (tooManyHits && (!fullOutput || (disMatchCount == ps->getMatchCount()))) {
        return true;
      }
      position += curBuffer*4;
      seqSize -= curBuffer;
      bufferHalf = (bufferHalf + 1) % 2;
    }
           
    
  }
  targetFile.close();
 
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// searchBlastFile                                                           //
//  inputs: blast db filename <std::string>, max allowed mm <int>,           //
//          max number hits <int>, primer search <PrimerSearch*>, blast info //
//          <BlastInfo[]>                                                    //
//  output: true if too many priming sites                                   //
//    desc: runs the core search alg on a single blast db file               //
/////////////////////////////////////////////////////////////////////////////// 
  
bool searchBlastFile(std::string tfilename, int kmm, int maxHits, PrimerSearch *ps, BlastInfo *blastInfo[MAX_BLAST_ENTRIES]) {

  std::string filename = tfilename + ".nsq";

  // open the nsq file 
  //  create a buffer that is twice as big as we need so
  //  that we can always pull previous flanking sequence 
  //  load each half in turn

  std::ifstream targetFile(filename.c_str(), std::ios::in | std::ios::binary);
  char buffer[BUFFERSIZE*2];
  int curBuffer;
  int curMatchCount;
  int disMatchCount;

  bool tooManyHits;
  //targetFile.open (filename, std::ios::in | std::ios::binary);
  // ignore first byte (which is always 0)
  char ignore;
  //std::cerr << "Read file\n";
  targetFile.read((char*)&ignore, 1);
  
  int bi = 0;
  while (blastInfo[bi] != NULL) {
  		
    // first read in name and size of seq
    int bufferHalf = 0;
    unsigned int position = 0;

    unsigned int seqSize = blastInfo[bi]->getSeqLength();
    
    // reset search for start of new sequence
    ps->reset(blastInfo[bi]->getSeqName());
       
    // record current number of matches so we can cache new hits
    curMatchCount = ps->getMatchCount();
    disMatchCount = ps->getMatchCount();

    while (seqSize) {      
      if (seqSize < BUFFERSIZE) {
        curBuffer = seqSize;
      } 
      else {
        curBuffer = BUFFERSIZE;
      }
      targetFile.read(buffer + (bufferHalf*BUFFERSIZE),curBuffer);
      if (!targetFile) {
        // error!!! did not have enough seq!
        std::cerr << "Error: Not enough sequence found when reading from file: " << filename << "\n";
        exit(0);
      } 
      
      tooManyHits = ps->findPattern(buffer, (bufferHalf*BUFFERSIZE), (bufferHalf*BUFFERSIZE)+ curBuffer, position, maxHits);
       
      // any caching to do?
      if (ps->getMatchCount() > curMatchCount) {
        // produce output? 
        if (fullOutput) { 
          displayHits(ps,disMatchCount,blastInfo[bi]->getSeqName(),buffer,position,bufferHalf*BUFFERSIZE,
                       (bufferHalf*BUFFERSIZE)+ curBuffer,(seqSize-curBuffer)); 
        }
        // add to cache        
        addHitsToCache(ps,curMatchCount,blastInfo[bi]->getSeqName(),buffer,position,bufferHalf*BUFFERSIZE,
                       (bufferHalf*BUFFERSIZE)+ curBuffer,(seqSize-curBuffer)); 
      }
      
      if (tooManyHits && (!fullOutput || (disMatchCount == ps->getMatchCount()))) {
        return true;
      }
      position += curBuffer*4;
      seqSize -= curBuffer;
      bufferHalf = (bufferHalf + 1) % 2;
      
    }
    ++bi;
  }
  targetFile.close();
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// searchAllDbFiles                                                          //
//  inputs: primer search <PrimerSearch*>, number allowed mm <int>,          //
//          max number of hits <int>, vector of database files               //
//          <vector std::string>                                             //
//  output: true if primers found and exceed max hits, false otherwise       //                                                             //
//    desc: main search loop over all pgrep db files                         //
/////////////////////////////////////////////////////////////////////////////// 

bool searchAllDbFiles(PrimerSearch *ps, int kmm, int maxHits, std::vector<std::string> dbFiles, int dbIndex) {
  for (int i=0; i < dbIndex; i++) {
    if(searchFile(dbFiles[i], kmm, maxHits, ps)) {
      return true;
    }
  }
  return false;
}
   
///////////////////////////////////////////////////////////////////////////////
// searchAllBlastFiles                                                       //
//  inputs: primer search pointer, number allowed mm, max number of hits,    //
//          array (pointer) of blast database files,                         //
//          number files, pointer to array of BlastInfo objects              //
//  output: true if primers found and exceed max hits, false otherwise       //                                                             //
//    desc: main search loop over all pgrep db files                         //
/////////////////////////////////////////////////////////////////////////////// 

bool searchAllBlastFiles(PrimerSearch *ps, int kmm, int maxHits, std::vector<std::string> dbFiles, int dbIndex, BlastInfo *blastInfo[][MAX_BLAST_ENTRIES]) {

  for (int i=0; i < dbIndex; i++) {
    if(searchBlastFile(dbFiles[i], kmm, maxHits, ps, blastInfo[i])) {
      return true;
    }
  }

  return false;
}   

///////////////////////////////////////////////////////////////////////////////
// searchCache                                                               //
//  inputs: primer search pointer, number allowed mm, max number of hits     //
//  output: true if primers found and exceed max hits, false otherwise       //                                                             //
//    desc: performs a search over a small cache to try and eliminate        //
//          candidates early                                                 //
/////////////////////////////////////////////////////////////////////////////// 

bool searchCache(PrimerSearch *ps, int kmm, int maxHits) {

  for (int i=0; i < CACHE_SIZE; i++) {
    int curMatchCount = ps->getMatchCount();
    
    // quit if we hit an empty slot
    TargetCache tc = SEQ_CACHE[i];
    if (tc.isEmpty()) {
      return false;
    }
    
    // reset search and screen cache
    ps->reset(tc.getSeqName());
    
    bool searchResult = ps->findPattern(tc.getBuffer(), 0, tc.getBufferSize(),
                                        tc.getStart(), maxHits);
    if (fullOutput && ps->getMatchCount() > curMatchCount) {
      displayHits(ps,curMatchCount,tc.getSeqName(),tc.getBuffer(), tc.getStart(), 0, tc.getBufferSize(), 0);
    }

    if (searchResult) {
      return true;
    }                       
  }

  return false;
}   

///////////////////////////////////////////////////////////////////////////////
// usage                                                                     //
//  inputs: none                                                             //
//  output: none                                                             //
//    desc: displays usage info                                              //
/////////////////////////////////////////////////////////////////////////////// 
  
void usage() {
  std::cerr << "usage: pgrep [options]\n\n"; 
  std::cerr << "Option\tArgument\tDescription\n";
  std::cerr << " l\tseq\tleft primer sequence for checking just one primer pair\n";
  std::cerr << " r\tseq\tright primer sequence for checking just one primer pair\n";
  std::cerr << " c\tdb filename\tStart searches with this blast or pgrep db file.\n";
  std::cerr << " d\tfilenames\tthe pgrep database files to search OR\n";
  std::cerr << " b\tblastdb names\tthe blastn database files to search\n";
  std::cerr << " f\tfactor\tMinimum amplicon size factor (default 2.0).\n";
  std::cerr << " a\tamp size\tMinimum absolute amplicon size (default not set).\n";
  std::cerr << " F\tfactor\tMaximum amplicon size factor (default 2.0).\n";
  std::cerr << " A\tamp size\tMaximum absolute amplicon size (default not set).\n";
  std::cerr << " k\tmismatch cnt\tMaximum number mismatches allowed (REQUIRED).\n";
  std::cerr << " m\thit cnt\tMinimum number of hits (default 1).\n";
  std::cerr << " M\thit cnt\tMaximum number of hits (default 1).\n";
  std::cerr << " o\tnone\tFull output (default short output).\n";
  std::cerr << " p\tfilename\tPrimer file name (REQUIRED).\n";
  std::cerr << " C\tnone\tDo not complement rev primer (default false).\n";
  std::cerr << " L\tmax returned\tReturn all passing primers in list (in short output mode) up to <max>.\n";
  std::cerr << " w\tfilename\tWeight matrix file name.\n";  
  std::cerr << " S\tnone\tDo not check for F-F and R-R amplicons.\n";
  exit(1);
  
}  
   
///////////////////////////////////////////////////////////////////////////////
// main                                                                      //
//  inputs: argc, argv                                                       //
//  output: int                                                              //
//    desc: where the magic happens                                          //
/////////////////////////////////////////////////////////////////////////////// 
    
int main(int argc, char *argv[]) {
    
  if (argc < 2) usage();
  
  // input params to be specified
  std::string dbDir;
  std::string blastDb;
  std::vector<std::string> dbFiles(MAX_DB_FILES);
  std::vector<std::string> blastFiles(MAX_DB_FILES);
  std::string primerFile;
  std::string weightsFile;
  std::string leftSinglePrimer;
  std::string rightSinglePrimer;
  int kmm = 0;
  int maxHits = 1;
  int minHits = 1;
  int maxReturned = -1;
  
  bool noRevC = false;
  bool noSelfSelf = false;
  int dbIndex = 0;
  int blastIndex = 0;
  double minFactor = 2.0;
  double maxFactor = 2.0; 
  int minAmpSize = -1;
  int maxAmpSize = -1;
  fullOutput = 0;
  std::string chrom;
  
  // parse the command line options
  std::vector<std::string> args(argv, argv + argc);
  int ai = 1;
  
  while (ai < argc && args[ai][0] == '-') {
    char option = args[ai++][1]; 
 
    switch (option) {
      case 'l' : 
        if (ai < argc && args[ai][0] != '-') {
          leftSinglePrimer = args[ai];
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting primer sequence after -l option.\n";
          usage();
        }
        break;
      case 'r' : 
        if (ai < argc && args[ai][0] != '-') {
          rightSinglePrimer = args[ai];
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting primer sequence after -r option.\n";
          usage();
        }
        break;
      case 'p' : 
        if (ai < argc && args[ai][0] != '-') {
          primerFile = args[ai];
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting primer file name after -p option.\n";
          usage();
        }
        break;
      case 'w' : 
        if (ai < argc && args[ai][0] != '-') {
          weightsFile = args[ai];
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting weight matrix file name after -w option.\n";
          usage();
        }
        break;
      case 'd' : 
        while (ai < argc && args[ai][0] != '-') {
          dbFiles[dbIndex] = args[ai];
          ++ai;
          ++dbIndex;
          if (dbIndex >= MAX_DB_FILES) {
          	std::cerr << "ERROR:  Too many database files.  Max allowed: " << MAX_DB_FILES << "\n";
          	exit(1);
          }
        }
        break;
      case 'b' : 
        while (ai < argc && args[ai][0] != '-') {
          blastFiles[blastIndex] = args[ai];
          ++ai;
          ++blastIndex;
          if (blastIndex >= MAX_DB_FILES) {
          	std::cerr << "ERROR:  Too many BLAST database files.  Max allowed: " << MAX_DB_FILES << "\n";
          	exit(1);
          }
        }
        break;
      case 'k' : 
        if (ai < argc && args[ai][0] != '-') {
          kmm = atoi(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting number allowed mismatches after -k option.\n";
          usage();
        }
        break;
      case 'f' : 
        if (ai < argc && args[ai][0] != '-') {
          minFactor = atof(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting minimum amplicon factor allowance after -f option.\n";
          usage();
        }
        break;
	  case 'a' : 
        if (ai < argc && args[ai][0] != '-') {
          minAmpSize = atoi(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting minimum amplicon size after -a option.\n";
          usage();
        }
        break;
      case 'F' : 
        if (ai < argc && args[ai][0] != '-') {
          maxFactor = atof(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting maximum amplicon factor allowance after -F option.\n";
          usage();
        }
        break;
	  case 'A' : 
        if (ai < argc && args[ai][0] != '-') {
          maxAmpSize = atoi(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting maximum amplicon size after -A option.\n";
          usage();
        }
        break;
      case 'm' : 
        if (ai < argc && args[ai][0] != '-') {
          minHits = atoi(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting number allowed matches after -m option.\n";
          usage();
        }
        break;
      case 'M' : 
        if (ai < argc && args[ai][0] != '-') {
          maxHits = atoi(args[ai].c_str());
          if (maxHits > MAX_MATCHES) {
            std::cerr << "ERROR:  Max number of matches that PGREP can return is set at [" << MAX_MATCHES << "].\n";
            exit(1);
          }
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting number allowed matches after -M option.\n";
          usage();
        }
        break;
      case 'o' : 
        fullOutput = 1;
        break;
      case 'L' : 
        if (ai < argc && args[ai][0] != '-') {
          maxReturned = atoi(args[ai].c_str());
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting max number passing primer pairs to report.\n";
          usage();
        }
        break;
      case 'C' : 
        noRevC = true;
        break;
      case 'S' :
        noSelfSelf = true;
        break;
      case 'c' : 
        if (ai < argc && args[ai][0] != '-') {
          chrom = args[ai];
          ++ai;
        }
        else {
          std::cerr << "ERROR:  Expecting database file name after -c option.\n";
          usage();
        }
        break;
      case 'h' :
        usage();
        break;  
      default :
        std::cerr << "ERROR:  Unknown option [-" << option << "].\n";
        usage();
    }
        
  }
  
  if (blastIndex == 0 && dbIndex == 0) {
    std::cerr << "ERROR:  Must specify a sequence database to search against.\n";
    usage();
  }
  if ((minFactor <= 0 || maxFactor <= 0) && (minAmpSize < 0 || maxAmpSize < 0)) {
    std::cerr << "ERROR:  Amplicon relative or absolute size thresholds (min, max) must be > 0.\n";
    exit(1);
  }
  if (minAmpSize > maxAmpSize) {
  	std::cerr << "ERROR:  Min amp size must be less than or equal to max amp size threhsold.\n";
  	exit(1);
  }
  
  // rearrange files to place the chrom file first
  if (chrom.length() > 0) {
    if (dbIndex > 0) {
      for (int i=1; i < dbIndex; ++i) {	
        if (chrom == dbFiles[i]) {
          dbFiles[i] = dbFiles[0];
          dbFiles[0] = chrom;
          break;
        }
      }
    } else {
      for (int i=1; i < blastIndex; ++i) {	
        if (chrom == blastFiles[i]) {
          blastFiles[i] = blastFiles[0];
          blastFiles[0] = chrom;
          break;
        }
      }
    }	
  }
  
  // read in blast info if a blastdb was provided
  BlastInfo* blastDbSeqs[MAX_DB_FILES][MAX_BLAST_ENTRIES];
  if (blastIndex > 0) {
  	loadAllBlastInfo(blastFiles, blastDbSeqs, blastIndex);
  }
  
  WeightMatrix* wm = new WeightMatrix();
  
  if (weightsFile.length() != 0) {
    // load in the specified weight matrix
    wm->loadWeightsFile(weightsFile);
  }
  
  PrimerPair* ppSet[MAX_PRIMER_PAIRS];
  int npps = 0;
  bool primersFromStdin = false;
 
  // either load primers from a file, command line args or boulder stream
  std::string preamble;
  std::string bldrText;
  if (primerFile.length() > 0) {
    // primer file
    npps = loadPrimerFile(primerFile, ppSet, noRevC);
  }
  else {
  	if (leftSinglePrimer.length() > 0) {
  	  // command line
  	  if (rightSinglePrimer.length() == 0) {
  		// single primer test
  		minAmpSize = leftSinglePrimer.length();
  		maxAmpSize = leftSinglePrimer.length();
  	  }
  	  else {
  	    if (minAmpSize < 0 || maxAmpSize < 0) {
  	  	  std::cerr << "ERROR:  Min/max amp size must be specified for primers supplied on command line.\n";
  	  	  exit(1);
  	    }
  	  }
  	  ppSet[0] = createPrimerPair("cmdline",leftSinglePrimer,rightSinglePrimer,(maxAmpSize+minAmpSize)/2, noRevC);
  	  npps = 1;
  	}
  	else {
  	  // we'll read in primers from STDIN
  	  primersFromStdin = true;
  	  npps = MAX_PRIMER_PAIRS;
  	}
  }
       
  // display a header if using full output mode
  if (fullOutput) {
  	std::cout << "PrimerPairId\tTemplateSequenceName\tMatchStart\tMatchStop\tMatchStrand\tMatchLength\t";
  	std::cout << "ForwardPrimer\tForwardPrimerAlignment\tForwardTemplateSeq\tForwardMismatchScore\t";      
  	std::cout << "ReversePrimer\tReversePrimerAlignment\tReverseTemplateSeq\tReverseMismatchScore\t";      
  	std::cout << "FullMatchSequence\n";
  }      
    
  // TIMING - REMOVE
  clock_t commence,complete;
  commence=clock();

  // loop through primer pairs until we find a working pair or reach
  // the end of the list
  int nReturned = 0;
  for (int i=0; i < npps; i++) {
    PrimerPair* pp;
    
    if (primersFromStdin) {
      bldrText = "";
      preamble = "";
      pp = readPrimersFromStdin(preamble, bldrText, noRevC);
      if (pp == NULL) {
      	break;
      }
      if (!fullOutput) {
        std::cout << preamble; // should only print something on first primer pair
      }
    }
    else {
      pp = ppSet[i];
    }	
    PrimerSearch ps = PrimerSearch(pp, wm, noRevC, noSelfSelf, minFactor, maxFactor, minAmpSize, maxAmpSize, kmm);
       
    // first search cache
    if (searchCache(&ps, kmm, maxHits)) {
      continue;
    }
    
    if (dbIndex > 0 && !searchAllDbFiles(&ps, kmm, maxHits, dbFiles, dbIndex) && !fullOutput &&
        ps.getMatchCount() >= minHits) {
      // passed primer pair!
      ++nReturned;
      if (primersFromStdin) {
      	std::cout << bldrText;
      	if (maxReturned <= nReturned) {
      	  	std::cout << "=\n"; // make valid bldr
      	  	exit(0);
      	 }
      }
      else {
        std::cout << "Primer pair: " << i << " is OK.  Name: [" << ppSet[i]->getId() << "].\n";
      }
      if (maxReturned <= nReturned) {
        exit(0);
      }
    } else {
      if (blastIndex > 0 && !searchAllBlastFiles(&ps, kmm, maxHits, blastFiles, blastIndex, blastDbSeqs) && !fullOutput &&
        ps.getMatchCount() >= minHits) {
        // passed primer pair!
        ++nReturned;
        if (primersFromStdin) {
      	  std::cout << bldrText;
      	  if (maxReturned <= nReturned) {
      	  	std::cout << "=\n"; // make valid bldr
      	  	exit(0);
      	  }
        }
        else {
          std::cout << "Primer pair: " << i << " is OK.  Name: [" << ppSet[i]->getId() << "].\n";
        }
        if (maxReturned <= nReturned) {
          exit(0);
        }
      }
    }
  }
  //std::cerr << lTime << "\n";
  
  if (primersFromStdin && maxReturned > 0 && nReturned < maxReturned) {
    std::cout << "=\n"; // make valid bldr
    exit(0);
  }
  	
  
  if (!fullOutput && nReturned == 0) {
    std::cout << "No good primers found.\n";
  }
}
