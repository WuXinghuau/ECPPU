#ifndef DATANODE_H
#define DATANODE_H

#include <libmemcached/memcached.h>
#include <asio.hpp>
#include <string>
#include "logmanager.h"
#include "azure_lrc.h"
#include "blockfilesaver.h"
#include "versionedLogManager.h"

class DataNode
{
public:
    DataNode(std::string ip, int port) : ip(ip), port(port),
                                         acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ip.c_str()), port))
    {
        //wxh

        char buff[512];
        getcwd(buff, 512);
        std::cout<<"pwd:"<<buff<<std::endl;
        //std::string log_file_path = std::string(buff) + "/../../../log/paritylog/PL"+std::to_string(port%1000)+".log";
        std::string log_file_path="/home/wxh/Documents/oppoEC/OOPPO/log/paritylog/PL"+std::to_string(port%1000)+".log";
        std::cout<<"log file path:"<<log_file_path<<std::endl;
        logmanager.init(log_file_path);

        //init diskfile
        //std::string disk_file_path = std::string(buff) + "/../../../log/disk/disk"+std::to_string(port%1000)+".log";
        std::string disk_file_path ="/home/wxh/Documents/oppoEC/OOPPO/log/disk/disk"+std::to_string(port%1000)+".log";
        std::cout<<"disk file path:"<<disk_file_path<<std::endl;
        if(!blocksaver.init(disk_file_path)) std::cerr<<"init disk file error\n";
        std::string versioned_log_file_path="/home/wxh/Documents/oppoEC/OOPPO/log/versionPL/PL"+std::to_string(port%1000)+".log";
        std::cerr<<"vesioned log file path:"<<versioned_log_file_path<<std::endl;
        if(!versioned_log_manager.init(versioned_log_file_path))   std::cerr<<"init version_log_manager file error\n";      
        
        memcached_return rc;
        m_memcached = memcached_create(NULL);
        memcached_server_st *servers;
        // libmemcached和memcached在同一个节点上，端口号相差1000
        servers = memcached_server_list_append(NULL, ip.c_str(), port - 1000, &rc);
        rc = memcached_server_push(m_memcached, servers);
        memcached_server_free(servers);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5);
        memcached_behavior_set(m_memcached, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, true);


    }
    void start();

private:
    void do_work();
    void put_log(int flag,int port,std::string key,std::string print_data,std::string stored_data="\n",std::string merged_data="\n");
    memcached_st *m_memcached;
    std::string ip;
    int port;
    BlockFileSaver blocksaver;
    LogManager logmanager;
    VersionedLogManager versioned_log_manager;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor;

    static std::mutex m_mutex;
    std::string m_log_file_path="/home/wxh/Documents/oppoEC/OOPPO/log/data_node.log";
};

#endif