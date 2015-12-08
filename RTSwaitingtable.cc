//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "RBVTP/RTSwaitingtable.h"





void RTSWaitingTableforRBVTP::addRTS(RBVTPPacket* rtspacket,RBVTPPacket* rbvtppacket)
{
    waitinglist[rtspacket] = rbvtppacket;
 }


bool RTSWaitingTableforRBVTP::hasWaitingRTS(RBVTPPacket* rtspacket)   {

    RTSWaitlist::const_iterator it = waitinglist.find(rtspacket);
           if (it == waitinglist.end())
               return false;
           else
           {
               return true;
           }
}
bool RTSWaitingTableforRBVTP::getRBVTPPacket(RBVTPPacket* rtspacket)   {

    RTSWaitlist::const_iterator it = waitinglist.find(rtspacket);
           if (it == waitinglist.end())
               return NULL;
           else
           {
               return it->second;
           }
}
void RTSWaitingTableforRBVTP::removePacket(RBVTPPacket* rtspacket) {
    RTSWaitlist::iterator it = waitinglist.find(rtspacket);
     waitinglist.erase(it);
 }

void RTSWaitingTableforRBVTP::clear() {
    waitinglist.clear();
 }


