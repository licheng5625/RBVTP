/*
 * RBVTPPacket.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: chengli
 */

#include <RBVTP/RBVTP.h>
#define RBVTP_EV EV << "RBVTP at " << getHostName() << " "
#include "TraCIMobility.h"
#include "Radio80211aControlInfo_m.h"

Define_Module(RBVTP);

RBVTP::RBVTP(){
    interfaceTable=NULL;
}

RBVTP::~RBVTP(){

}

void RBVTP::initialize( int stage){
    RouteInterface::initialize(stage);

    if (stage == 0)
    {
        std::string myroad=getRoadID();
        EV_LOG("RBVTP",getHostName()+"road "+myroad);
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);
        squmCP=0;
        squmRR=0;
        squmRU=0;
        squmDATA=0;
        squmRTS=0;
        nextCPtimer=15;
        HoldingIndex = par("HoldingIndex");
        CPliftime= par("CPliftime");
        RTSTimeout= par("RTSTimeout");
        RRliftime= par("RRliftime");
       // DATAliftime= par("DATAliftime");
       // nextRUtimer= par("nextRUtimer");
      //  RUliftime= par("RUliftime");
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));
        CPTimer = new cMessage("CPTimer");
        Tmax= par("Tmax");;
        dopt= par("dopt");;
        dmax= par("dmax");;

        a1= par("a1");;
        a2= par("a2");;
        a3= par("a3");;
        //reBoardcastRDTimer = new cMessage("ReBoardcastRDTimer");;
    }else{
    if (stage == 4)
        {
        oldroadID=getRoadID();
        RouteInterface::configureInterfaces(par("interfaces"));
        EV_LOG("RBVTP",getHostName());
        std::list<std::string> interjections=tracimanager->commandGetJunctionIds();
       /* for(std::list<std::string>::iterator iter=interjections.begin();iter!=interjections.end();++iter)
        {
            RBVTP_EV<<*iter<<"   "<<tracimanager->commandGetJunctionPosition(*iter)<<endl;
        }*/
        initConnctionsTable();
        nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
        scheduleAt(simTime() + nextCPtimer, CPTimer);
        }
    }
 }
void RBVTP::initConnctionsTable()
{
    std::list<std::string> roadsname=tracimanager->commandGetLaneIds();
    for(std::list<std::string>::iterator iter=roadsname.begin();iter!=roadsname.end();++iter)
    {
        std::string templane=*iter;
        if(templane.length()==10)
        {
        //RBVTP_EV<<*iter<<endl;
        templane=templane.substr(0,8);
         myconnectionTable.addConnection(getConnOfRoad(templane)[0],getConnOfRoad(templane)[1]);
         EV_LOG("RBVTP","add conn to static   :"+getConnOfRoad(templane)[0]+"  to  "+getConnOfRoad(templane)[1]);

         staticConnectionTable.addConnection(getConnOfRoad(templane)[0],getConnOfRoad(templane)[1]);
        //RBVTP_EV<<templane.substr(5,7)<<endl;
        }
    }
}

void RBVTP::receiveChangeNotification(int category, const cObject *details)
{
    EV_LOG("RBVTP","receiveChangeNotification");
    Enter_Method("receiveChangeNotification");
    if (category == NF_LINK_FULL_PROMISCUOUS)
      {
          cPacket * pk = dynamic_cast<cPacket*>(  const_cast<cObject *>(details) );
          if (pk)
          {
             Radio80211aControlInfo * cinfo = dynamic_cast<Radio80211aControlInfo*>(pk->getControlInfo());
             if (cinfo)
             {
                recPow = cinfo->getRecPow();
             }
          }
      }
}
Coord RBVTP::getConnectPosition(std::string conn)
{
    return tracimanager->commandGetJunctionPosition(conn);
}
void RBVTP::processSelfMessage(cMessage * message)
{
       // case RTSTimeOutTimer:
        //    processRTSTimeOutTimer(nextCPtimer);
        //    break;

    if (message == CPTimer)
        {
            processCPTimer(nextCPtimer);
            return ;
        }
    if (message == RTSTimeOutTimer)
       {
           processRTSTimeOutTimer(message);
           return ;
       }
    RBVTPPacket* mypacket=NULL;
    mypacket =check_and_cast<RBVTPPacket *>(packetwaitinglist.getcPacket(message));
   if (mypacket!=NULL)
   {
       processWaitingTimer(message,mypacket,packetwaitinglist.getDataPacket(message));
       clearMessage(message,mypacket);
   }

}

void RBVTP::processWaitingTimer(cMessage * message,RBVTPPacket *rbvtpPacket,const IPv4Datagram * pakcet)
{
    EV_LOG("RBVTR", "processRDTimer" );
    if(rbvtpPacket->getPacketType()==RBVTP_CTS)
    {
        MulticastRIPacket(rbvtpPacket);
    }else
    {
        if(pakcet!=NULL)
        {
          networkProtocol->reinjectQueuedDatagram( const_cast<const IPv4Datagram *>(pakcet));
        }
        else
        {
            EV_LOG("RBVTR","I got a NULL");
            throw cRuntimeError("I got a NULL packet");
        }
    }
}
std::vector<std::string>  RBVTP::getConnections(std::string srcconn)
{
    return myconnectionTable.getConnections(srcconn);
}
void RBVTP::processCPTimer(simtime_t timer)
{
    EV_LOG("RBVTP", "processCPTimer" );
    if(getHostName()=="host[3]"){
        RBVTPPacket *rbvtpPacket=createCPPacket(getRoadID(),getConnOfRoad(getRoadID())[0] ,getHostName());
        //rbvtpPacket->addjournal(getRoadID());
        sendRIPacket(rbvtpPacket,rbvtpPacket->getdesAddress(),255,0);
        EV_LOG("RBVTP", "Scheduling RU timer" );
        scheduleAt(simTime() + timer, CPTimer);
    }
    else
    {
        EV_LOG("RBVTP", "processCPTimer2" );
    }
 }

void RBVTP::processRTSTimeOutTimer(cMessage* timer)
{
    EV_LOG("RBVTP", "processRTSTimeOutTimer" );
    cout<<timer<<endl;
    RBVTPPacket *rbvtpPacket=(check_and_cast<RBVTPPacket *>(RTSpacketwaitinglist.getcPacket(timer)))->dup();
    RBVTP_EV<<"getjournal size: "<<rbvtpPacket->getjournal().size()<<endl;
    EV<<rbvtpPacket->getjournal().size()<<endl;
    for (int i=0;i<rbvtpPacket->getjournal().size();i++)
    {
        EV_LOG("RBVTP", rbvtpPacket->getjournal()[i]);
    }

    //rbvtpPacket->getlastjournal();
    Connectstate conn(Unreachable);
    rbvtpPacket->thisConnectionTable.addTwoWayConnection(rbvtpPacket->getsrcconn(),rbvtpPacket->getdesconn(),conn);
    std::string nexthopconn="";
    std::string srcconn=rbvtpPacket->getsrcconn();
    nexthopconn=findnextConn(srcconn,  rbvtpPacket->thisConnectionTable);
   if(nexthopconn==srcconn)
   {
       if(srcconn==rbvtpPacket->getjournal().front()&&rbvtpPacket->getjournal().size()==1)
       {
           RBVTP_EV<<"end of CP"<<endl;
           return ;
       }
       else
       {
          nexthopconn=rbvtpPacket->getlastjournal();
       }
    }else
    {
       rbvtpPacket->addjournal(nexthopconn);
    }
   rbvtpPacket->setdesconn(nexthopconn);
   if(nexthopconn.size()==3)
   {
       rbvtpPacket->setdesPosition(getConnectPosition(nexthopconn));
   }
   else
   {
       rbvtpPacket->setdesPosition(rbvtpPacket->getscrPosition());
   }
   RBVTP_EV<<"send CP to"<<nexthopconn<<endl;
   std::cout<<"send CP to"<<nexthopconn<<endl;
   sendRIPacket(rbvtpPacket,rbvtpPacket->getdesAddress(),255,0);
  }

INetfilter::IHook::Result RBVTP::datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){
     EV_LOG("RBVTP","datagramPreRoutingHook");
     Enter_Method("datagramPreRoutingHook");
      return ACCEPT;
}
INetfilter::IHook::Result RBVTP::datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop)
{
    EV_LOG("RBVTR","datagramPostRoutingHook");
       const IPv4Address & destination = datagram->getDestAddress();

       if (destination.isMulticast() || destination.isLimitedBroadcastAddress()|| routingTable->isLocalAddress(destination))
                  return ACCEPT;
       else{
           RBVTPPacket * rbvtppacket=check_and_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->getEncapsulatedPacket());
           if (rbvtppacket)
           {
               if(rbvtppacket->getPacketType()==RBVTP_CP)
               {
                   rbvtppacket->setdesAddress(IPv4Address::UNSPECIFIED_ADDRESS);
               }
                nextHop=rbvtppacket->getnexthopAddress().get4();
           }else
           {
            nextHop=IPv4Address::ALLONES_ADDRESS;
           }
       }
       return ACCEPT;
}
INetfilter::IHook::Result RBVTP::datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) {
       EV_LOG("RBVTP","datagramLocalOutHook");
       Enter_Method("datagramLocalOutHook");
       const IPv4Address & destination = datagram->getDestAddress();
       if (destination.isMulticast() || destination.isLimitedBroadcastAddress()|| routingTable->isLocalAddress(destination))
       {
           EV_LOG("RBVTP","Multicast");
           return ACCEPT;
       }
           else {
               EV_LOG("RBVTP","check rbvtppacket");

               RBVTPPacket * rbvtppacket=check_and_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->getEncapsulatedPacket());
               if ( rbvtppacket !=NULL)
               {
                   EV_LOG("RBVTP","rbvtppacket not null");
                   if(rbvtppacket->getPacketType()==RBVTP_CP)
                   {
                       RBVTPPacket * rtspacket=BroadcastRTS(rbvtppacket);
                       RTSTimeOutTimer = new cMessage("RTSTimeOutTimer");
                       RTSpacketwaitinglist.addPacket(RTSTimeOutTimer,rbvtppacket,NULL);
                       scheduleAt(simTime() + RTSTimeout, RTSTimeOutTimer);
                       // return QUEUE;
                   }
                  //  if (routingRoad.hasRoadTable(destination))
                 //  {
                    // RBVTP_EV << "Sending datagram: source " << datagram->getSrcAddress() << ", destination " << datagram->getDestAddress() << endl;
                  //  cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
                   // rbvtpPacket * rbvtrdataPacket = createDataPacket(datagram->getDestAddress(), networkPacket->decapsulate());
                   // rbvtrdataPacket->setroads(routingRoad.getRoadTable(destination));
                   // rbvtrdataPacket->setdesPosition(routingRoad.getPositionTable(datagram->getDestAddress()));
                   // networkPacket->encapsulate(rbvtrdataPacket);
                    //DATASeenlist.SeePacket(rbvtrdataPacket->getsrcAddress(), rbvtrdataPacket->getSeqnum());
                    //nextHop=IPv4Address::ALLONES_ADDRESS;
                    // BroadcastRTS(rbvtrdataPacket);
                       //return ACCEPT;
                   /* }
                   else{
                    RBVTP_EV << "No raod in table :"<<datagram->getDestAddress() <<endl;
                    return DROP;
                    }*/
               }else
               {
                   EV_LOG("RBVTP","rbvtppacket is null");

                   //RBVTPPacket * rbvtpPacket = dynamic_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->decapsulate());
                  // BroadcastRTS(rbvtpPacket);
               }
           }
        delayPacketlist.addPacket(destination,datagram);
       return  QUEUE;
}
void RBVTP::processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo)
{
    RBVTPPacket *rbvtpPacket = dynamic_cast<RBVTPPacket *>(ctrlPacket);
    if(rbvtpPacket->getLifetime()<simTime())
    {
        EV_LOG("RBVTR","Time out droped");
        return;
    }
     switch( rbvtpPacket->getPacketType())
    {
     case RBVTP_DATA:
               EV_LOG("RBVTR","Process RBVTP_DATA");
               //processRDPACKET(rbvtpPacket);
               break;
     case RBVTP_CP:
               EV_LOG("RBVTR","Process RBVTP_CP");
               processCPPACKET(rbvtpPacket);
               break;
     case RBVTP_RTS:
                EV_LOG("RBVTR","Process RBVTP_RTS");
                processRTSPACKET(rbvtpPacket);
                break;
     case RBVTP_CTS:
                EV_LOG("RBVTR","Process RBVTP_CTS");
                processCTSPACKET(rbvtpPacket);
                break;

     case RBVTP_RU:
               EV_LOG("RBVTR","Process RBVTP_RU");
              // processRUPACKET(rbvtpPacket);
               break;
     default :
         throw cRuntimeError("Unknown packet type");
    }
}

RBVTPPacket * RBVTP::BroadcastRTS(RBVTPPacket * rbvtpPacket)
{
   EV_LOG("RBVTP","BroadcastRTS");
   RBVTPPacket *RTSPacket=createRTSPacket(rbvtpPacket);
   std::string name=RTSPacket->getName();
   RSTSeenlist.push_back(name);
   MulticastRIPacket(RTSPacket);
   return RTSPacket;
   //RTSlist.addPacket(RTSPacket->getSeqnum(),rbvtpPacket,NULL);
}
RBVTPPacket *RBVTP::createCPPacket(std::string scrconn,  std::string desconn,  std::string packetname)
{
    stringstream ss;
    std::string packettype="CP_";
    ss<<packettype<<packetname<<"_"<<squmCP;
     RBVTPPacket * rBVTPPacket = new RBVTPPacket(ss.str().c_str());
    //std::cout <<"rBVTPPacket-getName = " << rBVTPPacket(strcat("RD_",content->getName())) << endl;
    rBVTPPacket->setsrcAddress(getSelfIPAddress());
    rBVTPPacket->setRBVTPPacketType(RBVTP_CP);
    rBVTPPacket->setSeqnum(squmCP);
    rBVTPPacket->setdesAddress(IPv4Address::UNSPECIFIED_ADDRESS);
    rBVTPPacket->setscrPosition(getSelfPosition());
    rBVTPPacket->setdesPosition(getConnectPosition(desconn));
    rBVTPPacket->setBitLength(rBVTPPacket->getPacketlength());
    rBVTPPacket->addroad(getRoadID());
    rBVTPPacket->setdesconn(desconn);
    rBVTPPacket->setsrcconn(scrconn);
    rBVTPPacket->setLifetime(simTime()+CPliftime);
    rBVTPPacket->addjournal(scrconn);
    squmCP++;
   // rBVTPPacket->encapsulate(content);
    return rBVTPPacket;
}

void RBVTP::processRTSPACKET(RBVTPPacket * rbvtpPacket)
{
    if(rbvtpPacket->getsrcAddress()!=getSelfIPAddress())
    {
          std::vector <std::string> routingroad=rbvtpPacket->getroads();
          std::string name=rbvtpPacket->getName();
          RBVTP_EV<<"RTS:"<<name<<endl;

         /* for(int i=0;i<RSTSeenlist.size();i++)
          {
              RBVTP_EV<<"RTSSEEN:"<<RSTSeenlist[i]<<endl;
          }*/
          RBVTP_EV<<"myRoadia"<<getRoadID()<<"    desconn::"<<rbvtpPacket->getdesconn()<<"  "<<isRoadOfConn(getRoadID(),rbvtpPacket->getdesconn())<<endl;
          if((isRoadOfConn(getRoadID(),rbvtpPacket->getdesconn()))&&(std::find(RSTSeenlist.begin(),RSTSeenlist.end(),(name))==RSTSeenlist.end()))
          {
                RBVTP_EV<<"got RTS IP:"<<rbvtpPacket->getsrcAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
                RBVTPPacket *ctspacket=  createCTSPacket( rbvtpPacket);
                scheduleReBoardcastTimer(CaculateHoldTime(ctspacket->getscrPosition(),ctspacket->getdesPosition()),ctspacket,NULL);
                RSTSeenlist.push_back(name);
          }
    }
}
void RBVTP::processCTSPACKET(RBVTPPacket * rbvtpPacket)
{
    if(rbvtpPacket->getdesAddress()!=getSelfIPAddress())
      {
        cMessage *mymsg=NULL;
        mymsg=packetwaitinglist.findPacket(rbvtpPacket);
        std::string name=rbvtpPacket->getName();
        //RSTSeenlist.push_back(name.substr(3));
         if(mymsg!=NULL)
         {
          clearMessage(mymsg,rbvtpPacket);
          RBVTP_EV<<"cancel ctspacket IP:"<<rbvtpPacket->getdesAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
         }
      }
    else
      {
        string packetname=rbvtpPacket->getName();
        packetname=packetname.substr(4);
        packetname=packetname.substr(0,packetname.rfind("_"));
        RBVTP_EV<<"packetname"<<packetname<<endl;
        if(packetname.find("CP")!=-1)
        {
           cMessage* timer= RTSpacketwaitinglist.findPacket(packetname);
           if (timer!=NULL)
           {
               cancelAndDelete(timer);
           }
           else
           {
               throw cRuntimeError("can't find RTSWaiting Timer");
           }
        }
        RBVTP_EV<<"send packets IP:"<<rbvtpPacket->getsrcAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
        sendQueuePacket((rbvtpPacket->getnexthopAddress()),rbvtpPacket->getroads(),rbvtpPacket->getsrcAddress());
      }
}

void RBVTP::processCPPACKET(RBVTPPacket * rbvtpPacket)
{
    double distence =(getConnectPosition(rbvtpPacket->getdesconn())-getSelfPosition()).length();
    rbvtpPacket->nexthop_ip=IPv4Address::UNSPECIFIED_ADDRESS;
    std::string srcconn=rbvtpPacket->getdesconn();
    string nexthopconn;
    if(distence<300)// close enough to the intersection
      {
        RBVTP_EV<<"close to "<<srcconn<<"   "<<distence<<endl;
        RBVTP_EV<<"add journal "<<nexthopconn<<endl;
        rbvtpPacket->addjournal(srcconn);
       // std::vector <std::string> desconnlist=getConnections(srcconn);
        Connectstate conn(Reachable);
        rbvtpPacket->thisConnectionTable.addTwoWayConnection(rbvtpPacket->getsrcconn(),rbvtpPacket->getdesconn(),conn);
        nexthopconn=findnextConn(srcconn,  rbvtpPacket->thisConnectionTable);
        if(nexthopconn==srcconn)
        {
            if(srcconn==rbvtpPacket->getjournal().front()&&rbvtpPacket->getjournal().size()==1)
            {
                RBVTP_EV<<"end of CP"<<endl;
                return ;
            }else
            {
                nexthopconn=rbvtpPacket->getlastjournal();
            }
        }else
        {
            RBVTP_EV<<"add journal "<<nexthopconn<<endl;
            rbvtpPacket->addjournal(nexthopconn);
            for(int j=0;j<(rbvtpPacket->getjournal().size());j++)
            {
                RBVTP_EV<<" journal: "<<rbvtpPacket->getjournal()[j]<<endl;
            }
        }
      }
    else
      {
        nexthopconn=srcconn;
      }
    rbvtpPacket->setdesconn(nexthopconn);
    sendRIPacket(rbvtpPacket,rbvtpPacket->getdesAddress(),255,0);
}

std::string RBVTP::findnextConn(std::string srcconn,ConnectionTable myconnectionTable)
{
    std::vector<std::string> listofnextconn=staticConnectionTable.getConnections(srcconn);
    if(listofnextconn.size()==0) //srcconn if is a virtual connection
    {
        RBVTP_EV<<srcconn<<" has no nexthop!"<<endl;
        return srcconn;
    }
    std::vector<std::string> listofcheckednextconn=myconnectionTable.getConnections(srcconn);
    std::string desconn="";
    for (int i=0;i<listofnextconn.size();i++)
       {
        RBVTP_EV<<srcconn<<" to nexthop:  "<<listofnextconn[i]<<endl;
       }
    for (int i=0;i<listofnextconn.size();i++)
    {
        if(std::find(listofcheckednextconn.begin(),listofcheckednextconn.end(),listofnextconn[i])==listofcheckednextconn.end())
            {//find a link has never checked
                if(desconn=="")
                {
                    desconn=listofnextconn[i];
                    break;
                }
            }
    }
    if(desconn!="")
    {
        return desconn;
    }
    else
    {
       return srcconn;
    }
 }
INetfilter::IHook::Result RBVTP::datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry)
{
    EV_LOG("RBVTR","datagramLocalInHook");
    UDPPacket *udpPacket = dynamic_cast<UDPPacket *>(datagram->getEncapsulatedPacket());

    // check_and_cast<cPacket *>(udpPacket->decapsulate());
//    cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
//    RBVTPPacket * rbvtppacket=dynamic_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->getEncapsulatedPacket());
//    if (rbvtppacket&&rbvtppacket->getPacketType()==RBVTP_DATA) {
//        UDPPacket *  udppacket= dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket());
//
//        networkPacket->decapsulate();
//        networkPacket->encapsulate(udppacket->decapsulate());
//        delete udppacket;
//    }
    return ACCEPT;
}

void RBVTP::sendQueuePacket(const IPvXAddress& target,std::vector<std::string> roads,const IPvXAddress nexthop)
{
    RBVTP_EV << "Completing route discovery, originator " << getSelfIPAddress() << ", target " << target <<", nexthop: " <<nexthop<< endl;
    std::multimap<IPvXAddress, IPv4Datagram *>::iterator lt = delayPacketlist.getlowbound(target);
    std::multimap<IPvXAddress, IPv4Datagram *>::iterator ut = delayPacketlist.getupperbound(target);
    RBVTPPacket * rbvtrdataPacket;
    // reinject the delayed datagrams
    for (std::multimap<IPvXAddress, IPv4Datagram *>::iterator it = lt; it != ut; it++) {
        IPv4Datagram *datagram = it->second;
        RBVTPPacket * rbvtppacket=check_and_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->getEncapsulatedPacket());

        if (rbvtppacket==NULL)
        {
         cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
         RBVTP_EV << "Sending queued datagram: source " << datagram->getSrcAddress() << ", destination " << datagram->getDestAddress()<<", name: " << networkPacket->getName() << endl;
         //rbvtrdataPacket= createDataPacket(datagram->getDestAddress(), networkPacket->decapsulate());
        // rbvtrdataPacket->setroads(roads);
        // rbvtrdataPacket->setdesPosition(routingRoad.getPositionTable(datagram->getDestAddress()));
        // rbvtrdataPacket->setnexthopAddress(nexthop);
        // networkPacket->encapsulate(rbvtrdataPacket);
        }else
        {
            RBVTP_EV << "Sending queued RBVTP: source " << rbvtppacket->getsrcAddress() << ", destination " << rbvtppacket->getdesAddress()<<", name: " << rbvtppacket->getName()<<", nexthop: "<<nexthop << endl;
            std::cout << "Sending queued RBVTP: source " << rbvtppacket->getsrcAddress() << ", destination " << rbvtppacket->getdesAddress()<<", name: " << rbvtppacket->getName()<<", nexthop: "<<nexthop << endl;

            //UDPPacket * rbvtppascket=check_and_cast<UDPPacket *>(rbvtppascket);
           // rbvtrdataPacket->setdesAddress(nexthop);
             rbvtrdataPacket->nexthop_ip=nexthop;
         }
         // DATASeenlist.SeePacket(rbvtrdataPacket->getsrcAddress(), rbvtrdataPacket->getSeqnum());
        RBVTP_EV << "Sending queued RBVTP: source " << rbvtppacket->getsrcAddress() << ", destination " << rbvtppacket->getdesAddress()<<", name: " << rbvtppacket->getName()<<", nexthop: "<<nexthop << endl;
        std::cout << "Sending queued RBVTP: source " << rbvtppacket->getsrcAddress() << ", destination " << rbvtppacket->getdesAddress()<<", name: " << rbvtppacket->getName()<<", nexthop: "<<nexthop << endl;
        datagram->setDestAddress(nexthop.get4());
        networkProtocol->reinjectQueuedDatagram( const_cast<const IPv4Datagram *>(datagram));
    }
    delayPacketlist.removePacket(target);
}
RBVTPPacket *RBVTP::createRTSPacket(RBVTPPacket *rbvtpPacket)
{
    EV_LOG("RBVTP","createRTSPacket");
    std::string packetname=rbvtpPacket->getName();
  //  std::string packettype="RD_";
    //packetname=packetname.replace(packetname.begin(),packetname.begin()+2,"RTS");
    packetname="RTS_"+packetname+"_"+std::to_string(squmRTS++);
    RBVTPPacket * RTSPacket = new RBVTPPacket(packetname.c_str());
    //std::cout <<"rbvtpPacket-getName = " << rbvtpPacket(strcat("RD_",content->getName())) << endl;
    RTSPacket->setsrcAddress(getSelfIPAddress());
    RTSPacket->setdesAddress(rbvtpPacket->getdesAddress());
    RTSPacket->setdesPosition(rbvtpPacket->getdesPosition());
    EV<<"Create RTS : scr: "<<RTSPacket->getsrcAddress()<<" des: "<<RTSPacket->getdesAddress()<<endl;
    RTSPacket->setRBVTPPacketType(RBVTP_RTS);
    RTSPacket->setSeqnum(squmRTS);
    RTSPacket->setscrPosition(getSelfPosition());
    RTSPacket->setdesPosition(rbvtpPacket->getdesPosition());
    RTSPacket->setdesconn(rbvtpPacket->getdesconn());
    RTSPacket->setroads(rbvtpPacket->getroads());
    RTSPacket->setBitLength(RTSPacket->getPacketlength());
    RTSPacket->setLifetime(simTime()+RRliftime);
    return RTSPacket;
}
RBVTPPacket *RBVTP::createCTSPacket(RBVTPPacket *rbvtpPacket)
{
    std::string packetname=rbvtpPacket->getName();
  //  std::string packettype="RD_";
    packetname=packetname.replace(packetname.begin(),packetname.begin()+1,"C");
    RBVTPPacket * CTSPacket = new RBVTPPacket(packetname.c_str());
    CTSPacket->setsrcAddress(getSelfIPAddress());
    CTSPacket->setnexthopAddress(rbvtpPacket->getdesAddress());
    CTSPacket->setdesAddress(rbvtpPacket->getsrcAddress());
    CTSPacket->setdesPosition(rbvtpPacket->getdesPosition());
    EV<<"Create CTS : scr: "<<CTSPacket->getsrcAddress()<<" des: "<<CTSPacket->getdesAddress()<<endl;
    CTSPacket->setRBVTPPacketType(RBVTP_CTS);
    CTSPacket->setSeqnum(rbvtpPacket->getSeqnum());
    CTSPacket->setscrPosition(rbvtpPacket->getscrPosition());
    CTSPacket->setroads(rbvtpPacket->getroads());
    CTSPacket->setBitLength(CTSPacket->getPacketlength());
    CTSPacket->setLifetime(simTime()+RRliftime);
   // rBVTRPacket->encapsulate(content);
    return CTSPacket;
}
void RBVTP::clearMessage(cMessage * message,RBVTPPacket *rbvtpPacket)
{
     packetwaitinglist.removePacket(message);
     cancelAndDelete(message);
     networkProtocol->dropQueuedDatagram(dynamic_cast<const IPv4Datagram *>(rbvtpPacket));
 }

simtime_t RBVTP::CaculateHoldTime(Coord srcPosition)
{
    return 1/(srcPosition - getSelfPosition()).length()*2;
}
simtime_t RBVTP::CaculateHoldTime(Coord srcPosition,Coord desPosition)
{
    double distence=0;
     if (desPosition==Coord(0,0,0))
     {
         distence =  (srcPosition - getSelfPosition()).length();

     }else
     {
        distence = (srcPosition-desPosition).length()-(desPosition - getSelfPosition()).length();
        if(distence<0)
            distence=dmax;
     }
    double parmax=(pow(dmax,a1)*pow(CaculateF(dopt),a2)*pow(0.2,a3));
    double A=-Tmax/parmax;
    double f=CaculateF(distence);
    double p=recPow;
    RBVTP_EV<<"A  "<<A<<endl;
    double ss= pow(distence,a1)*pow(f,a2)*pow(p,a3);
    RBVTP_EV<<" f "<<f <<" p "<<p<<" ss "<<ss<<" parmax  "<<parmax<<endl;

   // simtime_t result=A*pow(distence,a1)*pow(f,a2)*pow(p,a3);//+Tmax;
   // RBVTP_EV<<"wait "<<result<<"  seconds with distence "<<distence<<endl;
    simtime_t result=A*pow(distence,a1)*pow(f,a2)*pow(p,a3)+Tmax;
    RBVTP_EV<<"wait "<<result<<"  seconds with distence "<<distence<<endl;

    return result;
 }
double  RBVTP::CaculateF(double distence)
{
    double dtrans=dmax-dopt;
    RBVTP_EV<<" distence "<<distence<<endl;
   if(dmax<distence)
       distence=dmax-1;
   if(distence<=dopt)
   {
       return distence+dtrans;
   }
   else
   {
       return dmax- distence;
   }
}
void RBVTP::scheduleReBoardcastTimer(simtime_t holdingtime,RBVTPPacket *rbvtpPacket,IPv4Datagram * datagram)
{
    EV_LOG("RBVTR", "Scheduling ReBoardcast timer" );
    std::string timername="reBoardcastRDTimer_";
    if (rbvtpPacket->getPacketType() ==RBVTP_RU)
    {
     timername="reBoardcastRUTimer_";
    }
    else
    {
        if (rbvtpPacket->getPacketType() ==RBVTP_DATA)
            {
            timername="reBoardcastDATATimer_";
            }
    }
    timername=timername+rbvtpPacket->getName();
    EV_LOG("RBVTR", timername);
    cMessage * reBoardcastTimer = new cMessage(timername.c_str());
    packetwaitinglist.addPacket(reBoardcastTimer,rbvtpPacket,datagram);
    scheduleAt(simTime() + holdingtime, reBoardcastTimer);
}

