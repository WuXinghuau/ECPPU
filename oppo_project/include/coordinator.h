#ifndef COORDINATOR_H
#define COORDINATOR_H
#include "coordinator.grpc.pb.h"
#include "proxy.grpc.pb.h"
#include <grpc++/create_channel.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <meta_definition.h>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace OppoProject
{
  class CoordinatorImpl final
      : public coordinator_proto::CoordinatorService::Service
  {
  public:
    CoordinatorImpl() : cur_az(0) {}
    grpc::Status setParameter(
        ::grpc::ServerContext *context,
        const coordinator_proto::Parameter *parameter,
        coordinator_proto::RepIfSetParaSucess *setParameterReply)
        override;
    grpc::Status sayHelloToCoordinator(
        ::grpc::ServerContext *context,
        const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
        coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
        override;
    grpc::Status uploadOriginKeyValue(
        grpc::ServerContext *context,
        const coordinator_proto::RequestProxyIPPort *keyValueSize,
        coordinator_proto::ReplyProxyIPPort *proxyIPPort) override;

    grpc::Status checkalive(
        grpc::ServerContext *context,
        const coordinator_proto::RequestToCoordinator *helloRequestToCoordinator,
        coordinator_proto::ReplyFromCoordinator *helloReplyFromCoordinator)
        override;
    grpc::Status
    reportCommitAbort(grpc::ServerContext *context,
                      const coordinator_proto::CommitAbortKey *commit_abortkey,
                      coordinator_proto::ReplyFromCoordinator
                          *helloReplyFromCoordinator) override;
    grpc::Status
    requestRepair(::grpc::ServerContext *context,
                  const coordinator_proto::FailNodes *failed_node_list,
                  coordinator_proto::RepIfRepairSucess *reply) override;
    grpc::Status
    checkCommitAbort(grpc::ServerContext *context,
                     const coordinator_proto::AskIfSetSucess *key,
                     coordinator_proto::RepIfSetSucess *reply) override;
    grpc::Status
    getValue(::grpc::ServerContext *context,
             const coordinator_proto::KeyAndClientIP *keyClient,
             coordinator_proto::RepIfGetSucess *getReplyClient) override;

    grpc::Status
    updateReportSuccess(::grpc::ServerContext *context,
                        const coordinator_proto::CommitAbortKey *request,
                        coordinator_proto::ReplyFromCoordinator *response) override;

    grpc::Status
    checkUpdateFinished(::grpc::ServerContext *context,
                        const ::coordinator_proto::AskIfSetSucess *request,
                        coordinator_proto::RepIfSetSucess *response) override;

    bool init_AZinformation(std::string Azinformation_path);
    bool init_proxy(std::string proxy_information_path);
    void generate_placement(std::vector<unsigned int> &stripe_nodes, int stripe_id);
    void do_repair(int stripe_id, std::vector<int> failed_shard_idxs);
    void generate_repair_plan(int stripe_id, bool one_shard, std::vector<int> &failed_shard_idxs,
                              std::vector<std::vector<std::pair<std::pair<std::string, int>, int>>> &shards_to_read,
                              std::vector<int> &repair_span_az,
                              std::vector<std::pair<int, int>> &new_locations_with_shard_idx,
                              std::unordered_map<int, bool> &merge);

    // update
    grpc::Status
    updateGetLocation(::grpc::ServerContext *context,
                      const coordinator_proto::UpdatePrepareRequest *request,
                      coordinator_proto::UpdateDataLocation *data_location) override;
    grpc::Status
    updateRCW(::grpc::ServerContext *context,
                      const coordinator_proto::UpdatePrepareRequest *request,
                      coordinator_proto::UpdateDataLocation *data_location) override;
    grpc::Status
    updateRMW(::grpc::ServerContext *context,
                      const coordinator_proto::UpdatePrepareRequest *request,
                      coordinator_proto::UpdateDataLocation *data_location) override;
    grpc::Status 
    printVersion(::grpc::ServerContext *context,
                     const coordinator_proto::printVersionReq* request, 
                     coordinator_proto::printVersionResponse* response) override;
    grpc::Status
    PURMWGetLocation(::grpc::ServerContext *context,
                      const coordinator_proto::PURMWPrepareReq *request,
                      coordinator_proto::UpdateDataLocation *response) override;
    grpc::Status 
    ECPUGetLocation(::grpc::ServerContext *context,
                      const coordinator_proto::PURMWPrepareReq *request,
                      coordinator_proto::UpdateDataLocation *response) override;
    grpc::Status
    PURMWCheckFinished(::grpc::ServerContext *context,
                      const coordinator_proto::AskRMWPUSucess *request,
                      coordinator_proto::RepIfSetSucess *response) override;

    grpc::Status
    PURMWReportSuccess(::grpc::ServerContext *context,
                      const coordinator_proto::PURMWCoordinatorReply *request,
                      coordinator_proto::ReplyFromCoordinator *response) override;
  
  private:
    std::mutex m_mutex;
    int m_next_stripe_id = 0;
    int m_next_update_opration_id = 0;
    std::map<std::string, std::unique_ptr<proxy_proto::proxyService::Stub>>
        m_proxy_ptrs;
    std::unordered_map<std::string, ObjectItemBigSmall>
        m_object_table_big_small_updating;
    std::unordered_map<std::string, ObjectItemBigSmall>
        m_object_table_big_small_commit;
    ECSchema m_encode_parameter;
    std::map<unsigned int, AZitem> m_AZ_info;
    std::map<unsigned int, Nodeitem> m_Node_info;
    std::map<unsigned int, StripeItem> m_Stripe_info;
    int cur_az;
    std::condition_variable cv;
    /*for small object*/
    std::vector<int> buf_rest;
    StripeItem cur_stripe;
    int az_id_for_cur_stripe;
    std::string cur_smallobj_proxy_ip;
    int cur_smallobj_proxy_port;
    std::string cur_smallobj_proxy_ip_port;
    std::set<std::string> key_in_buffer;

    /*for buffer update*/
    std::unordered_map<std::string, int> m_obj_buffer_idx; // key->buffer_idx

    // update
    int m_updating_az_num = 0;
    std::map<unsigned int, std::vector<ShardidxRange>>
    split_update_length(std::string key, int update_offset_infile, int update_length);

    bool split_AZ_info(unsigned int temp_stripe_id, std::vector<ShardidxRange> &idx_ranges,
                       std::map<int, std::vector<ShardidxRange>> &AZ_updated_idxrange,
                       std::map<int, std::vector<int>> &AZ_global_parity_idx,
                       std::map<int, std::vector<int>> &AZ_local_parity_idx);

    bool fill_data_location(std::string key,StripeItem &temp_stripe, int shard_size, bool padding,
                            std::map<int, std::vector<ShardidxRange>> &AZ_updated_idxrange,
                            coordinator_proto::UpdateDataLocation *data_location);

    bool RMW(std::string key, int update_offset_infile, int update_length, coordinator_proto::UpdateDataLocation *data_location);
    bool RCW(std::string key, int update_offset_infile, int update_length, coordinator_proto::UpdateDataLocation *data_location);
    bool fill_update_plan(std::unordered_map<int, proxy_proto::DataProxyUpdatePlan> &dataproxy_notices,
                                          const std::map<int, std::vector<ShardidxRange>> &AZ_updated_idxrange,
                                          std::string key,StripeItem temp_stripe,int shard_update_offset,int shard_update_len);
    bool fill_reconstruct_write_plan(proxy_proto::ReconstructWriteNotice &reconstructor_notice,int stripeid);
    grpc::Status
    updateBuffer(std::string key,int update_offset_infile,int update_length,coordinator_proto::UpdateDataLocation *data_location);



    /*for multiple version and update*/
    std::map<unsigned int,std::vector<int>> 
      m_data_block_version;//stripeid->k datablock version
    std::map<unsigned int,std::vector<std::vector<int>>> 
      m_parity_block_version;//stripe id -> m v_version
    std::map<unsigned int,std::map<int,std::map<int,std::vector<int>>>> 
      m_partial_update_placement;//stripeid dataidx dataverion if_parity_receive_delta
    std::map<unsigned int,std::vector<int> > 
      m_temp_old_stripe_version;//stripeid->v_version，在同步交代校验块状态到最新前保存开始更新时的v_version、
    //通过old_v_version以及data idx  version标记DeltaP，
    std::mutex m_PU_Meta_lock;
    
    /*for outtime reconstruce,update one stripe once*/
    int m_PURMWupdating_stripeid=-1;
    int m_PURMWupdating_AZid=-1;
    int m_PURMWupate_stat=-1;
    int m_PU_update_method=-1;//PURMW  or RCW
    bool syn_stripe_parity(unsigned int stripeid);//根据 new_v_version old_v_version，以及partial_update_placement生成补全方案。

    /*
    grpc::Status  ECPUGetLocation(std::string key, int update_offset_infile,int  update_length);
    grpc::Status  ECPUGetLocation(::grpc::ServerContext *context,
                      const coordinator_proto::PURMWPrepareReq *request,
                      coordinator_proto::UpdateDataLocation *response);
    */

    grpc::Status RPCPURMW(int stripeid,std::vector<OppoProject::ShardidxRange> idx_ranges,
                          int shard_update_offset,int shard_update_len,
                          std::map<int, std::vector<ShardidxRange>> &AZ_updated_idxrange,std::string key,
                          std::map<int, std::vector<int>>& AZ_global_parity_idx,std::map<int, std::vector<int>> &AZ_local_parity_idx,
                          int minimal_tolerance,int max_wait_time_ms);
     
  };

  class Coordinator
  {
  public:
    Coordinator(
        std::string m_coordinator_ip_port,
        std::string m_Azinformation_path)
        : m_coordinator_ip_port{m_coordinator_ip_port},
          m_Azinformation_path{m_Azinformation_path}
    {
      m_coordinatorImpl.init_AZinformation(m_Azinformation_path);
      m_coordinatorImpl.init_proxy(m_Azinformation_path);
    };
    // Coordinator
    void Run()
    {
      grpc::EnableDefaultHealthCheckService(true);
      grpc::reflection::InitProtoReflectionServerBuilderPlugin();
      grpc::ServerBuilder builder;
      std::string server_address(m_coordinator_ip_port);
      builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
      builder.RegisterService(&m_coordinatorImpl);
      std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
      std::cout << "Server listening on " << server_address << std::endl;
      server->Wait();
    }

  private:
    std::string m_coordinator_ip_port;
    std::string m_Azinformation_path;
    OppoProject::CoordinatorImpl m_coordinatorImpl;
  };
} // namespace OppoProject

#endif