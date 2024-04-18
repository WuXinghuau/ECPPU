#include "blockfilesaver.h"


bool BlockFileSaver::data_set(const std::string key,std::shared_ptr<std::vector<unsigned char>> value_ptr,int version)
{
    m_data_version[key]=version;
    return set(key,value_ptr);
}
bool BlockFileSaver::parity_set(const std::string key,std::shared_ptr<std::vector<unsigned char>> value_ptr,std::vector<int> &v_version)
{
    m_parity_version[key]=v_version;
    return set(key,value_ptr);
}

bool BlockFileSaver::set(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr)
{
    if(!block_in_file(shard_id)) return append_to_log(shard_id,value_ptr);
    else return overwrite(shard_id,value_ptr); 
}

bool BlockFileSaver::append_to_log(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr)
{
    log_file_ptr=fopen(m_log_file_path.c_str(),"a+b");
    if(log_file_ptr==NULL)
        std::cout<<"append to open error"<<'\n';
    fseek(log_file_ptr,0,SEEK_END);
    long long pos=ftell(log_file_ptr);
    std::cout<<shard_id<<" block 写入位置："<<pos<<"len: "<<value_ptr->size()<<std::endl;
    block_addr_len tempmeta;
    tempmeta.address=pos;
    tempmeta.length=value_ptr->size();
    m_block_meta[shard_id]=tempmeta;
    fwrite(value_ptr->data(),sizeof(char),value_ptr->size(),log_file_ptr);
    fclose(log_file_ptr);
    return true;
}

bool BlockFileSaver::overwrite(std::string shard_id,std::shared_ptr<std::vector<unsigned char>> value_ptr)
{
    log_file_ptr=fopen(m_log_file_path.c_str(),"r+b");
    if(log_file_ptr==NULL)
        std::cout<<"overwrite to open error"<<'\n';
    auto addr=m_block_meta.at(shard_id).address;
    auto len=m_block_meta.at(shard_id).length;
    if(addr<0 || len<=0) 
    {
        std::cout<<"overwrite addr len wrong addr: "<<addr<<" len: "<<len<<'\n';
        if(log_file_ptr!=NULL ) fclose(log_file_ptr);
        return false;
    }
    fseek(log_file_ptr,addr,SEEK_SET);
    std::cout<<shard_id<<" overwrite  addr: "<<addr<<" len: "<<len<<'\n';
    if(len!=value_ptr->size())
    {
        std::cout<<"overwite different len"<<value_ptr->size()<<std::endl;
    }
    fwrite(value_ptr->data(),sizeof(char),value_ptr->size(),log_file_ptr);
    fclose(log_file_ptr);
    return true;
}

std::shared_ptr<std::vector<unsigned char>> BlockFileSaver::get(const std::string key)
{
    if(!block_in_file(key))
    {
        std::cout<<"no block: "<<key<<std::endl;
        return nullptr;
    }
    log_file_ptr=log_file_ptr=fopen(m_log_file_path.c_str(),"rb");
    if(log_file_ptr==NULL)
    {
        std::cout<<"open log file error\n";
        return nullptr;
    }
    std::cout<<"open log file success\n";
    auto addr=m_block_meta.at(key).address;
    auto len=m_block_meta.at(key).length;
    std::cout<<key<<" overwrite  addr: "<<addr<<" len: "<<len<<'\n';
    fseek(log_file_ptr,addr,SEEK_SET);
    auto value_ptr=std::make_shared<std::vector<unsigned char>>(len); 
    fread(value_ptr.get()->data(),sizeof(char),len,log_file_ptr);
    fclose(log_file_ptr);
    return value_ptr;
}

bool BlockFileSaver::block_in_file(std::string shard_id)
{
    auto ttt=m_block_meta.find(shard_id);
    if(ttt==m_block_meta.end()) return false;
    else return true;
}


bool BlockFileSaver::append_to_log(OppoProject::LogEntry &log_entry)
{
    log_file_ptr=fopen(m_log_file_path.c_str(),"a+b");
    if(log_file_ptr==NULL)
        std::cout<<"append to open error"<<'\n';
    fseek(log_file_ptr,0,SEEK_END);
    long long pos=ftell(log_file_ptr);
    std::cout<<log_entry.shard_id<<"  写入位置："<<pos<<std::endl;
    m_info_table[log_entry.shard_id].push_back(pos);
    
    for(int i=0;i<m_info_table[log_entry.shard_id].size();i++)
    {
        std::cout<<"pos: "<<m_info_table[log_entry.shard_id][i]<<std::endl;;
    }
    int shard_id_len=(log_entry.shard_id).length();

    fwrite(&shard_id_len,sizeof(shard_id_len),1,log_file_ptr);
    
    fwrite((log_entry.shard_id).c_str(),sizeof(char),shard_id_len,log_file_ptr);

    int offset=log_entry.offset_in_shard;
    int len=log_entry.length;
    fwrite(&offset,sizeof(int),1,log_file_ptr);
    fwrite(&len,sizeof(int),1,log_file_ptr);

    int delta_type=(int) log_entry.delta_type;
    fwrite(&delta_type,sizeof(int),1,log_file_ptr);

    fwrite((log_entry.delta_value_ptr).get()->data(),sizeof(char),len,log_file_ptr);
    fclose(log_file_ptr);
    /*
    std::cout<<"add entry success ,enty is: "<<std::endl;
    std::cout<<log_entry.shard_id<<" "<<offset<<" "<<len<<" "<<log_entry.delta_value_ptr.get()->data()<<std::endl;
    std::cout<<"add entry finished \n"<<std::endl;
    */

    return true;
}


bool BlockFileSaver::init(std::string log_file_path)
{
    this->m_log_file_path=log_file_path;
    log_file_ptr=fopen(m_log_file_path.c_str(),"wb");
    if(log_file_ptr==NULL) std::cout<<"file saver create file failed"<<std::endl;
    fclose(log_file_ptr);
    return true;
}

bool BlockFileSaver::set_parity_latest_verson(std::string shard_id,std::vector<int> & v_version)
{
    m_latest_v_version[shard_id]=v_version;
    return true;
}

bool BlockFileSaver::erase_parity_latest_verson(std::string shard_id)
{
    if(m_latest_v_version.find(shard_id)!=m_latest_v_version.end()) m_latest_v_version.erase(shard_id);
    return true;
}
