#include "PrimerPair.h"
#include <iostream>
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
// PrimerPair (constructor)                                                  //
//  inputs: id <string>, for primer <string>, rev primer <string>,           //
//          amp sz <int>                                                     //
//    desc: initializes the primers and expected amp size                    //                                                   
///////////////////////////////////////////////////////////////////////////////  

PrimerPair::PrimerPair(std::string cPrimerPairId, std::string cForPrimer, std::string cRevPrimer, int cAmpSize) {
  primerPairId = cPrimerPairId;
  forPrimer = cForPrimer;
  revPrimer = cRevPrimer;
  ampSize = cAmpSize;
}

///////////////////////////////////////////////////////////////////////////////
// accessors                                                                 //                                                  
///////////////////////////////////////////////////////////////////////////////  

std::string PrimerPair::getForPrimer() {
  return forPrimer;
}

std::string PrimerPair::getRevPrimer() {
  return revPrimer;
}

int PrimerPair::getAmpSize() {
  return ampSize;
}

std::string PrimerPair::getId() {
  return primerPairId;
}

