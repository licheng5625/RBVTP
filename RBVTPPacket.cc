/*
 * RBVTPPacket.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: chengli
 */

#include <RBVTP/RBVTPPacket.h>


#include <omnetpp.h>


 #include "IPvXAddress.h"
#include "simtime_t.h"



template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}
Register_Class(RBVTPPacket);

RBVTPPacket::RBVTPPacket(const char *name, int kind) : ::cPacket(name,kind)
{   roads=std::vector<std::string>();
    journal=std::vector<std::string>();
   // version=0;
    // TODO Auto-generated constructor stub
}
RBVTPPacket::RBVTPPacket(const RBVTPPacket& other) : ::cPacket(other)
{
    copy(other);
}
simtime_t & RBVTPPacket::getLifetime()
{
    return lifeTime;
}

void  RBVTPPacket::setLifetime(simtime_t mylifetime)
{
 lifeTime =mylifetime;
}

RBVTPPacket::~RBVTPPacket() {
    // TODO Auto-generated destructor stub
}
RBVTPPacket& RBVTPPacket::operator=(const RBVTPPacket& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void RBVTPPacket::copy(const RBVTPPacket& other)
{
     this->packetTpye_var= other.packetTpye_var;
     this->src_ip= other.src_ip;
     this->des_ip= other.des_ip;
     this->nexthop_ip= other.nexthop_ip;
     this->src_position= other.src_position;
     this->lastsender_ip= other.lastsender_ip;
     this->des_position= other.des_position;
     this->seqNum= other.seqNum;
     this->roads= other.roads;
     this->journal= other.journal;
     this->lifeTime=other.lifeTime;
     this->src_Connection=other.src_Connection;
     this->des_Connection=other.des_Connection;
     this->thisConnectionTable=other.thisConnectionTable;
     this->version=other.version;
}

void RBVTPPacket::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->packetTpye_var);
    doPacking(b,this->version);
    doPacking(b,this->src_ip);
    doPacking(b,this->des_ip);
    doPacking(b,this->lastsender_ip);
    doPacking(b,this->nexthop_ip);
    doPacking(b,this->src_position);
    doPacking(b,this->des_position);
    doPacking(b,this->seqNum);
    doPacking(b,this->roads);
    doPacking(b,this->journal);
    doPacking(b,this->lifeTime);
    doPacking(b,this->src_Connection);
    doPacking(b,this->des_Connection);
    doPacking(b,this->thisConnectionTable);
}

void RBVTPPacket::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->packetTpye_var);
    doUnpacking(b,this->src_ip);
    doUnpacking(b,this->version);
    doUnpacking(b,this->des_ip);
    doUnpacking(b,this->lastsender_ip);
    doUnpacking(b,this->nexthop_ip);
    doUnpacking(b,this->src_position);
    doUnpacking(b,this->des_position);
    doUnpacking(b,this->seqNum);
    doUnpacking(b,this->roads);
    doUnpacking(b,this->journal);
    doUnpacking(b,this->lifeTime);
    doUnpacking(b,this->src_Connection);
    doUnpacking(b,this->des_Connection);
    doUnpacking(b,this->thisConnectionTable);

}

IPvXAddress& RBVTPPacket::getsrcAddress()
{
    return src_ip;
}

void RBVTPPacket::setsrcAddress(const IPvXAddress& address)
{
    this->src_ip = address;
}

IPvXAddress& RBVTPPacket::getdesAddress()
{
    return des_ip;
}
void RBVTPPacket::setlastsenderAddress(const IPvXAddress& address)
{
    this->lastsender_ip = address;
}

IPvXAddress& RBVTPPacket::getlastsenderAddress()
{
    return lastsender_ip;
}

void RBVTPPacket::setdesAddress(const IPvXAddress& address)
{
    this->des_ip = address;
}

IPvXAddress& RBVTPPacket::getnexthopAddress()
{
    return nexthop_ip;
}

void RBVTPPacket::setnexthopAddress(const IPvXAddress& address)
{
    this->nexthop_ip = address;
}
Coord& RBVTPPacket::getscrPosition()
{
    return src_position;
}
Coord& RBVTPPacket::getdesPosition()
{
    return des_position;
}
int& RBVTPPacket::getVersion()
{
    return version;
}
void RBVTPPacket::setVersion()
{
     version++;
}
void RBVTPPacket::setVersion(int version)
{
     this->version=version;
}
int RBVTPPacket::getPacketlength()
{
    // routingMode
    int routingMode = 1;
    // destinationPosition, perimeterRoutingStartPosition, perimeterRoutingForwardPosition
    int positions = 8 * 3 * 2 * 4;
    // currentFaceFirstSenderAddress, currentFaceFirstReceiverAddress, senderAddress
    int addresses = 8 * 3 * 4 * 3;
    // TODO: address size
    int squm=8;
    int road=roads.size()*8*2;
    int time =8;
    return routingMode + positions +  addresses+squm+road+time;
 }

void RBVTPPacket::setSeqnum(const unsigned int& seqNum)
{
    this->seqNum = seqNum;
}
unsigned int& RBVTPPacket::getSeqnum()
{
    return seqNum;
}

void RBVTPPacket::setroads(std::vector<std::string> myroads)
{
    roads=myroads;
}

std::vector<std::string>& RBVTPPacket::getroads(){
    return roads;
}
void RBVTPPacket::addroad(std::string road){
    roads.push_back(road);
}

void RBVTPPacket::setjournal(std::vector<std::string> myjournal)
{
    journal=myjournal;
}
std::vector<std::string>& RBVTPPacket::getjournal(){
    return journal;
}
void RBVTPPacket::addjournal(std::string hostid){
    journal.push_back(hostid);
}
std::string& RBVTPPacket::getlastjournal(bool ispop){
    if(ispop){
        journal.pop_back();
    }
    return journal.back();
}
std::string& RBVTPPacket::getroadsToStr(){
    std::string ret="";
    for(int i=0;i<roads.size();i++)
    {
        ret+=roads[i];
    }
    return ret;
}
std::string& RBVTPPacket::getsrcconn()
{
    return this->src_Connection;
}
void RBVTPPacket::setsrcconn(std::string myroads)
{
    this->src_Connection=myroads;
}
std::string& RBVTPPacket::getdesconn()
{
    return  this->des_Connection;
}
void RBVTPPacket::setdesconn(std::string myroads)
{
    this->des_Connection=myroads;
}
void RBVTPPacket::setscrPosition(const Coord& position)
{
    this->src_position = position;
}
void RBVTPPacket::setdesPosition(const Coord& position)
{
    this->des_position = position;
}
class RBVTPPacketDescriptor : public cClassDescriptor
{
  public:
    RBVTPPacketDescriptor();
    virtual ~RBVTPPacketDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(RBVTPPacketDescriptor);

RBVTPPacketDescriptor::RBVTPPacketDescriptor() : cClassDescriptor("RBVTPPacket", "cPacket")
{
}

RBVTPPacketDescriptor::~RBVTPPacketDescriptor()
{
}

bool RBVTPPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<RBVTPPacket *>(obj)!=NULL;
}

const char *RBVTPPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int RBVTPPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 7;
}

unsigned int RBVTPPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
            FD_ISEDITABLE,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *RBVTPPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "packetTpye_var",
        "src_ip",
        "des_ip",
        "nexthop_ip",
        "src_position",
        "des_position",
        "seqNum",
        "roads",
        "lifetime",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int RBVTPPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='p' && strcmp(fieldName, "packetTpye_var")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "src_ip")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "des_ip")==0) return base+2;
    if (fieldName[0]=='n' && strcmp(fieldName, "nexthop_ip")==0) return base+3;
    if (fieldName[0]=='s' && strcmp(fieldName, "src_position")==0) return base+4;
    if (fieldName[0]=='d' && strcmp(fieldName, "des_position")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "seqNum")==0) return base+6;
    if (fieldName[0]=='r' && strcmp(fieldName, "roads")==0) return base+7;
    if (fieldName[0]=='l' && strcmp(fieldName, "lifetime")==0) return base+8;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *RBVTPPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {

            "RBVTPPacketType",
            "IPvXAddress",
            "IPvXAddress",
            "IPvXAddress",
            "Coord",
            "Coord",
            "unsigned int"  ,
            "std::vector<std::string>"  ,
            "simtime_t",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *RBVTPPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int RBVTPPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    RBVTPPacket *pp = (RBVTPPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string RBVTPPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    RBVTPPacket *pp = (RBVTPPacket *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getPacketType(); return out.str();}
        case 1: {std::stringstream out; out << pp->getsrcAddress(); return out.str();}
        case 2: {std::stringstream out; out << pp->getdesAddress(); return out.str();}
        case 3: {std::stringstream out; out << pp->getnexthopAddress(); return out.str();}
        case 4: {std::stringstream out; out << pp->getscrPosition(); return out.str();}
        case 5: {std::stringstream out; out << pp->getdesPosition(); return out.str();}
        case 6: {std::stringstream out; out << pp->getSeqnum(); return out.str();}
        case 7: {std::stringstream out; out << pp->getroadsToStr(); return out.str();}
        case 8: {std::stringstream out; out << pp->getLifetime(); return out.str();}
        default: return "";
    }
}

bool RBVTPPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    RBVTPPacket *pp = (RBVTPPacket *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}



const char *RBVTPPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
   switch (field) {
            case 0: return opp_typename(typeid(RBVTPPacketType));
            case 1: return opp_typename(typeid(IPvXAddress));
            case 2: return opp_typename(typeid(IPvXAddress));
            case 3: return opp_typename(typeid(IPvXAddress));
            case 4: return opp_typename(typeid(Coord));
            case 5: return opp_typename(typeid(Coord));
            case 6: return opp_typename(typeid(unsigned int));
            case 7: return opp_typename(typeid(std::vector<std::string>));
            case 8: return opp_typename(typeid(simtime_t));
        default: return NULL;
    };
}

void *RBVTPPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    RBVTPPacket *pp = (RBVTPPacket *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getPacketType()); break;
        case 1: return (void *)(&pp->getsrcAddress()); break;
        case 2: return (void *)(&pp->getdesAddress()); break;
        case 3: return (void *)(&pp->getnexthopAddress()); break;
        case 4: return (void *)(&pp->getscrPosition()); break;
        case 5: return (void *)(&pp->getdesPosition()); break;
        case 6: return (void *)(&pp->getSeqnum()); break;
        case 7: return (void *)(&pp->getroads()); break;
        case 8: return (void *)(&pp->getLifetime()); break;
        default: return NULL;
    }
}
