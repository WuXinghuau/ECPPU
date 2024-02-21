#include "diskedblcockmeta.h"
#include <map>
#include<unordered_map>
#include<iostream>
#include<fstream>
#include <sys/statvfs.h>
class DiskSaver
{
private:
    /* data */
    std::unordered_map<std::string,datablock_meta> m_datablock_meta;//shardid->offset_in_file
    std::unordered_map<std::string,parity_block_meta> m_paritylock_meta;
    std::string fname;
    FILE* file_ptr;
    long long rest_space;

    unsigned long long get_freeS_space() const;

public:
    DiskSaver(/* args */);
    DiskSaver(std::string fname,long long init_free);
    bool init(std::string fname,long long init_free);
    bool data_set(const std::string key,char* value,int value_size,int version);
    bool parity_set(const std::string key,char* value,int value_size,std::vector<int> version);
    std::string data_get(const std::string key,int& version) const;
    std::string parity_get(const std::string key,std::vector<int> &version) const;
    bool write_value_to_file(char * data,long long &retaddr,int length,long long offset_in_file);//default 0,set ,else update
    bool read_value_from_file(char* data, long long addr,int length) const;
    ~DiskSaver();
};

