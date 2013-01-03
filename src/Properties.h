#ifndef PROPERTIES_H__
#define PROPERTIES_H__

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
// Properties                                                                //
//    desc: constants required by PGREP                                      //
///////////////////////////////////////////////////////////////////////////////  

const unsigned int BUFFER_FLANK = 500; // amount of extra sequence to keep with hits
const unsigned int MAXPRIMERSIZE = 29; // max primer size before truncated.  This is limited by integer word size
                                       // and should not be increased beyond 29!
const int BLAST_INT = 4; // blast files assumed to use 32-bit ints
const int MAX_BLAST_ENTRIES = 150; // max seqs in a blast file
const int MAX_FILENAME_LENGTH = 256; // filename lengths accepted by program
const int MAX_DB_FILES = 32; // max number of db files that will be searched at one time
const int MAX_PRIMER_PAIRS = 200000; // max number of primer pairs
const unsigned int BUFFERSIZE = 2000000; // sequence buffer size
const int CACHE_SIZE = 20; // size of cache used to hold previous hits
const int MAX_SEQNAME_SIZE = 256; // size of seq names in pgrep db
const int MAX_AMP_SIZE = 50000; // max allowed size of amplicon

const int NBASES = 4;

enum PrimerDirection {FOR, REV};
const int NDIRS = 2; // directions = for and rev   

enum PrimerEnd {P5, P3}; // 5', 3'

enum DNAStrand {PS, MS}; // plus strand, minus strand
const int NDNASTRANDS = 2; // two pairs used to search - PS and MS

const unsigned int NFOURMERS = 256; // number of fourmers = 4^4
const int MAX_BITS = 6; // (MB+1) allow mismatch scores up to 2^5 - 1 = 31

const int MATCH_BUFFER = 4096;
const int MAX_MATCHES = 20000; // max number of matches we can return for any one primer pair  
 

#endif // PROPERTIES_H__ 
