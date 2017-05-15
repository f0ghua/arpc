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

#include "./gen-cpp/DemoService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

using namespace demo;

class DemoServiceHandler : virtual public DemoServiceIf {
public:
    DemoServiceHandler() {
        // Your initialization goes here
        intValue_ = 0;
        strValue_ = "uninitialize";
    }

    void setStruct(const int32_t intValue, const std::string& strValue) {
        // Your implementation goes here
        cout << "from server, setStruct: intValue = " << intValue << ", strValue = " << strValue << endl;
        intValue_ = intValue;
        strValue_ = strValue;
    }

    void getStruct(DemoStruct& _return) {
        // Your implementation goes here
        cout << "from server getStruct" << endl;
        _return.intValue = intValue_;
        _return.strValue = strValue_;
        cout << "to server struct value " << _return.intValue << ", " << _return.strValue << endl;
    }
    
private:
    int32_t intValue_;
    string strValue_;
};

int main(int argc, char **argv) {
    int port = 9090;
    int hostId = 0;
    
    if (argc > 2) {
        hostId = atoi(argv[1]);
        port = atoi(argv[2]);
    }

    // act as a server to handle service requests
    shared_ptr<DemoServiceHandler> handler(new DemoServiceHandler());
    shared_ptr<TProcessor> processor(new DemoServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    //shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TJSONProtocolFactory());
    
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);

    cout << "Host is in service ..." << endl;
    server.serve();
   
    return 0;
}


