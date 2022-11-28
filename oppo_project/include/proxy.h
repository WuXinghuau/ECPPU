#ifndef PROXY_H
#define PROXY_H
#include "proxy.grpc.pb.h"
#include "devcommon.h"
#include <grpcpp/grpcpp.h>
#include <grpc++/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
namespace OppoProject{
    class ProxyImpl final : public proxy_proto::proxyService::Service,
                        public std::enable_shared_from_this<OppoProject::ProxyImpl>{
                
        public:
            grpc::Status checkalive(grpc::ServerContext *context, const proxy_proto::CheckaliveCMD *request,
                                    proxy_proto::RequestResult *response) override;
        private:
    };
    class Proxy
    {
    public:
        void Run(){
            grpc::EnableDefaultHealthCheckService(true);
            grpc::reflection::InitProtoReflectionServerBuilderPlugin();
            grpc::ServerBuilder builder;
            std::string proxy_ip_port = "localhost:50055";
            std::cout << "proxy_ip_port:" << proxy_ip_port << std::endl;
            builder.AddListeningPort(proxy_ip_port, grpc::InsecureServerCredentials());
            builder.RegisterService(&m_proxyImpl_ptr);
            std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
            server->Wait();
        }

    private:
        //std::shared_ptr<OppoProject::ProxyImpl> m_proxyImpl_ptr;
        OppoProject::ProxyImpl m_proxyImpl_ptr;
    };
}
#endif