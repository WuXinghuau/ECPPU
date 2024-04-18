#include "fileaccesser.h"


bool FileAccesser::init(std::string log_file_path)
{
    this->m_log_file_path=log_file_path;
    log_file_ptr=fopen(m_log_file_path.c_str(),"wb");
    if(log_file_ptr==NULL)
    {
        std::cout<<"file accesser create file failed"<<std::endl;
        return false;
    } 
    fclose(log_file_ptr);
    return true;
}
long long FileAccesser::append(std::shared_ptr<std::vector<unsigned char>> value_ptr)
{
    log_file_ptr=fopen(m_log_file_path.c_str(),"a+b");
    if(log_file_ptr==NULL)
        std::cout<<"append to open error"<<'\n';
    fseek(log_file_ptr,0,SEEK_END);
    long long pos=ftell(log_file_ptr);
    std::cout<<m_log_file_path<<" block 写入位置："<<pos<<"len: "<<value_ptr->size()<<std::endl;
    fwrite(value_ptr->data(),sizeof(char),value_ptr->size(),log_file_ptr);
    fclose(log_file_ptr);
    return pos;
}
bool FileAccesser::overwrite(std::shared_ptr<std::vector<unsigned char>> value_ptr,long long addr,int len)
{
    log_file_ptr=fopen(m_log_file_path.c_str(),"r+b");
    if(log_file_ptr==NULL)
        std::cout<<"overwrite to open error"<<'\n';
    if(addr<0 || len<=0) 
    {
        std::cout<<"overwrite addr len wrong addr: "<<addr<<" len: "<<len<<'\n';
        if(log_file_ptr!=NULL ) fclose(log_file_ptr);
        return false;
    }
    fseek(log_file_ptr,addr,SEEK_SET);
    std::cout<<" overwrite  addr: "<<addr<<" len: "<<len<<'\n';
    if(len!=value_ptr->size())
    {
        std::cout<<"overwite different len"<<value_ptr->size()<<std::endl;
    }
    fwrite(value_ptr->data(),sizeof(char),value_ptr->size(),log_file_ptr);
    fclose(log_file_ptr);
    return true;
}
std::shared_ptr<std::vector<unsigned char>> 
    FileAccesser::read(long long addr,int len)
{
    log_file_ptr=log_file_ptr=fopen(m_log_file_path.c_str(),"rb");
    if(log_file_ptr==NULL)
    {
        std::cout<<"open log file error\n";
        return nullptr;
    }
    std::cout<<"open log file success\n";
    std::cout<<" read  addr: "<<addr<<" len: "<<len<<'\n';
    fseek(log_file_ptr,addr,SEEK_SET);
    auto value_ptr=std::make_shared<std::vector<unsigned char>>(len); 
    fread(value_ptr.get()->data(),sizeof(char),len,log_file_ptr);
    fclose(log_file_ptr);
    return value_ptr;
}