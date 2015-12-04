/*
 * RBVTPPacket.h
 *
 *  Created on: Jul 6, 2015
 *      Author: chengli
 */

#ifndef _RBVTPPACKET_H_
#define _RBVTPPACKET_H_

#include <omnetpp.h>

#include "IPvXAddress.h"
#include "Coord.h"
// opp_msgc version check
#define MSGC_VERSION 0x0405
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif

// dll export symbol
#ifndef INET_API
#  if defined(INET_EXPORT)
#    define INET_API  OPP_DLLEXPORT
#  elif defined(INET_IMPORT)
#    define INET_API  OPP_DLLIMPORT
#  else
#    define INET_API
#  endif
#endif
#include <cmessage.h>
#include"RBVTP/RBVTPDefs.h"
#include "RBVTP/ConnectionTable.h"

class INET_API RBVTPPacket: public cPacket {

  protected:
    RBVTPPacketType packetTpye_var;
    IPvXAddress src_ip;
    IPvXAddress des_ip;

    Coord src_position;
    Coord des_position;
    string des_Connection;
    string src_Connection;
    unsigned int seqNum;
    std::vector<std::string> roads;
    std::vector<std::string> journal;
    ConnectionTable thisConnectionTable;
    simtime_t lifeTime;


public:
    IPvXAddress nexthop_ip;
    RBVTPPacket(const char *name=NULL, int kind=0);
    RBVTPPacket(const RBVTPPacket& other);
    virtual ~RBVTPPacket();
    virtual const RBVTPPacketType& getPacketType() const {return const_cast<RBVTPPacket*>(this)->packetTpye_var;}
    virtual void setRBVTPPacketType(const RBVTPPacketType& type){this->packetTpye_var=type;}

    RBVTPPacket& operator=(const RBVTPPacket& other);
   virtual RBVTPPacket *dup() const {return new RBVTPPacket(*this);}
   virtual void parsimPack(cCommBuffer *b);
   virtual void parsimUnpack(cCommBuffer *b);
   void copy(const RBVTPPacket& other);
   virtual IPvXAddress&  getsrcAddress();
   void setsrcAddress(const IPvXAddress& address);
   virtual IPvXAddress& getdesAddress();
   virtual void setdesAddress(const IPvXAddress& address);
   virtual IPvXAddress& getnexthopAddress();
   virtual void setnexthopAddress(const IPvXAddress& address);
   virtual Coord& getscrPosition();
   virtual void setscrPosition(const Coord& position);
   virtual void  setSeqnum(const unsigned int& sequnm);
   virtual void setdesPosition(const Coord& position);
   virtual Coord& getdesPosition();

   virtual simtime_t &  getLifetime();
   virtual void  setLifetime(simtime_t mylifetime);


   virtual unsigned int& getSeqnum();
   virtual std::vector<std::string>& getroads();
   virtual void setroads(std::vector<std::string> myroads);
   virtual int  getPacketlength();
   virtual void addroad(std::string road);
   virtual  std::string& getsrcconn();
   virtual void setsrcconn(std::string myroads);
   virtual  std::string& getdesconn();
   virtual void setdesconn(std::string myroads);
   virtual std::string&  getroadsToStr();
   ConnectionTable myconnectionTable;

};
inline void doPacking(cCommBuffer *b, RBVTPPacket& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, RBVTPPacket& obj) {obj.parsimUnpack(b);}

#endif /* RBVTPPACKET_H_ */
