#ifndef FILEACCESSER_H
#define FILEACCESSER_H

#include <memory>
#include <vector>
#include <iostream>

class FileAccesser
{
private:
    std::string m_log_file_path;
    FILE* log_file_ptr;
public:
    FileAccesser()
    {

    };
    ~ FileAccesser()
    {
        if(log_file_ptr!=NULL)
        {
            fclose(log_file_ptr);
        }
    }
    bool init(std::string log_file_path);
    long long append(std::shared_ptr<std::vector<unsigned char>> value_ptr);
    bool overwrite(std::shared_ptr<std::vector<unsigned char>> value_ptr,long long addr,int len);
    std::shared_ptr<std::vector<unsigned char>> 
        read(long long addr,int len);
};

#endif