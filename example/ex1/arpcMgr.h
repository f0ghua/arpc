#ifndef ARPC_MGR_H
#define ARPC_MGR_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>
#include <thrift/cxxfunctional.h>

namespace arpc {

#ifdef _WIN32
#pragma warning( push )
#pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class ManagerProcessor : public ::apache::thrift::TDispatchProcessor {
protected:
    virtual bool process(boost::shared_ptr<apache::thrift::protocol::TProtocol> in,
                         boost::shared_ptr<apache::thrift::protocol::TProtocol> out,
                         void* connectionContext);
    virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
private:
    typedef std::map<std::string, std::string> ServiceMap;
    ServiceMap serviceMap_;
public:
    ManagerProcessor() {
        serviceMap_["setStruct"] = "localhost:9090";
        serviceMap_["getStruct"] = "localhost:9090";
    }

    virtual ~ManagerProcessor() {}
};

class ManagerProcessorFactory : public ::apache::thrift::TProcessorFactory {
public:
    ManagerProcessorFactory() {}

    ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

protected:

};

#ifdef _WIN32
#pragma warning( pop )
#endif

} // namespace

#endif
