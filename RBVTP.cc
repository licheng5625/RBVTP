/*
 * RBVTPPacket.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: chengli
 */

#include <RBVTP/RBVTP.h>
#define RBVTP_EV EV << "RBVTP at " << getHostName() << " "
#define LOG_EV inFile <<"#"<<EventNumber()<<"  "<<simTime()<< " " << getHostName() << " "
#include "TraCIMobility.h"
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
        EV_LOG(getHostName()+"road "+myroad);
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);
        squmCP=0;
        squmRR=0;
        squmRU=0;
        squmDATA=0;
        squmRTS=0;
        nextCPtimer=0.5;
        distenceOfclose = par("distenceOfclose");
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
        EV_LOG(getHostName());
        std::list<std::string> interjections=tracimanager->commandGetJunctionIds();
       /* for(std::list<std::string>::iterator iter=interjections.begin();iter!=interjections.end();++iter)
        {
            RBVTP_EV<<*iter<<"   "<<tracimanager->commandGetJunctionPosition(*iter)<<endl;
        }*/
        globalPositionTable.setHostName(getSelfIPAddress(),getHostName());
        initConnctionsTable();
        nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
        scheduleAt(simTime()+3 , CPTimer);
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
         myconnectionTable.addConnection(getJunctionsOfRoad(templane)[0],getJunctionsOfRoad(templane)[1]);
         EV_LOG("add conn to static   :"+getJunctionsOfRoad(templane)[0]+"  to  "+getJunctionsOfRoad(templane)[1]);

         staticConnectionTable.addConnection(getJunctionsOfRoad(templane)[0],getJunctionsOfRoad(templane)[1]);
        //RBVTP_EV<<templane.substr(5,7)<<endl;
        }
    }
   // staticConnectionTable.addTwoWayConnection(getRoadID(),getJunctionsOfRoad(getRoadID())[0]);
   // staticConnectionTable.addTwoWayConnection(getRoadID(),getJunctionsOfRoad(getRoadID())[1]);
}

void RBVTP::processSelfMessage(cMessage * message)
{
       // case RTSTimeOutTimer:
        //    processRTStTimer(nextCPtimer);
        //    break;
    EV_LOG( "processSelfMessage" );
    RBVTP_EV<<"processSelfMessage  "<<message->getName()<<endl;

    if (message == CPTimer)
        {
            EV_LOG( "process  CPTimer" );
            processCPTimer(nextCPtimer);
            return ;
        }
    if (std::string(message->getName()) == "CPRTSTimeOutTimer")
       {
           processRTSTimeOutTimer(message);
           return ;
       }
    RBVTPPacket* mypacket=NULL;
    mypacket =check_and_cast<RBVTPPacket *>(packetwaitinglist.getcPacket(message));
   if (mypacket!=NULL)
   {
       processWaitingTimer(message,mypacket,packetwaitinglist.getDataPacket(message));
       clearMessage(message,mypacket,packetwaitinglist);
   }

}

void RBVTP::processWaitingTimer(cMessage * message,RBVTPPacket *rbvtpPacket,const IPv4Datagram * pakcet)
{
    EV_LOG( "processRDTimer" );
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
            EV_LOG("I got a NULL");
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
    EV_LOG( "processCPTimer" );
    if( rand() % 10<2&&getHostName()=="host[4]")
    {
        RBVTPPacket *rbvtpPacket;
        double distence =(getPositionOfJunction(getJunctionsOfRoad(getRoadID())[1])-getSelfPosition()).length();
        if(distence<distenceOfclose)
        {
          Connectstate conn(Reachable);
          RBVTP_EV<<"i am close to Connection "<<getJunctionsOfRoad(getRoadID())[1]<<endl;
           rbvtpPacket=createCPPacket(getRoadID(),getJunctionsOfRoad(getRoadID())[0] ,getHostName());
          rbvtpPacket->thisConnectionTable.addTwoWayConnection(getRoadID(),getJunctionsOfRoad(getRoadID())[1],conn);
          RBVTP_EV<<"add Connection "<<getRoadID()<<" to "<<getJunctionsOfRoad(getRoadID())[1]<<" Reachable "<<endl;

        }else
        {
            double distence =(getPositionOfJunction(getJunctionsOfRoad(getRoadID())[0])-getSelfPosition()).length();
            rbvtpPacket=createCPPacket(getRoadID(),getJunctionsOfRoad(getRoadID())[1] ,getHostName());
           if(distence<distenceOfclose)
           {
             Connectstate conn(Reachable);
             rbvtpPacket->thisConnectionTable.addTwoWayConnection(getRoadID(),getJunctionsOfRoad(getRoadID())[0],conn);
             RBVTP_EV<<"i am close to Connection "<<getJunctionsOfRoad(getRoadID())[0]<<endl;
             RBVTP_EV<<"add Connection "<<getRoadID()<<" to "<<getJunctionsOfRoad(getRoadID())[0]<<" Reachable "<<endl;
           }
        }
           scheduleAt(simTime() + timer, CPTimer);
    RBVTP_EV<<"send CP to"<<rbvtpPacket->getdesconn()<<endl;
    sendRIPacket(rbvtpPacket,rbvtpPacket->getdesAddress(),255,0);
    }
 }

void RBVTP::processRTSTimeOutTimer(cMessage* timer)
{
    EV_LOG( "process RTSTimeOutTimer" );
    cout<<getHostName()<<"  timer "<<timer<<endl;
    RBVTPPacket *rbvtpPacket=(check_and_cast<RBVTPPacket *>(RTSpacketwaitinglist.getcPacket(timer)))->dup();
    RBVTP_EV<<"get connections size: "<<rbvtpPacket->thisConnectionTable.getAllConnections().size()<<endl;
    for(int i =0 ;i<rbvtpPacket->thisConnectionTable.getAllConnections().size();i++)
    {
      std::pair<std::string,std::string> connes=rbvtpPacket->thisConnectionTable.getAllConnections()[i];
      string src=connes.first;
      string des=connes.second;
      RBVTP_EV<<src<<" to "<<des<<" state: "<<rbvtpPacket->thisConnectionTable.getConnectState(src,des).thisflag<<endl;
    }
    RBVTP_EV<<"getjournal size: "<<rbvtpPacket->getjournal().size()<<endl;
    cout<<"getjournal size: "<<rbvtpPacket->getjournal().size()<<endl;
    if(rbvtpPacket->getjournal().size()==0)
    {
        throw cRuntimeError("journal is empty");
    }
    for (int i=0;i<rbvtpPacket->getjournal().size();i++)
    {
        EV_LOG("journal "+std::to_string(i)+" : "+rbvtpPacket->getjournal()[i]);
    }

    //rbvtpPacket->getlastjournal();
    std::string srcconn=rbvtpPacket->getlastjournal(false);
    Connectstate conn(Unreachable);
    RBVTP_EV<<"add Connection "<<srcconn<<" to "<<rbvtpPacket->getdesconn()<<" UnReachable "<<endl;
    rbvtpPacket->thisConnectionTable.addTwoWayConnection(srcconn,rbvtpPacket->getdesconn(),conn);
    std::string nexthopconn="";
    nexthopconn=findnextConn(srcconn,  rbvtpPacket->thisConnectionTable);
    if(nexthopconn==srcconn)
    {
        srcconn=rbvtpPacket->getlastjournal(true);
    }
    if(srcconn.size()==3) //normal connection
       {
        double distence =(getPositionOfJunction(srcconn)-getSelfPosition()).length();
        if(distence<distenceOfclose)
        {
            RBVTP_EV<<"i am close to Connection "<<srcconn<<endl;
            nexthopconn=findnextConn(srcconn,  rbvtpPacket->thisConnectionTable);
        }
        else
        {
            RBVTP_EV<<"i am not close to Connection "<<srcconn<<endl;
            nexthopconn=srcconn;
        }
       // if(nexthopconn==srcconn)
        //   {
        //    nexthopconn=srcconn;
        //    }else
        //    {
              // rbvtpPacket->addjournal(nexthopconn);
        //    }
       }else//send to src
       {
           std::cout<<"send back to src "<<srcconn<<endl;
           RBVTP_EV<<"send back to src "<<srcconn<<endl;
           if(getSelfIPAddress()==rbvtpPacket->getsrcAddress().get4())
                 {
                     if(rbvtpPacket->thisConnectionTable.getConnections(srcconn).size()==2)
                     {
                         RBVTP_EV<<"end of CP"<<endl;
                         return ;
                     }else
                     {
                       if(rbvtpPacket->thisConnectionTable.getConnections(srcconn)[0]==getJunctionsOfRoad(srcconn)[0])
                       {
                           nexthopconn=   getJunctionsOfRoad(srcconn)[0];
                       }else
                       {
                           nexthopconn=   getJunctionsOfRoad(srcconn)[1];
                       }
                     }
                 }
                 else
                 {
                    nexthopconn=srcconn;
                 }
            std::cout<<"add Connection "<<getJunctionsOfRoad(srcconn)[0]<<" to "<<getJunctionsOfRoad(srcconn)[1]<<" UnReachable "<<endl;
            Connectstate conn(Unreachable);
            rbvtpPacket->thisConnectionTable.addTwoWayConnection(getJunctionsOfRoad(srcconn)[0],getJunctionsOfRoad(srcconn)[1],conn);
            RBVTP_EV<<"add Connection "<<getJunctionsOfRoad(srcconn)[0]<<" to "<<getJunctionsOfRoad(srcconn)[1]<<" UnReachable "<<endl;
       }

   rbvtpPacket->setdesconn(nexthopconn);
   if(nexthopconn.size()==3)
   {
       rbvtpPacket->setdesPosition(getPositionOfJunction(nexthopconn));
   }
   else
   {
       rbvtpPacket->setdesPosition(rbvtpPacket->getscrPosition());
   }
    string packetname=rbvtpPacket->getName();
    RBVTP_EV<<"delete packet "<<packetname<<endl;
   delayPacketlist.removePacket(rbvtpPacket->getdesAddress(),packetname);
   cout<<"delete packet "<<packetname<<endl;

   RTSpacketwaitinglist.removePacket(timer);
   cout<<"delete timer "<<packetname<<endl;

   RBVTP_EV<<"send CP to"<<rbvtpPacket->getdesconn()<<endl;
   std::cout<<"send CP to"<<rbvtpPacket->getdesconn()<<endl;
   rbvtpPacket->setLifetime(simTime()+CPliftime);
   rbvtpPacket->setVersion();
   rbvtpPacket->setlastsenderAddress(getSelfIPAddress());
   if(rbvtpPacket->getdesconn().size()!=3&&rbvtpPacket->getsrcAddress().get4()==getSelfIPAddress())
   {
       sendRIPacket(rbvtpPacket,IPv4Address::LOOPBACK_ADDRESS,255,0);
   }
   sendRIPacket(rbvtpPacket,rbvtpPacket->getdesAddress(),255,0);
  }

INetfilter::IHook::Result RBVTP::datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){
     EV_LOG("datagramPreRoutingHook");
     Enter_Method("datagramPreRoutingHook");
      return ACCEPT;
}
INetfilter::IHook::Result RBVTP::datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop)
{
    EV_LOG("datagramPostRoutingHook");
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
       EV_LOG("datagramLocalOutHook");
       Enter_Method("datagramLocalOutHook");
       const IPv4Address & destination = datagram->getDestAddress();
       if (destination.isMulticast() || destination.isLimitedBroadcastAddress()|| routingTable->isLocalAddress(destination)||destination==IPv4Address::LOOPBACK_ADDRESS)
       {
           EV_LOG("Multicast");
           return ACCEPT;
       }
           else {
               EV_LOG("check rbvtppacket");

               RBVTPPacket * rbvtppacket=dynamic_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->getEncapsulatedPacket());
               if ( rbvtppacket !=NULL)
               {
                   EV_LOG("rbvtppacket not null");
                   if(rbvtppacket->getPacketType()==RBVTP_CP)
                   {
                       RBVTPPacket * rtspacket=BroadcastRTS(rbvtppacket);
                       cMessage* RTSTimeOutTimer = new cMessage("CPRTSTimeOutTimer");
                       cout<<getHostName()<<" add timer "<<RTSTimeOutTimer<<"  ";
                       printf("%p\n",RTSTimeOutTimer);
                       RTSpacketwaitinglist.addPacket(RTSTimeOutTimer,rbvtppacket,datagram);
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
                   EV_LOG("rbvtppacket is null");
                   return  DROP;
                    //RBVTPPacket * rbvtpPacket = dynamic_cast<RBVTPPacket *>( (dynamic_cast<UDPPacket *>((dynamic_cast<cPacket *>(datagram))->getEncapsulatedPacket()))->decapsulate());
                    //BroadcastRTS(rbvtpPacket);
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
        EV_LOG("Time out droped");
        return;
    }
     switch( rbvtpPacket->getPacketType())
    {
     case RBVTP_DATA:
               EV_LOG("Process RBVTP_DATA");
               //processRDPACKET(rbvtpPacket);
               break;
     case RBVTP_CP:
               EV_LOG("Process RBVTP_CP");
               processCPPACKET(rbvtpPacket);
               break;
     case RBVTP_RTS:
                EV_LOG("Process RBVTP_RTS");
                processRTSPACKET(rbvtpPacket);
                break;
     case RBVTP_CTS:
                EV_LOG("Process RBVTP_CTS");
                processCTSPACKET(rbvtpPacket);
                break;

     case RBVTP_RU:
               EV_LOG("Process RBVTP_RU");
              // processRUPACKET(rbvtpPacket);
               break;
     default :
         throw cRuntimeError("Unknown packet type");
    }
}

RBVTPPacket * RBVTP::BroadcastRTS(RBVTPPacket * rbvtpPacket)
{
   EV_LOG("BroadcastRTS");
   RBVTPPacket *RTSPacket=createRTSPacket(rbvtpPacket);
   std::string name=RTSPacket->getName();
   RSTSeenlist.push_back(RTSInfor(RTSPacket->getVersion(),RTSPacket->getSeqnum(),name,getHostName()));
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
    rBVTPPacket->setdesPosition(getPositionOfJunction(desconn));
    rBVTPPacket->setBitLength(rBVTPPacket->getPacketlength());
    rBVTPPacket->addroad(getRoadID());
    rBVTPPacket->setdesconn(desconn);
    rBVTPPacket->setsrcconn(scrconn);
    rBVTPPacket->setLifetime(simTime()+CPliftime);
    rBVTPPacket->addjournal(scrconn);
    rBVTPPacket->setVersion(0);
    rBVTPPacket->setlastsenderAddress(getSelfIPAddress());
    squmCP++;
   // rBVTPPacket->encapsulate(content);
    return rBVTPPacket;
}
std::string RBVTP::getRoadID()
{
    string myroad =RouteInterface::getRoadID();
    if(myroad.length()!=8) //if the car in the intersection get the roadID before the car go into the intersection
    {
        return oldroadID;
    }else
    {
        return myroad;
    }
}
void RBVTP::processRTSPACKET(RBVTPPacket * rbvtpPacket)
{
    EV_LOG("here");
    if(rbvtpPacket->getlastsenderAddress()!=getSelfIPAddress())
    {
          std::vector <std::string> routingroad=rbvtpPacket->getroads();
          std::string name=rbvtpPacket->getName();
          RBVTP_EV<<"RTS:"<<name<<endl;
          cout<<"RTS:"<<name<<endl;
         /* for(int i=0;i<RSTSeenlist.size();i++)
          {
              RBVTP_EV<<"RTSSEEN:"<<RSTSeenlist[i]<<endl;
          }*/
          RBVTP_EV<<"myRoadid "<<getRoadID()<<"    desconn::"<<rbvtpPacket->getdesconn()<<"  "<<isRoadOfJunction(getRoadID(),rbvtpPacket->getdesconn())<<endl;
          cout<<"myRoadia"<<getRoadID()<<"    desconn::"<<rbvtpPacket->getdesconn()<<"  "<<isRoadOfJunction(getRoadID(),rbvtpPacket->getdesconn())<<endl;

          RTSInfor myinfor=RTSInfor(rbvtpPacket->getVersion(),rbvtpPacket->getSeqnum(),name,getHostName());
          if(((isRoadOfJunction(getRoadID(),rbvtpPacket->getdesconn()))||(getRoadID()==rbvtpPacket->getdesconn()))&&(std::find(RSTSeenlist.begin(),RSTSeenlist.end(),myinfor)==RSTSeenlist.end()))
          {
                RBVTP_EV<<"got RTS IP:"<<rbvtpPacket->getsrcAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
                RBVTPPacket *ctspacket=  createCTSPacket( rbvtpPacket);
                scheduleReBoardcastTimer(CaculateHoldTime(ctspacket->getscrPosition(),ctspacket->getdesPosition()),ctspacket,NULL);
                RSTSeenlist.push_back(myinfor);
          }else
          {
              RBVTP_EV<<"drop RTS IP:"<<endl;
          }
    }
}
void RBVTP::processCTSPACKET(RBVTPPacket * rbvtpPacket)
{
    std::cout<<"processCTSPACKET"<<endl;
    std::cout<<getHostName()<<" got CTS From "<<globalPositionTable.getHostName(rbvtpPacket->getsrcAddress())<<endl;
    RBVTP_EV<<getHostName()<<" got CTS "<<rbvtpPacket<<" From "<<globalPositionTable.getHostName(rbvtpPacket->getsrcAddress())<< " to "<<globalPositionTable.getHostName(rbvtpPacket->getdesAddress())<<endl;
    if(rbvtpPacket->getdesAddress()!=getSelfIPAddress())
      {
        cMessage *mymsg=NULL;
        cout<<"other nodes boardcast CTS first"<<endl;
        RBVTP_EV<<"other nodes boardcast CTS first"<<endl;
        std::string name=rbvtpPacket->getName();
        std::vector<cMessage*>meslist= packetwaitinglist.getAllmessages();
        for(int i=0;i<meslist.size();i++)
        {
            RBVTP_EV<<"list of meg "<<i<<" : "<<meslist[i]<<" refer to packet "<<packetwaitinglist.getcPacket(meslist[i])<<endl;
        }
        mymsg=packetwaitinglist.findPacket(rbvtpPacket->getName());
        //RSTSeenlist.push_back(name.substr(3));
         if(mymsg!=NULL)
         {
          clearMessage(mymsg,rbvtpPacket,packetwaitinglist);
          std::cout<<"cancel ctspacket IP:"<<rbvtpPacket->getdesAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
          RBVTP_EV<<"cancel ctspacket IP:"<<rbvtpPacket->getdesAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
         }
      }
    else
      {
        cout<<"CTS is for me"<<endl;
        RBVTP_EV<<"CTS is for me"<<endl;
        string packetname=rbvtpPacket->getName();
        packetname=packetname.substr(4);
        packetname=packetname.substr(0,packetname.rfind("_"));
        cout<<"packetname: "<<packetname<<endl;
        RBVTP_EV<<"packetname: "<<packetname<<endl;
        if(packetname.find("CP")!=-1)
        {
            cout<<"process CP RTS "<<endl;
            RBVTP_EV<<"process CP RTS "<<endl;
            cMessage* timer= dynamic_cast<cMessage*>(RTSpacketwaitinglist.findPacket(packetname));
           if (timer!=NULL)
           {
               showpackets(RTSpacketwaitinglist);
               printf("my timer is %p\n",timer);
               cout<<getHostName()<<" find timer : "<<timer<<"  "<<endl;
              // cout<<"find timer"<<timer->getName()<<"  "<<* timer<<endl;
               clearMessage(timer,check_and_cast<RBVTPPacket*>(RTSpacketwaitinglist.getcPacket(timer)),RTSpacketwaitinglist);
               showpackets(RTSpacketwaitinglist);
               //cancelAndDelete(timer);
           }
           else
           {
               cout<<"can't find RTSWaiting Timer "<<endl;
              // throw cRuntimeError("can't find RTSWaiting Timer");
           }
        }
        std::cout<<"send packets IP:"<<rbvtpPacket->getsrcAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
        RBVTP_EV<<"send packets IP:"<<rbvtpPacket->getsrcAddress()<<"  SQUM: "<<rbvtpPacket->getSeqnum()<<endl;
        sendQueuePacket((rbvtpPacket->getnexthopAddress()),rbvtpPacket->getroads(),rbvtpPacket->getsrcAddress());
      }
}
void RBVTP::showpackets(PacketWaitingTable RTSpacketwaitinglist)
{
    vector<cMessage*> listofMsg=RTSpacketwaitinglist.getAllmessages();
    for(int i=0;i<listofMsg.size();i++)
    {
       // cout<<listofMsg[i];
        printf("  %p  %s\n",listofMsg[i],RTSpacketwaitinglist.getDataPacket(listofMsg[i])->getName());
    }
    cout<<"end of show"<<endl;
}


void RBVTP::processCPPACKET(RBVTPPacket * rbvtpPacket)
{
    std::string srcconn=rbvtpPacket->getdesconn();
    RBVTP_EV<<"receive CPPACKET from "<<globalPositionTable.getHostName(rbvtpPacket->getlastsenderAddress()) <<" with des "<<srcconn<<" version "<<rbvtpPacket->getVersion()<<endl;
    RBVTP_EV<<"getjournal size: "<<rbvtpPacket->getjournal().size()<<endl;
   cout<<"getjournal size: "<<rbvtpPacket->getjournal().size()<<endl;
   if(rbvtpPacket->getjournal().size()==0)
   {
       throw cRuntimeError("journal is empty");
   }
   for (int i=0;i<rbvtpPacket->getjournal().size();i++)
   {
       EV_LOG("journal "+std::to_string(i)+" : "+rbvtpPacket->getjournal()[i]);
       LOG_EV<<"journal "+std::to_string(i)+" : "+rbvtpPacket->getjournal()[i]<<endl;
   }

   RBVTP_EV<<"get connections size: "<<rbvtpPacket->thisConnectionTable.getAllConnections().size()<<endl;
   for(int i =0 ;i<rbvtpPacket->thisConnectionTable.getAllConnections().size();i++)
   {
       std::pair<std::string,std::string> connes=rbvtpPacket->thisConnectionTable.getAllConnections()[i];
       string src=connes.first;
       string des=connes.second;
       RBVTP_EV<<src<<" to "<<des<<" state: "<<rbvtpPacket->thisConnectionTable.getConnectState(src,des).thisflag<<endl;
       LOG_EV<<src<<" to "<<des<<" state: "<<rbvtpPacket->thisConnectionTable.getConnectState(src,des).thisflag<<endl;
   }
    double distence ;
    if(srcconn.size()==3){
        distence= (getPositionOfJunction(srcconn)-getSelfPosition()).length();
    }else
    {
        distence=0;
    }
    rbvtpPacket->nexthop_ip=IPv4Address::UNSPECIFIED_ADDRESS;
    string nexthopconn;
    if(distence<distenceOfclose)// close enough to the intersection
      {
        RBVTP_EV<<"close to "<<srcconn<<"   "<<distence<<endl;
        string lastconn=rbvtpPacket->getlastjournal(false);
        if(lastconn!=srcconn)
        {
            RBVTP_EV<<"add journal "<<srcconn<<endl;
            rbvtpPacket->addjournal(srcconn);
        }
       // std::vector <std::string> desconnlist=getConnections(srcconn);
        if(lastconn!=srcconn){
            Connectstate conn(Reachable);
            RBVTP_EV<<"add Connection "<<lastconn<<" to "<<srcconn<<" Reachable "<<endl;
            rbvtpPacket->thisConnectionTable.addTwoWayConnection(lastconn,srcconn,conn);
            if(lastconn.size()!=3&&rbvtpPacket->thisConnectionTable.getReachableConnections(lastconn).size()!=1)
            {
                rbvtpPacket->thisConnectionTable.addTwoWayConnection(rbvtpPacket->thisConnectionTable.getReachableConnections(lastconn)[0],rbvtpPacket->thisConnectionTable.getReachableConnections(lastconn)[1],conn);
                RBVTP_EV<<"add Connection "<<rbvtpPacket->thisConnectionTable.getReachableConnections(lastconn)[0]<<" to "<<rbvtpPacket->thisConnectionTable.getReachableConnections(lastconn)[1]<<" Reachable "<<endl;
            }
        }
        RBVTP_EV<<"get connections size: "<<rbvtpPacket->thisConnectionTable.getAllConnections().size()<<endl;
          for(int i =0 ;i<rbvtpPacket->thisConnectionTable.getAllConnections().size();i++)
          {
              std::pair<std::string,std::string> connes=rbvtpPacket->thisConnectionTable.getAllConnections()[i];
              string src=connes.first;
              string des=connes.second;
              RBVTP_EV<<src<<" to "<<des<<" state: "<<rbvtpPacket->thisConnectionTable.getConnectState(src,des).thisflag<<endl;
          }
        nexthopconn=findnextConn(srcconn,  rbvtpPacket->thisConnectionTable);
        if(nexthopconn==srcconn)
        {
          RBVTP_EV<<"can't find next for"<<nexthopconn<<endl;
          if(getSelfIPAddress()==rbvtpPacket->getsrcAddress().get4())
                {
                  RBVTP_EV<<nexthopconn<<" is the src"<<endl;
                    if(rbvtpPacket->thisConnectionTable.getConnections(srcconn).size()==2)
                    {
                        for (int i=0;i<2;i++)
                        {
                            RBVTP_EV<<"conn of src in table : "<<rbvtpPacket->thisConnectionTable.getConnections(srcconn)[i]<<endl;
                        }
                        RBVTP_EV<<"end of CP"<<endl;
                        return ;
                    }else
                    {
                      if(rbvtpPacket->thisConnectionTable.getConnections(srcconn)[0]==getJunctionsOfRoad(srcconn)[0])
                      {
                          nexthopconn=   getJunctionsOfRoad(srcconn)[1];
                          RBVTP_EV<<"src got pacekt return from "<<getJunctionsOfRoad(srcconn)[0]<<endl;

                      }else
                      {
                          nexthopconn=   getJunctionsOfRoad(srcconn)[0];
                          RBVTP_EV<<"src got pacekt return from "<<getJunctionsOfRoad(srcconn)[1]<<endl;

                      }
                    }
                }
                else
                {
                    RBVTP_EV<<nexthopconn<<" is not the src! my ip "<<getSelfIPAddress()<<" src of packet "<<rbvtpPacket->getsrcAddress().get4()<<endl;
                    nexthopconn=rbvtpPacket->getlastjournal(true);
                }
        }else //find a next hop
        {
            /*RBVTP_EV<<"add journal "<<nexthopconn<<endl;
            rbvtpPacket->addjournal(nexthopconn);*/
            RBVTP_EV<<"find a nexthop: "<<nexthopconn<<endl;
            for(int j=0;j<(rbvtpPacket->getjournal().size());j++)
            {
                RBVTP_EV<<" journal: "<<rbvtpPacket->getjournal()[j]<<endl;
            }
        }
        rbvtpPacket->setVersion();
        RBVTP_EV<<" setVersion "<<rbvtpPacket->getVersion()<<endl;

      }
    else
      {
        RBVTP_EV<<" too far to: "<<srcconn<< "  "<<distence<<endl;
        nexthopconn=srcconn;
      }
    rbvtpPacket->setlastsenderAddress(getSelfIPAddress());
    rbvtpPacket->setdesconn(nexthopconn);
    rbvtpPacket->setLifetime(simTime()+CPliftime);
    RBVTP_EV<<" send cp tp: "<<nexthopconn<<endl;
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
    EV_LOG("datagramLocalInHook");
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
    std::multimap<IPvXAddress, IPv4Datagram *>::iterator lt = delayPacketlist.getlowerbound(target);
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
    EV_LOG("createRTSPacket");
    std::string packetname=rbvtpPacket->getName();
  //  std::string packettype="RD_";
    //packetname=packetname.replace(packetname.begin(),packetname.begin()+2,"RTS");
   // stringstream convert; // stringstream used for the conversion
   // convert << squmRTS++;//add the value of Number to the characters in the stream
   // packetname="RTS_"+packetname+"_"+convert.str();
    packetname="RTS_"+packetname+"_"+std::to_string(squmRTS++);
    RBVTPPacket * RTSPacket = new RBVTPPacket(packetname.c_str());
    //std::cout <<"rbvtpPacket-getName = " << rbvtpPacket(strcat("RD_",content->getName())) << endl;
    RTSPacket->setsrcAddress(getSelfIPAddress());
    RTSPacket->setdesAddress(rbvtpPacket->getdesAddress());
    RTSPacket->setdesPosition(rbvtpPacket->getdesPosition());
    RBVTP_EV<<"Create RTS : scr: "<<RTSPacket->getsrcAddress()<<" des: "<<RTSPacket->getdesAddress()<<endl;
    RTSPacket->setRBVTPPacketType(RBVTP_RTS);
    RTSPacket->setSeqnum(squmRTS);
    RTSPacket->setVersion(rbvtpPacket->getVersion());
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
    RBVTP_EV<<"Create CTS : scr: "<<CTSPacket->getsrcAddress()<<" des: "<<CTSPacket->getdesAddress()<<"  "<<globalPositionTable.getHostName(CTSPacket->getdesAddress())<<endl;
    CTSPacket->setRBVTPPacketType(RBVTP_CTS);
    CTSPacket->setSeqnum(rbvtpPacket->getSeqnum());
    CTSPacket->setscrPosition(rbvtpPacket->getscrPosition());
    CTSPacket->setroads(rbvtpPacket->getroads());
    CTSPacket->setBitLength(CTSPacket->getPacketlength());
    CTSPacket->setLifetime(simTime()+RRliftime);
   // rBVTRPacket->encapsulate(content);
    return CTSPacket;
}
void RBVTP::clearMessage(cMessage * message,RBVTPPacket *rbvtpPacket,PacketWaitingTable& packetwaitinglist)
{
     std::cout<<"clearMessage  :";
     std::cout<<message<<endl;
     packetwaitinglist.removePacket(message);
     if(message->isSelfMessage())
     {
         RBVTP_EV<<"cancel timer : "<<message->getName()<<endl;
         cout<<"cancel timer : "<<message->getName()<<endl;
         cancelAndDelete(message);
     }else
     {
         throw cRuntimeError((string(message->getName())+string("time is not a self message ")).c_str());
     }
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

    return result;//+normal(0.2, 0.05);;
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
    EV_LOG( "Scheduling ReBoardcast timer" );
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
    EV_LOG( timername);
    cMessage * reBoardcastTimer = new cMessage(timername.c_str());
    packetwaitinglist.addPacket(reBoardcastTimer,rbvtpPacket,datagram);
    scheduleAt(simTime() + holdingtime, reBoardcastTimer);
}

void RBVTP::EV_LOG(std::string context)
{
    RouteInterface::EV_LOG("RBVTP",context);
}
