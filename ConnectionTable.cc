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

#include "ConnectionTable.h"


Connectstate::Connectstate( ConnFalg thisflag, simtime_t timestamp, simtime_t entry_timeout)
{
     this->thisflag=thisflag;
     this->timestamp=timestamp;
     this->entry_timeout=entry_timeout;
}



void ConnectionTable::addConnection(std::string roadsrc,std::string roaddes)
{
    Connectstate connstate;
    Connection conn=std::pair<std::string ,std::string >(roadsrc, roaddes);
    connectionstable[conn]=connstate;
}
void ConnectionTable::addConnection(std::string roadsrc,std::string roaddes,Connectstate connstate)
{
     Connection conn=std::pair<std::string ,std::string >(roadsrc, roaddes);
     connectionstable[conn]=connstate;
}
void ConnectionTable::addTwoWayConnection(std::string roadsrc,std::string roaddes)
{
    addConnection(roadsrc,roaddes);
    addConnection(roaddes,roadsrc);
}
void ConnectionTable::addTwoWayConnection(std::string roadsrc,std::string roaddes,Connectstate conn)
{
    addConnection(roadsrc,roaddes,conn);
    addConnection(roaddes,roadsrc,conn);
}
bool ConnectionTable::hasConnect(std::string roadsrc)   {
    for (Connectiontable::const_iterator it = connectionstable.begin(); it != connectionstable.end(); it++)
       {
           if(it->first.first==roadsrc)
           {
               return true;
           }
       }
    return false;
}
Connectstate ConnectionTable::getConnectState(std::string roadsrc,std::string roaddes)   {
    for (Connectiontable::const_iterator it = connectionstable.begin(); it != connectionstable.end(); it++)
       {
           if(it->first.first==roadsrc&&it->first.second==roaddes)
           {
               return it->second;
           }
       }
}
void ConnectionTable::clear() {
    connectionstable.clear();
 }

std::vector<std::string>  ConnectionTable::getConnections(std::string srcconn)
{
    vector<std::string> ret;
    for (Connectiontable::const_iterator it = connectionstable.begin(); it != connectionstable.end(); it++)
           {
               if(it->first.first==srcconn)
               {
                   ret.push_back( it->first.second);
               }
           }
    return ret;
}
std::vector<std::string>  ConnectionTable::getReachableConnections(std::string srcconn)
{
    vector<std::string> ret;
    for (Connectiontable::const_iterator it = connectionstable.begin(); it != connectionstable.end(); it++)
           {
               if(it->first.first==srcconn&&it->second.thisflag==Reachable)
               {
                   ret.push_back( it->first.second);
               }
           }
    return ret;
}
std::vector<std::pair<std::string,std::string> >  ConnectionTable::getAllConnections()
{
    vector<std::pair<std::string,std::string> > ret;
    for (Connectiontable::const_iterator it = connectionstable.begin(); it != connectionstable.end(); it++)
           {
                   ret.push_back( it->first);
           }
    return ret;
}
