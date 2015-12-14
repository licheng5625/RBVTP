/*
 * RBVTRPacket.h
 *
 *  Created on: Jul 6, 2015
 *      Author: chengli
 */

#ifndef __INET_RBVTP_H_
#define __INET_RBVTP_H_

#include <cmessage.h>
#include <RouteInterface.h>
#include  "RBVTPPacket.h"
#include "IPSocket.h"
#include "TraCIScenarioManager.h"
#include <algorithm>
#include <string>
#include "RBVTP/ConnectionTable.h"
#include "RBVTP/searchtable.h"
#include "RBVTP/RTSwaitingtable.h"
#include "routeInterface/BroadcastWaitingTable.h"
#include "routeInterface/DelayPacketTable.h"
class RTSInfor
{
    public:
        int version;
        int seqsum;
        std::string name;
        std::string desHostname;
        bool operator == (const RTSInfor &t1) {
             return (this->version==t1.version&&this->seqsum==t1.seqsum&&name==t1.name&&desHostname==t1.desHostname);
         }
        RTSInfor( int version,int seqsum, std::string name,std::string desHostname)
        {
            this->version=version;
            this->seqsum=seqsum;
            this->name=name;
            this->desHostname=desHostname;
            }
};
class INET_API RBVTP:public RouteInterface {
public:
    RBVTP();
    virtual ~RBVTP();
  //  typedef std::map<IPvXAddress, int> RDPacketSeenlist;

protected:
    PacketWaitingTable packetwaitinglist;
    PacketWaitingTable RTSpacketwaitinglist;

    double HoldingIndex ;
    double recPow;
    simtime_t CPliftime;
    simtime_t RRliftime;
    simtime_t RUliftime;
    simtime_t DATAliftime;
    simtime_t nextCPtimer;
    simtime_t RTSTimeout;

    cMessage * CPTimer;
    //cMessage * RTSTimeOutTimer;

    double Tmax;
    double dopt;
    double dmax;

    double a1;
    double a2;
    double a3;

    IInterfaceTable *interfaceTable;
    SearchTable mysearchedtable;
    ConnectionTable myconnectionTable;
    ConnectionTable staticConnectionTable;

    RTSWaitingTableforRBVTP RTSwaitinglist;
    void processSelfMessage(cMessage * message);
  //   void processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo){ return ; }
    void processWaitingTimer(cMessage * message,RBVTPPacket *rbvtpPacket,const IPv4Datagram * pakcet);

     void initialize(int stage);

   //  void  processRTSPACKET(RBVTRPacket * rbvtrPacket);
    // void  processCTSPACKET(RBVTRPacket * rbvtrPacket);
    // void  processRDPACKET(RBVTRPacket * rbvtrPacket);
    // void  processRUPACKET(RBVTRPacket * rbvtrPacket);
    // void  processRRPACKET(RBVTRPacket * rbvtrPacket);

    // void  BroadcastRTS(RBVTRPacket * rbvtrPacket);

   // void  handleMessage(cMessage *message);


   // void  scheduleReBoardcastRDTimer(simtime_t holdingtime,RBVTRPacket *rbvtrPacket,IPv4Datagram * datagram);

    //typedef std::map<cMessage *,IPvXAddress> AddressToSqum;
   // PacketWaitingTable packetwaitinglist;
   // std::vector< > reBoardcastRDTimerlist ;
   // void  processRDTimer(cMessage * message,RBVTRPacket *rbvtrPacket,const IPv4Datagram * pakcet);

   // void  clearMessage(cMessage * message,RBVTRPacket *rbvtrPacket);
    // virtual void receiveChangeNotification(int category, const cObject *details);

private:
     std::vector<RTSInfor >RSTSeenlist;
    virtual Result datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);
    virtual Result datagramForwardHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){ return ACCEPT; }
    virtual Result datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) ;
    virtual Result datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry);//{ return ACCEPT; }
    virtual Result datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);

    int squmCP;
    int squmRR;
    int squmRU;
    int squmRTS;
    int squmDATA;
    double distenceOfclose;
    std::string  oldroadID;
    void initConnctionsTable();

    virtual void receiveChangeNotification(int category, const cObject *details);
    RBVTPPacket * createCPPacket(std::string scrconn,  std::string desconn,  std::string packetname);
    RBVTPPacket *createCTSPacket(RBVTPPacket *rbvtpPacket);
    RBVTPPacket * createRTSPacket(RBVTPPacket *rbvtpPacket);

    void processCPTimer(simtime_t timer);
    void processRTSTimeOutTimer(cMessage* timer);

   // std::vector<std::vector<std::string>> RoutingTable;

    Coord  getConnectPosition(std::string conn);
    std::vector<std::string>  getConnections(std::string srcconn);
    RBVTPPacket * BroadcastRTS(RBVTPPacket * rbvtpPacket);
    void processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo);
    void processRTSPACKET(RBVTPPacket * rbvtpPacket);
    void processCTSPACKET(RBVTPPacket * rbvtrPacket);
    void processCPPACKET(RBVTPPacket * rbvtpPacket);
    simtime_t CaculateHoldTime(Coord srcPosition,Coord desPosition);
    simtime_t CaculateHoldTime(Coord srcPosition);
    void scheduleReBoardcastTimer(simtime_t holdingtime,RBVTPPacket *rbvtpPacket,IPv4Datagram * datagram);
    void clearMessage(cMessage * message,RBVTPPacket *rbvtpPacket,PacketWaitingTable &packetwaitinglist);
    double CaculateF(double distence);
    void sendQueuePacket(const IPvXAddress& target,std::vector<std::string> roads,const IPvXAddress nexthop);
    std::string findnextConn(std::string srcconn,ConnectionTable myconnectionTable);
    void showpackets(PacketWaitingTable RTSpacketwaitinglist);

 };

#endif /* RBVTRPACKET_H_ */
