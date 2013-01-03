
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

///////////////////////////////////////////////////////////////////////////////////////
// BlastInfo                                                                         //
// desc: holds information on seq names and seq offsets within the blast file        //
///////////////////////////////////////////////////////////////////////////////////////

class BlastInfo {
  public:
    BlastInfo(char *seqName, int startOffset, int seqLength);
    BlastInfo(char *seqName);
    
    // accessors
    char *getSeqName();
    int getSeqLength();
    int getStartOffset();
    void setSeqLength(int so);
    void setStartOffset(int so);
    
  private:
    char seqName[256];
    int startOffset;
    int seqLength;
};

