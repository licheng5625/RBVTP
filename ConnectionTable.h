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

#ifndef __INET_CONNECTIONTABLE_H_
#define __INET_CONNECTIONTABLE_H_

#include <vector>
#include <map>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include <IPv4Datagram.h>
#include <RBVTP/RBVTPDefs.h>

using namespace std;


class Connectstate {
public:
    Connectstate( ConnFalg thisflag=Unreachable, simtime_t timestamp=simTime(), simtime_t entry_timeout=simTime()+100);
    ConnFalg thisflag;
    simtime_t timestamp;
    simtime_t entry_timeout;
};

/**
 * This class provides a mapping between node addresses and their positions.
 */
class INET_API ConnectionTable {
    private:
        typedef std::pair<std::string,std::string>Connection;

         typedef std::map<Connection,Connectstate > Connectiontable;
         Connectiontable connectionstable;

    public:
         ConnectionTable() { }



        bool hasConnect( std::string connect);

        void addConnection(std::string roadsrc,std::string roaddes);
        void addConnection(std::string roadsrc,std::string roaddes,Connectstate conn);
        Connectstate getConnectState(std::string roadsrc,std::string roaddes)   ;
        std::vector<std::string>  getConnections(std::string srcconn);
        std::vector<std::string>  getReachableConnections(std::string srcconn);

        void clear();

};

#endif
