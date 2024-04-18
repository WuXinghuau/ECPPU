#include "versionedLogManager.h"
#include <algorithm>
bool VersionedLogManager::init(std::string fname)
{
    return m_file_accesser.init(fname);
}

 
bool VersionedLogManager::append_parity_delta(std::string shardid,int dataidx,int dataversion,std::shared_ptr<std::vector<unsigned char>> value_ptr)
{
    long long addr=m_file_accesser.append(value_ptr);
    if(addr < 0 ) return false;
    block_addr_len temp_a_l(addr,value_ptr->size());
    m_delta_meta[shardid][dataidx][dataversion]=temp_a_l;
    return true;
    
}

bool VersionedLogManager::up_to_date(std::string shardid,const std::vector<int> &oldversion,const std::vector<int> &latest_version)
{
    if(oldversion==latest_version) return true;
    if(oldversion.size()!=latest_version.size()) return false;

    try
    {
        /* code */
        auto t_delta_meta=m_delta_meta.at(shardid);
        std::vector<bool> temp_res(oldversion.size());
        for(int i=0;i<oldversion.size();i++)
        {
            if(oldversion[i] != latest_version[i]) temp_res[i]=false;
            else temp_res[i]=true;
        }

        for(auto const &aaa : t_delta_meta)//aaa:  dataidx dataversion 
        {
            int idx=aaa.first;
            std::vector<int> missed_data_version;
            std::vector<int> saveed_data_version;
            for(int i=oldversion[idx]+1;i<=latest_version[i];i++) missed_data_version.push_back(i);

            for(auto const &ttt : aaa.second)
            {
                saveed_data_version.push_back(ttt.first);
            }
            std::sort(missed_data_version.begin(),missed_data_version.end());
            std::sort(saveed_data_version.begin(),saveed_data_version.end());
            if(missed_data_version == saveed_data_version ) temp_res[idx]=true;
            else temp_res[idx]=false;
        }
        auto tempiter=std::find(temp_res.begin(),temp_res.end(),false);
        if(tempiter == temp_res.end()) return true;
        else return false;

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    

    

    
}

std::shared_ptr<std::vector<unsigned char>>  
    VersionedLogManager::get_one_delta(std::string shardid,int dataidx,int dataversion) 
{
    auto addr_len=m_delta_meta.at(shardid).at(dataidx).at(dataversion);
    return m_file_accesser.read(addr_len.address,addr_len.length);
}

std::map<int,std::vector<int>> 
    VersionedLogManager::needed_data_versions(std::string shardid,const std::vector<int> &oldversion,const std::vector<int> &latest_version)
{
    std::map<int,std::vector<int>> final_result;
    if(oldversion==latest_version) return final_result;
    try
    {
        auto t_delta_meta=m_delta_meta.at(shardid);
        for(auto const &aaa : t_delta_meta)//aaa:  dataidx dataversion addrlen
        {
            int idx=aaa.first;
            std::vector<int> missed_data_version;
            std::vector<int> saveed_data_version;
            std::vector<int> temp_vv;
            for(int i=oldversion[idx]+1;i<=latest_version[i];i++) missed_data_version.push_back(i);

            for(auto const &ttt : aaa.second)
            {
                saveed_data_version.push_back(ttt.first);
            }
            std::sort(missed_data_version.begin(),missed_data_version.end());
            std::sort(saveed_data_version.begin(),saveed_data_version.end());
            std::set_difference(missed_data_version.begin(),missed_data_version.end(),saveed_data_version.begin(),saveed_data_version.end(),temp_vv.begin());
            std::sort(temp_vv.begin(),temp_vv.end());
            final_result[idx]=temp_vv;
        }
    }
    catch(const std::exception& e)//不存在
    {
        std::cerr << e.what() << '\n';
        for(int idx=0;idx<oldversion.size();idx++)
        {
            std::vector<int> temp_vv;
            for(int tv=oldversion[idx]+1;tv<=latest_version[idx];tv++)
            {
                temp_vv.push_back(tv);
            }
            if(temp_vv.size()>0) final_result[idx]= temp_vv;
        }
        return final_result;
    }
    
}
//std::map<std::string,std::map<int,std::map<int,block_addr_len>>> m_delta_meta
bool VersionedLogManager::mark_delete_delta(std::string shard_id)
{
    if(m_delta_meta.find(shard_id)!=m_delta_meta.end())
    {
        auto delta_metas=m_delta_meta[shard_id];
        for (auto const &aaa:delta_metas)
        {
            int dataidx=aaa.first;
            for(auto const & bbb:aaa.second)
            {
                int dataverion=bbb.first;
                m_delta_flag[shard_id][dataidx][dataverion]=OppoProject::MarkDelete;
            }
        }
    }
    return true;
}