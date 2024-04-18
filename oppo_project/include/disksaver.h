#include "diskedblcockmeta.h"
#include "toolbox.h"
#include <map>
#include<unordered_map>
#include<iostream>
#include<fstream>
#include <sys/statvfs.h>
class DiskSaver
{
private:

    
    /*版本号与地址分离的元数据*/
    std::map<std::string,block_addr_len> m_block_meta;
    std::map<std::string,int> m_data_veraion;
    std::map<std::string,std::vector<int>> m_parity_veraion;
    std::map<std::string,std::vector<int> > m_latest_v_version;

    

    std::string fname;
    FILE* file_ptr;
    unsigned long long rest_space;

    unsigned long long get_freeS_space() const;

public:
    DiskSaver(/* args */);
    DiskSaver(std::string fname,long long init_free);
    ~DiskSaver();

    bool init(std::string fname,unsigned long long init_free);
    std::shared_ptr<std::vector<unsigned char>> data_get(const std::string key,int& version) ;
    int get_value_len(std::string key) const;
    std::shared_ptr<std::vector<unsigned char>> parity_get(const std::string key,std::vector<int> &version) ;
    bool data_set(const std::string key,char* value,int value_size,int version);
    bool parity_set(const std::string key,char* value,int value_size,std::vector<int> version);
    bool unified_append_value(const std::string key,char* value,int value_size);
    std::shared_ptr<std::vector<unsigned char>> unified_get_value(const std::string key);
    bool write_value(char * data,long long &retaddr,int length,long long offset_in_file);//default 0,set ,else update
    bool read_value(void* data, long long addr,int length);
    bool print_data_version(std::string key) const;
    bool print_parity_version(std::string key) const;
    bool print_all_data_version() const;
    bool print_all_parity_version() const;
};

