#include<iostream>
#include<string>
#include<vector>
#include"disksaver.h"
#include"meta_definition.h"
using namespace std;
//test disksaver
void random_generate_KV(string &k,string &v,int i);
template<typename T>
void printVector(const std::vector<T>& vec);
void printVector(const std::shared_ptr<std::vector<unsigned char>> arrPtr);

int main(){
    string fname("/home/wxh/Documents/oppoEC/OOPPO/oppo_project/cmake/build/out.log");
    DiskSaver saver;
    saver.init(fname,1<<30LL);
    
    for(int i=1;i<60;i++)
    {

        string key,value;
        random_generate_KV(key,value,i);
        saver.data_set(key,const_cast<char*> (value.c_str()),value.size(),0);
    }

    for(int i=1;i<60;i++)
    {
        cout<<"start get data\n";
        string key,value;
        random_generate_KV(key,value,i);
        int version;
        auto storedv = saver.data_get(key,version);
        cout<<key<<" "<<key<<" "<<version<<endl;
        printVector(storedv);
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.data_set(key,const_cast<char*> (value.c_str()),value.size(),i+60);
    }
    for(int i=1;i<60;i++)
    {
        string key,value;
        int version;
        random_generate_KV(key,value,i);
        auto storedv = saver.data_get(key,version);
        cout<<key<<" "<<key<<" "<<version<<endl;
        printVector(storedv);
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.parity_set(key,const_cast<char*> (value.c_str()),value.size(),vector<int>(4,1));
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        vector<int> version;
        random_generate_KV(key,value,i);
        saver.parity_set(key,const_cast<char*> (value.c_str()),value.size(),vector<int>(4,2));
        cout<<key<<" "<<key<<" "<<endl;
        auto storedv = saver.parity_get(key,version);
        printVector(storedv);
        printVector(version);
    }
    /*
    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.parity_set(key,const_cast<char*> (value.c_str()),value.size(),vector<int>(4,2));
    }
    for(int i=1;i<60;i++)
    {
        string key,value;
        vector<int> version;
        random_generate_KV(key,value,i);
        value = saver.parity_get(key,version);
        cout<<key<<" "<<value<<" "<<version[0]<<" "<<version[1]<<" "<<version[2]<< " "<<version[3]<<endl;
    }
    */
    return 0;


}


void random_generate_KV(string &k,string &v,int i)
{   
    cout<<"generate"<<endl;
    for(int j=0;j<i;j++)
    {
        k+=string("key");
        v+=string("value");
    }
    k+=to_string(i);
    v+=to_string(i);
    cout<<"key:"<<k<<endl; 
    return;
}


template<typename T>
void printVector(const std::vector<T> & vec)
{
    std::cout << "Vector data: \n";
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

void printVector(const std::shared_ptr<std::vector<unsigned char>> arrPtr)
{
    std::cout << "Vector data: \n";
    for(int i=0;i<arrPtr->size();i++) std::cout<<(*arrPtr)[i];
    std::cout<<std::endl;
}