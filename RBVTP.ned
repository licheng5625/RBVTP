//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.networklayer.routing.RBVTP;
  
//moduleinterface IRBVTR
//{
//    parameters:
//        @display("i=block/routing");
////        string routingTableModule = default("^.routingTable");
//        
//    gates:
//        input ipIn;
//        output ipOut;
//}
       
simple RBVTP  
{
    parameters:
        @display("i=block/routing");
      string networkProtocolModule = default("^.networkLayer.ip");
       string routingTableModule = default("^.routingTable");
      
        string interfaces = default("*");
		double HoldingIndex  =default(1);
		double CPliftime @unit("s") = default(20s); 
	    double RUliftime @unit("s") = default(20s); 
		double RRliftime @unit("s") = default(20s); 
		double DATAliftime @unit("s") = default(20s); 
		double nextRUtimer @unit("s") = default(5s); 
		double RTSTimeout @unit("s") = default(3s); 
		double Tmax  @unit("s") = default(0.5s);
    	double dopt @unit("m") = default(400m); 
   		double dmax @unit("m") = default(800m); 
		double distenceOfclose @unit("m") = default(50m); 
    	double a1 =default(0.2);
    	double a2 =default(1.2);
    	double a3 =default(0.03);
    gates:
        input ipIn;
        output ipOut;
}
