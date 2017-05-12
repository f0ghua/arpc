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

#include <iostream>

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
#include "./gen-cpp/DemoEvent.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

using namespace demo;
using namespace shared;

class DemoEventHandler : virtual public DemoEventIf {
 public:
  DemoEventHandler() {
    // Your initialization goes here
  }

  void notifyDemoSevice(const DemoStruct& vars) {
    // Your implementation goes here
    GlobalOutput.printf("event from server: notifyDemoSevice - %d, %s", 
        vars.intValue, vars.strValue.c_str());
  }

  void notifySecdSevice(const DemoStruct& vars) {
    // Your implementation goes here
    GlobalOutput.printf("event from server: notifySecdSevice - %d, %s", 
        vars.intValue, vars.strValue.c_str());
  }

};

int main(int argc, char **argv) 
{
    int port = 8081;
    int clientId = 0;
    
    if (argc > 2) {
        clientId = atoi(argv[1]);
        port = atoi(argv[2]);
    }

	// act as a client to subscribe events
    {
        boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        //boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));
        SharedProtocolClient client(protocol);

        transport->open();
        std::vector<std::string> events;
        if (clientId == 0) {
            events.push_back("notifyDemoSevice");
        }
        else {
            events.push_back("notifySecdSevice");
        }
        client.eventSubscribe(events, port);
        transport->close();
    }
    
    // act as client to request services
	{
        boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
        boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
        //boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
        boost::shared_ptr<TProtocol> protocol(new TJSONProtocol(transport));
    
        DemoServiceClient client(protocol);

        try {
            transport->open();

            client.setStruct(100, "Hello World");
            cout << "Set struct: " << endl;

            // Note that C++ uses return by reference for complex types to avoid
            // costly copy construction
            DemoStruct ds;
            client.getStruct(ds);
            cout << "Received struct: " << ds << endl;

            transport->close();
        } catch (TException& tx) {
            cout << "ERROR: " << tx.what() << endl;
        }
	}

    // act as a server to handle events
    shared_ptr<DemoEventHandler> handler(new DemoEventHandler());
    shared_ptr<TProcessor> processor(new DemoEventProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    //shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TJSONProtocolFactory());
    
    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
	
    cout << "client is in event listening ..." << endl;
    server.serve();    
}
