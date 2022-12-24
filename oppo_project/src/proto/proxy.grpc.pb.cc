// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: proxy.proto

#include "proxy.pb.h"
#include "proxy.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace proxy_proto {

static const char* proxyService_method_names[] = {
  "/proxy_proto.proxyService/checkalive",
  "/proxy_proto.proxyService/EncodeAndSetObject",
};

std::unique_ptr< proxyService::Stub> proxyService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< proxyService::Stub> stub(new proxyService::Stub(channel, options));
  return stub;
}

proxyService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_checkalive_(proxyService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_EncodeAndSetObject_(proxyService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status proxyService::Stub::checkalive(::grpc::ClientContext* context, const ::proxy_proto::CheckaliveCMD& request, ::proxy_proto::RequestResult* response) {
  return ::grpc::internal::BlockingUnaryCall< ::proxy_proto::CheckaliveCMD, ::proxy_proto::RequestResult, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_checkalive_, context, request, response);
}

void proxyService::Stub::async::checkalive(::grpc::ClientContext* context, const ::proxy_proto::CheckaliveCMD* request, ::proxy_proto::RequestResult* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::proxy_proto::CheckaliveCMD, ::proxy_proto::RequestResult, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_checkalive_, context, request, response, std::move(f));
}

void proxyService::Stub::async::checkalive(::grpc::ClientContext* context, const ::proxy_proto::CheckaliveCMD* request, ::proxy_proto::RequestResult* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_checkalive_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::proxy_proto::RequestResult>* proxyService::Stub::PrepareAsynccheckaliveRaw(::grpc::ClientContext* context, const ::proxy_proto::CheckaliveCMD& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::proxy_proto::RequestResult, ::proxy_proto::CheckaliveCMD, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_checkalive_, context, request);
}

::grpc::ClientAsyncResponseReader< ::proxy_proto::RequestResult>* proxyService::Stub::AsynccheckaliveRaw(::grpc::ClientContext* context, const ::proxy_proto::CheckaliveCMD& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsynccheckaliveRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status proxyService::Stub::EncodeAndSetObject(::grpc::ClientContext* context, const ::proxy_proto::ObjectAndPlacement& request, ::proxy_proto::SetReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::proxy_proto::ObjectAndPlacement, ::proxy_proto::SetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_EncodeAndSetObject_, context, request, response);
}

void proxyService::Stub::async::EncodeAndSetObject(::grpc::ClientContext* context, const ::proxy_proto::ObjectAndPlacement* request, ::proxy_proto::SetReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::proxy_proto::ObjectAndPlacement, ::proxy_proto::SetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_EncodeAndSetObject_, context, request, response, std::move(f));
}

void proxyService::Stub::async::EncodeAndSetObject(::grpc::ClientContext* context, const ::proxy_proto::ObjectAndPlacement* request, ::proxy_proto::SetReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_EncodeAndSetObject_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::proxy_proto::SetReply>* proxyService::Stub::PrepareAsyncEncodeAndSetObjectRaw(::grpc::ClientContext* context, const ::proxy_proto::ObjectAndPlacement& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::proxy_proto::SetReply, ::proxy_proto::ObjectAndPlacement, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_EncodeAndSetObject_, context, request);
}

::grpc::ClientAsyncResponseReader< ::proxy_proto::SetReply>* proxyService::Stub::AsyncEncodeAndSetObjectRaw(::grpc::ClientContext* context, const ::proxy_proto::ObjectAndPlacement& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncEncodeAndSetObjectRaw(context, request, cq);
  result->StartCall();
  return result;
}

proxyService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      proxyService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< proxyService::Service, ::proxy_proto::CheckaliveCMD, ::proxy_proto::RequestResult, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](proxyService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::proxy_proto::CheckaliveCMD* req,
             ::proxy_proto::RequestResult* resp) {
               return service->checkalive(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      proxyService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< proxyService::Service, ::proxy_proto::ObjectAndPlacement, ::proxy_proto::SetReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](proxyService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::proxy_proto::ObjectAndPlacement* req,
             ::proxy_proto::SetReply* resp) {
               return service->EncodeAndSetObject(ctx, req, resp);
             }, this)));
}

proxyService::Service::~Service() {
}

::grpc::Status proxyService::Service::checkalive(::grpc::ServerContext* context, const ::proxy_proto::CheckaliveCMD* request, ::proxy_proto::RequestResult* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status proxyService::Service::EncodeAndSetObject(::grpc::ServerContext* context, const ::proxy_proto::ObjectAndPlacement* request, ::proxy_proto::SetReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace proxy_proto

