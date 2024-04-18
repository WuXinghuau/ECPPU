#ifndef BLOCKFILESAVER_H
#define BLOCKFILEFAVER_H

#include <mutex>
#include <unordered_map>
#include <vector>
#include "meta_definition.h"
#include "azure_lrc.h"
#include <map>
#include "diskedblcockmeta.h"




class BlockFileSaver
{
private:
    /* data */
    std::mutex m_mutex;
    std::string m_log_file_path;
    std::unordered_map<std::string,std::vector<long long> > m_info_table;//shardid->offset_in_file

    std::map<std::string,block_addr_len> m_block_meta;
    std::map<std::string,int> m_data_version;
    std::map<std::string,std::vector<int>> m_parity_version;
    std::map<std::string,std::vector<int> > m_latest_v_version;

    FILE* log_file_ptr;

public:
    BlockFileSaver()
    {
        
    };
    ~BlockFileSaver()
    {
        if(log_file_ptr!=NULL)
        {
            fclose(log_file_ptr);
        }
        
    };
    
    bool data_set(const std::string key,std::shared_ptr<std::vector<unsigned char>> value_ptr,int version);
    bool parity_set(const std::string key,std::shared_ptr<std::vector<unsigned char>> value_ptr,std::vector<int> &v_version);
    bool set(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr);
    std::shared_ptr<std::vector<unsigned char>> get(const std::string key);
    bool append_to_log(OppoProject::LogEntry &log_entry);
    bool append_to_log(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr);
    bool overwrite(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr);
    bool block_in_file(std::string shard_id);
    bool init(std::string log_file_path);
    bool set_parity_latest_verson(std::string shard_id,std::vector<int> & v_version);
    bool erase_parity_latest_verson(std::string shard_id);
};



#endif
