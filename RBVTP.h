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
#include "RBVTP/ConnectionTable.h"
#include "RBVTP/searchTable.h"
#include "RBVTP/RTSwaitingtable.h"
#include "RBVTP/BroadcastWaitingTable.h"
#include "RBVTP/DelayPacketTable.h"

class INET_API RBVTP:public RouteInterface {
public:
    RBVTP();
    virtual ~RBVTP();
  //  typedef std::map<IPvXAddress, int> RDPacketSeenlist;

protected:
    PacketWaitingTableforRBVTP packetwaitinglist;

    double HoldingIndex ;
    double recPow;
    simtime_t RDliftime;
    simtime_t RRliftime;
    simtime_t RUliftime;
    simtime_t DATAliftime;
    simtime_t nextCPtimer;
    cMessage * CPTimer;
    cMessage * WaitingCTSTimer;

    double Tmax;
    double dopt;
    double dmax;

    double a1;
    double a2;
    double a3;

    DelayPacketTableforRBVTP delayPacketlist;
    IInterfaceTable *interfaceTable;
    SearchTable mysearchedtable;
    ConnectionTable myconnectionTable;
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
     std::vector<string> RSTSeenlist;

    virtual Result datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);
    virtual Result datagramForwardHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){ return ACCEPT; }
    virtual Result datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) ;
    virtual Result datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry){ return ACCEPT; }
    virtual Result datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);

    int squmCP;
    int squmRR;
    int squmRU;
    int squmRTS;
    int squmDATA;
    std::string  oldroadID;
    virtual void receiveChangeNotification(int category, const cObject *details);
    RBVTPPacket * createCPPacket(std::string scrconn,  std::string desconn,  std::string packetname);
    RBVTPPacket *createCTSPacket(RBVTPPacket *rbvtpPacket);
    RBVTPPacket * createRTSPacket(RBVTPPacket *rbvtpPacket);

    void processCPTimer(simtime_t timer);
    std::vector<std::string> getConnOfRoad(std::string road);
   // std::vector<std::vector<std::string>> RoutingTable;

    Coord  getConnectPosition(std::string conn);
    std::vector<std::string>  getConnections(std::string srcconn);
    void BroadcastRTS(RBVTPPacket * rbvtpPacket);
    void processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo);
    void processRTSPACKET(RBVTPPacket * rbvtpPacket);
    void processCTSPACKET(RBVTPPacket * rbvtrPacket);
    simtime_t CaculateHoldTime(Coord srcPosition,Coord desPosition);
    simtime_t CaculateHoldTime(Coord srcPosition);
    void scheduleReBoardcastTimer(simtime_t holdingtime,RBVTPPacket *rbvtpPacket,IPv4Datagram * datagram);
    void clearMessage(cMessage * message,RBVTPPacket *rbvtpPacket);
    double CaculateF(double distence);
    void sendQueuePacket(const IPvXAddress& target,std::vector<std::string> roads,const IPvXAddress nexthop);

    bool  isRoadOfConn(std::string road,std::string conn);
};

#endif /* RBVTRPACKET_H_ */
