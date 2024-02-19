#include<iostream>
#include<string>
#include<vector>
#include"disksaver.h"
using namespace std;
//test disksaver
void random_generate_KV(string &k,string &v,int i);
int main(){
    string fname("D:\\Users\\ZY\\Documents\\ECPU\\code\\file.txt");
    DiskSaver saver(fname);
    
    for(int i=1;i<60;i++)
    {

        string key,value;
        random_generate_KV(key,value,i);
        saver.data_set(key,i,value);
    }

    for(int i=1;i<60;i++)
    {
        cout<<"start get data\n";
        string key,value;
        random_generate_KV(key,value,i);
        int version;
        value = saver.data_get(key,version);
        cout<<key<<" "<<value<<" "<<version<<endl;
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.data_set(key,i+60,value);
    }
    for(int i=1;i<60;i++)
    {
        string key,value;
        int version;
        random_generate_KV(key,value,i);
        value = saver.data_get(key,version);
        cout<<key<<" "<<value<<" "<<version<<endl;
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.parity_set(key,vector<int> (4,1),value);
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        vector<int> version;
        random_generate_KV(key,value,i);
        value = saver.parity_get(key,version);
        cout<<key<<" "<<value<<" "<<version[0]<<" "<<version[1]<<" "<<version[2]<<" "<<version[3]<<endl;
    }

    for(int i=1;i<60;i++)
    {
        string key,value;
        random_generate_KV(key,value,i);
        saver.parity_set(key,vector<int>(4,2),value);
    }
    for(int i=1;i<60;i++)
    {
        string key,value;
        vector<int> version;
        random_generate_KV(key,value,i);
        value = saver.parity_get(key,version);
        cout<<key<<" "<<value<<" "<<version[0]<<" "<<version[1]<<" "<<version[2]<< " "<<version[3]<<endl;
    }
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