#ifndef VERSIONEDLOGMANAGER_H
#define VERSIONEDLOGMANAGER_H
#include <map>
#include <mutex>
#include <vector>
#include "diskedblcockmeta.h"
#include "meta_definition.h"
#include "fileaccesser.h"
#include <memory>
#include <iostream>

class VersionedLogManager
{
private:
    /* data */
    std::mutex m_mutex;
    std::string m_log_file_path;
    std::map<std::string,std::map<int,std::map<int,block_addr_len>>> 
        m_delta_meta;//shardid,dataidx,dataverion,[addr,len]
    
    std::map<std::string,std::map<int,std::map<int,int>>> 
        m_delta_flag;//标删。
    
    FileAccesser m_file_accesser;
public:
    VersionedLogManager(/* args */)
    {

    };
    ~VersionedLogManager()
    {

    };

    bool init(std::string fname);
    bool append_parity_delta(std::string shardid,int dataidx,int dataversion,std::shared_ptr<std::vector<unsigned char>> value_ptr);
    bool up_to_date(std::string shardid,const std::vector<int> &oldversion,const std::vector<int> &latest_version);
    std::shared_ptr<std::vector<unsigned char>>  
        get_one_delta(std::string shardid,int dataidx,int dataversion);
    
    std::map<int,std::vector<int>> //idx ,缺的[version]
        needed_data_versions(std::string shardid,const std::vector<int> &oldversion,const std::vector<int> &latest_version);
    bool mark_delete_delta(std::string shard_id);
    
};




#endif