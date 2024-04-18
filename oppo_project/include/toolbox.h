#ifndef TOOLBOX_H
#define TOOLBOX_H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <asio.hpp>
#define MAX_KEY_LENGTH 200
#define MAX_VALUE_LENGTH 20000
namespace OppoProject
{

    extern bool random_generate_kv(std::string &key, std::string &value,
                                   int key_length = 0, int value_length = 0);
    extern bool random_generate_value(std::string &value, int value_length = 0);
    std::vector<unsigned char> int_to_bytes(int);
    int bytes_to_int(std::vector<unsigned char> &bytes);
    std::string gen_key(int key_len, std::unordered_set<std::string> keys);
    int receive_int(asio::ip::tcp::socket &socket,asio::error_code &error);
    bool send_int(asio::ip::tcp::socket &socket,int data);
    bool send_vec_int(asio::ip::tcp::socket &socket,std::vector<int> &vec);
    bool receive_vec_int(asio::ip::tcp::socket &socket,std::vector<int> &vec,asio::error_code &error);
    bool write_value_to_file(char * data,long long &retaddr,int length,long long offset_in_file,FILE* file_ptr);//default 0,set ,else update
    bool read_value_from_file(char* data, long long addr,int length,FILE* file_ptr) ;

} // namespace OppoProject
#endif