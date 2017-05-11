/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <boost/make_shared.hpp>

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "./gen-cpp/SharedProtocol.h"
#include "./gen-cpp/DemoService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

using namespace demo;
using namespace shared;

struct HostInfo {
    std::string addr;
    int port;
};

static std::map<std::string, HostInfo> hostServiceMap;

class ServerEventHandler : public TServerEventHandler {

    void *createContext(boost::shared_ptr<TProtocol> input, boost::shared_ptr<TProtocol> output) {
        (void)input;
        (void)output;
        return (void *)(new char[256]); //TODO
    }

    virtual void deleteContext(void *serverContext, 
                               boost::shared_ptr<TProtocol>input,
                               boost::shared_ptr<TProtocol>output) {
        delete [](char *)serverContext;
    }

    virtual void processContext(void *serverContext, boost::shared_ptr<TTransport> transport) {
    }
};

class PSHandler /*: virtual public SharedProtocolIf, virtual public DemoServiceIf*/ {
public:
    PSHandler() {
        // Your initialization goes here
    }

    void setStruct(const int32_t intValue, const std::string& strValue, void *callContext) {
        // Your implementation goes here
        cout << "from client, setStruct: intValue = " << intValue << ", strValue = " << strValue << endl;

#ifndef NDEBUG
        map<string, HostInfo>::const_iterator ci = hostServiceMap.find((char *)callContext);
        if (ci == hostServiceMap.end()) {
            cout << "The request service has not been registeted." << endl;
            for (ci = hostServiceMap.begin(); ci != hostServiceMap.end(); ci++) {
                GlobalOutput.printf("hostServiceMap - %s, %s:%d", 
                    ci->first.c_str(), ci->second.addr.c_str(), ci->second.port);
            }
            // how to handle if no service exist?
            return;
            //throw; // std::exception();
        }

        HostInfo hi = ci->second;
        GlobalOutput.printf("The service is registered by %s:%d", hi.addr.c_str(), hi.port);
#endif

        boost::shared_ptr<TTransport> socket(new TSocket(hi.addr, hi.port));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        //boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));
    
        DemoServiceClient client(protocol);
        try {
            transport->open();
            client.setStruct(intValue, strValue);
            cout << "route message to host, Set struct: " << intValue << ", " << strValue << endl;
            transport->close();
        } catch (TException& tx) {
            cout << "ERROR: " << tx.what() << endl;
        }
    }

    void getStruct(DemoStruct& _return, void *callContext) {
        // Your implementation goes here
        cout << "from client, getStruct:" << endl;
        
#ifndef NDEBUG
        map<string, HostInfo>::const_iterator ci = hostServiceMap.find((char *)callContext);
        if (ci == hostServiceMap.end()) {
            cout << "The request service has not been registeted." << endl;
            for (ci = hostServiceMap.begin(); ci != hostServiceMap.end(); ci++) {
                GlobalOutput.printf("hostServiceMap - %s, %s:%d", 
                    ci->first.c_str(), ci->second.addr.c_str(), ci->second.port);
            }
            // how to handle if no service exist?
            return;
            //throw; // std::exception();
        }

        HostInfo hi = ci->second;
        GlobalOutput.printf("The service is registered by %s:%d", hi.addr.c_str(), hi.port);
#endif        

        boost::shared_ptr<TTransport> socket(new TSocket(hi.addr, hi.port));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        //boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));

        DemoServiceClient client(protocol);
        try {
            transport->open();
            client.getStruct(_return);
            cout << "route message from host, getStruct: " << _return.intValue << ", " << _return.strValue << endl;
            transport->close();
        } catch (TException& tx) {
            cout << "ERROR: " << tx.what() << endl;
        }
    }

    int32_t serviceRegister(const std::vector<std::string> & methodName, const int16_t sevicePort, void *callContext) {
        cout << "from host, register service: " << endl;
        for (std::vector<std::string>::const_iterator i = methodName.begin(); 
             i != methodName.end(); 
             ++i)
            std::cout << *i << ' ';
        cout << endl;

        return 0;
    }
    
    void serviceUnregister(const std::vector<std::string> & methodName, void *callContext) {

    }

private:
    int32_t intValue_;
    string strValue_;
};

/*
  class PSProcessor : virtual public SharedProtocolProcessor, virtual public DemoServiceProcessor {
  public:
  PSProcessor(boost::shared_ptr<PSHandler> handler) :
  SharedProtocolProcessor(handler),
  DemoServiceProcessor(handler) {
  }
  virtual ~PSProcessor() {}
  };
*/

class PSProcessor : public ::apache::thrift::TDispatchProcessor {
protected:
    boost::shared_ptr<PSHandler> iface_;
    virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
private:
    typedef  void (PSProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
    typedef std::map<std::string, ProcessFunction> ProcessMap;
    ProcessMap processMap_;
    void process_serviceRegister(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
    void process_serviceUnregister(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);

    void process_setStruct(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
    void process_getStruct(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);


public:
    PSProcessor(boost::shared_ptr<PSHandler> iface) :
        iface_(iface) {
        processMap_["serviceRegister"] = &PSProcessor::process_serviceRegister;
        processMap_["serviceUnregister"] = &PSProcessor::process_serviceUnregister;

        processMap_["setStruct"] = &PSProcessor::process_setStruct;
        processMap_["getStruct"] = &PSProcessor::process_getStruct;
    }

    virtual ~PSProcessor() {}
};

bool PSProcessor::dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext) {
    ProcessMap::iterator pfn;
    pfn = processMap_.find(fname);
    if (pfn == processMap_.end()) {
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

#ifndef NDEBUG
    strncpy((char *)callContext, fname.c_str(), 256); 
#endif
    (this->*(pfn->second))(seqid, iprot, oprot, callContext);
    return true;
}

void PSProcessor::process_serviceRegister(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
    void* ctx = NULL;
    if (this->eventHandler_.get() != NULL) {
        ctx = this->eventHandler_->getContext("SharedProtocol.serviceRegister", callContext);
    }
    ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "SharedProtocol.serviceRegister");

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preRead(ctx, "SharedProtocol.serviceRegister");
    }

    SharedProtocol_serviceRegister_args args;
    args.read(iprot);
    iprot->readMessageEnd();
    uint32_t bytes = iprot->getTransport()->readEnd();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postRead(ctx, "SharedProtocol.serviceRegister", bytes);
    }

#ifndef NDEBUG
    // try to get the address and port of peer
    TBufferedTransport *tbuf = dynamic_cast<TBufferedTransport *>(iprot->getTransport().get()); 
    TSocket *socket = dynamic_cast<TSocket *>(tbuf->getUnderlyingTransport().get()); 
    cout << "host registered from addr " << socket->getPeerHost() << ", port " << socket->getPeerPort() << endl;
    
    cout << "callContext = " << (char *)callContext << endl;
    
    HostInfo hi;
    hi.addr = socket->getPeerHost();
    hi.port = args.sevicePort;

    for (vector<string>::const_iterator ci = args.methodName.begin();
         ci != args.methodName.end();
         ci++) {
        hostServiceMap[*ci] = hi;
    }

    /*
      for (map<string, HostInfo>::const_iterator ci = hostServiceMap.begin(); 
      ci != hostServiceMap.end(); 
      ci++) {
      GlobalOutput.printf("hostServiceMap - %s, %s:%d\n", 
      ci->first.c_str(), ci->second.addr.c_str(), ci->second.port);
      }
    */
#endif

    SharedProtocol_serviceRegister_result result;
    try {
        result.success = iface_->serviceRegister(args.methodName, args.sevicePort, callContext);
        result.__isset.success = true;
    } catch (const std::exception& e) {
        if (this->eventHandler_.get() != NULL) {
            this->eventHandler_->handlerError(ctx, "SharedProtocol.serviceRegister");
        }

        ::apache::thrift::TApplicationException x(e.what());
        oprot->writeMessageBegin("serviceRegister", ::apache::thrift::protocol::T_EXCEPTION, seqid);
        x.write(oprot);
        oprot->writeMessageEnd();
        oprot->getTransport()->writeEnd();
        oprot->getTransport()->flush();
        return;
    }

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preWrite(ctx, "SharedProtocol.serviceRegister");
    }

    oprot->writeMessageBegin("serviceRegister", ::apache::thrift::protocol::T_REPLY, seqid);
    result.write(oprot);
    oprot->writeMessageEnd();
    bytes = oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postWrite(ctx, "SharedProtocol.serviceRegister", bytes);
    }
}

void PSProcessor::process_serviceUnregister(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
    void* ctx = NULL;
    if (this->eventHandler_.get() != NULL) {
        ctx = this->eventHandler_->getContext("SharedProtocol.serviceUnregister", callContext);
    }
    ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "SharedProtocol.serviceUnregister");

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preRead(ctx, "SharedProtocol.serviceUnregister");
    }

    SharedProtocol_serviceUnregister_args args;
    args.read(iprot);
    iprot->readMessageEnd();
    uint32_t bytes = iprot->getTransport()->readEnd();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postRead(ctx, "SharedProtocol.serviceUnregister", bytes);
    }

    SharedProtocol_serviceUnregister_result result;
    try {
        iface_->serviceUnregister(args.methodName, callContext);
    } catch (const std::exception& e) {
        if (this->eventHandler_.get() != NULL) {
            this->eventHandler_->handlerError(ctx, "SharedProtocol.serviceUnregister");
        }

        ::apache::thrift::TApplicationException x(e.what());
        oprot->writeMessageBegin("serviceUnregister", ::apache::thrift::protocol::T_EXCEPTION, seqid);
        x.write(oprot);
        oprot->writeMessageEnd();
        oprot->getTransport()->writeEnd();
        oprot->getTransport()->flush();
        return;
    }

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preWrite(ctx, "SharedProtocol.serviceUnregister");
    }

    oprot->writeMessageBegin("serviceUnregister", ::apache::thrift::protocol::T_REPLY, seqid);
    result.write(oprot);
    oprot->writeMessageEnd();
    bytes = oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postWrite(ctx, "SharedProtocol.serviceUnregister", bytes);
    }
}

void PSProcessor::process_setStruct(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
    void* ctx = NULL;
    if (this->eventHandler_.get() != NULL) {
        ctx = this->eventHandler_->getContext("DemoService.setStruct", callContext);
    }
    ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "DemoService.setStruct");

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preRead(ctx, "DemoService.setStruct");
    }

    DemoService_setStruct_args args;
    args.read(iprot);
    iprot->readMessageEnd();
    uint32_t bytes = iprot->getTransport()->readEnd();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postRead(ctx, "DemoService.setStruct", bytes);
    }

    DemoService_setStruct_result result;
    try {
        iface_->setStruct(args.intValue, args.strValue, callContext);
    } catch (const std::exception& e) {
        if (this->eventHandler_.get() != NULL) {
            this->eventHandler_->handlerError(ctx, "DemoService.setStruct");
        }

        ::apache::thrift::TApplicationException x(e.what());
        oprot->writeMessageBegin("setStruct", ::apache::thrift::protocol::T_EXCEPTION, seqid);
        x.write(oprot);
        oprot->writeMessageEnd();
        oprot->getTransport()->writeEnd();
        oprot->getTransport()->flush();
        return;
    }

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preWrite(ctx, "DemoService.setStruct");
    }

    oprot->writeMessageBegin("setStruct", ::apache::thrift::protocol::T_REPLY, seqid);
    result.write(oprot);
    oprot->writeMessageEnd();
    bytes = oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postWrite(ctx, "DemoService.setStruct", bytes);
    }
}

void PSProcessor::process_getStruct(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext)
{
    void* ctx = NULL;
    if (this->eventHandler_.get() != NULL) {
        ctx = this->eventHandler_->getContext("DemoService.getStruct", callContext);
    }
    ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "DemoService.getStruct");

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preRead(ctx, "DemoService.getStruct");
    }

    DemoService_getStruct_args args;
    args.read(iprot);
    iprot->readMessageEnd();
    uint32_t bytes = iprot->getTransport()->readEnd();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postRead(ctx, "DemoService.getStruct", bytes);
    }

    DemoService_getStruct_result result;
    try {
        iface_->getStruct(result.success, callContext);
        result.__isset.success = true;
    } catch (const std::exception& e) {
        if (this->eventHandler_.get() != NULL) {
            this->eventHandler_->handlerError(ctx, "DemoService.getStruct");
        }

        ::apache::thrift::TApplicationException x(e.what());
        oprot->writeMessageBegin("getStruct", ::apache::thrift::protocol::T_EXCEPTION, seqid);
        x.write(oprot);
        oprot->writeMessageEnd();
        oprot->getTransport()->writeEnd();
        oprot->getTransport()->flush();
        return;
    }

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->preWrite(ctx, "DemoService.getStruct");
    }

    oprot->writeMessageBegin("getStruct", ::apache::thrift::protocol::T_REPLY, seqid);
    result.write(oprot);
    oprot->writeMessageEnd();
    bytes = oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();

    if (this->eventHandler_.get() != NULL) {
        this->eventHandler_->postWrite(ctx, "DemoService.getStruct", bytes);
    }
}


int main(int argc, char **argv) {
    int port = 9090;
    shared_ptr<PSHandler> handler(new PSHandler());
    shared_ptr<TProcessor> processor(new PSProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    //shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TJSONProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    shared_ptr<ServerEventHandler> serverEventHandler(new ServerEventHandler());
    server.setServerEventHandler(serverEventHandler);

    cout << "Server is in service ..." << endl;
    server.serve();
   
    return 0;
}


