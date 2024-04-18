#include "disksaver.h"
#include <unordered_map>

#define xhdebug std::cout

DiskSaver::DiskSaver(/* args */)
{
    file_ptr=nullptr;

}
DiskSaver::DiskSaver(std::string fname,long long init_free)
{
    
    this->fname=fname;
    this->rest_space=init_free;
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
        //file_ptr=fopen(fname.c_str(),"w+b");
        //if(file_ptr!=NULL) fclose(file_ptr);
    }
}
bool DiskSaver::init(std::string fname,unsigned long long init_free)
{
    this->fname=fname;
    this->rest_space=init_free;
    file_ptr=fopen(fname.c_str(),"w+b");
    if(file_ptr==NULL){
        std::cout<<"open or create disk file failed \n";
        return false;
    }
    return true;

}
int DiskSaver::get_value_len(std::string key) const
{
    
    return m_block_meta.at(key).length;
}
/*
unsigned long long DiskSaver::get_freeS_space() const
{
    struct statvfs stat;

    if (statvfs(fname.c_str(), &stat) == 0) {
        unsigned long long freeSpace = stat.f_frsize * stat.f_bavail;
        printf("剩余空间: %llu 字节\n", freeSpace);
        printf("rest 剩余空间: %llu 字节\n", rest_space);
        //return rest_space<freeSpace ? rest_space:freeSpace;
        return freeSpace;
    } else {
        fprintf(stderr, "获取剩余空间失败\n");
        return -1;
    }
}
*/

unsigned long long DiskSaver::get_freeS_space() const
{
    return rest_space;
}




bool DiskSaver::write_value( char * data,long long &retaddr,int length,long long offset_in_file=0)
{
    if(file_ptr==NULL) return false;
    else if(offset_in_file==0)//append
    {   
        std::cout<<"start to write \n";
        fseek(file_ptr,0,SEEK_END);
        retaddr=ftell(file_ptr);
        fwrite(data,length,1,file_ptr);
        fflush(file_ptr);
        return true;
    }
    else if(offset_in_file>0)//update
    {
        fseek(file_ptr,offset_in_file,SEEK_SET);
        fwrite(data,length,1,file_ptr);
        fflush(file_ptr);
        return true;
    }
    return false;
}

bool DiskSaver::read_value(void* data, long long addr,int length)
{
    if(file_ptr==nullptr)
    {
        return false;
    } 
    std::cout<<"read feek:"<<fseek(file_ptr,addr,SEEK_SET);
    std::cout<<"read fread:"<<fread(data,sizeof(char),length,file_ptr);
    return true;
}


bool DiskSaver::data_set(const std::string key,char* value,int value_size,int version)
{
    if(m_block_meta.find(key)==m_block_meta.end())
    {
        if(!unified_append_value(key,value,value_size)) return false;
        m_data_veraion[key]=version;
        xhdebug<<"data version: "<<m_data_veraion.at(key)<<'\n';
        return true;
    }
    else
    {
        std::cout<<"disk overwrite value  \n key:"<<key<<std::endl;
        long long addr=m_block_meta[key].address;
        long long pos;//useless
        m_data_veraion[key]=version;
        xhdebug<<"data version: "<<m_data_veraion.at(key)<<'\n';
        xhdebug<<"addr len "<<m_block_meta.at(key).address<<"  "<<m_block_meta.at(key).length<<std::endl;
        if(!write_value(value,pos,value_size,addr)) return false;
        return true;
    }
    return false;
}
bool DiskSaver::parity_set(const std::string key,char* value,int value_size,std::vector<int> version)
{
    if(m_block_meta.find(key)==m_block_meta.end())
    {
        if(!unified_append_value(key,value,value_size)) return false;
        m_parity_veraion[key]=version;
        xhdebug<<"parity version: "<<m_parity_veraion.at(key).size()<<' '<<m_parity_veraion.at(key)[0]<<'\n';
        return true;
    }
    else
    {
        long long addr=m_block_meta[key].address;
        long long pos;//useless
        xhdebug<<"addr len "<<m_block_meta.at(key).address<<"  "<<m_block_meta.at(key).length<<std::endl;
        if(!write_value(value,pos,value_size,addr)) return false;
        m_parity_veraion[key]=version;
        return true;
    }
    return false;
}

 std::shared_ptr<std::vector<unsigned char>> DiskSaver::data_get(const std::string key,int& version) 
{
    if(m_data_veraion.find(key)==m_data_veraion.end()) version = -1;
    else version=m_data_veraion.at(key);
    return unified_get_value(key);
}
 std::shared_ptr<std::vector<unsigned char>> DiskSaver::parity_get(const std::string key,std::vector<int> &version) 
{
    if(m_parity_veraion.find(key)==m_parity_veraion.end()) version.push_back(-1);
    else version=m_parity_veraion.at(key);
    return unified_get_value(key);
}

bool DiskSaver::unified_append_value(const std::string key,char* value,int value_size)
{
    std::cout<<"append value to disk\n";
    std::cout<<std::string(value,value_size)<<'\n';
    unsigned long long freeSpace=get_freeS_space();
    std::cout<<"vsize: "<<value_size<<" (int) free: "<<(int) freeSpace<<std::endl;
    if(freeSpace - (unsigned long long )value_size < 0)
    {
        std::cout<<"vsize: "<<value_size<<std::endl;
        std::cout<<"usignen "<<(unsigned long long )value_size<<std::endl;
        std::cout<<"space use out \n";
        return false;
    } 
    long long pos=0L;
    if(!write_value(value,pos,value_size,0)) return false;
    rest_space-=value_size;
    block_addr_len tempmeta;
    tempmeta.address=pos;
    tempmeta.length=value_size;
    m_block_meta[key]=tempmeta;
    xhdebug<<"addr len "<<m_block_meta.at(key).address<<"  "<<m_block_meta.at(key).length<<std::endl;
    return true;
}

std::shared_ptr<std::vector<unsigned char>> DiskSaver::unified_get_value(const std::string key)
{
    if(m_block_meta.find(key)==m_block_meta.end())
    {
        return nullptr;
    } 
    auto tempmeta=m_block_meta.at(key);
    int len=tempmeta.length;
    
    long long addr=tempmeta.address;
    std::shared_ptr<std::vector<unsigned char>> dddd=std::make_shared<std::vector<unsigned char>>(len);
    
    //read_value_from_file(const_cast<char*>(temp.c_str()), addr, len);
    if(!read_value( dddd.get()->data(), addr, len)) xhdebug<<"read failed file not open \n";
    std::cout<<"read value from disk:\n";
    std::cout<<"key: "<<key<<std::endl;
    xhdebug<<"addr len "<<addr<<"  "<<len<<std::endl;
    for(int i=0;i<len;i++) std::cout<<(*dddd)[i];
    std::cout<<std::endl;
    return dddd;
}


