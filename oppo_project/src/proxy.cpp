#include "../include/proxy.h"
#include "jerasure.h"
#include "reed_sol.h"
#include "tinyxml2.h"
#include "toolbox.h"
#include "azure_lrc.h"
#include <thread>
#include <cassert>
#include <string>
#include <iomanip>
template <typename T>
inline T ceil(T const &A, T const &B)
{
  return T((A + B - 1) / B);
};
namespace OppoProject
{
  grpc::Status ProxyImpl::checkalive(grpc::ServerContext *context,
                                     const proxy_proto::CheckaliveCMD *request,
                                     proxy_proto::RequestResult *response)
  {

    std::cout << "checkalive" << request->name() << std::endl;
    response->set_message(false);
    init_coordinator();
    return grpc::Status::OK;
  }
  bool ProxyImpl::SetToMemcached(const char *key, size_t key_length,
                                 const char *value, size_t value_length, const char *ip, int port)
  {
    try
    {
      std::cout << "SetToMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
      
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::setw(3) << std::setfill('0')<<port%1000<<".log"<<std::endl;

      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = 0;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      std::vector<unsigned char> int_buf_value_size = OppoProject::int_to_bytes(value_length);
      asio::write(socket, asio::buffer(int_buf_value_size, int_buf_value_size.size()));

      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(value, value_length));
      std::string str(value,value_length);
      std::cout << "Value to Set is :"
                << " " << str << std::endl;
      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }
  bool ProxyImpl::VersionedBlockToNode(const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port,std::vector<int> v_version,
                             int idx,int k,OppoProject::DataNodeOp datanode_op)
  {
     try
    {
      std::cout << "DataSetToDataNodeWithVersion"
                << " " << std::string(ip) << " " << port << std::endl;
      
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::setw(3) <<std::setfill('0')<<port%1000<<".log"<<std::endl;

      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = datanode_op;
      OppoProject::send_int(socket,flag);
      OppoProject::send_int(socket,key_length);
      OppoProject::send_int(socket,value_length);
      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(value, value_length));

      //send verison
      if(idx<k)//data block version
      {
        OppoProject::send_int(socket,v_version[idx]);
      }
      else//v_versoin
      {
        OppoProject::send_vec_int(socket,v_version);
      }

      /*for debug*/
      std::string str(value,value_length);
      std::cout << "Value to Set is :"
                << " " << str << std::endl;

      /*check success*/
      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }

  bool ProxyImpl::DataSetToDisk(const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port,int version)
  {
    try
    {
      std::cout << "DataSetToDataNodeWithVersion"
                << " " << std::string(ip) << " " << port << std::endl;
      
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::to_string(port%1000)<<".log"<<std::endl;

      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = OppoProject::DiskDataSetWithVersion;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      std::vector<unsigned char> int_buf_value_size = OppoProject::int_to_bytes(value_length);
      asio::write(socket, asio::buffer(int_buf_value_size, int_buf_value_size.size()));

      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(value, value_length));

      //send verison
      std::vector<unsigned char> int_buf_version = OppoProject::int_to_bytes(OppoProject::initial_version_num);
      asio::write(socket, asio::buffer(int_buf_version, int_buf_version.size()));

      std::string str(value,value_length);
      std::cout << "Value to Set is :"
                << " " << str << std::endl;
      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;

  }
  bool ProxyImpl::ParitySetToDisk(const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port,
                                                 std::vector<int> v_version)
  {
    try
    {
      std::cout << "ParitySetToDataNodeWithVersion"
                << " " << std::string(ip) << " " << port << std::endl;
      
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::to_string(port%1000)<<".log"<<std::endl;

      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = OppoProject::DiskParitySetWithVersion;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      std::vector<unsigned char> int_buf_value_size = OppoProject::int_to_bytes(value_length);
      asio::write(socket, asio::buffer(int_buf_value_size, int_buf_value_size.size()));

      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(value, value_length));

      //send v_version
      OppoProject::send_int(socket,v_version.size());
      for(int i=0;i<v_version.size();i++)
      {
        OppoProject::send_int(socket,v_version[i]);
      }

      std::string str(value,value_length);
      std::cout << "Value to Set is :"
                << " " << str << std::endl;
      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }

  bool ProxyImpl::GetFromMemcached(const char *key, size_t key_length,
                                   char *value, size_t *value_length, int offset, int lenth, const char *ip, int port)
  {
    static std::mutex get_mutex;
    get_mutex.lock();
    try
    {
      std::cerr << "proxy GetFromMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::to_string(port%1000)<<".log"<<std::endl;
      std::cout<<"get key:"<<std::endl;
      std::cout<<std::string(key,key_length)<<std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);

      int flag = 1;
      std::vector<unsigned char> int_buf_flag = OppoProject::int_to_bytes(flag);
      asio::write(socket, asio::buffer(int_buf_flag, int_buf_flag.size()));

      std::vector<unsigned char> int_buf_key_size = OppoProject::int_to_bytes(key_length);
      asio::write(socket, asio::buffer(int_buf_key_size, int_buf_key_size.size()));

      asio::write(socket, asio::buffer(key, key_length));

      std::vector<unsigned char> int_buf_offset = OppoProject::int_to_bytes(offset);
      asio::write(socket, asio::buffer(int_buf_offset, int_buf_offset.size()));

      std::vector<unsigned char> int_buf_lenth = OppoProject::int_to_bytes(lenth);
      asio::write(socket, asio::buffer(int_buf_lenth, int_buf_lenth.size()));

      std::vector<unsigned char> value_from_datanode(lenth);
      
      // std::cerr << "getfromMemcached start read" << std::endl;
      // std::cerr << "getfromMemcached key " << key << std::endl;
      // std::cerr << "getfromMemcached offset " << offset << std::endl;
      // std::cerr << "getfromMemcached len" << lenth << std::endl;
      asio::read(socket, asio::buffer(value_from_datanode, value_from_datanode.size()));
      // std::cerr << "getfromMemcached finish read" << std::endl;
      std::cerr << "value_from_datanode.data() is " << value_from_datanode.data() << std::endl;

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);

      memcpy(value, value_from_datanode.data(), lenth);
      // std::cerr << "value_from_datanode is:" << value << std::endl;
    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }
    
    get_mutex.unlock();
    return true;
  }

  grpc::Status ProxyImpl::EncodeAndSetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response)
  {

    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    std::vector<unsigned int> stripe_ids;
    for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
    {
      stripe_ids.push_back(object_and_placement->stripe_ids(i));
    }
    std::vector<std::pair<std::string, int>> nodes_ip_and_port;
    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      nodes_ip_and_port.push_back(std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)));
    }
    auto encode_and_save = [this, big, key, value_size_bytes, k, m, real_l, shard_size, tail_shard_size, stripe_ids, nodes_ip_and_port, encode_type]() mutable
    {
      try
      {
        if (big == true)
        {
          asio::ip::tcp::socket socket_data(io_context);
          acceptor.accept(socket_data);
          asio::error_code error;

          int extend_value_size_byte = shard_size * k * stripe_ids.size();
          std::vector<char> buf_key(key.size());
          std::vector<char> v_buf(extend_value_size_byte);
          for (int i = value_size_bytes; i < extend_value_size_byte; i++)
          {
            v_buf[i] = '0';
          }
          // size_t len = asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
          asio::read(socket_data, asio::buffer(buf_key, key.size()), error);
          if (error == asio::error::eof)
          {
            std::cout << "error == asio::error::eof" << std::endl;
          }
          else if (error)
          {
            throw asio::system_error(error);
          }
          int flag = 1;
          for (int i = 0; i < int(key.size()); i++)
          {
            if (key[i] != buf_key[i])
            {
              flag = 0;
            }
          }
          if (flag)
          {
            // len = asio::read(socket_data, asio::buffer(v_buf, value_size_bytes), error);
            asio::read(socket_data, asio::buffer(v_buf, value_size_bytes), error);
          }
          asio::error_code ignore_ec;
          socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
          socket_data.close(ignore_ec);

          char *buf = v_buf.data();
          auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port)
          {
            std::vector<int> v_version(k,0);
            if (j < k)
            {
              //SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
              this->VersionedBlockToNode(shard_id.c_str(),shard_id.size(), data[j],  x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,v_version,
                                j,k, OppoProject::DiskDataSetWithVersion);
              //DataSetToDisk(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,OppoProject::initial_version_num);
            }
            else
            {
              //SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
              this->VersionedBlockToNode(shard_id.c_str(),shard_id.size(),coding[j - k],  x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,v_version,
                                j,k, OppoProject::DiskParitySetWithVersion);
              //ParitySetToDisk(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,std::vector<int>(k,OppoProject::initial_version_num));
            }
          };
          for (int i = 0; i < int(stripe_ids.size()); i++)
          {
            std::vector<char *> v_data(k);
            std::vector<char *> v_coding(m + real_l + 1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            int true_shard_size;
            if ((i == int(stripe_ids.size() - 1)) && tail_shard_size != -1)
            {
              true_shard_size = tail_shard_size;
            }
            else
            {
              true_shard_size = shard_size;
            }
            std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(true_shard_size));
            for (int j = 0; j < k; j++)
            {
              data[j] = &buf[j * true_shard_size];
            }
            for (int j = 0; j < m + real_l + 1; j++)
            {
              coding[j] = v_coding_area[j].data();
            }
            int send_num;
            if (encode_type == RS)
            {
              encode(k, m, 0, data, coding, true_shard_size, encode_type);
              send_num = k + m;
            }
            else if (encode_type == Azure_LRC_1)
            {
              // m = g for lrc
              encode(k, m, real_l, data, coding, true_shard_size, encode_type);
              send_num = k + m + real_l + 1;
            }
            else if (encode_type == OPPO_LRC)
            {
              encode(k, m, real_l, data, coding, true_shard_size, encode_type);
              send_num = k + m + real_l;
            }
            std::vector<std::thread> senders;
            for (int j = 0; j < send_num; j++)
            {
              std::string shard_id = std::to_string(stripe_ids[i] * 1000 + j);
              std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[i * send_num + j];
              senders.push_back(std::thread(send_to_datanode, j, k, shard_id, data, coding, true_shard_size, ip_and_port));
            }
            for (int j = 0; j < int(senders.size()); j++)
            {
              senders[j].join();
            }

            buf += (k * shard_size);
          }

          coordinator_proto::CommitAbortKey commit_abort_key;
          coordinator_proto::ReplyFromCoordinator result;
          grpc::ClientContext context;
          commit_abort_key.set_key(key);
          commit_abort_key.set_ifcommitmetadata(true);
          grpc::Status status;
          status = this->m_coordinator_stub->reportCommitAbort(
              &context, commit_abort_key, &result);
          if (status.ok())
          {
            std::cout << "connect coordinator,ok" << std::endl;
          }
          else
          {
            std::cout << "oooooH coordinator,fail!!!!" << std::endl;
          }
        }
      }
      catch (std::exception &e)
      {
        std::cout << "exception in encode_and_save" << std::endl;
        std::cout << e.what() << std::endl;
      }
    };
    try
    {
      std::thread my_thread(encode_and_save);
      my_thread.detach();
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }
    std::cout << "encode receive askDNhandling rpc!\n";

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::WriteBufferAndEncode(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::SetReply *response)
  {
    /*metadata record*/
    //std::unique_lock<std::mutex> lck(proxybuf_lock);
    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int value_size_bytes = object_and_placement->valuesizebyte();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    int buf_idx = object_and_placement->writebufferindex();
    /*init prxox buffer infomation*/
    static int init_flag = true;
    if (init_flag)
    {
      std::vector<std::vector<char>> tmp_buf(k, std::vector<char>(shard_size));
      std::vector<int> tmp_buf_off(k, 0);
      proxy_buf = tmp_buf;
      buf_offset = tmp_buf_off;
      init_flag = false;
    }
    if (buf_idx == -1)
    {
      /*encode old buffer*/
      std::vector<unsigned int> stripe_ids;
      for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
      {
        stripe_ids.push_back(object_and_placement->stripe_ids(i));
      }
      std::vector<std::pair<std::string, int>> nodes_ip_and_port;
      for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
      {
        nodes_ip_and_port.push_back(std::make_pair(object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)));
      }
      auto encode_buf_and_save = [this, big, key, value_size_bytes, k, m, real_l, shard_size, tail_shard_size, stripe_ids, nodes_ip_and_port, encode_type]() mutable
      {
        try
        {
          /*lambda expression for data_send*/
          // sem_init(&sem,0,0);
          auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port)
          {
            if (j < k)
            {
              SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
            else
            {
              SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
            }
          };
          // encoding
          std::vector<char *> v_data(k);
          std::vector<char *> v_coding(m + real_l + 1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          int true_shard_size = shard_size;
          std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(true_shard_size));
          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(true_shard_size));
          {// lock the proxy_buf
            std::lock_guard<std::mutex> lock(proxybuf_lock);
            for (int j = 0; j < k; j++)
            {
              data[j] = v_data_area[j].data();
              std::copy(proxy_buf[j].begin(), proxy_buf[j].end(), data[j]);
              std::cout << "proxy[" << j << "] is " << std::string(proxy_buf[j].begin(), proxy_buf[j].end()) << std::endl;
            }
            m_obj_offset_in_buffer.clear();
          }
          sem_post(&sem);
          for (int j = 0; j < m + real_l + 1; j++)
          {
            coding[j] = v_coding_area[j].data();
          }
          int send_num;
          if (encode_type == RS)
          {
            encode(k, m, 0, data, coding, true_shard_size, encode_type);
            send_num = k + m;
          }
          else if (encode_type == Azure_LRC_1)
          {
            // m = g for lrc
            encode(k, m, real_l, data, coding, true_shard_size, encode_type);
            send_num = k + m + real_l + 1;
          }
          else if (encode_type == OPPO_LRC)
          {
            encode(k, m, real_l, data, coding, true_shard_size, encode_type);
            send_num = k + m + real_l;
          }
          // set to memcached
          std::vector<std::thread> senders;
          for (int j = 0; j < send_num; j++)
          {
            if(j<k)
              std::cout << "data["<< j <<"] is :" << data[j] << std::endl;
            std::string shard_id = std::to_string(stripe_ids[0] * 1000 + j);
            std::pair<std::string, int> ip_and_port = nodes_ip_and_port[0 * send_num + j];
            senders.push_back(std::thread(send_to_datanode, j, k, shard_id, data, coding, true_shard_size, ip_and_port));
            std::cout << "shard_id : " << shard_id << " port: " << ip_and_port.second <<std::endl;
          }
          for (int j = 0; j < int(senders.size()); j++)
          {
            senders[j].join();
          }
        }
        catch (std::exception &e)
        {
          std::cout << "exception in encode_buf_and_save" << std::endl;
          std::cout << e.what() << std::endl;
        }
      };
      try
      {
        std::thread my_thread(encode_buf_and_save);
        my_thread.detach();
      }
      catch (std::exception &e)
      {
        std::cout << "exception" << std::endl;
        std::cout << e.what() << std::endl;
      }
      std::cout << "encode receive askDNhandling rpc!\n";
      /*reinit the proxy buffer(to lock)*/
      // sleep(1);
      sem_wait(&sem);
      std::cout << "reinit proxy_buf & buf_offset" << std::endl;
      proxy_buf = std::vector<std::vector<char>>(k, std::vector<char>(shard_size));
      memset(&buf_offset[0], 0, sizeof(buf_offset[0]) * buf_offset.size());
      //sem_destroy(&sem);
    }
    else
    {
      /*write data into proxy buffer*/
      auto write_buffer = [this, key, value_size_bytes, k, m, shard_size,
                           buf_idx]() mutable
      {
        asio::ip::tcp::socket socket_data(io_context);
        acceptor.accept(socket_data);
        asio::error_code error;

        /*read data*/
        std::vector<char> buf_key(key.size());
        size_t len = socket_data.read_some(asio::buffer(buf_key, key.size()), error);
        if (error == asio::error::eof)
        {
          std::cout << "error == asio::error::eof" << std::endl;
        }
        else if (error)
        {
          throw asio::system_error(error);
        }
        /*check key*/
        int flag = 1;
        for (int i = 0; i < key.size(); i++)
        {
          if (key[i] != buf_key[i])
          {
            flag = 0;
            std::cout << "write buffer key wrong!" << std::endl;
          }
        }
        /*write value to buffer*/
        if (flag)
        {
          std::vector<char> buf(value_size_bytes);
          len = socket_data.read_some(asio::buffer(buf, value_size_bytes), error);

          /*wxh update small in buffer*/

          if(m_obj_offset_in_buffer.find(key)!=m_obj_offset_in_buffer.end())
          {
            std::cout<<"update buffer: "<<buf_idx<<" offset: "<<m_obj_offset_in_buffer[key]<<" len: "<<value_size_bytes<<std::endl;
            std::copy(buf.begin(),buf.end(),proxy_buf[buf_idx].begin()+m_obj_offset_in_buffer[key]);
            std::string s(proxy_buf[buf_idx].begin()+m_obj_offset_in_buffer[key],proxy_buf[buf_idx].begin()+m_obj_offset_in_buffer[key]+len);
            std::cout << "proxy_buf updated write:" << s << std::endl;
          }
          else
          {
            std::copy(buf.begin(),buf.end(),proxy_buf[buf_idx].begin()+buf_offset[buf_idx]);
            // std::cout << "proxy_ip_port is :" << proxy_ip_port << std::endl; 
            // std::cout << "obj key is :" << key << std::endl;
            // std::cout << "buffer write:" << len << " bytes" << std::endl;
            // std::cout << "buffer idx:" << buf_idx << std::endl;
            // std::cout << "buffer offset:" << buf_offset[buf_idx] << " bytes" << std::endl;
            std::string s(proxy_buf[buf_idx].begin()+buf_offset[buf_idx],proxy_buf[buf_idx].begin()+buf_offset[buf_idx]+len);
            // std::cout << "proxy_buf write:" << s << std::endl;
            
            //for update
            m_obj_offset_in_buffer[key]=buf_offset[buf_idx];
            
            buf_offset[buf_idx] += value_size_bytes;
            // std::cout << "buffer offset after update:" << buf_offset[buf_idx] << " bytes" << std::endl;

          }
          
        }
        asio::error_code ignore_ec;
        socket_data.shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
        socket_data.close(ignore_ec);
        /*commitAbort*/
        coordinator_proto::CommitAbortKey commit_abort_key;
        coordinator_proto::ReplyFromCoordinator result;
        grpc::ClientContext context;
        commit_abort_key.set_key(key);
        commit_abort_key.set_ifcommitmetadata(true);
        grpc::Status status;

        /*wxh*/
        if(m_obj_offset_in_buffer.find(key)!=m_obj_offset_in_buffer.end())
        {
          status = this->m_coordinator_stub->updateReportSuccess(
            &context, commit_abort_key, &result);
        }
        else
        {
          status = this->m_coordinator_stub->reportCommitAbort(
            &context, commit_abort_key, &result);
        }
        
        if (status.ok())
        {
          std::cout << "connect coordinator,ok" << std::endl;
        }
        else
        {
          std::cout << "oooooH coordinator,fail!!!!" << std::endl;
        }
      };
      try
      {
        std::thread my_thread(write_buffer);
        my_thread.detach();
      }
      catch (std::exception &e)
      {
        std::cout << "exception in write buffer" << std::endl;
        std::cout << e.what() << std::endl;
      }
      std::cout << "receive askDNhandling rpc!\n";
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::decodeAndGetObject(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response)
  {
    OppoProject::EncodeType encode_type = (OppoProject::EncodeType)object_and_placement->encode_type();
    std::string key = object_and_placement->key();
    bool big = object_and_placement->bigobject();
    int k = object_and_placement->k();
    int m = object_and_placement->m();
    int real_l = object_and_placement->real_l();
    int shard_size = object_and_placement->shard_size();
    int tail_shard_size = object_and_placement->tail_shard_size();
    int value_size_bytes = object_and_placement->valuesizebyte();
    std::string clientip = object_and_placement->clientip();
    int clientport = object_and_placement->clientport();
    int obj_offset = object_and_placement->offset();
    int shard_idx = object_and_placement->shard_idx();
    int obj_size = object_and_placement->obj_size();
    std::vector<unsigned int> stripe_ids;

    // std::cerr << "stripe_ids_size is:  "<< object_and_placement->stripe_ids_size() << std::endl;
    // std::cerr << "shard_idx:  "<< object_and_placement->shard_idx() << std::endl;

    for (int i = 0; i < object_and_placement->stripe_ids_size(); i++)
    {
      stripe_ids.push_back(object_and_placement->stripe_ids(i));
    }

    std::vector<std::pair<std::string, int>> nodes_ip_and_port;

    for (int i = 0; i < object_and_placement->datanodeip_size(); i++)
    {
      nodes_ip_and_port.push_back({object_and_placement->datanodeip(i), object_and_placement->datanodeport(i)});
    }

    auto decode_and_get = [this, big, key, k, m, real_l, shard_size, tail_shard_size, value_size_bytes, obj_offset, shard_idx,obj_size,
                           clientip, clientport, stripe_ids, nodes_ip_and_port, encode_type]() mutable
    {
      if (big)
      {
        std::string value;
        for (int i = 0; i < int(stripe_ids.size()); i++)
        {
          auto shards_ptr = std::make_shared<std::vector<std::vector<char>>>();
          auto shards_idx_ptr = std::make_shared<std::vector<int>>();
          auto myLock_ptr = std::make_shared<std::mutex>();
          auto cv_ptr = std::make_shared<std::condition_variable>();
          int expect_block_number = (encode_type == Azure_LRC_1) ? (k + real_l - 1) : k;
          int all_expect_blocks = (encode_type == Azure_LRC_1) ? (k + m + real_l) : (k + m);
          int send_num;
          if (encode_type == RS)
          {
            send_num = k + m;
          }
          else if (encode_type == Azure_LRC_1)
          {
            // m = g for lrc
            send_num = k + m + real_l + 1;
          }
          else if (encode_type == OPPO_LRC)
          {
            send_num = k + m + real_l;
          }

          std::vector<char *> v_data(k);
          std::vector<char *> v_coding(all_expect_blocks - k);
          char **data = v_data.data();
          char **coding = v_coding.data();

          auto getFromNode = [this, k, shards_ptr, shards_idx_ptr, myLock_ptr, cv_ptr](int expect_block_number, int stripe_id, int shard_idx, int x_shard_size, std::string ip, int port)
          {
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            std::vector<char> temp(x_shard_size);
            size_t temp_size;
            bool ret = GetFromMemcached(shard_id.c_str(), shard_id.size(), temp.data(), &temp_size, 0, x_shard_size, ip.c_str(), port);

            if (!ret)
            {
              std::cout << "getFromNode !ret" << std::endl;
              return;
            }
            myLock_ptr->lock();

            if (!check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
            {
              shards_ptr->push_back(temp);
              shards_idx_ptr->push_back(shard_idx);
              if (check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
              {
                cv_ptr->notify_all();
              }
              // 检查已有的块是否满足要求
            }
            myLock_ptr->unlock();
          };

          int true_shard_size;
          if ((i == int(stripe_ids.size() - 1)) && tail_shard_size != -1)
          {
            true_shard_size = tail_shard_size;
          }
          else
          {
            true_shard_size = shard_size;
          }

          std::vector<std::vector<char>> v_data_area(k, std::vector<char>(true_shard_size));
          std::vector<std::vector<char>> v_coding_area(all_expect_blocks - k, std::vector<char>(true_shard_size));
          for (int j = 0; j < k; j++)
          {
            data[j] = v_data_area[j].data();
          }
          for (int j = 0; j < all_expect_blocks - k; j++)
          {
            coding[j] = v_coding_area[j].data();
          }
          std::vector<std::thread> read_memcached_treads;

          for (int j = 0; j < all_expect_blocks; j++)
          {
            std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[i * send_num + j];
            read_memcached_treads.push_back(std::thread(
                getFromNode, expect_block_number, stripe_ids[i], j, true_shard_size, ip_and_port.first, ip_and_port.second));
          }
          for (int j = 0; j < all_expect_blocks; j++)
          {
            read_memcached_treads[j].detach();
          }

          std::unique_lock<std::mutex> lck(*myLock_ptr);

          while (!check_received_block(k, expect_block_number, shards_idx_ptr, shards_ptr->size()))
          {
            cv_ptr->wait(lck);
          }
          std::cout<<"get key:"<<key<<std::endl;
          std::cout<<"get idx: "<<std::endl;//for debug
          for (int j = 0; j < int(shards_idx_ptr->size()); j++)
          {
            int idx = (*shards_idx_ptr)[j];
            std::cout<<" : "<<idx;//for debug
            if(idx==send_num-1 && encode_type==OppoProject::Azure_LRC_1) std::cout<<"get local of global"<<std::endl;
            if (idx < k)
            {
              memcpy(data[idx], (*shards_ptr)[j].data(), true_shard_size);
            }
            else
            {
              memcpy(coding[idx - k], (*shards_ptr)[j].data(), true_shard_size);
            }
          }
          std::cout<<std::endl;//for debug

          auto erasures = std::make_shared<std::vector<int>>();
          for (int j = 0; j < all_expect_blocks; j++)
          {
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), j) == shards_idx_ptr->end())
            {
              erasures->push_back(j);
            }
          }
          erasures->push_back(-1);
          if (encode_type == RS || encode_type == OPPO_LRC)
          {
            decode(k, m, 0, data, coding, erasures, true_shard_size, encode_type);
          }
          else if (encode_type == Azure_LRC_1)
          {
            if (!decode(k, m, real_l, data, coding, erasures, true_shard_size, encode_type))
            {
              std::cout << "cannot decode!" << std::endl;
            }
          }
          else
          {
            std::cout << "proxy解码类型出错了友友" << std::endl;
          }

          for (int j = 0; j < k; j++)
          {
            value += std::string(data[j], true_shard_size);
          }
        }
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(clientip, std::to_string(clientport));

        asio::ip::tcp::socket sock_data(io_context);
        asio::connect(sock_data, endpoints);

        asio::write(sock_data, asio::buffer(key, key.size()), error);
        asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
      }else{
        std::cerr << "proxy decode_and_get" << std::endl;
        std::string value;
        auto shards_ptr = std::make_shared<std::vector<std::vector<char>>>();
        auto shards_idx_ptr = std::make_shared<std::vector<int>>();
        auto myLock_ptr = std::make_shared<std::mutex>();
        auto cv_ptr = std::make_shared<std::condition_variable>();

        std::vector<char *> v_data(1);
        char **data = v_data.data();
        std::vector<char> v_data_area(obj_size);
        data[0] = v_data_area.data();
        auto getFromNode = [this, k, shards_ptr, shards_idx_ptr, myLock_ptr, cv_ptr,obj_offset,obj_size](int expect_block_number, int stripe_id, int shard_idx, int x_shard_size, std::string ip, int port)
        {
          // std::cerr << "proxy get_from_node" << std::endl;
          // std::cerr << "stripe_id: " << stripe_id <<std::endl;
          // std::cerr << "shard_idx: " << shard_idx << std::endl;
          // std::cerr << "obj_size: " << obj_size << std::endl;
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          // std::cerr << "proxy shard_id is " << shard_id <<std::endl;
          std::vector<char> temp(obj_size);
          size_t temp_size;
          // std::cerr << "get_from_memcached begin" << std::endl;
          bool ret = GetFromMemcached(shard_id.c_str(), shard_id.size(), temp.data(), &temp_size, obj_offset, obj_size, ip.c_str(), port);
          // std::cerr << "get_from_memcached finish" << std::endl;
          if (!ret)
          {
            std::cout << "getFromNode !ret" << std::endl;
            return;
          }
          // std::cerr << "get_from_memcached is " << temp.data() << std::endl;
          myLock_ptr->lock();

          if (!check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
          {
            shards_ptr->push_back(temp);
            shards_idx_ptr->push_back(shard_idx);
            if (check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
            {
              cv_ptr->notify_all();
            }
            // 检查已有的块是否满足要求
          }
          myLock_ptr->unlock();
        };

        int true_shard_size = shard_size;
        std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[0];
        // std::cerr << "stripeId: " << stripe_ids[0] << std::endl; 
        // std::cerr << "shard_idx: " << shard_idx << std::endl; 
        try
        {
          // std::cerr << "get_from_node_thread start" << std::endl;
          std::thread read_memcached_tread(
            getFromNode, 1, stripe_ids[0], shard_idx, true_shard_size, ip_and_port.first, ip_and_port.second);
          read_memcached_tread.detach();
          // std::cerr << "get_from_node_thread detach" << std::endl;
        }
        catch (std::exception &e)
        {
          std::cout << "exception" << std::endl;
          std::cout << e.what() << std::endl;
        }
        std::unique_lock<std::mutex> lck(*myLock_ptr);
        while (!check_received_block(k, 1, shards_idx_ptr, shards_ptr->size()))
        {
          cv_ptr->wait(lck);
        }
        // std::cerr << "shards_ptr size(): " << shards_ptr->size() << std::endl;
        // std::cerr << "shards_ptr[0] size(): " << (*shards_ptr)[0].size() << std::endl;
        // std::cerr << "obj_size is "<< obj_size << std::endl;
        // std::cerr << "before decode_and_get's memcpy" << std::endl;
        memcpy(data[0], (*shards_ptr)[0].data(), obj_size);
        // std::cerr << "after decode_and_get's memcpy" << std::endl;
        value += std::string(data[0], obj_size);
        // std::cerr<< "length of value " << value.size() << std::endl;
        // std::cerr<< "value is" << value.data() << std::endl;
        asio::error_code error;
        asio::ip::tcp::resolver resolver(io_context);
        asio::ip::tcp::resolver::results_type endpoints =
            resolver.resolve(clientip, std::to_string(clientport));

        asio::ip::tcp::socket sock_data(io_context);
        asio::connect(sock_data, endpoints);

        asio::write(sock_data, asio::buffer(key, key.size()), error);
        asio::write(sock_data, asio::buffer(value, value_size_bytes), error);
        asio::error_code ignore_ec;
        sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
        sock_data.close(ignore_ec);
      }
    };
    try
    {
      // std::cerr << "decode_and_get_thread start" << std::endl;
      std::thread my_thread(decode_and_get);
      my_thread.detach();
      // std::cerr << "decode_and_get_thread detach" << std::endl;
    }
    catch (std::exception &e)
    {
      std::cout << "exception" << std::endl;
      std::cout << e.what() << std::endl;
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::getObjectFromBuffer(
      grpc::ServerContext *context,
      const proxy_proto::ObjectAndPlacement *object_and_placement,
      proxy_proto::GetReply *response)
  {
    // std::cout << "start get object from buffer!" << std::endl;
    /*find the obj*/
    std::string key = object_and_placement->key();
    int obj_offset = object_and_placement->offset();
    int shard_idx = object_and_placement->shard_idx();
    int obj_size = object_and_placement->obj_size();
    std::string clientip = object_and_placement->clientip();
    int clientport = object_and_placement->clientport();
    std::vector<char> v_data_area(obj_size);
    char *data = v_data_area.data();
    // std::cout << "read from buffer key: " << key << std::endl;
    // std::cout << "read from buffer shard_idx: " << shard_idx << std::endl;
    // std::cout << "read from buffer offset: " << obj_offset << std::endl;
    // std::cout << "read from buffer size: " << obj_size << std::endl;
    std::copy(proxy_buf[shard_idx].begin()+obj_offset, proxy_buf[shard_idx].begin()+obj_offset+obj_size, data);
    // std::cout << "read from buffer: " << data << std::endl;
    asio::error_code error;
    asio::ip::tcp::resolver resolver(io_context);
    asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(clientip, std::to_string(clientport));
    asio::ip::tcp::socket sock_data(io_context);
    asio::connect(sock_data, endpoints);
    asio::write(sock_data, asio::buffer(key, key.size()), error);
    asio::write(sock_data, asio::buffer(data, obj_size), error);
    asio::error_code ignore_ec;
    sock_data.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
    sock_data.close(ignore_ec);
    return grpc::Status::OK;
  }

  bool
  ProxyImpl::init_coordinator()
  {
    std::string coordinator_ip_port = coordinator_ip + ":55555";
    m_coordinator_stub =
        coordinator_proto::CoordinatorService::NewStub(grpc::CreateChannel(
            coordinator_ip_port, grpc::InsecureChannelCredentials()));
    return true;
  }

  grpc::Status ProxyImpl::mainRepair(
      grpc::ServerContext *context,
      const proxy_proto::mainRepairPlan *mainRepairPlan,
      proxy_proto::mainRepairReply *reply)
  {
    std::cout << "mainRepair" << std::endl;
    bool one_shard_fail = mainRepairPlan->one_shard_fail();
    std::vector<std::pair<std::pair<std::string, int>, int>> inner_az_shards_to_read;
    for (int i = 0; i < mainRepairPlan->inner_az_help_shards_ip_size(); i++)
    {
      inner_az_shards_to_read.push_back({{mainRepairPlan->inner_az_help_shards_ip(i), mainRepairPlan->inner_az_help_shards_port(i)},
                                         mainRepairPlan->inner_az_help_shards_idx(i)});
    }
    int k = mainRepairPlan->k();
    int g = mainRepairPlan->g();
    int real_l = mainRepairPlan->real_l();
    std::vector<std::pair<std::pair<std::string, int>, int>> new_locations_with_shard_idx;
    for (int i = 0; i < mainRepairPlan->new_location_ip_size(); i++)
    {
      new_locations_with_shard_idx.push_back({{mainRepairPlan->new_location_ip(i), mainRepairPlan->new_location_port(i)},
                                              mainRepairPlan->new_location_shard_idx(i)});
    }
    int self_az_id = mainRepairPlan->self_az_id();
    bool if_partial_decoding = mainRepairPlan->if_partial_decoding();
    std::vector<int> help_azs_id;
    std::unordered_map<int, bool> merge;
    for (int i = 0; i < mainRepairPlan->help_azs_id_size(); i++)
    {
      help_azs_id.push_back(mainRepairPlan->help_azs_id(i));
    }
    if (if_partial_decoding && one_shard_fail == false)
    {
      for (int i = 0; i < mainRepairPlan->merge_size(); i++)
      {
        merge[help_azs_id[i]] = mainRepairPlan->merge(i);
      }
    }
    int stripe_id = mainRepairPlan->stripe_id();
    int shard_size = mainRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(mainRepairPlan->encode_type());
    std::vector<int> all_failed_shards_idx;
    for (int i = 0; i < mainRepairPlan->all_failed_shards_idx_size(); i++)
    {
      all_failed_shards_idx.push_back(mainRepairPlan->all_failed_shards_idx(i));
    }
    partial_helper data_or_parity;
    std::vector<std::thread> readers_inner_az;
    std::vector<std::thread> readers_other_az;

    if (one_shard_fail)
    {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock(); }));
      }
      for (int i = 0; i < int(help_azs_id.size()); i++)
      {
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*socket_ptr);
        readers_other_az.push_back(std::thread([&, socket_ptr]()
                                               {
          std::vector<unsigned char> int_buf(sizeof(int));
          asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
          int az_id = OppoProject::bytes_to_int(int_buf);
          if (if_partial_decoding) {
            std::vector<char> buf(shard_size);
            asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
            repair_buffer_lock.lock();
            data_or_parity[az_id][-1] = buf;
            repair_buffer_lock.unlock();
          } else {
            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int num_of_shard = OppoProject::bytes_to_int(int_buf);
            for (int j = 0; j < num_of_shard; j++) {
              asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
              int shard_idx = OppoProject::bytes_to_int(int_buf);
              std::vector<char> buf(shard_size);
              asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
              repair_buffer_lock.lock();
              data_or_parity[az_id][shard_idx] = buf;
              repair_buffer_lock.unlock();
            }
          }
          asio::error_code ignore_ec;
          socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
          socket_ptr->close(ignore_ec); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      for (auto &th : readers_other_az)
      {
        th.join();
      }
      std::vector<char> repaired_shard(shard_size);
      int failed_shard_idx = new_locations_with_shard_idx[0].second;
      if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
      {
        int g_row = failed_shard_idx - k;
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
        std::vector<int> matrix(g * k, 0);
        memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
        free(rs_matrix);
        int *coefficients = matrix.data() + g_row * k;
        int count1 = data_or_parity[self_az_id].size();
        int count2 = 0;
        for (auto &p : data_or_parity)
        {
          if (p.first != self_az_id)
          {
            count2 += p.second.size();
          }
        }
        std::vector<char *> v_data(count1 + count2);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(count1 + count2, 1);
        int idx = 0;
        for (auto &q : data_or_parity[self_az_id])
        {
          data[idx] = q.second.data();
          std::cout << new_matrix[idx] << " ";
          new_matrix[idx] = coefficients[q.first];
          idx++;
        }
        for (auto &p : data_or_parity)
        {
          if (p.first != self_az_id)
          {
            for (auto &q : p.second)
            {
              data[idx] = q.second.data();
              if (!if_partial_decoding)
              {
                std::cout << new_matrix[idx] << " ";
                new_matrix[idx] = coefficients[q.first];
              }
              idx++;
            }
          }
        }
        std::cout << std::endl;
        coding[0] = repaired_shard.data();
        jerasure_matrix_encode(count1 + count2, 1, 8, new_matrix.data(), data, coding, shard_size);
      }
      else
      {
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        int idx = 0;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            data[idx++] = q.second.data();
          }
        }
        coding[0] = repaired_shard.data();
        std::vector<int> new_matrix(1 * count, 1);
        jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
      }
      std::string &new_node_ip = new_locations_with_shard_idx[0].first.first;
      int new_node_port = new_locations_with_shard_idx[0].first.second;
      std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_shard_idx);
      SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), repaired_shard.data(), shard_size, new_node_ip.c_str(), new_node_port);
    }
    else
    {
      std::vector<int> failed_data_and_parity;
      int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);

      for (int i = 0; i < int(all_failed_shards_idx.size()); i++)
      {
        if (all_failed_shards_idx[i] < all_shard_number)
        {
          failed_data_and_parity.push_back(all_failed_shards_idx[i]);
        }
      }
      std::set<int> func_idx;
      if (if_partial_decoding)
      {
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < all_shard_number)
          {
            func_idx.insert(failed_data_and_parity[i]);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (func_idx.size() == failed_data_and_parity.size())
          {
            break;
          }
          func_idx.insert(i);
        }
      }
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
            std::string &ip = inner_az_shards_to_read[i].first.first;
            int port = inner_az_shards_to_read[i].first.second;
            int shard_idx = inner_az_shards_to_read[i].second;
            std::vector<char> buf(shard_size);
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            size_t temp_size;
            GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
            repair_buffer_lock.lock();
            data_or_parity[self_az_id][shard_idx] = buf;
            repair_buffer_lock.unlock(); }));
      }
      for (int i = 0; i < int(help_azs_id.size()); i++)
      {
        std::shared_ptr<asio::ip::tcp::socket> socket_ptr = std::make_shared<asio::ip::tcp::socket>(io_context);
        acceptor.accept(*socket_ptr);
        readers_other_az.push_back(std::thread([&, socket_ptr]()
                                               {
            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int az_id = OppoProject::bytes_to_int(int_buf);
            asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
            int num_of_shard = OppoProject::bytes_to_int(int_buf);
            for (int j = 0; j < num_of_shard; j++) {
              asio::read(*socket_ptr, asio::buffer(int_buf, int_buf.size()));
              if (if_partial_decoding && merge[az_id]) {
                int temp_func_idx = OppoProject::bytes_to_int(int_buf);
                std::vector<char> buf(shard_size);
                asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
                repair_buffer_lock.lock();
                data_or_parity[az_id][temp_func_idx] = buf;
                repair_buffer_lock.unlock();
              } else {
                int shard_idx = OppoProject::bytes_to_int(int_buf);
                std::vector<char> buf(shard_size);
                asio::read(*socket_ptr, asio::buffer(buf, buf.size()));
                repair_buffer_lock.lock();
                data_or_parity[az_id][shard_idx] = buf;
                repair_buffer_lock.unlock();
              }
            }
            asio::error_code ignore_ec;
            socket_ptr->shutdown(asio::ip::tcp::socket::shutdown_receive, ignore_ec);
            socket_ptr->close(ignore_ec); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      for (auto &th : readers_other_az)
      {
        th.join();
      }
      if (if_partial_decoding)
      {
        for (auto &p : merge)
        {
          std::cout << p.second << " && ";
        }
        std::cout << std::endl;
        std::vector<int> matrix;
        if (encode_type == Azure_LRC_1)
        {
          matrix.resize((g + real_l) * k);
          lrc_make_matrix(k, g, real_l, matrix.data());
        }
        else
        {
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
        }

        for (auto &p : data_or_parity)
        {
          if (merge[p.first] == false || p.first == self_az_id)
          {
            std::vector<std::pair<int, std::vector<char>>> saved_merge;
            std::vector<char> merge_result(shard_size);
            int count = p.second.size();
            std::vector<char *> v_data(count);
            std::vector<char *> v_coding(1);
            char **data = (char **)v_data.data();
            char **coding = (char **)v_coding.data();
            for (auto &t : func_idx)
            {
              std::vector<int> new_matrix(1 * count, 1);
              int real_idx = t - k;
              int *coff = &(matrix[real_idx * k]);
              int idx = 0;
              for (auto &q : p.second)
              {
                if (q.first < k)
                {
                  new_matrix[idx] = coff[q.first];
                }
                else
                {
                  new_matrix[idx] = (q.first == t);
                }
                data[idx] = q.second.data();
                idx++;
              }
              int sum = 0;
              for (auto &num : new_matrix)
              {
                sum += num;
              }
              coding[0] = merge_result.data();
              jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
              if (sum == 0)
              {
                std::vector<char> temp(shard_size, 0);
                merge_result = temp;
              }
              saved_merge.push_back({t, merge_result});
            }
            p.second.clear();
            for (auto &q : saved_merge)
            {
              p.second[q.first] = q.second;
            }
          }
        }
        std::unordered_map<int, std::vector<char>> right;
        for (auto &p : func_idx)
        {
          int count = data_or_parity.size();
          std::vector<char *> v_data(count);
          std::vector<char *> v_coding(1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          std::vector<char> temp(shard_size);
          int idx = 0;
          for (auto &q : data_or_parity)
          {
            data[idx++] = q.second[p].data();
          }
          coding[0] = temp.data();
          std::vector<int> new_matrix(1 * count, 1);
          jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
          right[p] = temp;
        }
        std::vector<int> left;
        std::sort(failed_data_and_parity.begin(), failed_data_and_parity.end());
        for (auto &p : failed_data_and_parity)
        {
          std::cout << p << " ";
        }
        std::cout << std::endl;
        for (auto &p : func_idx)
        {
          int real_idx = p - k;
          int *coff = &(matrix[real_idx * k]);
          for (int i = 0; i < int(failed_data_and_parity.size()); i++)
          {
            if (failed_data_and_parity[i] < k)
            {
              left.push_back(coff[failed_data_and_parity[i]]);
            }
            else
            {
              left.push_back(failed_data_and_parity[i] == p);
            }
          }
        }
        std::vector<int> invert(left.size());
        jerasure_invert_matrix(left.data(), invert.data(), failed_data_and_parity.size(), 8);
        std::vector<char *> v_data(failed_data_and_parity.size());
        std::vector<char *> v_coding(failed_data_and_parity.size());
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> repaired_shards(failed_data_and_parity.size(), std::vector<char>(shard_size));
        int idx = 0;
        for (auto &p : func_idx)
        {
          std::cout << p << " ";
          data[idx] = right[p].data();
          coding[idx] = repaired_shards[idx].data();
          idx++;
        }
        std::cout << std::endl;
        jerasure_matrix_encode(failed_data_and_parity.size(), failed_data_and_parity.size(), 8, invert.data(), data, coding, shard_size);
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          int j = 0;
          for (; j < int(new_locations_with_shard_idx.size()); j++)
          {
            if (new_locations_with_shard_idx[j].second == failed_data_and_parity[i])
            {
              break;
            }
          }
          std::string &new_node_ip = new_locations_with_shard_idx[j].first.first;
          int new_node_port = new_locations_with_shard_idx[j].first.second;
          std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_data_and_parity[i]);
          SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), coding[i], shard_size, new_node_ip.c_str(), new_node_port);
        }
      }
      else
      {
        int help_area_size = (encode_type == Azure_LRC_1) ? (g + real_l) : g;
        int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);
        std::vector<char *> v_data(k);
        std::vector<char *> v_coding(help_area_size);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> help_area(help_area_size, std::vector<char>(shard_size));
        std::unordered_set<int> help;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            help.insert(q.first);
            if (q.first >= k)
            {
              coding[q.first - k] = q.second.data();
            }
            else
            {
              data[q.first] = q.second.data();
            }
          }
        }
        int idx = 0;
        auto erasures = std::make_shared<std::vector<int>>();
        for (int i = 0; i < k; i++)
        {
          if (help.count(i) == 0)
          {
            data[i] = help_area[idx++].data();
            erasures->push_back(i);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (help.count(i) == 0)
          {
            coding[i - k] = help_area[idx++].data();
            erasures->push_back(i);
          }
        }
        erasures->push_back(-1);

        if (encode_type == Azure_LRC_1)
        {
          if (!decode(k, g, real_l, data, coding, erasures, shard_size, encode_type, true))
          {

            std::cout << "cannot decode!" << std::endl;
          }
        }
        else
        {
          decode(k, g, 0, data, coding, erasures, shard_size, encode_type);
        }
        for (int i = 0; i < int(new_locations_with_shard_idx.size()); i++)
        {
          int failed_shard_idx = new_locations_with_shard_idx[i].second;
          if (failed_shard_idx < all_shard_number)
          {
            std::string &new_node_ip = new_locations_with_shard_idx[i].first.first;
            int new_node_port = new_locations_with_shard_idx[i].first.second;
            std::string failed_shard_id = std::to_string(stripe_id * 1000 + failed_shard_idx);
            char *transform;
            if (failed_shard_idx >= k)
            {
              transform = coding[failed_shard_idx - k];
            }
            else
            {
              transform = data[failed_shard_idx];
            }
            SetToMemcached(failed_shard_id.c_str(), failed_shard_id.size(), transform, shard_size, new_node_ip.c_str(), new_node_port);
          }
        }
      }
    }
    std::cout << "main repair done" << std::endl;
    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::helpRepair(
      grpc::ServerContext *context,
      const proxy_proto::helpRepairPlan *helpRepairPlan,
      proxy_proto::helpRepairReply *reply)
  {
    std::cout << "helpRepair" << std::endl;
    bool one_shard_fail = helpRepairPlan->one_shard_fail();
    std::vector<std::pair<std::pair<std::string, int>, int>> inner_az_shards_to_read;
    for (int i = 0; i < helpRepairPlan->inner_az_help_shards_ip_size(); i++)
    {
      inner_az_shards_to_read.push_back({{helpRepairPlan->inner_az_help_shards_ip(i), helpRepairPlan->inner_az_help_shards_port(i)},
                                         helpRepairPlan->inner_az_help_shards_idx(i)});
    }
    int k = helpRepairPlan->k();
    int g = helpRepairPlan->g();
    int real_l = helpRepairPlan->real_l();
    int self_az_id = helpRepairPlan->self_az_id();
    bool if_partial_decoding = helpRepairPlan->if_partial_decoding();
    int stripe_id = helpRepairPlan->stripe_id();
    int shard_size = helpRepairPlan->shard_size();
    EncodeType encode_type = EncodeType(helpRepairPlan->encode_type());
    std::string main_proxy_ip = helpRepairPlan->main_proxy_ip();
    int main_proxy_port = helpRepairPlan->main_proxy_port();
    int failed_shard_idx = helpRepairPlan->failed_shard_idx();
    std::vector<int> all_failed_shards_idx;
    for (int i = 0; i < helpRepairPlan->all_failed_shards_idx_size(); i++)
    {
      all_failed_shards_idx.push_back(helpRepairPlan->all_failed_shards_idx(i));
    }
    bool merge;
    if (if_partial_decoding && one_shard_fail == false)
    {
      merge = helpRepairPlan->merge();
    }
    std::unordered_map<int, std::unordered_map<int, std::vector<char>>> data_or_parity;
    std::vector<std::thread> readers_inner_az;

    if (one_shard_fail)
    {
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
          std::string &ip = inner_az_shards_to_read[i].first.first;
          int port = inner_az_shards_to_read[i].first.second;
          int shard_idx = inner_az_shards_to_read[i].second;
          std::vector<char> buf(shard_size);
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          size_t temp_size;
          GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
          repair_buffer_lock.lock();
          data_or_parity[self_az_id][shard_idx] = buf;
          repair_buffer_lock.unlock(); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(main_proxy_ip, std::to_string(main_proxy_port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::vector<unsigned char> int_buf_self_az_id = OppoProject::int_to_bytes(self_az_id);
      asio::write(socket, asio::buffer(int_buf_self_az_id, int_buf_self_az_id.size()));
      if (if_partial_decoding)
      {
        std::vector<char> merge_result(shard_size, 1);
        std::vector<int> matrix;
        int *coefficients = NULL;
        if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
        {
          int g_row = failed_shard_idx - k;
          std::cout << "k: " << k << " g: " << g << " failed_shard_idx: " << failed_shard_idx << std::endl;
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
          for (int i = 0; i < g; i++)
          {
            for (int j = 0; j < k; j++)
            {
              std::cout << matrix[i * k + j] << " ";
            }
            std::cout << std::endl;
          }
          coefficients = matrix.data() + g_row * k;
        }
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char *> v_data(count);
        std::vector<char *> v_coding(1);
        char **data = (char **)v_data.data();
        char **coding = (char **)v_coding.data();
        std::vector<int> new_matrix(1 * count, 1);
        int idx = 0;
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            data[idx] = q.second.data();
            if (encode_type == RS && (failed_shard_idx >= k && failed_shard_idx <= (k + g - 1)))
            {
              new_matrix[idx] = coefficients[q.first];
            }
            idx++;
          }
        }
        coding[0] = merge_result.data();
        jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
        asio::write(socket, asio::buffer(merge_result, merge_result.size()));
      }
      else
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(inner_az_shards_to_read.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            std::vector<unsigned char> int_buf_shard_idx = OppoProject::int_to_bytes(q.first);
            asio::write(socket, asio::buffer(int_buf_shard_idx, int_buf_shard_idx.size()));
            asio::write(socket, asio::buffer(q.second, q.second.size()));
          }
        }
      }
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      socket.close(ignore_ec);
    }
    else
    {
      int all_shard_number = (encode_type == Azure_LRC_1) ? (k + g + real_l) : (k + g);
      std::vector<int> failed_data_and_parity;

      for (int i = 0; i < int(all_failed_shards_idx.size()); i++)
      {
        if (all_failed_shards_idx[i] < all_shard_number)
        {
          failed_data_and_parity.push_back(all_failed_shards_idx[i]);
        }
      }
      std::set<int> func_idx;
      if (if_partial_decoding)
      {
        for (int i = 0; i < int(failed_data_and_parity.size()); i++)
        {
          if (failed_data_and_parity[i] >= k && failed_data_and_parity[i] < all_shard_number)
          {
            func_idx.insert(failed_data_and_parity[i]);
          }
        }
        for (int i = k; i < all_shard_number; i++)
        {
          if (func_idx.size() == failed_data_and_parity.size())
          {
            break;
          }
          func_idx.insert(i);
        }
      }
      for (int i = 0; i < int(inner_az_shards_to_read.size()); i++)
      {
        readers_inner_az.push_back(std::thread([&, i]()
                                               {
            std::string &ip = inner_az_shards_to_read[i].first.first;
            int port = inner_az_shards_to_read[i].first.second;
            int shard_idx = inner_az_shards_to_read[i].second;
            std::vector<char> buf(shard_size);
            std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
            size_t temp_size;
            GetFromMemcached(shard_id.c_str(), shard_id.size(), buf.data(), &temp_size, 0, shard_size, ip.c_str(), port);
            repair_buffer_lock.lock();
            data_or_parity[self_az_id][shard_idx] = buf;
            repair_buffer_lock.unlock(); }));
      }
      for (auto &th : readers_inner_az)
      {
        th.join();
      }
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(main_proxy_ip, std::to_string(main_proxy_port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::vector<unsigned char> int_buf_self_az_id = OppoProject::int_to_bytes(self_az_id);
      asio::write(socket, asio::buffer(int_buf_self_az_id, int_buf_self_az_id.size()));
      if (if_partial_decoding && merge)
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(func_idx.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));

        std::vector<int> matrix;
        if (encode_type == Azure_LRC_1)
        {
          matrix.resize((g + real_l) * k);
          lrc_make_matrix(k, g, real_l, matrix.data());
        }
        else
        {
          int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, g, 8);
          matrix.resize(g * k);
          memcpy(matrix.data(), rs_matrix, g * k * sizeof(int));
          free(rs_matrix);
        }
        int count = 0;
        for (auto &p : data_or_parity)
        {
          count += p.second.size();
        }
        std::vector<char> merge_result(shard_size);
        for (auto &p : func_idx)
        {
          std::vector<char *> v_data(count);
          std::vector<char *> v_coding(1);
          char **data = (char **)v_data.data();
          char **coding = (char **)v_coding.data();
          std::vector<int> new_matrix(1 * count, 1);
          int real_idx = p - k;
          int *coff = &(matrix[real_idx * k]);
          int idx = 0;
          for (auto &q : data_or_parity[self_az_id])
          {
            if (q.first < k)
            {
              new_matrix[idx] = coff[q.first];
            }
            else
            {
              new_matrix[idx] = (q.first == p);
            }
            data[idx] = q.second.data();
            idx++;
          }
          int sum = 0;
          for (auto &num : new_matrix)
          {
            sum += num;
          }
          coding[0] = merge_result.data();
          jerasure_matrix_encode(count, 1, 8, new_matrix.data(), data, coding, shard_size);
          if (sum == 0)
          {
            std::vector<char> temp(shard_size, 0);
            merge_result = temp;
          }
          std::vector<unsigned char> int_buf_func_idx = OppoProject::int_to_bytes(p);
          asio::write(socket, asio::buffer(int_buf_func_idx, int_buf_func_idx.size()));
          asio::write(socket, asio::buffer(merge_result, merge_result.size()));
        }
      }
      else
      {
        std::vector<unsigned char> int_buf_num_of_shards = OppoProject::int_to_bytes(inner_az_shards_to_read.size());
        asio::write(socket, asio::buffer(int_buf_num_of_shards, int_buf_num_of_shards.size()));
        for (auto &p : data_or_parity)
        {
          for (auto &q : p.second)
          {
            std::vector<unsigned char> int_buf_shard_idx = OppoProject::int_to_bytes(q.first);
            asio::write(socket, asio::buffer(int_buf_shard_idx, int_buf_shard_idx.size()));
            asio::write(socket, asio::buffer(q.second, q.second.size()));
          }
        }
      }
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_send, ignore_ec);
      socket.close(ignore_ec);
    }
    std::cout << "helpRepair done" << std::endl;
    return grpc::Status::OK;
  }

  
  grpc::Status ProxyImpl::dataProxyUpdate(
      grpc::ServerContext *context,
      const proxy_proto::DataProxyUpdatePlan *dataProxyPlan,
      proxy_proto::DataProxyReply *reply)
  { 
    
    std::cout<<"there is data proxy"<<std::endl;
    auto receive_update = [this](proxy_proto::DataProxyUpdatePlan data_proxy_plan)
    {
      //从client收数据需要的结构：
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
      std::vector<int> localparity_idxes;
      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();

      //本AZ相关
      std::vector<int> globalparity_idxes;  
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      std::unordered_map<int,std::vector<char> > old_shard_data_map;
      std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的

      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
      
      
      std::cout<<"idx ranges size:" <<data_idx_ranges.size()<<std::endl;
      for(auto const & ttt:data_idx_ranges)
        std::cout<<":"<<ttt.first;
      std::cout<<std::endl;
      
      std::cout<<"local info"<<std::endl;
      for(int j=0;j<localparity_idxes.size();j++)
      {
        std::cout<<"local idx:"<<localparity_idxes[j]<<std::endl;;
        std::cout<<data_nodes_ip_port[localparity_idxes[j]].first<<":"<<data_nodes_ip_port[localparity_idxes[j]].second<<std::endl;
      }
      for(int j=0;j<globalparity_idxes.size();j++)
      {
        std::cout<<"global idx:"<<globalparity_idxes[j]<<std::endl;;
        std::cout<<data_nodes_ip_port[globalparity_idxes[j]].first<<":"<<data_nodes_ip_port[globalparity_idxes[j]].second<<std::endl;
      }
          
     

      
      try
      {
        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }
        

        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cout<<"getting from data node fails"<<std::endl;
            return;
          } 
        };

        //2. 给old data 以及delta分配空间
        
        std::cout<<"2. 给old data 以及delta分配空间"<<std::endl;;
        for(auto const & ttt:data_idx_ranges)
        {
          int idx=ttt.first;
          int len=data_idx_ranges[idx].range_length;
          std::vector<char> old_data(len);
          std::vector<char> data_delta(len);
          old_shard_data_map[idx]=old_data;//need to assignment
          cur_az_data_delta_map[idx]=data_delta;
          std::cout<<"old and cur delta size:"<<old_shard_data_map[idx].size()<<" :"<<cur_az_data_delta_map[idx].size()<<std::endl;
        }

        //3. read old
        for(auto const & ttt : data_idx_ranges){
          int idx=ttt.first;
          int offset=ttt.second.offset_in_shard;
          int len=ttt.second.range_length;

          std::cout<<"len receive from coordinator"<<std::endl;

          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }

        //4. 计算本AZ内 delta
        std::vector<std::thread> delta_calculators;

        for(auto const & ttt : data_idx_ranges){//read old
          int idx=ttt.first;
          int len=ttt.second.range_length;
          calculate_data_delta(new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len);
          std::cout<<"in proxy"<<std::endl;
          std::cout<<std::string(cur_az_data_delta_map[idx].data(),cur_az_data_delta_map[idx].size())<<std::endl;
        
          //delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len));
        }
        /*
        for(int i=0;i<delta_calculators.size();i++){
          delta_calculators[i].join();
        }
        */

        //5. send delta to collector
        
        std::vector<int> sent_data_idxes;
        std::vector<std::vector<char>> sent_deltas;
        std::string collector_ip=data_proxy_plan.collector_proxyip();
        int collector_port=data_proxy_plan.collector_proxyport();
        int blocksize=data_proxy_plan.shard_update_len();
        int real_shard_offset=data_proxy_plan.shard_update_offset();

        for(auto const &tt : cur_az_data_delta_map){
          sent_data_idxes.push_back(tt.first);
          sent_deltas.push_back(tt.second);
        }
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          DeltaSendToProxy(OppoProject::RoleDataProxy,sent_data_idxes,sent_deltas,real_shard_offset,blocksize,OppoProject::DataDelta,collector_ip.c_str(),collector_port);
        }
        



        //6. 更新本地 是否需要从collector接收

        int k=data_proxy_plan.k();
        int real_l=data_proxy_plan.real_l();
        int m=data_proxy_plan.g_m();



        /*6.1 从collector 接收*/
        std::map<int,std::vector<char>> received_idx_delta;
        int offset_from_collector;
        int length_from_collector;
        OppoProject::DeltaType received_delta_type=OppoProject::NullTypeDelta;
        std::cout<<"receive_delta_cross_az_num:"<<data_proxy_plan.receive_delta_cross_az_num()<<std::endl;
        if(data_proxy_plan.receive_delta_cross_az_num()>0)
        {
          std::cout<<"waiting collector's connect"<<std::endl;
          asio::ip::tcp::socket socket_delta(io_context);
          acceptor.accept(socket_delta);
          asio::error_code error;
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_delta,error);
          if(role!=OppoProject::RoleCollectorProxy) 
          {
            std::cout<<"data proxy收到除collector外连接,Role:"<<(int) role<<std::endl;
          }
          ReceiveDeltaFromeProxy(socket_delta,received_idx_delta,offset_from_collector,length_from_collector,received_delta_type);
          socket_delta.shutdown(asio::ip::tcp::socket::shutdown_receive, error);
          socket_delta.close(error);
        }

        /*开始本AZ的更新*/
        //6.2 准备数据
        

        std::cout<<"blocksize:"<<blocksize<<"  real_offset:"<<real_shard_offset<<std::endl;



        std::vector<std::vector<char> > cur_az_global_deltas;
        std::vector<char *> cur_global_parity_ptrs;
        std::vector<char> local_parity_delta(blocksize);
        std::vector<char *> local_ptrs;
        local_ptrs.push_back(local_parity_delta.data());//默认一个局部校验块



        std::vector<char*> all_update_data_delta_ptrs;
        std::vector<int> all_update_data_idx;


        OppoProject::EncodeType encode_type=(OppoProject::EncodeType) data_proxy_plan.encode_type();
        
        

        
        if(data_proxy_plan.receive_delta_cross_az_num()>0 && received_delta_type==OppoProject::ParityDelta)
        {
          //从collector 接受global parity delta
          globalparity_idxes.clear();
          int count=0;
          for(auto const & tt:received_idx_delta)
          {
            globalparity_idxes.push_back(tt.first);
            cur_az_global_deltas.push_back(tt.second);
            cur_global_parity_ptrs.push_back(cur_az_global_deltas[count].data());
            count++;
          }
        }
        else if(data_proxy_plan.receive_delta_cross_az_num()>0 && received_delta_type==OppoProject::DataDelta)//生成方案指定，全局校验块<=更新块数就是parity delta based，收到data delta则global一定多于更新块数
        {
          for(auto const & tt:received_idx_delta)
          {
            all_update_data_idx.push_back(tt.first);
            all_update_data_delta_ptrs.push_back(const_cast<char*>(tt.second.data()));
          }
          for(int i=0;i<globalparity_idxes.size();i++)
          {//还是需要给global 分配空间
            std::vector<char> temp_delta(blocksize);
            cur_az_global_deltas.push_back(temp_delta);
            cur_global_parity_ptrs.push_back(cur_az_global_deltas[i].data());
          }
        }
        else if(encode_type==OPPO_LRC || encode_type==Azure_LRC_1) //没从collector接受,需要在本proxy内计算，只有这两种需要
        {
          for(int i=0;i<globalparity_idxes.size();i++)
          {//给global 分配空间
            std::vector<char> temp_delta(blocksize);
            cur_az_global_deltas.push_back(temp_delta);
            cur_global_parity_ptrs.push_back(cur_az_global_deltas[i].data());
          }

          all_update_data_idx=sent_data_idxes;
          for(int i=0;i<sent_deltas.size();i++)
          {
            all_update_data_delta_ptrs.push_back(sent_deltas[i].data());
          }

        }


        //for debug
        std::cout<<"all update idx:";
        for(int i=0;i<all_update_data_idx.size();i++)
        {
          std::cout<<all_update_data_idx[i]<<":";
        }
        std::cout<<'\n';


        //6.3 准备global parity 数据
        std::cout<<"encode_type:"<<(int) encode_type<<std::endl;;
        if(data_proxy_plan.receive_delta_cross_az_num()>0 && received_delta_type==OppoProject::ParityDelta)
        {//只有收到parity delta才不用计算global 
          ;
        }
        else if(encode_type==OPPO_LRC || encode_type==Azure_LRC_1 || encode_type==RS && received_delta_type==OppoProject::DataDelta) //没有收到parity delta，
        {
          calculate_global_parity_delta(k,m,real_l,all_update_data_delta_ptrs.data(),cur_global_parity_ptrs.data(),blocksize,all_update_data_idx,globalparity_idxes,encode_type);
        }

        std::vector<char *> cur_az_data_delta_ptrs;//OPPOLRC 需要当前的,与发送给collector proxy的idx顺序一样
        for(int i=0;i<sent_data_idxes.size();i++)
          cur_az_data_delta_ptrs.push_back(sent_deltas[i].data());
        //计算local


        for(int i=0;i<sent_data_idxes.size();i++)
        {
          std::cout<<"send idx: "<<sent_data_idxes[i]<<std::endl;
          std::cout<<std::string(sent_deltas[i].data(),blocksize)<<std::endl;
        }

        if(encode_type==OPPO_LRC)
        {
          calculate_local_parity_delta_oppo_lrc(k,m,real_l,cur_az_data_delta_ptrs.data(),cur_global_parity_ptrs.data(),local_ptrs.data(),blocksize,sent_data_idxes,globalparity_idxes,localparity_idxes);
        }
        else if(encode_type==Azure_LRC_1)
        {
          calculate_local_parity_delta_azure_lrc1(k,m,real_l,cur_az_data_delta_ptrs.data(),local_ptrs.data(),blocksize,sent_data_idxes,localparity_idxes,false);
        }

        auto delta_to_datanode = [this](int j,std::string shard_id,int offset_inshard,char** ptr,int blocksize,OppoProject::DeltaType deltatype,std::string node_ip,int node_port)
        {
          DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),offset_inshard,ptr[j],blocksize,deltatype,node_ip.c_str(),node_port);
        };

       
        std::vector<std::thread> cur_az_senders;
        if(encode_type==OPPO_LRC || encode_type==Azure_LRC_1 || encode_type==RS && received_delta_type==OppoProject::DataDelta)
        {
          for(int i=0;i<globalparity_idxes.size();i++)
          {
            int global_idx=globalparity_idxes[i];
            std::cout<<"send global:"<<global_idx<<std::endl;
            std::string shard_id=std::to_string(stripeid*1000+global_idx);
            /*
            cur_az_senders.push_back(std::thread(delta_to_datanode,i,real_shard_offset,cur_global_parity_ptrs.data(),blocksize,OppoProject::ParityDelta,data_nodes_ip_port[global_idx].first,data_nodes_ip_port[global_idx].second));
            */
            std::cout<<"send global parity delta"<<std::endl;
            this->DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),real_shard_offset,cur_global_parity_ptrs[i],blocksize,OppoProject::ParityDelta,data_nodes_ip_port[global_idx].first.c_str(),data_nodes_ip_port[global_idx].second);
          }
        }
        
        if(localparity_idxes.size()>0){
          std::cout<<"send local parity delta"<<std::endl;
          int local_idx=localparity_idxes[0];
          std::cout<<"send local:"<<local_idx<<std::endl;
          std::string shard_id=std::to_string(stripeid*1000+local_idx);
          /*
          cur_az_senders.push_back(std::thread(delta_to_datanode,0,real_shard_offset,local_ptrs.data(),blocksize,OppoProject::ParityDelta,data_nodes_ip_port[local_idx].first,data_nodes_ip_port[local_idx].second));
          */
         this->DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),real_shard_offset,local_parity_delta.data(),blocksize,(int)OppoProject::ParityDelta,data_nodes_ip_port[local_idx].first.c_str(),data_nodes_ip_port[local_idx].second);
        }

        

        /*
        for(int j=0;j<cur_az_senders.size();j++)
        {
          cur_az_senders[j].join();
        }
        */

       auto send_to_datanode = [this](std::string shard_id, char *data, int x_shard_size, std::pair<std::string, int> ip_and_port)
       {
           std::cout<<"new data send to data node"<<std::endl;
           SetToMemcached(shard_id.c_str(), shard_id.size(), data, x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
       };
 
       std::vector<std::thread> new_data_senders;
       /*
       for(auto const & ttt:new_shard_data_map)
       {
         int idx=ttt.first;
         std::string shard_id=std::to_string(stripeid*1000+idx);
         std::cout<<"new data set to port: "<<data_nodes_ip_port[idx].second<<std::endl;
         new_data_senders.push_back(std::thread(send_to_datanode,shard_id,const_cast<char*> (ttt.second.data()),blocksize,data_nodes_ip_port[idx]));
       }

       for(int i=0;i<new_data_senders.size();i++)
        new_data_senders[i].join();
        */
       for(auto const & ttt:cur_az_data_delta_map)
       {
         int idx=ttt.first;
         std::string shard_id=std::to_string(stripeid*1000+idx);
         std::cout<<"new data set to port: "<<data_nodes_ip_port[idx].second<<std::endl;
         //new_data_senders.push_back(std::thread(send_to_datanode,shard_id,const_cast<char*> (ttt.second.data()),blocksize,data_nodes_ip_port[idx]));
         this->DeltaSendToMemcached(shard_id.data(),shard_id.length(),real_shard_offset,ttt.second.data(),blocksize,1,data_nodes_ip_port[idx].first.data(),data_nodes_ip_port[idx].second);
       }
       
       /*commitSuccess*/
      coordinator_proto::CommitAbortKey commit_abort_key;
      coordinator_proto::ReplyFromCoordinator result;
      grpc::ClientContext context;
      commit_abort_key.set_key(this->proxy_ip_port);
      commit_abort_key.set_ifcommitmetadata(true);
      grpc::Status status;
      status = this->m_coordinator_stub->updateReportSuccess(
            &context, commit_abort_key, &result);
      if(status.ok())
        std::cout<<"data proxy finished!"<<std::endl;
      else 
        std::cout<<"data proxy fail!"<<std::endl;

      }
      catch(const std::exception& e)
      {
         std::cerr << e.what() << '\n';
         
      }
    };
      
      
      

      

    try
    {
      std::thread my_thread(receive_update,*dataProxyPlan);
      my_thread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  




  grpc::Status ProxyImpl::collectorProxyUpdate(
      grpc::ServerContext *context,
      const proxy_proto::CollectorProxyUpdatePlan *collectorProxyPlan,
      proxy_proto::CollectorProxyReply *reply)
  {
    std::cout<<"there is collector"<<std::endl;
    std::string key=collectorProxyPlan->key();
    unsigned int stripeid=collectorProxyPlan->stripeid();
    std::cout<<"this collector updated stripeid:"<<stripeid<<std::endl;

    //std::vector<ShardidxRange> delta_idx_ranges;//receive from data proxy
    auto collector_receive_update=[this](proxy_proto::CollectorProxyUpdatePlan collector_proxy_plan) 
    {


      std::string key =collector_proxy_plan.key();
      unsigned int stripeid =collector_proxy_plan.stripeid();
      std::cout<<"collector proxy received key from MDS:"<<key<<std::endl;
      std::cout<<"collector proxy received stipeid from MDS:"<<stripeid<<std::endl;

      int update_op_id =collector_proxy_plan.update_operation_id();
      OppoProject::EncodeType encode_type=(OppoProject::EncodeType)collector_proxy_plan.encode_type();
    
    //从client收数据需要的结构,以及本AZ相关
     std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
     std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
     std::vector<int> localparity_idxes;
     std::vector<int> globalparity_idxes;
     int offset_from_client;
     int length_from_client;

     
     

     std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
     std::unordered_map<int,std::vector<char> > old_shard_data_map;
     std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//本AZ计算得到的
  
     ClientUpdateInfoConvert(collector_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
    
    //for debug
    std::cout<<"idx ranges size:" <<data_idx_ranges.size()<<std::endl;
    for(auto const & ttt:data_idx_ranges)
      std::cout<<":"<<ttt.first;
    std::cout<<std::endl;
    
    std::cout<<"local info"<<std::endl;
    for(int j=0;j<localparity_idxes.size();j++)
    {
      std::cout<<"local idx:"<<localparity_idxes[j]<<std::endl;;
      std::cout<<data_nodes_ip_port[localparity_idxes[j]].first<<":"<<data_nodes_ip_port[localparity_idxes[j]].second<<std::endl;
    } 
    for(int j=0;j<globalparity_idxes.size();j++)
    {
      std::cout<<"global idx:"<<globalparity_idxes[j]<<std::endl;;
      std::cout<<data_nodes_ip_port[globalparity_idxes[j]].first<<":"<<data_nodes_ip_port[globalparity_idxes[j]].second<<std::endl;
    }
        

 

      //跨AZ传输parity delta
      std::vector<OppoProject::ParityDeltaSendType> v_parity_delta_send_type;
      std::vector<int> cross_AZ_parityshard_idxes;
      std::unordered_map<int,std::pair<std::string, int>> cross_AZ_parityshard_nodes_ip_port;//上面位于其他AZ global parity idx->node ip port 


      //从data proxy收delta需要的结构     
      std::map<int,std::vector<char>> all_idx_data_delta;//存delta idx->delta  包括从proxy 接收以及本AZ计算得到的
      //std::unordered_map<std::pair<std::string,int>, std::vector<int> > datadelta_idx_dataproxy_had;//记录data proxy有的delta ，防止data delta更新时跨AZ发送冗余
      int offset_from_data_proxy=0;
      int length_from_data_proxy=0;
      OppoProject::DeltaType delta_type_from_proxy;


      //1. 开始接收
      proxy_proto::StripeUpdateInfo temp_client_info= collector_proxy_plan.client_info();
      int whether_receive_clint=temp_client_info.receive_client_shard_idx_size()>0? 1 : 0;
      int socket_num=collector_proxy_plan.data_proxy_num()+whether_receive_clint;
      
      
      std::cout<<"socket num"<<socket_num<<std::endl;
      for(int receive_count=0;receive_count<socket_num;receive_count++){
        std::cout<<"receiving loop  "<<receive_count<<std::endl;

        asio::ip::tcp::socket socket_data_collector(io_context);
        asio::error_code error;
  
        
        acceptor.accept(socket_data_collector,error);
        if(error)
        {
          std::cout<<"client's connet wrong"<<std::endl;
          
        }
        int int_role=receive_int(socket_data_collector,error);//发送方 先发role   然后shard idx然后 
        std::cout<<"role :"<<int_role<<std::endl;
        OppoProject::Role role=(OppoProject::Role) int_role;

        std::cout<<"Role"<<role<<std::endl;

        if(role==OppoProject::RoleClient){
          std::cout<<"start receive from client"<<std::endl;
          if(whether_receive_clint==0) std::cout<<"Collector connected by client not in plan"<<std::endl;
          else std::cout<<"connected by client"<<std::endl;
          //receive from client
          
          ReceiveDataFromClient(socket_data_collector,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          
          //给old data 以及delta分配空间
          for(auto const &ttt:data_idx_ranges)
          {
            int idx=ttt.first;
            int len=data_idx_ranges[idx].range_length;
            std::vector<char> old_data(len);
            std::vector<char> data_delta(len);
            old_shard_data_map[idx]=old_data;//need to assignment
            cur_az_data_delta_map[idx]=data_delta;
          }
          

        }
        else if(role==OppoProject::RoleDataProxy){
          //receive from data proxy
          std::cout<<"start receive from data proxy"<<std::endl;
          ReceiveDeltaFromeProxy(socket_data_collector,all_idx_data_delta,offset_from_data_proxy,length_from_data_proxy,delta_type_from_proxy);
          if(delta_type_from_proxy==OppoProject::ParityDelta) std::cout<<"receive parity delta from dataproxy"<<std::endl;
          std::cout<<std::endl;
        }
        else{
          std::cout<<"未知身份连接到Collector"<<std::endl;
        }
      }

      //2. 处理
      
      auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
      {
        size_t temp_size;//
        bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
        if(!reslut){
          std::cout<<"getting from data node fails"<<std::endl;
          return;
        } 
      };

      


        
      std::vector<std::thread> old_data_readers;
      //read old
      for(auto const & ttt : data_idx_ranges){
        int idx=ttt.first;
        int offset=ttt.second.offset_in_shard;
        int len=ttt.second.range_length;
        std::string shardid=std::to_string(stripeid*1000+idx);
        //read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
        old_data_readers.push_back(std::thread(read_from_datanode,shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second));
        std::cout<<"read shard:"<<shardid<<std::endl;
      }
      for(int i=0;i<old_data_readers.size();i++)
        old_data_readers[i].join();


      // 3. 计算本AZ内delta
      std::vector<std::thread> delta_calculators;

      for(auto const & ttt : data_idx_ranges){//read old
        int idx=ttt.first;
        int len=ttt.second.range_length;
        calculate_data_delta(new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len);
        std::cout<<"in collector delta is:"<<std::endl;
        std::cout<<std::string(cur_az_data_delta_map[idx].data(),cur_az_data_delta_map[idx].size())<<std::endl;


        //delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len));
      }
      /*
      for(int i=0;i<(int)delta_calculators.size();i++){
        delta_calculators[i].join();
      }
      */


      // 4. 本AZ内的和data proxy送来的delta放一块
      for(auto const & temp_delta : cur_az_data_delta_map){
        int idx=temp_delta.first;
        if(all_idx_data_delta.find(idx)!=all_idx_data_delta.end()) std::cout<<"接收的有冲突"<<std::endl;
        all_idx_data_delta[idx]=temp_delta.second;
        std::cout<<"collector 汇总delta"<<std::endl;
        std::cout<<std::string(all_idx_data_delta[idx].data(),all_idx_data_delta[idx].size())<<std::endl;//for debug
      }
      
      std::cout<<"collector obtain all delta!"<<std::endl;

      

      //5.   selective update
      int offset_inshard=0;
      int blocksize=0;

      blocksize=collector_proxy_plan.shard_update_len();
      int real_shard_offset=collector_proxy_plan.shard_update_offset();
      std::cout<<"blocksize:"<<blocksize<<"  real_offset:"<<real_shard_offset<<std::endl;


      //6. 准备数据
      std::vector<int> all_update_data_idx;
      all_update_data_idx.clear();
      std::vector<char *> all_update_data_delta_ptrs;
      all_update_data_delta_ptrs.clear();
      std::vector<std::vector<char> > all_delta_vector;
      all_delta_vector.clear();

      for(auto const& t_delta:all_idx_data_delta){
        all_update_data_idx.push_back(t_delta.first);
        all_delta_vector.push_back(t_delta.second);
      }
      for(int i=0;i<all_delta_vector.size();i++){
        all_update_data_delta_ptrs.push_back(all_delta_vector[i].data());
      }
      //for debug

      std::cout<<"all update idx and delta:"<<std::endl;

      for(int i=0;i<all_update_data_idx.size();i++)
      {
        std::cout<<"idx: "<<all_update_data_idx[i]<<" blocksize "<<blocksize<<std::endl;
        std::cout<<std::string(all_update_data_delta_ptrs[i],blocksize)<<std::endl;;
      }
        
      std::cout<<'\n';
      
      int k=collector_proxy_plan.k();
      int real_l=collector_proxy_plan.real_l();
      int m=collector_proxy_plan.g_m();
      //7. 跨AZ更新
      std::cout<<"7. 跨AZ更新"<<std::endl;
      std::vector<int> cross_az_parity_idxes;
      std::vector<std::vector<char> > cross_az_parity_deltas;
      std::vector<char *> cross_parity_ptrs;

      auto delta_to_datanode=[this](int j,std::string shard_id,int offset_inshard,std::vector<char*> &ptr,int blocksize,int deltatype,std::string node_ip,int node_port)
      {
        DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),offset_inshard,ptr[j],blocksize,deltatype,node_ip.c_str(),node_port);
      };

      if(encode_type==OppoProject::RS)
      {
        //7.1 parity delta based
        std::cout<<"collector process RS"<<std::endl;
        proxy_proto::RSCrossAZUpdate cross_info=collector_proxy_plan.rs_cross_az();
        //send parity delta
        for(int i=0;i<cross_info.global_parity_idx_size();i++){
          std::cout<<cross_info.global_parity_idx(i)<<std::endl;
          cross_az_parity_idxes.push_back(cross_info.global_parity_idx(i));//bug   
          std::vector<char> temp_delta(blocksize);
          cross_az_parity_deltas.push_back(temp_delta);
          cross_parity_ptrs.push_back(cross_az_parity_deltas[i].data());
        }

        std::cout<<"collector process RS2"<<std::endl;
        calculate_global_parity_delta(k,m,real_l,all_update_data_delta_ptrs.data(),cross_parity_ptrs.data(),blocksize,all_update_data_idx,cross_az_parity_idxes,OppoProject::RS);

        std::vector<std::thread> senders;

        
        for(int i=0;i<cross_info.global_parity_idx_size();i++){//只有RS送到节点上
          std::string shard_id = std::to_string(stripeid * 1000 + cross_info.global_parity_idx(i));
          std::string nodeip=cross_info.nodeip(i);
          int nodeport=cross_info.nodeport(i);
          //senders.push_back(std::thread(delta_to_datanode,i,shard_id,real_shard_offset,cross_parity_ptrs,blocksize,(int)OppoProject::ParityDelta,nodeip,nodeport));
          this->DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),real_shard_offset,cross_parity_ptrs[i],blocksize,OppoProject::ParityDelta,nodeip.c_str(),nodeport);
         
        }

        std::cout<<"RS send parity delta to node"<<std::endl;
        
        for (int j = 0; j < senders.size(); j++){
          senders[j].join();
        }
        

        //x.2 data delta to proxy

        for(int i=0;i<cross_info.proxyip_size();i++){
          std::string proxy_ip=cross_info.proxyip(i);
          int proxy_port=cross_info.proxyport(i);
          this->DeltaSendToProxy(OppoProject::RoleCollectorProxy,all_update_data_idx,all_delta_vector,real_shard_offset,blocksize,OppoProject::DataDelta,proxy_ip.c_str(),proxy_port);
        }
      }

      else if(encode_type==OPPO_LRC )
      {
        //编码 发送
        proxy_proto::OPPOLRCCrossAZUpdate cross_info=collector_proxy_plan.oppo_lrc_cross_az();
        std::unordered_map<int,int> parity_idx_to_vector_index;
        //x.1 calculate global ,算出其他AZ中所有的global delta
        if(cross_info.to_proxy_global_parity_idx_size()>0)
        {
          for(int i=0;i<cross_info.to_proxy_global_parity_idx_size();i++)
          {
            cross_az_parity_idxes.push_back(cross_info.to_proxy_global_parity_idx(i));
            parity_idx_to_vector_index[cross_info.to_proxy_global_parity_idx(i)]=i;
            std::vector<char> temp_delta(blocksize);
            cross_az_parity_deltas.push_back(temp_delta);
            cross_parity_ptrs.push_back(cross_az_parity_deltas[i].data());
          }
          calculate_global_parity_delta(k,m,real_l,all_update_data_delta_ptrs.data(),cross_parity_ptrs.data(),blocksize,all_update_data_idx,cross_az_parity_idxes,OppoProject::OPPO_LRC);
        }
        
        int j=0;
        for(int i=0;i<cross_info.proxyip_size();i++)
        {
          std::string proxy_ip=cross_info.proxyip(i);
          int proxy_port=cross_info.proxyport(i);
          if(cross_info.sendflag(i)==0)//hard encode 硬编码
          {//send data delta
            std::cout<<"send data delta to:"<<proxy_ip<<":"<<proxy_port<<"send num"<<all_update_data_idx.size()<<std::endl;
            //j+=all_update_data_idx.size();
            DeltaSendToProxy(OppoProject::RoleCollectorProxy,all_update_data_idx,all_delta_vector,real_shard_offset,blocksize,OppoProject::DataDelta,proxy_ip.c_str(),proxy_port);
          }
          else if(cross_info.sendflag(i)==1)
          {
            std::vector<int> temp_cross_global_idxes;
            std::vector<std::vector<char> > global_deltas;
            std::cout<<"send data delta to:"<<proxy_ip<<":"<<proxy_port;
            for(int tt=0;tt<cross_info.num_each_proxy(i);tt++)
            {
              int tttt_parity_idx=cross_info.to_proxy_global_parity_idx(j);
              temp_cross_global_idxes.push_back(tttt_parity_idx);
              j++;

              int vec_idx=parity_idx_to_vector_index[tttt_parity_idx];
              global_deltas.push_back(cross_az_parity_deltas[vec_idx]);
              
            }
            DeltaSendToProxy(OppoProject::RoleCollectorProxy,temp_cross_global_idxes,global_deltas,real_shard_offset,blocksize,OppoProject::ParityDelta,proxy_ip.c_str(),proxy_port);
            std::cout<<"send num:"<<temp_cross_global_idxes.size()<<std::endl;
          }
        }

      }
      else if(encode_type==Azure_LRC_1)
      {
        //编码 发送
        proxy_proto::AzureLRC_1CrossAZUpdate cross_info=collector_proxy_plan.azure_lrc1_cross_az();
        // 计算出global parity delta，然后发送到global 以及 global的local

      }


      


      //本AZ更新
      

      std::cout<<"start  update self"<<std::endl;
      std::vector<std::vector<char> > cur_az_global_deltas;
      std::vector<char *> cur_global_parity_ptrs;


      std::vector<char> local_parity_delta(blocksize);
      std::vector<char *> local_ptrs;
      local_ptrs.push_back(local_parity_delta.data());
  
      std::cout<<"cur az global idx:"<<std::endl;
      for(int i=0;i<globalparity_idxes.size();i++){
        std::cout<<globalparity_idxes[i]<<" ";
        std::vector<char> temp_delta(blocksize);

        cur_az_global_deltas.push_back(temp_delta);

        cur_global_parity_ptrs.push_back(cur_az_global_deltas[i].data());

      }

      std::cout<<"global vec size: "<<cur_global_parity_ptrs.size()<<std::endl;
      calculate_global_parity_delta(k,m,real_l,all_update_data_delta_ptrs.data(),cur_global_parity_ptrs.data(),blocksize,all_update_data_idx,globalparity_idxes,encode_type);
      
      std::cout<<std::endl<<"start  update self2"<<std::endl;

      std::vector<char *> cur_az_data_delta_ptrs;
      std::vector<int> cur_az_data_delta_idxes;
      for(auto const & tt:cur_az_data_delta_map)
      {
        cur_az_data_delta_idxes.push_back(tt.first);
        cur_az_data_delta_ptrs.push_back(const_cast<char*> (tt.second.data()));
      }

      std::cout<<"local vec size: "<<local_ptrs.size()<<std::endl;
      if(encode_type==OPPO_LRC)
      {
        calculate_local_parity_delta_oppo_lrc(k,m,real_l,cur_az_data_delta_ptrs.data(),cur_global_parity_ptrs.data(),local_ptrs.data(),blocksize,cur_az_data_delta_idxes,globalparity_idxes,localparity_idxes);
      }
      else if(encode_type==Azure_LRC_1)
      {
        calculate_local_parity_delta_azure_lrc1(k,m,real_l,cur_global_parity_ptrs.data(),local_ptrs.data(),blocksize,globalparity_idxes,localparity_idxes,true);
      }

      std::vector<std::thread> cur_az_senders;
      for(int i=0;i<globalparity_idxes.size();i++)
      {

        std::cout<<"send global :"<<globalparity_idxes[i]<<std::endl;
        int global_idx=globalparity_idxes[i];
        std::string shard_id=std::to_string(stripeid*1000+global_idx);
        //cur_az_senders.push_back(std::thread(delta_to_datanode,i,real_shard_offset,cur_global_parity_ptrs,blocksize,(int)OppoProject::ParityDelta,data_nodes_ip_port[global_idx].first,data_nodes_ip_port[global_idx].second));
        this->DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),real_shard_offset,cur_global_parity_ptrs[i],blocksize,OppoProject::ParityDelta,data_nodes_ip_port[global_idx].first.c_str(),data_nodes_ip_port[global_idx].second);
      }

      if(localparity_idxes.size()>0){
        int local_idx=localparity_idxes[0];
        std::cout<<"send local :"<<local_idx<<std::endl;
        std::string shard_id=std::to_string(stripeid*1000+local_idx);
        //cur_az_senders.push_back(std::thread(delta_to_datanode,0,real_shard_offset,local_ptrs,blocksize,(int)OppoProject::ParityDelta,data_nodes_ip_port[local_idx].first,data_nodes_ip_port[local_idx].second));
        this->DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),real_shard_offset,local_parity_delta.data(),blocksize,OppoProject::ParityDelta,data_nodes_ip_port[local_idx].first.c_str(),data_nodes_ip_port[local_idx].second);
      }


      /*
      for(int j=0;j<cur_az_senders.size();j++)
      {
        cur_az_senders[j].join();
      }
      */

      /*new data set to memcached*/
      //std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      auto send_to_datanode = [this](std::string shard_id, char *data, int x_shard_size, std::pair<std::string, int> ip_and_port)
      {
        std::cout<<"new data send to data node"<<std::endl;
        SetToMemcached(shard_id.c_str(), shard_id.size(), data, x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
      };

      
      std::vector<std::thread> new_data_senders;
      for(auto const & ttt:cur_az_data_delta_map)
      {
        int idx=ttt.first;
        std::string shard_id=std::to_string(stripeid*1000+idx);
        std::cout<<"new data set to port: "<<data_nodes_ip_port[idx].second<<std::endl;
        //new_data_senders.push_back(std::thread(send_to_datanode,shard_id,const_cast<char*> (ttt.second.data()),blocksize,data_nodes_ip_port[idx]));
        std::cout<<"collector send data delta to data node"<<std::endl;
        this->DeltaSendToMemcached(shard_id.c_str(),shard_id.length(),real_shard_offset,ttt.second.data(),blocksize,1,data_nodes_ip_port[idx].first.c_str(),data_nodes_ip_port[idx].second);
      }

      
      //for(int i=0;i<new_data_senders.size();i++)
        //new_data_senders[i].join();
      

      /*
      for(int i=0;i<all_update_data_idx.size();i++)
      {
        int idx=all_update_data_idx[i];
        std::string shard_id=std::to_string(stripeid*1000+idx);
        this->DeltaSendToMemcached(shard_id.data(),shard_id.length(),real_shard_offset,all_update_data_delta_ptrs[i],blocksize,1,data_nodes_ip_port[idx].first.data(),data_nodes_ip_port[idx].second);
      }
      */
      


      /*commitSuccess*/
      coordinator_proto::CommitAbortKey commit_abort_key;
      coordinator_proto::ReplyFromCoordinator result;
      grpc::ClientContext context;
      commit_abort_key.set_key(this->proxy_ip_port);
      commit_abort_key.set_ifcommitmetadata(true);
      grpc::Status status;
      status = this->m_coordinator_stub->updateReportSuccess(
            &context, commit_abort_key, &result);
      if(status.ok())
        std::cout<<"collector finished!"<<std::endl;
      else 
        std::cout<<"collector proxy fail!"<<std::endl;
      return;
    };

    try
    {
      std::thread mythread(collector_receive_update,*collectorProxyPlan);
      std::cout<<"collector proxy ,begin processing"<<std::endl;
      mythread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    return grpc::Status::OK;
  }


  grpc::Status ProxyImpl::dataProxyRMW(
        grpc::ServerContext* context, 
        const proxy_proto::DataProxyUpdatePlan* request, 
        proxy_proto::DataProxyReply* response)
  {
    
    std::cout<<"there is RMW proxy"<<std::endl;
    auto receive_RMW_update = [this](proxy_proto::DataProxyUpdatePlan data_proxy_plan)
    {
      //从client收数据需要的结构：
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
      
      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();

      //相关
      std::vector<int> localparity_idxes;
      std::vector<int> globalparity_idxes;  
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      std::unordered_map<int,std::vector<char> > old_shard_data_map;
      std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的

      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
      try
      {
        
        
        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cout<<"getting from data node fails"<<std::endl;
            return;
          } 
        };
        
        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }

        //2. 给old data 以及delta分配空间
        
        std::cout<<"2. 给old data 以及delta分配空间"<<std::endl;;
        for(auto const & ttt:data_idx_ranges)
        {
          int idx=ttt.first;
          int len=data_idx_ranges[idx].range_length;
          std::vector<char> old_data(len);
          std::vector<char> data_delta(len);
          old_shard_data_map[idx]=old_data;//need to assignment
          cur_az_data_delta_map[idx]=data_delta;
          std::cout<<"old and cur delta size:"<<old_shard_data_map[idx].size()<<" :"<<cur_az_data_delta_map[idx].size()<<std::endl;
        }


        //3. read old
        for(auto const & ttt : data_idx_ranges){
          int idx=ttt.first;
          int offset=ttt.second.offset_in_shard;
          int len=ttt.second.range_length;

          std::cout<<"len receive from coordinator"<<std::endl;

          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }

        //4. 计算本AZ内 delta
        std::vector<std::thread> delta_calculators;

        for(auto const & ttt : data_idx_ranges){//read old
          int idx=ttt.first;
          int len=ttt.second.range_length;
          //calculate_data_delta(new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len);
        
          delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len));
        }
        
        for(int i=0;i<delta_calculators.size();i++){
          delta_calculators[i].join();
        }
        
        //5. calculate parity delta

        int k=data_proxy_plan.k();
        int real_l=data_proxy_plan.real_l();
        int m=data_proxy_plan.g_m();
        OppoProject::EncodeType encode_type=(OppoProject::EncodeType) data_proxy_plan.encode_type();

        std::vector<char*> all_update_data_delta_ptrs;
        std::vector<int> all_update_data_idx;

        std::vector<char *> v_coding(m + real_l + 1);
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(length_from_client));



        for(auto const & iter:cur_az_data_delta_map)
        {
          all_update_data_delta_ptrs.push_back(const_cast<char*>(iter.second.data()));
          all_update_data_idx.push_back(iter.first);
        }

        for (int j = 0; j < m + real_l + 1; j++)
        {
          coding[j] = v_coding_area[j].data();
          bzero(coding[j],length_from_client);
        }

        RMW_encode(k,m,real_l,all_update_data_delta_ptrs.data(),coding,length_from_client,encode_type,all_update_data_idx);
        
        //6. send to memcached
        /*
        auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port)
        {
          if (j < k)
          {
            SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
          }
          else
          {
            SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
          }
        };
        */
        auto send_to_datanode = [this](std::string shard_id, char *data, int x_shard_size, std::pair<std::string, int> ip_and_port)
       {
           std::cout<<"new data send to data node"<<std::endl;
           SetToMemcached(shard_id.c_str(), shard_id.size(), data, x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
       };
         
        std::vector<std::thread> new_data_senders;
        for(int i=0;i<new_shard_data_map.size();i++)
        {
          int idx=all_update_data_idx[i];
          std::string shard_id = std::to_string(stripeid * 1000 + idx);
          data_nodes_ip_port[idx];
          new_data_senders.push_back(std::thread(send_to_datanode,shard_id,new_shard_data_map[idx].data(),length_from_client,data_nodes_ip_port[idx]));
          //new_data_senders.push_back(std::thread(send_to_datanode, idx, k, shard_id, all_update_data_delta_ptrs.data(), coding, length_from_client, data_nodes_ip_port[idx]));
        }
        
        for(int i=0;i<new_data_senders.size();i++) new_data_senders[i].join();


        //7. send parity delta to data node
        std::vector<std::thread> delta_senders;
        auto delta_to_datanode=[this](int j,std::string shard_id,int offset_inshard,char** coding,int blocksize,int deltatype,std::string node_ip,int node_port)
        {
          DeltaSendToMemcached(shard_id.c_str(),shard_id.size(),offset_inshard,coding[j],blocksize,deltatype,node_ip.c_str(),node_port);
        };

        int delta_send_num=0;
        if(encode_type==RS) delta_send_num=m;
        else if(encode_type==OppoProject::OPPO_LRC) delta_send_num=m+real_l;
        else if(encode_type==OppoProject::Azure_LRC_1) delta_send_num=m+real_l+1;

        for(int i=0;i<delta_send_num;i++)
        {
          int idx=i+k;
          std::string shard_id=std::to_string(stripeid*1000+idx);
          delta_senders.push_back(std::thread(delta_to_datanode,i,shard_id,offset_from_client,coding,length_from_client,(int)OppoProject::ParityDelta,data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second));
        }

        for(int i=0;i<delta_senders.size();i++) delta_senders[i].join();
        std::cout<<"data proxy RMW send over!"<<std::endl;
        try
        {
          /* code */
          
          /*commitSuccess*/
          coordinator_proto::CommitAbortKey commit_abort_key;
          coordinator_proto::ReplyFromCoordinator result;
          grpc::ClientContext context;
          commit_abort_key.set_key(this->proxy_ip_port);
          commit_abort_key.set_ifcommitmetadata(true);
          grpc::Status status;
          status = this->m_coordinator_stub->updateReportSuccess(
                &context, commit_abort_key, &result);
          if(status.ok())
            std::cout<<"data proxy RMW finished!"<<std::endl;
          else 
            std::cout<<"data proxy RMW fail!"<<std::endl;
          }
        catch(const std::exception& e)
        {
          std::cerr << e.what() << '\n';
        }
        
        
        return;
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
    };



    try
    {
      std::thread mythread(receive_RMW_update, *request);
      mythread.detach();
    
    }
    catch(const std::exception& e)
    {
      std::cout << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  grpc::Status ProxyImpl::dataProxyRCW( 
        grpc::ServerContext* context, 
        const proxy_proto::DataProxyUpdatePlan* request,
        proxy_proto::DataProxyReply* response)
  {
    //just receive from client and set memcached;

    auto receive_and_set=[this](proxy_proto::DataProxyUpdatePlan data_proxy_plan)
    {
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port


      std::vector<int> localparity_idxes;
      std::vector<int> globalparity_idxes;  

      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);

      try
      {
        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }

        //2. set to datanode  
        auto send_to_datanode = [this](std::string shard_id, char *data,int offset, int update_len, std::pair<std::string, int> ip_and_port)
        {
            std::cout<<"new data send to data node"<<std::endl;
            SetToMemcached(shard_id.c_str(), shard_id.size(), data, update_len, ip_and_port.first.c_str(), ip_and_port.second);
        };
        std::vector<std::thread> new_data_senders;
        std::unordered_map<int,char*> new_data_ptrs;
        for(auto const & ttt:new_shard_data_map)
          new_data_ptrs[ttt.first]=const_cast<char*> (ttt.second.data());
        for(auto const & ttt:new_shard_data_map)
        {
          int idx=ttt.first;
          std::string shard_id=std::to_string(stripeid*1000+idx);
          std::cout<<"new data set to port: "<<data_nodes_ip_port[idx].second<<std::endl;
          new_data_senders.push_back(std::thread(send_to_datanode,shard_id,new_data_ptrs[idx],offset_from_client,length_from_client,data_nodes_ip_port[idx]));//bug  need set_to_memcached with offset
        }

       for(int i=0;i<new_data_senders.size();i++)
        new_data_senders[i].join();
        

        
        //3. inform coordinator
        /*commitSuccess*/
        coordinator_proto::CommitAbortKey commit_abort_key;
        coordinator_proto::ReplyFromCoordinator result;
        grpc::ClientContext context;
        commit_abort_key.set_key(this->proxy_ip_port);
        commit_abort_key.set_ifcommitmetadata(true);
        grpc::Status status;
        status = this->m_coordinator_stub->updateReportSuccess(
              &context, commit_abort_key, &result);
        if(status.ok())
          std::cout<<"rcw set new data finished!"<<std::endl;
        else 
          std::cout<<"rcw set new data fail!"<<std::endl;

      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
    };
    

    try
    {
      std::thread mythread(receive_and_set, *request);
      std::cout<<"data proxy ,begin  receive data of RCW "<<std::endl;
      mythread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
  }

  grpc::Status  ProxyImpl::ReconstructWrite(
        grpc::ServerContext* context, 
        const proxy_proto::ReconstructWriteNotice* request, 
        proxy_proto::DataProxyReply* response)
  {

    auto reconstruc_write=[this](proxy_proto::ReconstructWriteNotice request)
    {
      int stripeid=    request.stripeid();
      int k =          request.k();
      int m =          request.m() ;
      int real_l =     request.real_l();
      int shard_size=  request.shard_size();
      OppoProject::EncodeType encode_type =(OppoProject::EncodeType) request.encode_type();
      /*
      repeated int32 idxes=2;
      repeated string nodeip=3;
      repeated int32 nodeport=4;    
      */
    
      std::vector<std::pair<std::string, int>> nodes_ip_and_port;
  
      for(int i=0;i<request.nodeip_size();i++)
      {
        nodes_ip_and_port.push_back(std::make_pair(request.nodeip(i),request.nodeport(i)));
      }

      std::vector<char *> v_data(k);
      std::vector<char *> v_coding(m + real_l + 1);
      char **data = (char **)v_data.data();
      char **coding = (char **)v_coding.data();
      
      std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(shard_size));
      std::vector<std::vector<char>> v_data_area(k,std::vector<char>(shard_size));
      
      for (int j = 0; j < k; j++)
      {
        data[j] = v_data_area[j].data();
      }
  
      for (int j = 0; j < m + real_l + 1; j++)
      {
        coding[j] = v_coding_area[j].data();
      }
  
      std::vector<std::thread> readers;
      std::vector<std::thread> parity_senders;
  
      auto get_from_node=[this](std::string shard_id,int offset,int length,char* value,std::string ip, int port)
      {
        size_t temp_size;
        bool ret=GetFromMemcached(shard_id.c_str(),shard_id.size(),value,&temp_size,offset,length,ip.c_str(),port);
        if (!ret)
        {
          std::cout << "reconstruct write getFromNode !ret" << std::endl;
          return;
        }
      };
  
      for(int i=0;i<k;i++)
      {
        std::string shard_id=std::to_string(stripeid*1000+i);
        readers.push_back(std::thread(get_from_node,shard_id,0,shard_size,data[i],nodes_ip_and_port[i].first,nodes_ip_and_port[i].second));
      }
      
      for(int i=0;i<readers.size();i++) readers[i].join();
  
  
  
      std::cout<<"reconstruct read success\n";
      int send_num=0;
      if (encode_type == RS)
      {
        encode(k, m, 0, data, coding, shard_size, encode_type);
        send_num =  m;
      }
      else if (encode_type == Azure_LRC_1)
      {
        // m = g for lrc
        encode(k, m, real_l, data, coding, shard_size, encode_type);
        send_num =  m + real_l + 1;
      }
      else if (encode_type == OPPO_LRC)
      {
        encode(k, m, real_l, data, coding, shard_size, encode_type);
        send_num =  m + real_l;
      }
  
      auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port)
      {
        if (j < k)
        {
          SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
        }
        else
        {
          SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
        }
      };
  
  
      for (int j = 0; j < send_num; j++)
      {
        int idx=j+k;
        std::string shard_id = std::to_string(stripeid * 1000 + idx);
        std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[idx];
        parity_senders.push_back(std::thread(send_to_datanode, idx, k, shard_id, data, coding, shard_size, ip_and_port));
      }
  
      for (int j = 0; j < int(parity_senders.size()); j++)
      {
        parity_senders[j].join();
      }
      /*commitSuccess*/
      coordinator_proto::CommitAbortKey commit_abort_key;
      coordinator_proto::ReplyFromCoordinator result;
      grpc::ClientContext context;
      commit_abort_key.set_key(std::to_string(stripeid));
      commit_abort_key.set_ifcommitmetadata(true);
      grpc::Status status;
      status = this->m_coordinator_stub->updateReportSuccess(
            &context, commit_abort_key, &result);
      if(status.ok())
        std::cout<<"collector finished!"<<std::endl;
      else 
        std::cout<<"collector proxy fail!"<<std::endl;
      return;

    };
    try
    {
      /* code */

      std::thread mythread(reconstruc_write,*request);
      mythread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return grpc::Status::OK;
    
     
  }


  //copy SetToMemcached
  bool ProxyImpl::DeltaSendToMemcached(const char *key,size_t key_length,int offset_in_shard,const char *update_data,size_t update_data_length,int delta_type,const char* ip,int port)
  {//key is shardid
    try{
      std::cout << "DeltaSendToMemcached"
                << " " << std::string(ip) << " " << port << std::endl;
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::to_string(port%1000)<<".log"<<std::endl;
      std::cout << "key:"<<std::string(key)<<std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      OppoProject::send_int(socket,3);
      OppoProject::send_int(socket,key_length);
      OppoProject::send_int(socket,update_data_length);
      std::cout<<"offset: "<<offset_in_shard<<" len: "<<update_data_length<<std::endl;
      
      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(update_data, update_data_length));
      
      std::cout<<"send data is "<<std::endl;
      std::cout<<std::string(update_data,update_data_length)<<std::endl;
      std::cout<<std::endl;

      OppoProject::send_int(socket,offset_in_shard);
      OppoProject::send_int(socket,delta_type);


      

      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);

    }
    catch (std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }

    // std::cout << "set " << std::string(key) << " " << std::string(ip) << " " << port << " " << (MEMCACHED_SUCCESS==rc) << " " << value_length << std::endl;
    return true;
  }
  
  bool ProxyImpl::DeltaSendToProxy(OppoProject::Role role,std::vector<int> idxes,std::vector<std::vector<char>> &deltas,int offset_inshard,int length,OppoProject::DeltaType delta_type,const char *ip, int port)
  {
    try
    {
      std::cout<<"send to proxy"<<std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);
      std::cout<<"Role:"<<(int) role<<std::endl;
      OppoProject::send_int(socket,(int) role);

      OppoProject::send_int(socket,idxes.size());
      OppoProject::send_int(socket,offset_inshard);
      OppoProject::send_int(socket,length);
      OppoProject::send_int(socket,(int) delta_type);
      std::cout<<"send num:"<<idxes.size()<<std::endl;
      for(int i=0;i<idxes.size();i++){
        std::cout<<"sending:"<<i<<" th"<<std::endl;
        std::cout<<"send value is"<<std::endl<<std::string(deltas[i].data(),deltas[i].size())<<std::endl;
        std::cout<<std::endl<<"value length: "<<deltas[i].size()<<std::endl;
        OppoProject::send_int(socket,idxes[i]);
        asio::write(socket,asio::buffer(deltas[i].data(),deltas[i].size()));
      }
      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);
      std::cout<<"send to proxy finished"<<std::endl;
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    
    return true;
  }

  bool ProxyImpl::PUDeltaSendToNode(const char *key,size_t key_length,int offset_in_shard,const char *update_data,size_t update_data_length,int delta_type,const char* ip,int port,
                                    std::vector<int> &latest_v_version,int data_idx,int data_verion,int stripeid)
  {
    try
    {
      std::cout << "PUDeltaSendToNode"
                << " " << std::string(ip) << " " << port << std::endl;
      std::cout<<"log path: "<<"/home/wxh/Documents/oppoEC/OOPPO/log/datanode/node"<<std::to_string(port%1000)<<".log"<<std::endl;
      std::cout << "key:"<<std::string(key)<<std::endl;
      asio::ip::tcp::resolver resolver(io_context);
      asio::ip::tcp::resolver::results_type endpoint = resolver.resolve(std::string(ip), std::to_string(port));
      asio::ip::tcp::socket socket(io_context);
      asio::connect(socket, endpoint);


      //strat to send
      OppoProject::send_int(socket,OppoProject::PUDeltaSetWithVersion);
      OppoProject::send_int(socket,key_length);
      OppoProject::send_int(socket,update_data_length);
      std::cout<<"offset: "<<offset_in_shard<<" len: "<<update_data_length<<std::endl;
      
      asio::write(socket, asio::buffer(key, key_length));
      asio::write(socket, asio::buffer(update_data, update_data_length));
      
      std::cout<<"send data is "<<std::endl;
      std::cout<<std::string(update_data,update_data_length)<<std::endl;
      std::cout<<std::endl;

      OppoProject::send_int(socket,offset_in_shard);
      OppoProject::send_int(socket,delta_type);


      //send data idx version
      OppoProject::send_int(socket,stripeid);
      OppoProject::send_vec_int(socket,latest_v_version);
      OppoProject::send_int(socket,data_idx);
      OppoProject::send_int(socket,data_verion);


      //

      std::vector<char> finish(1);
      asio::read(socket, asio::buffer(finish, finish.size()));

      asio::error_code ignore_ec;
      socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignore_ec);
      socket.close(ignore_ec);

    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    
    return true;

  }

  bool ProxyImpl::ReceiveDeltaFromeProxy(asio::ip::tcp::socket &socket,std::map<int,std::vector<char>> &idx_delta,int &offset,int &length,OppoProject::DeltaType &delta_type){
    try
    {
      std::cout<<"receive from proxy"<<std::endl;
      asio::error_code error;

      int idx_num=OppoProject::receive_int(socket,error);

      std::cout<<"receive "<<idx_num<<" delta"<<std::endl;

      offset=OppoProject::receive_int(socket,error);
      length=OppoProject::receive_int(socket,error);
      delta_type=(OppoProject::DeltaType) OppoProject::receive_int(socket,error);
      for(int i=0;i<idx_num;i++){
        std::cout<<"receiving"<<i<<"th delta"<<std::endl;

        int temp_idx=OppoProject::receive_int(socket,error);

        std::cout<<"receiving idx:"<<temp_idx<<std::endl;

        std::vector<unsigned char> t_v_delta(length);
        
        asio::read(socket, asio::buffer(t_v_delta, t_v_delta.size()));
        std::vector<char> v_delta(length);
        memcpy(v_delta.data(),t_v_delta.data(),length);
        if(idx_delta.find(temp_idx)!=idx_delta.end()) std::cerr<<"receive repeated idx from prox"<<std::endl;
        idx_delta[temp_idx]=v_delta;

        std::cout<<"received delta is"<<std::endl;

        std::cout<<std::string(v_delta.data(),length);
        std::cout<<'\n';
      }
      std::cout<<"received from proxy over";
      std::cout<<std::endl;
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
  }

  bool ProxyImpl::ClientUpdateInfoConvert(proxy_proto::StripeUpdateInfo client_update_info,std::unordered_map<int,OppoProject::ShardidxRange>& data_idx_ranges,
                               std::vector<int>& local_idxes,std::vector<int>& global_idxes,std::unordered_map<int,std::pair<std::string, int>>& nodes_ip_port)
  {
    std::cout<<"ClientUpdateInfoConvert"<<std::endl;
    for (int i = 0; i < client_update_info.receive_client_shard_idx_size(); i++)
    {
      int idx = client_update_info.receive_client_shard_idx(i);
      int offset = client_update_info.receive_client_shard_offset(i);
      int len = client_update_info.receive_client_shard_length(i);
      data_idx_ranges[idx]=ShardidxRange(idx,offset,len);
      nodes_ip_port[idx]=std::make_pair(client_update_info.data_nodeip(i),client_update_info.data_nodeport(i));
    }
    for (int i = 0; i < client_update_info.local_parity_idx_size(); i++)
    {
      int temp_local_parity_idx=client_update_info.local_parity_idx(i);
      local_idxes.push_back(temp_local_parity_idx);
      nodes_ip_port[temp_local_parity_idx]=std::make_pair(client_update_info.local_parity_nodeip(i), client_update_info.local_parity_nodeport(i));
    }

    for(int i=0;i<client_update_info.global_parity_idx_size();i++)
    {
      int temp_global_parity_idx =client_update_info.global_parity_idx(i);
      global_idxes.push_back(temp_global_parity_idx);
      nodes_ip_port[temp_global_parity_idx]=std::make_pair(client_update_info.global_parity_nodeip(i),client_update_info.global_parity_nodeport(i));
    }

    return true;
  }
  
  bool ProxyImpl::ReceiveDataFromClient(asio::ip::tcp::socket &socket_data,std::unordered_map<int,std::vector<char> > &new_shard_data_map,
                                        int &offset,int &length,int stripeid,asio::error_code &error)
  {
    std::cout<<"start to recive data from client"<<std::endl;
    try
    {
      int client_stripeid = OppoProject::receive_int(socket_data,error);
      if(client_stripeid!=stripeid)
      {
        std::cout<<"client stripeid:"<<client_stripeid<<" stripeid from MDS:"<<stripeid<<std::endl;
        std::cerr<<"update stripeid doesn't match (client and coordinator)"<<std::endl;

      }

      int count=OppoProject::receive_int(socket_data,error);
      int final_offset=0;
      int final_length=0;

      std::cout<<"received num from client:"<<count<<std::endl;
      for(int i=0;i<count;i++)
      {
        std::cout<<"receive :"<<i<<" ith idx"<<std::endl;
        int idx=OppoProject::receive_int(socket_data,error);
        std::cout<<" idx:";
        int temp_offset=OppoProject::receive_int(socket_data,error);
        std::cout<<" offset:";
        int temp_len=OppoProject::receive_int(socket_data,error);
        std::cout<<" len:";
        if(i==0)
        {
          final_offset=temp_offset;
          final_length=temp_len;
        }
        else if(final_offset != temp_offset || final_length!=temp_len )
        {
          std::cout<<"cur offset:"<<temp_offset<<" cur len:"<<temp_len<<" finaloff:"<<final_offset<<" finallen:"<<final_length<<std::endl;
          std::cerr<<"error! different offset len fo two shard"<<std::endl;
        }
          
        std::vector<unsigned char> temp_data(temp_len);
        std::vector<char> v_data(temp_len);
        asio::read(socket_data,asio::buffer(temp_data,temp_data.size()), error);
        memcpy(v_data.data(),temp_data.data(),temp_len);
        std::cout<<"data is: "<<v_data.data()<<std::endl;
        new_shard_data_map[idx]=v_data;
      }
      
      offset=final_offset;
      length=final_length;

      std::cout<<"received from client finished"<<std::endl;

    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    return true;
    
  }

  grpc::Status  ProxyImpl::dataProxyPURMW(
        grpc::ServerContext* context, 
        const proxy_proto::PURMWNotice* request, 
        proxy_proto::PURMWResponse* response)
  {
    std::cout<<"there is RMW proxyPU"<<std::endl;
    std::vector<int> latest_version;
    for(int i=0;i<request->latest_data_version_size();i++)
    {
      latest_version.push_back(request->latest_data_version(i));
    }
    auto min_tolerace=request->minimal_tolerance();
    auto max_wait_time=request->max_wait_time_ms();
    auto receive_RMW_PU_update = [this](proxy_proto::DataProxyUpdatePlan data_proxy_plan,std::vector<int> latest_data_version,int min_tolerance,int max_wait_time_ms)
    {
      //从client收数据需要的结构：
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
      
      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();

      //相关
      std::vector<int> localparity_idxes;
      std::vector<int> globalparity_idxes;  
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      std::unordered_map<int,std::vector<char> > old_shard_data_map;
      std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的
      int purmw_stat=OppoProject::FailAndNoNewData;
      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
      
      for(auto const & tttt:data_nodes_ip_port)
      {
        std::cerr<<"idx: "<<tttt.first;
        std::cerr<<"ip port: "<<tttt.second.first<<' '<<tttt.second.second;
      }
      try
      {
        
        
        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cout<<"getting from data node fails"<<std::endl;
            return;
          } 
        };
        
        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }
        /*开始计时*/
        auto start_time = std::chrono::steady_clock::now();
        auto max_duration = std::chrono::milliseconds(max_wait_time_ms);
        // 获取结束时间点
        auto max_end_time = start_time+max_duration;
        

        //1.2  同时将新数据暂存到proxy中
        m_temp_new_shard_data_map.clear();
        auto stripeid=data_proxy_plan.stripeid();
        m_temp_updating_stripeid=stripeid;
        m_temp_new_shard_data_map = new_shard_data_map;
        m_temp_v_version.clear();
        m_temp_v_version=latest_data_version;
        purmw_stat=OppoProject::FailAndReceiveNewData;
        //2. 给old data 以及delta分配空间
        
        std::cout<<"2. 给old data 以及delta分配空间"<<std::endl;;
        for(auto const & ttt:data_idx_ranges)
        {
          int idx=ttt.first;
          int len=data_idx_ranges[idx].range_length;
          std::vector<char> old_data(len);
          std::vector<char> data_delta(len);
          old_shard_data_map[idx]=old_data;//need to assignment
          cur_az_data_delta_map[idx]=data_delta;
          std::cout<<"old and cur delta size:"<<old_shard_data_map[idx].size()<<" :"<<cur_az_data_delta_map[idx].size()<<std::endl;
        }


        //3. read old
        for(auto const & ttt : data_idx_ranges){
          int idx=ttt.first;
          int offset=ttt.second.offset_in_shard;
          int len=ttt.second.range_length;

          std::cout<<"len receive from coordinator"<<std::endl;

          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }

        //4. 计算本AZ内 delta
        std::vector<std::thread> delta_calculators;

        for(auto const & ttt : data_idx_ranges){//read old
          int idx=ttt.first;
          int len=ttt.second.range_length;
          //calculate_data_delta(new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len);
        
          delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len));
        }
        
        for(int i=0;i<delta_calculators.size();i++){
          delta_calculators[i].join();
        }
        
        //5. calculate parity delta

        int k=data_proxy_plan.k();
        int real_l=data_proxy_plan.real_l();
        int m=data_proxy_plan.g_m();

        std::cout<<"k: "<<k<<" real_l  :  "<<real_l<<" m: "<<m<<"\n";
        OppoProject::EncodeType encode_type=(OppoProject::EncodeType) data_proxy_plan.encode_type();

        std::vector<char*> all_update_data_delta_ptrs;
        std::vector<int> all_update_data_idx;

        std::vector<char *> v_coding(m + real_l + 1);
        char **coding = (char **)v_coding.data();
        std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(length_from_client));



        for(auto const & iter:cur_az_data_delta_map)
        {
          all_update_data_delta_ptrs.push_back(const_cast<char*>(iter.second.data()));
          all_update_data_idx.push_back(iter.first);
        }

        for (int j = 0; j < m + real_l + 1; j++)
        {
          coding[j] = v_coding_area[j].data();
          bzero(coding[j],length_from_client);
        }

        RMW_encode(k,m,real_l,all_update_data_delta_ptrs.data(),coding,length_from_client,encode_type,all_update_data_idx);
        
        
        std::cerr<<"1111\n";

        //6. send parity delta to data node

        auto saved_delta_parity_idx_ptr=std::make_shared<std::vector<int>>();
        auto cv_ptr = std::make_shared<std::condition_variable>();
        auto myLock_ptr=std::make_shared<std::mutex>();
        std::cerr<<"1.55555555555\n";
        int updated_data_idx=all_update_data_idx[0];
        
        std::cerr<<"2222\n";
        /*for debug*/
        std::cerr<<"received v_version: \n";
        for(auto const & a: latest_data_version)
          std::cerr<<a<<' ';
        std::cerr<<latest_data_version.at(updated_data_idx);
        int updated_data_version=latest_data_version[updated_data_idx];
        std::cout<<"dataidx :"<<updated_data_idx<<"version: "<<updated_data_version<<std::endl;
        std::cv_status cv_stat;
        auto rmw_encode_over_time = std::chrono::steady_clock::now();

        // 计算耗时
        auto spent_duration = std::chrono::duration_cast<std::chrono::milliseconds>(rmw_encode_over_time - start_time);
        
        if(spent_duration>=max_duration)
        {
          /*to inform MDS timeout for debug*/
          return;
          std::cerr<<"no time\n";
        }

        auto left_duration=max_duration-spent_duration;


        auto delta_to_datanode=[this,min_tolerance,saved_delta_parity_idx_ptr,myLock_ptr,cv_ptr](int stripe_id,int shard_idx,int offset_inshard,int blocksize,int j,char** coding,
                                                                                  int deltatype,std::string node_ip,int node_port,std::vector<int> latest_v_version,int data_idx,int data_verion)
        {
          std::string shard_id = std::to_string(stripe_id * 1000 + shard_idx);
          bool ret=PUDeltaSendToNode(shard_id.c_str(),shard_id.size(),offset_inshard,coding[j],blocksize,deltatype,node_ip.c_str(),node_port,
                                     latest_v_version,data_idx,data_verion,stripe_id);
          if(!ret)
          {
              std::cout << "getFromNode !ret" << std::endl;
              return;
          }

          myLock_ptr->lock();
          if(saved_delta_parity_idx_ptr->size()< min_tolerance)
          {
            std::cerr<<"parity : "<<shard_idx<<" return !\n";
            saved_delta_parity_idx_ptr->push_back(shard_idx);
            if(saved_delta_parity_idx_ptr->size() >=min_tolerance)
            {
              std::cerr<<saved_delta_parity_idx_ptr->size()<<"个 prity return ,inform MDS\n";
              cv_ptr->notify_all();
            }
          }
          myLock_ptr->unlock();
        };

        std::vector<std::thread> delta_senders;
        int delta_send_num=0;
        if(encode_type==RS) delta_send_num=m;
        else if(encode_type==OppoProject::OPPO_LRC) delta_send_num=m+real_l;
        else if(encode_type==OppoProject::Azure_LRC_1) delta_send_num=m+real_l+1;
        if(all_update_data_idx.size()!=1) std::cout<<"PU RMW new data num wrong!\n";//for debug

        for(int i=0;i<delta_send_num;i++)
        {
          int idx=i+k;
          std::string shard_id=std::to_string(stripeid*1000+idx);
          int dataversion=updated_data_version;
          std::cout<<"in PU make senddelta thread,dataversion: "<<dataversion<<std::endl;
          delta_senders.push_back(std::thread(delta_to_datanode,stripeid,idx,offset_from_client,length_from_client,i,coding,
                                                                (int)OppoProject::ParityDelta,data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second,latest_data_version,updated_data_idx,dataversion));
          //stripeid,idx,i,offset_from_client,coding,length_from_client,(int)OppoProject::ParityDelta,
                                              //data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second,latest_data_version,updated_data_idx,std::move(dataversion)));
        }

        for(int i=0;i<delta_senders.size();i++) delta_senders[i].detach();

        std::unique_lock<std::mutex> lck(*myLock_ptr);
        while(saved_delta_parity_idx_ptr->size()<min_tolerance)
        {
          //cv_ptr->wait(lck);
          cv_stat=cv_ptr->wait_for(lck,left_duration);
        }

        if(cv_stat==std::cv_status::timeout)
        {
          purmw_stat=OppoProject::FailAndReceiveNewData;
          /*to inform MDS timeout commit fail */
          try
          {
            /* code */
            /*commitSuccess*/
            coordinator_proto::PURMWCoordinatorReply purmw_report;

            coordinator_proto::ReplyFromCoordinator result;
            grpc::ClientContext context;


            purmw_report.set_stripeid(stripeid);
            purmw_report.set_dataidx(updated_data_idx);
            purmw_report.set_dataversion(updated_data_version);
            purmw_report.set_purmw_result(purmw_stat);
            grpc::Status status;
            status = this->m_coordinator_stub->PURMWReportSuccess(
                  &context, purmw_report, &result);
            std::cerr<<"reply from MDS: "<<result.message();
            if(status.ok())
              std::cout<<"data proxy PURMW outtime and inform MDS"<<std::endl;
            else 
              std::cout<<"data proxy PURMW outtime and MDS return false"<<std::endl;
            }
          catch(const std::exception& e)
          {
            std::cerr << e.what() << '\n';
          }

          return ;
        }
        
        auto delta_send_over_time = std::chrono::steady_clock::now();


        //for debug
        std::cout<<"success saved  parity idx";
        for(auto const &a : *saved_delta_parity_idx_ptr) std::cout<<a<<' ';
        std::cout<<"\n";



        //7. newdata send to memcached
        auto send_to_datanode = [this](std::string shard_id, char *data, int x_shard_size, std::pair<std::string, int> ip_and_port,std::vector<int> v_version,int idx,int k)
       {
           std::cout<<"new data send to data node"<<std::endl;
           //SetToMemcached(shard_id.c_str(), shard_id.size(), data, x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
           this->VersionedBlockToNode(shard_id.c_str(),shard_id.size(),data,  x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,v_version,
                                idx,k, OppoProject::DiskDataSetWithVersion);
       };
         
        std::vector<std::thread> new_data_senders;
        for(int i=0;i<new_shard_data_map.size();i++)
        {
          int idx=all_update_data_idx[i];
          std::string shard_id = std::to_string(stripeid * 1000 + idx);
          data_nodes_ip_port[idx];
          new_data_senders.push_back(std::thread(send_to_datanode,shard_id,new_shard_data_map[idx].data(),length_from_client,data_nodes_ip_port[idx],latest_data_version,idx,k));
          //new_data_senders.push_back(std::thread(send_to_datanode, idx, k, shard_id, all_update_data_delta_ptrs.data(), coding, length_from_client, data_nodes_ip_port[idx]));
        }
        
        for(int i=0;i<new_data_senders.size();i++) new_data_senders[i].join();
        std::cout<<"data proxy PURMW send over! clear temp saved"<<std::endl;
        m_temp_updating_stripeid=-1;
        m_temp_new_shard_data_map.clear();
        m_temp_v_version.clear();

        purmw_stat=OppoProject::PURMWSuccess;
        try
        {
          /* code */
          
          /*commitSuccess*/
          coordinator_proto::PURMWCoordinatorReply purmw_report;

          coordinator_proto::ReplyFromCoordinator result;
          grpc::ClientContext context;


          purmw_report.set_stripeid(stripeid);
          for(auto const &a : *saved_delta_parity_idx_ptr)
          {
            purmw_report.add_success_parity_ids(a);
          }
          purmw_report.set_dataidx(updated_data_idx);
          purmw_report.set_dataversion(updated_data_version);
          purmw_report.set_purmw_result(purmw_stat);

          grpc::Status status;
          status = this->m_coordinator_stub->PURMWReportSuccess(
                &context, purmw_report, &result);
          std::cerr<<"reply from MDS: "<<result.message();
          if(status.ok())
            std::cout<<"data proxy PURMW finished and inform MDS successfully"<<std::endl;
          else 
            std::cout<<"data proxy PURMW finished but MDS return false"<<std::endl;
        }
        catch(const std::exception& e)
        {
          std::cerr << e.what() << '\n';
        }
        
        
        return;
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
    };



    try
    {
      int version_len=request->latest_data_version_size();
  
      for(int i=0;i<version_len;i++)
        latest_version.push_back(request->latest_data_version(i));
      std::thread mythread(receive_RMW_PU_update, request->data_proxy_plan(),latest_version,min_tolerace,max_wait_time);
      mythread.detach();
    }
    catch(const std::exception& e)
    {
      std::cout << e.what() << '\n';
    }

    return grpc::Status::OK;
  }
  grpc::Status  
  ProxyImpl::PARIXUpdate(
        grpc::ServerContext* context, 
        const proxy_proto::DataProxyUpdatePlan* request, 
        proxy_proto::DataProxyReply* response)
  {
    std::cout<<"there is RMW proxy"<<std::endl;
    auto PARIX_receive_send = [this](proxy_proto::DataProxyUpdatePlan data_proxy_plan)
    {
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
      
      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();
      int k=data_proxy_plan.k();
      int real_l=data_proxy_plan.real_l();
      int m=data_proxy_plan.g_m();
      OppoProject::EncodeType encode_type=(OppoProject::EncodeType) data_proxy_plan.encode_type();
      //相关
      std::vector<int> localparity_idxes;
      std::vector<int> globalparity_idxes;  
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      std::unordered_map<int,std::vector<char> > old_shard_data_map;
      std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的
      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
      try
      {
        /* code */
        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cout<<"getting from data node fails"<<std::endl;
            return;
          } 
        };

        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }
        //2. 给old data 分配空间
         std::cout<<"2. 给old data 以及delta分配空间"<<std::endl;;
        for(auto const & ttt:data_idx_ranges)
        {
          int idx=ttt.first;
          int len=data_idx_ranges[idx].range_length;
          std::vector<char> old_data(len);
          std::vector<char> data_delta(len);
          old_shard_data_map[idx]=old_data;//need to assignment
          std::cout<<"old and cur delta size:"<<old_shard_data_map[idx].size()<<" :"<<cur_az_data_delta_map[idx].size()<<std::endl;
        }
        //3. read old
        for(auto const & ttt : data_idx_ranges)
        {
          int idx=ttt.first;
          int offset=ttt.second.offset_in_shard;
          int len=ttt.second.range_length;

          std::cout<<"len receive from coordinator"<<std::endl;

          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }
        //4. send old and new to parity node
        //已经有的send是用的coding[j]去做的，实际上只需将old new data发给每个节点即可
        int delta_send_num=0;
        if(encode_type==RS) delta_send_num=m;
        else if(encode_type==OppoProject::OPPO_LRC) delta_send_num=m+real_l;
        else if(encode_type==OppoProject::Azure_LRC_1) delta_send_num=m+real_l+1;

        //5. finish

      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      
    };
  }
  grpc::Status  
  ProxyImpl::TimeoutRCW(
        grpc::ServerContext* context, 
        const proxy_proto::ReconstructWriteNotice* request, proxy_proto::DataProxyReply* response)
  {

    int stripeid=    request->stripeid();
    int k =          request->k();
    int m =          request->m() ;
    int real_l =     request->real_l();
    int shard_size=  request->shard_size();
    OppoProject::EncodeType encode_type =(OppoProject::EncodeType) request->encode_type();
    std::vector<std::pair<std::string, int>> nodes_ip_and_port;
    for(int i=0;i<request->nodeip_size();i++)
    {
      nodes_ip_and_port.push_back(std::make_pair(request->nodeip(i),request->nodeport(i)));
    }
    
    std::vector<char *> v_data(k);
    std::vector<char *> v_coding(m + real_l + 1);
    char **data = (char **)v_data.data();
    char **coding = (char **)v_coding.data();

    std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(shard_size));
    std::vector<std::vector<char>> v_data_area(k,std::vector<char>(shard_size));

    for (int j = 0; j < k; j++)
    {
      data[j] = v_data_area[j].data();
    }

    for (int j = 0; j < m + real_l + 1; j++)
    {
      coding[j] = v_coding_area[j].data();
    }

    /* read old*/
    std::vector<std::thread> readers;

    auto get_from_node=[this](std::string shard_id,int offset,int length,char* value,std::string ip, int port)
    {
      size_t temp_size;
      bool ret=GetFromMemcached(shard_id.c_str(),shard_id.size(),value,&temp_size,offset,length,ip.c_str(),port);
      if (!ret)
      {
        std::cout << "reconstruct write getFromNode !ret" << std::endl;
        return;
      }
    };
    if(m_temp_new_shard_data_map.size()!=1 || m_temp_v_version.size()!=k)
    {
      std::cerr<<"timeout rcw wrong m_temp_new_shard_data_map_size() or m_temp_v_version.size() "<<m_temp_new_shard_data_map.size()<<" "<<m_temp_v_version.size()<<"\n";
    }
    for(auto const & a: m_temp_v_version) 
    {
      std::cerr<<a<<' ';
    }
    std::cerr<<"\n";

    int new_data_idx = m_temp_new_shard_data_map.begin()->first;
    std::cerr<<"reconstruct new_data_idx"<<new_data_idx<<'\n';
    for(int i=0;i<k;i++)
    {
      if(i==new_data_idx) continue;
      std::string shard_id=std::to_string(stripeid*1000+i);
      readers.push_back(std::thread(get_from_node,shard_id,0,shard_size,data[i],nodes_ip_and_port[i].first,nodes_ip_and_port[i].second));
    }
    for(int i=0;i<readers.size();i++) readers[i].join();

    std::copy(m_temp_new_shard_data_map.begin()->second.begin(),m_temp_new_shard_data_map.begin()->second.end(),v_data_area[new_data_idx].begin());
    std::cout<<"reconstruct read success\n";
    int send_num=0;
    if (encode_type == RS)
    {
      encode(k, m, 0, data, coding, shard_size, encode_type);
      send_num =  m;
    }
    else if (encode_type == Azure_LRC_1)
    {
      // m = g for lrc
      encode(k, m, real_l, data, coding, shard_size, encode_type);
      send_num =  m + real_l + 1;
    }
    else if (encode_type == OPPO_LRC)
    {
      encode(k, m, real_l, data, coding, shard_size, encode_type);
      send_num =  m + real_l;
    }


    /* set to node*/
    auto send_to_datanode = [this](int j, int k, std::string shard_id, char **data, char **coding, int x_shard_size, std::pair<std::string, int> ip_and_port,std::vector<int> v_version)
    {
      if (j < k)
      {
        //SetToMemcached(shard_id.c_str(), shard_id.size(), data[j], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
        this->VersionedBlockToNode(shard_id.c_str(),shard_id.size(), data[j],  x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,v_version,
                                j,k, OppoProject::DiskDataSetWithVersion);
      }
      else
      {

        //SetToMemcached(shard_id.c_str(), shard_id.size(), coding[j - k], x_shard_size, ip_and_port.first.c_str(), ip_and_port.second);
        this->VersionedBlockToNode(shard_id.c_str(),shard_id.size(), coding[j - k],  x_shard_size, ip_and_port.first.c_str(), ip_and_port.second,v_version,
                                j,k, OppoProject::DisKRCWParitySet);
      }
    };
    /*send parity*/
    std::vector<std::thread> dp_senders;
    for (int j = 0; j < send_num; j++)
    {
      int idx=j+k;
      std::string shard_id = std::to_string(stripeid * 1000 + idx);
      std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[idx];
      dp_senders.push_back(std::thread(send_to_datanode, idx, k, shard_id, data, coding, shard_size, ip_and_port,m_temp_v_version));
    }
    /*send new data*/
    {
      int idx=new_data_idx;
      std::string shard_id = std::to_string(stripeid * 1000 + idx);
      std::pair<std::string, int> &ip_and_port = nodes_ip_and_port[idx];
      dp_senders.push_back(std::thread(send_to_datanode, idx, k, shard_id, data, coding, shard_size, ip_and_port,m_temp_v_version));
    }
    
    for (int j = 0; j < int(dp_senders.size()); j++)
    {
      dp_senders[j].join();
    }
    m_temp_new_shard_data_map.clear();
    m_temp_v_version.clear();
    return grpc::Status::OK;
  }



  grpc::Status ProxyImpl::VersionedRCW(
        grpc::ServerContext* context, 
        const proxy_proto::VersionRCWNotice* request,
        proxy_proto::DataProxyReply* response)
  {
    auto receive_rcw=[this](proxy_proto::VersionRCWNotice versioned_notice)
    {
      /*1. info transfer*/
      auto  rcw_notice =versioned_notice.base_rcw_info();
      int stripeid=    rcw_notice.stripeid();
      int k =          rcw_notice.k();
      int m =          rcw_notice.m() ;
      int real_l =     rcw_notice.real_l();
      int shard_size=  rcw_notice.shard_size();
      std::vector<int> v_verison;
      for(int i=0;i<versioned_notice.v_version_size();i++)
      {
        v_verison.push_back(versioned_notice.v_version(i));
      }
      /*for debug*/
      std::cerr<<"Versioned RCW v_version: ";
      for(int i=0;i<v_verison.size();i++) std::cerr<<v_verison[i]<<' ';
      std::cerr<<std::endl;

      int offset_from_client=0;
      int length_from_client=0;
      OppoProject::EncodeType encode_type =(OppoProject::EncodeType) rcw_notice.encode_type();
    
      std::vector<std::pair<std::string, int>> nodes_ip_and_port;
  
      for(int i=0;i<rcw_notice.nodeip_size();i++)
      {
        nodes_ip_and_port.push_back(std::make_pair(rcw_notice.nodeip(i),rcw_notice.nodeport(i)));
      }

      /*2. receive */
      std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      try
      {
        std::cout<<"waiting for client's connet"<<std::endl;
        asio::ip::tcp::socket socket_data_dataproxy(io_context);
        asio::error_code error;
        acceptor.accept(socket_data_dataproxy,error);
        if(error)
        {
          std::cout<<"client's connet wrong"<<std::endl;
          std::cout<<error.message()<<std::endl;
        }
        OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
        if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
        ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
        socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
        socket_data_dataproxy.close(error);
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
      }
      if(length_from_client!=shard_size || offset_from_client!=0) std::cerr<<"wrong len offset"<<length_from_client<<' '<<shard_size<<' '<<offset_from_client<<'\n';

      /*3. read old*/
      std::vector<char *> v_data(k);
      std::vector<char *> v_coding(m + real_l + 1);
      char **data = (char **)v_data.data();
      char **coding = (char **)v_coding.data();
      std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(shard_size));
      std::vector<std::vector<char>> v_data_area(k,std::vector<char>(shard_size));
      for (int j = 0; j < k; j++)
      {
        data[j] = v_data_area[j].data();
      }
  
      for (int j = 0; j < m + real_l + 1; j++)
      {
        coding[j] = v_coding_area[j].data();
      }
      std::vector<std::thread> readers;


      auto get_from_node=[this](std::string shard_id,int offset,int length,char* value,std::string ip, int port)
      {
        size_t temp_size;
        bool ret=GetFromMemcached(shard_id.c_str(),shard_id.size(),value,&temp_size,offset,length,ip.c_str(),port);
        if (!ret)
        {
          std::cout << "reconstruct write getFromNode !ret" << std::endl;
          return;
        }
      };

      for(int i=0;i<k;i++)
      {
        if(new_shard_data_map.find(i) != new_shard_data_map.end())  continue;
        std::string shard_id=std::to_string(stripeid*1000+i);
        readers.push_back(std::thread(get_from_node,shard_id,0,shard_size,data[i],nodes_ip_and_port[i].first,nodes_ip_and_port[i].second));
      }
      for(int i=0;i<readers.size();i++) readers[i].join();
      /*3.2 copy new to area*/

      for(auto const & ttt : new_shard_data_map)
      {
        int idx=ttt.first;
        std::copy(ttt.second.begin(),ttt.second.end(),data[idx]);
      }
      std::cout<<"reconstruct read success\n";
      /*4. encode*/
      int all_parity_num=0;
      if (encode_type == RS)
      {
        encode(k, m, 0, data, coding, shard_size, encode_type);
        all_parity_num =  m;
      }
      else if (encode_type == Azure_LRC_1)
      {
        // m = g for lrc
        encode(k, m, real_l, data, coding, shard_size, encode_type);
        all_parity_num =  m + real_l + 1;
      }
      else if (encode_type == OPPO_LRC)
      {
        encode(k, m, real_l, data, coding, shard_size, encode_type);
        all_parity_num =  m + real_l;
      }

      /*5. send new data and parity with version*/
      auto send_to_node_with_version = [this](const char *key, size_t key_length, const char *value, size_t value_length, const char *ip, int port,std::vector<int> v_version,
                             int idx,int k,OppoProject::DataNodeOp datanode_op)
        {
          bool ret =this->VersionedBlockToNode(key,  key_length, value,  value_length, ip,  port,v_version,
                                idx,k, datanode_op);
        };
      std::vector<std::thread> senders;
      for(auto const ttt:new_shard_data_map)
      {
        
        int idx=ttt.first;
        std::string shard_id = std::to_string(stripeid * 1000 + idx);
        auto aa=OppoProject::DiskDataSetWithVersion;
        senders.push_back(std::thread(send_to_node_with_version,shard_id.c_str(),shard_id.size(),ttt.second.data(),ttt.second.size(),
                                      nodes_ip_and_port[idx].first.c_str(),nodes_ip_and_port[idx].second,v_verison,idx,k,aa));
      }
      for(int j=0;j<all_parity_num;j++)
      {
        int idx=k+j;
        std::string shard_id = std::to_string(stripeid * 1000 + idx);
        auto aa=OppoProject::DisKRCWParitySet;
        senders.push_back(std::thread(send_to_node_with_version,shard_id.c_str(),shard_id.size(),coding[j],shard_size,
                                      nodes_ip_and_port[idx].first.c_str(),nodes_ip_and_port[idx].second,v_verison,idx,k,aa));
      }
      for(int i=0;i<senders.size();i++) senders[i].join();
      /*6. inform coordinator*/
      /*继续用这个updateReportSuccess*/
      coordinator_proto::CommitAbortKey commit_abort_key;
      coordinator_proto::ReplyFromCoordinator result;
      grpc::ClientContext context;
      commit_abort_key.set_key(std::to_string(stripeid));
      commit_abort_key.set_ifcommitmetadata(true);
      grpc::Status status;
      status = this->m_coordinator_stub->updateReportSuccess(
            &context, commit_abort_key, &result);
      if(status.ok())
        std::cout<<"Versioned RCW finished!"<<std::endl;
      else 
        std::cout<<"Versioned RCW finished!"<<std::endl;
      return;
    };
    /*thread detach*/
    try
    {
      /* code */
      std::thread mythread(receive_rcw,*request);
      mythread.detach();
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }
    
    
    return grpc::Status::OK;
  }



  bool ProxyImpl::RMW_receive_calculate(proxy_proto::DataProxyUpdatePlan data_proxy_plan,std::unordered_map<int,std::vector<char> > &new_shard_data_map,
                              std::unordered_map<int,std::vector<char> > &cur_az_data_delta_map,std::vector<std::vector<char>> &v_coding_area)
  {
    //从client收数据需要的结构：
      std::unordered_map<int,OppoProject::ShardidxRange> data_idx_ranges; //idx->(idx,offset,length) 
      std::unordered_map<int,std::pair<std::string, int>> data_nodes_ip_port;//idx->node ip port
      
      int offset_from_client;
      int length_from_client;
      int stripeid=data_proxy_plan.stripeid();

      //相关
      std::vector<int> localparity_idxes;
      std::vector<int> globalparity_idxes;  
      //std::unordered_map<int,std::vector<char> > new_shard_data_map;//idx->data
      std::unordered_map<int,std::vector<char> > old_shard_data_map;
      //std::unordered_map<int,std::vector<char> > cur_az_data_delta_map;//包括从proxy 接收以及本AZ计算得到的

      ClientUpdateInfoConvert(data_proxy_plan.client_info(),data_idx_ranges,localparity_idxes,globalparity_idxes,data_nodes_ip_port);
      try
      {
        
        
        auto read_from_datanode=[this](std::string shardid,int offset,int length,char *data,std::string nodeip,int port)
        {
          size_t temp_size;//
          bool reslut=GetFromMemcached(shardid.c_str(),shardid.size(),data,&temp_size,offset,length,nodeip.c_str(),port);
          if(!reslut){
            std::cout<<"getting from data node fails"<<std::endl;
            return;
          } 
        };
        
        //1. 从client接收
        if(data_proxy_plan.client_info().receive_client_shard_idx_size()>0)
        {
          std::cout<<"waiting for client's connet"<<std::endl;
          asio::ip::tcp::socket socket_data_dataproxy(io_context);
          asio::error_code error;
          acceptor.accept(socket_data_dataproxy,error);
          if(error)
          {
            std::cout<<"client's connet wrong"<<std::endl;
            std::cout<<error.message()<<std::endl;
          }
          OppoProject::Role role=(OppoProject::Role)OppoProject::receive_int(socket_data_dataproxy,error);
          if(role!=OppoProject::RoleClient) std::cerr<<"wrong connection"<<std::endl;
          ReceiveDataFromClient(socket_data_dataproxy,new_shard_data_map,offset_from_client,length_from_client,stripeid,error);
          socket_data_dataproxy.shutdown(asio::ip::tcp::socket::shutdown_both, error);
          socket_data_dataproxy.close(error);
        }

        //2. 给old data 以及delta分配空间
        
        std::cout<<"2. 给old data 以及delta分配空间"<<std::endl;;
        for(auto const & ttt:data_idx_ranges)
        {
          int idx=ttt.first;
          int len=data_idx_ranges[idx].range_length;
          std::vector<char> old_data(len);
          std::vector<char> data_delta(len);
          old_shard_data_map[idx]=old_data;//need to assignment
          cur_az_data_delta_map[idx]=data_delta;
          std::cout<<"old and cur delta size:"<<old_shard_data_map[idx].size()<<" :"<<cur_az_data_delta_map[idx].size()<<std::endl;
        }


        //3. read old
        for(auto const & ttt : data_idx_ranges){
          int idx=ttt.first;
          int offset=ttt.second.offset_in_shard;
          int len=ttt.second.range_length;

          std::cout<<"len receive from coordinator"<<std::endl;

          std::string shardid=std::to_string(stripeid*1000+idx);
          read_from_datanode(shardid,offset,len,old_shard_data_map[idx].data(),data_nodes_ip_port[idx].first,data_nodes_ip_port[idx].second);
          std::cout<<"read shard:"<<shardid<<std::endl;
        }

        //4. 计算本AZ内 delta
        std::vector<std::thread> delta_calculators;

        for(auto const & ttt : data_idx_ranges){//read old
          int idx=ttt.first;
          int len=ttt.second.range_length;
          //calculate_data_delta(new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len);
        
          delta_calculators.push_back(std::thread(calculate_data_delta,new_shard_data_map[idx].data(),old_shard_data_map[idx].data(),cur_az_data_delta_map[idx].data(),len));
        }
        
        for(int i=0;i<delta_calculators.size();i++){
          delta_calculators[i].join();
        }
        
        //5. calculate parity delta

        int k=data_proxy_plan.k();
        int real_l=data_proxy_plan.real_l();
        int m=data_proxy_plan.g_m();
        OppoProject::EncodeType encode_type=(OppoProject::EncodeType) data_proxy_plan.encode_type();

        std::vector<char*> all_update_data_delta_ptrs;
        std::vector<int> all_update_data_idx;

        std::vector<char *> v_coding(m + real_l + 1);
        char **coding = (char **)v_coding.data();
        
        //std::vector<std::vector<char>> v_coding_area(m + real_l + 1, std::vector<char>(length_from_client));
        for(int ttttt=0;ttttt<m + real_l + 1;ttttt++)
        {
          v_coding_area.push_back(std::vector<char>(length_from_client));
        }


        for(auto const & iter:cur_az_data_delta_map)
        {
          all_update_data_delta_ptrs.push_back(const_cast<char*>(iter.second.data()));
          all_update_data_idx.push_back(iter.first);
        }

        for (int j = 0; j < m + real_l + 1; j++)
        {
          coding[j] = v_coding_area[j].data();
          bzero(coding[j],length_from_client);
        }

        RMW_encode(k,m,real_l,all_update_data_delta_ptrs.data(),coding,length_from_client,encode_type,all_update_data_idx);
        
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
        return false;
      }
      return true;
  }





  

  

} // namespace OppoProject
