#include "BlastInfo.h"
#include <string>
#include <string.h>

BlastInfo::BlastInfo(char *name, int offset, int length) {
  strcpy(seqName, name);
  startOffset = offset;
  seqLength = length;
}

BlastInfo::BlastInfo(char *name) {
  strcpy(seqName, name);
}

///////////////////////////////////////////////////////////////////////////////
// accessors/mutators                                                        //                                                  
/////////////////////////////////////////////////////////////////////////////// 

char* BlastInfo::getSeqName() {
  return seqName;
}

int BlastInfo::getStartOffset() {
  return startOffset;
}

int BlastInfo::getSeqLength() {
  return seqLength;
}

void BlastInfo::setStartOffset(int so) {
  startOffset = so;
}

void BlastInfo::setSeqLength(int sl) {
  seqLength = sl;
}

