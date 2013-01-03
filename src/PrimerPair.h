#ifndef PRIMERPAIR_H__
#define PRIMERPAIR_H__

#include <string>

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
// PrimerPair                                                                //
// - A simple class to hold primer pair info.                                //                                               
///////////////////////////////////////////////////////////////////////////////  

class PrimerPair{
  public:
    PrimerPair(std::string cPrimerPairId, std::string cForPrimer, std::string cRevPrimer, int cAmpSize);
    
    // accessors
    int getAmpSize();
    std::string getForPrimer();
    std::string getRevPrimer();
    std::string getId();
    
  private:
    std::string primerPairId;
    std::string forPrimer;
    std::string revPrimer;
    int ampSize;
};

#endif // PRIMERPAIR_H__

