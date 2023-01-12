#ifndef CLIENT_H
#define CLIENT_H

#ifdef BAZEL_BUILD
#include "src/proto/coordinator.grpc.pb.h"
#else
#include "coordinator.grpc.pb.h"
#endif

#include "meta_definition.h"
#include <grpcpp/grpcpp.h>
#include <asio.hpp>
namespace OppoProject {
class Client {
public:
  Client(std::string ClientIP, int ClientPort, std::string CoordinatorIpPort): m_coordinatorIpPort(CoordinatorIpPort),
                                                                               m_clientIPForGet(ClientIP),
                                                                               m_clientPortForGet(ClientPort),
                                                                               acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_clientPortForGet)) {
    auto channel = grpc::CreateChannel(m_coordinatorIpPort, grpc::InsecureChannelCredentials());
    m_coordinator_ptr = coordinator_proto::CoordinatorService::NewStub(channel);
  }
  std::string sayHelloToCoordinatorByGrpc(std::string hello);
  bool set(std::string key, std::string value, std::string flag);
  bool SetParameterByGrpc(ECSchema input_ecschema);
  bool get(std::string key, std::string &value);

private:
  std::unique_ptr<coordinator_proto::CoordinatorService::Stub> m_coordinator_ptr;
  std::string m_coordinatorIpPort;
  int m_clientPortForGet;
  std::string m_clientIPForGet;
  asio::io_context io_context;
  asio::ip::tcp::acceptor acceptor;
};

} // namespace OppoProject

#endif // CLIENT_H