#include "toolbox.h"
#include <ctime>
#include <cstring>
#include <random>
#include <unordered_set>

bool OppoProject::random_generate_kv(std::string &key, std::string &value,
                                     int key_length, int value_length)
{
  /*如果长度为0，则随机生成长度,key的长度不大于MAX_KEY_LENGTH，value的长度不大于MAX_VALUE_LENGTH*/
  /*现在生成的key,value内容是固定的，可以改成随机的(增加参数)*/
  /*如果需要生成的key太多，避免重复生成，可以改成写文件保存下来keyvalue，下次直接读文件的形式，
  但这个需要修改函数参数或者修改run_client的内容了*/

  struct timespec tp;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp);
  srand(tp.tv_nsec);
  if (key_length == 0)
  {
  }
  else
  {
    for (int i = 0; i < key_length; i++)
    {
      key = key + char('a' + rand() % 26);
    }
  }
  if (value_length == 0)
  {
  }
  else
  {
    for (int i = 0; i < value_length / 26; i++)
    {
      for (int j = 65; j <= 90; j++)
      {
        value = value + char(j);
      }
    }
    int temp_size=value.size();//确保得到value_length
    for (int i = 0; i < value_length - int(temp_size); i++)
    {
      value = value + char('A' + i);
    }
  }
  return true;
}

std::vector<unsigned char> OppoProject::int_to_bytes(int integer)
{
  std::vector<unsigned char> bytes(sizeof(int));
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < int(bytes.size()); i++)
  {
    memcpy(&bytes[i], p + i, 1);
  }
  return bytes;
}

int OppoProject::bytes_to_int(std::vector<unsigned char> &bytes)
{
  int integer;
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < int(bytes.size()); i++)
  {
    memcpy(p + i, &bytes[i], 1);
  }
  return integer;
}

bool OppoProject::random_generate_value(std::string &value, int value_length)
{
  /*生成一个固定大小的随机value*/
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, 25);
  for (int i = 0; i < value_length; i++)
  {
    value = value + (dis(gen) % 2 ? char('a' + dis(gen) % 26) : char('A' + dis(gen) % 26));
  }
  return true;
}
std::string OppoProject::gen_key(int key_len, std::unordered_set<std::string> keys)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> dis(0, 25);
  std::string key;
  do
  {
    key.clear();
    for (int i = 0; i < key_len; i++)
    {
      key = key + (dis(gen) % 2 ? char('a' + dis(gen) % 26) : char('A' + dis(gen) % 26));
    }
  } while (keys.count(key) > 0);
  return key;
}

int OppoProject::receive_int(asio::ip::tcp::socket &socket,asio::error_code &error){
   
   std::vector<unsigned char> int_buf(sizeof(int));
   asio::read(socket, asio::buffer(int_buf, int_buf.size()),error);
   //std::cout<<"receive int error:"<<error.message()<<std::endl;
   int result = OppoProject::bytes_to_int(int_buf);
   return result;
}

bool OppoProject::send_int(asio::ip::tcp::socket &socket,int data){
  std::vector<unsigned char> int_buf = OppoProject::int_to_bytes(data);
  asio::error_code error;
  int write_byte_num=asio::write(socket, asio::buffer(int_buf, int_buf.size()),error);
  return true;
}

bool OppoProject::send_vec_int(asio::ip::tcp::socket &socket,std::vector<int> &vec){
  int len=vec.size();
  send_int(socket,len);
  for(auto const &a:vec) send_int(socket,a);
  return true;
}
bool OppoProject::receive_vec_int(asio::ip::tcp::socket &socket,std::vector<int> &vec,asio::error_code &error){
  int len=receive_int(socket,error);
  for(int i=0;i<len;i++) vec.push_back(receive_int(socket,error));
  return true;
}

bool OppoProject::write_value_to_file(char * data,long long &retaddr,int length,long long offset_in_file,FILE* file_ptr)//default 0,set ,else update
{
  std::cout<<"write value:"<<std::string(data,length);
  if(file_ptr==nullptr)


  
  {
    std::cout<<"file not open write failed \n ";
    return false;
  } 
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
bool OppoProject::read_value_from_file(char* data, long long addr,int length,FILE* file_ptr)
{
  if(file_ptr==nullptr) return false;
  fseek(file_ptr,addr,SEEK_SET);
  fread(data,length,1,file_ptr);
  return true;
}

// namespace OppoProject