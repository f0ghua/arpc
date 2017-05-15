/**
 * Autogenerated by Thrift Compiler (0.10.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include <iostream>
#include <sstream>

#include "arpcMgr.h"

using namespace std;
using namespace ::apache::thrift;

namespace arpc {

bool ManagerProcessor::process(boost::shared_ptr<protocol::TProtocol> in,
                     boost::shared_ptr<protocol::TProtocol> out,
                     void* connectionContext) {
  std::string fname;
  protocol::TMessageType mtype;
  int32_t seqid;
  in->readMessageBegin(fname, mtype, seqid);

  GlobalOutput.printf("received message type %d from client", mtype);
  
  switch (mtype) {
    case protocol::T_CALL:
    case protocol::T_ONEWAY:
    {
        // got request from client, find the host and route to it
        break;
    }
    default:
        break;
  }

  //return dispatchCall(in.get(), out.get(), fname, seqid, connectionContext);
  return false;
}

bool ManagerProcessor::dispatchCall(::apache::thrift::protocol::TProtocol* iprot, 
	::apache::thrift::protocol::TProtocol* oprot, 
	const std::string& fname, 
	int32_t seqid, 
	void* callContext) 
{
    ServiceMap::iterator pfn;
    pfn = serviceMap_.find(fname);
    if (pfn == serviceMap_.end()) {
        iprot->skip(::apache::thrift::protocol::T_STRUCT);
        iprot->readMessageEnd();
        iprot->getTransport()->readEnd();
        ::apache::thrift::TApplicationException x(::apache::thrift::TApplicationException::UNKNOWN_METHOD, "Invalid method name: '"+fname+"'");
        oprot->writeMessageBegin(fname, ::apache::thrift::protocol::T_EXCEPTION, seqid);
        x.write(oprot);
        oprot->writeMessageEnd();
        oprot->getTransport()->writeEnd();
        oprot->getTransport()->flush();
        return true;
    }

    cout << "client request for service " << fname << endl;
    
    //(this->*(pfn->second))(seqid, iprot, oprot, callContext);
    return true;
}

::boost::shared_ptr< ::apache::thrift::TProcessor > ManagerProcessorFactory::getProcessor(
	const ::apache::thrift::TConnectionInfo& connInfo) 
{
    ::boost::shared_ptr< ::apache::thrift::TProcessor > processor(new ManagerProcessor());
    return processor;
}

}