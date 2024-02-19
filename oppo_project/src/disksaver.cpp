#include "disksaver.h"
#include <unordered_map>

#define xhdebug std::cout

DiskSaver::DiskSaver(/* args */)
{
    file_ptr=nullptr;

}
DiskSaver::DiskSaver(std::string fname)
{
    this->fname=fname;
    file_ptr=fopen(fname.c_str(),"w+b");
    if(file_ptr==NULL){
        std::cout<<"open or create disk file failed \n";
    }

}
DiskSaver::~DiskSaver()
{
    if(file_ptr!=NULL)
    {
        fclose(file_ptr);
        file_ptr=fopen(fname.c_str(),"w+b");
        if(file_ptr!=NULL) fclose(file_ptr);
    }
}

bool DiskSaver::data_set(const std::string key,int version,std::string &value)
{
    if(m_datablock_meta.find(key)==m_datablock_meta.end())
    {
        unsigned long long freeSpace=get_freeS_space();
        if(value.size()>freeSpace) return false;
        long long pos;
        if(!write_value_to_file(const_cast<char*>(value.c_str()),pos,(int)value.size(),0)) return false;
        datablock_meta tempmeta;
        tempmeta.address=pos;
        tempmeta.version=version;
        tempmeta.length=value.size();
        m_datablock_meta[key]=tempmeta;
        return true;
    }
    else
    {
        long long addr=m_datablock_meta[key].address;
        long long pos;//useless
        if(!write_value_to_file(const_cast<char*>(value.c_str()),pos,value.size(),addr)) return false;
        m_datablock_meta[key].version=version;
        return true;
    }
    return false;
}


std::string DiskSaver::data_get(const std::string key,int &version) const
{
    if(m_datablock_meta.find(key)==m_datablock_meta.end())
    {
        xhdebug<<"key not found\n";
        version=-1;
        std::string temp;
        return NULL;
    } 

    version=m_datablock_meta.at(key).version;
    
    int len=m_datablock_meta.at(key).length;
    
    long long addr=m_datablock_meta.at(key).address;
    
    
    
    std::vector<char> dddd(len);
    
    //read_value_from_file(const_cast<char*>(temp.c_str()), addr, len);
    read_value_from_file(dddd.data(), addr, len);

    return std::string(dddd.begin(),dddd.end());
    
}
bool DiskSaver::parity_set(const std::string key,std::vector<int> version,std::string &value)
{
    if(m_paritylock_meta.find(key)==m_paritylock_meta.end())
    {
        unsigned long long freeSpace=get_freeS_space();
        if(value.size()>freeSpace) return false;
        long long pos;
        if(!write_value_to_file(const_cast<char*> (value.c_str()),pos,value.size(),0)) return false;
        parity_block_meta tempmeta;
        tempmeta.address=pos;
        tempmeta.v_version=version;
        tempmeta.length=value.size();
        m_paritylock_meta[key]=tempmeta;
        return true;
    }
    else
    {
        long long addr=m_paritylock_meta[key].address;
        long long pos;//useless
        if(!write_value_to_file(const_cast<char*>(value.c_str()),pos,value.size(),addr)) return false;
        m_paritylock_meta[key].v_version=version;
        return true;

    }
    return false;
}
std::string DiskSaver::parity_get(const std::string key,std::vector<int> &version) const
{

    if(m_paritylock_meta.find(key)==m_paritylock_meta.end())
    {
        return NULL;
    } 
    parity_block_meta tempmeta=m_paritylock_meta.at(key);
    
    std::vector<int> tv=tempmeta.v_version;
    for(auto const & a:tv) version.push_back(a);
    
    int len=tempmeta.length;
    
    long long addr=tempmeta.address;
    
    std::vector<char> dddd(len);
    
    //read_value_from_file(const_cast<char*>(temp.c_str()), addr, len);
    read_value_from_file(dddd.data(), addr, len);
    return std::string(dddd.begin(),dddd.end());

}


unsigned long long DiskSaver::get_freeS_space() const
{
    struct statvfs stat;

    if (statvfs(fname.c_str(), &stat) == 0) {
        unsigned long long freeSpace = stat.f_frsize * stat.f_bavail;
        printf("剩余空间: %llu 字节\n", freeSpace);
        return freeSpace;
    } else {
        fprintf(stderr, "获取剩余空间失败\n");
        return -1;
    }
}


unsigned long long DiskSaver::get_freeS_space() const
{
    return 1<<30;
}



bool DiskSaver::write_value_to_file( char * data,long long &retaddr,int length,long long offset_in_file=0)
{
    if(file_ptr==nullptr) return false;
    if(offset_in_file==0)//append
    {
        fseek(file_ptr,0,SEEK_END);
        retaddr=ftell(file_ptr);
        fwrite(data,length,1,file_ptr);
        return true;
    }
    else//update
    {
        fseek(file_ptr,offset_in_file,SEEK_SET);
        fwrite(data,length,1,file_ptr);
        return true;
    }
    return false;
}

bool DiskSaver::read_value_from_file( char* data, long long addr,int length) const
{
    if(file_ptr==nullptr) return false;
    fseek(file_ptr,addr,SEEK_SET);
    fread(data,length,1,file_ptr);
    return true;
}


