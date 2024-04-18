/*
#include <iostream>
#include<string>
#include<sstream>
#include<vector>
#include <stdexcept> // 包含std::invalid_argument异常
#include<cstdint>
#include<ctime>
#include<stdio.h>
#include<stdlib.h>
#include<map>
#include<unordered_map>
#include<set>
#include<mutex>
#include<thread>
#include <algorithm>
#include "meta_definition.h"
#include <client.h>
#include "toolbox.h"
using namespace std;
*/

#include "meta_definition.h"
#include "azure_lrc.h"
#include "toolbox.h"
#include "client.h"
#include <fstream>

bool partial_decoding;
OppoProject::EncodeType encode_type;
OppoProject::PlacementType placement_type;
int k, real_l, g_m, b;
int small_file_upper, blob_size_upper;
int block_size;
int value_length;
int stripe_num=0;
int min_tolerance;
int max_wait_time_ms;
OppoProject::UpdateAlgorithm up_func;


//输出：块先排序

// 定义FileTime的结构体
/*
struct FileTime {
    uint32_t dwLowDateTime;
    uint32_t dwHighDateTime;
};
*/
// 将FileTime转换为Linux下的时间结构tm
struct tm *  ConvertFileTimeToLinuxTime(const uint64_t &total100ns) {
    // 将FileTime的100纳秒单位转换为秒
    //uint64_t total100ns = static_cast<uint64_t>(fileTime.dwHighDateTime) << 32 | fileTime.dwLowDateTime;
    time_t totalSeconds = total100ns / 10000000ULL - 11644473600ULL; // 1601年1月1日到1970年1月1日的秒数差

    // 使用gmtime函数将总秒数转换为tm结构体
    time_t tttt= (time_t) totalSeconds;
    struct tm * linuxTime=gmtime(&tttt);
    //cout<<asctime(linuxTime)<<endl;
    //cout<<linuxTime->tm_year+1900<<' '<<linuxTime->tm_mon+1<<' '<<linuxTime->tm_mday<<' '<<linuxTime->tm_hour<<' '<<linuxTime->tm_min<<' '<<linuxTime->tm_sec<<endl;
    return linuxTime;
}






enum Type{Read,Write};

struct trace_item
{
    struct tm *timestamp;
    uint64_t rqtime;
    std::string hostname;
    int diskNumber;
    Type type;
    uint64_t offset;
    int size;
    uint64_t responseTime;//second

    void display()
    {
        std::cout<<asctime(timestamp)<<' '<<rqtime<<' '<<type<<' '<<offset<<' '<<size<<' '<<responseTime<<std::endl;
        return;
    }

};

struct stripe_conf
{
    int k;
    int m;
    int l;
    int blocksize;
};

stripe_conf sp_conf={10,4,0,128<<10};

std::mutex g_pages_mutex;

uint64_t string2uin64(std::string &str){
    uint64_t num=-1;
    try {
        num= std::stoull(str);
       // std::cout << "转换后的数值为: " << num << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "字符串转换失败: " << e.what() << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "数值超出范围: " << e.what() << std::endl;
    }
    return num;
}


void readCSVFileAndStore(const std::string& filename, std::vector<trace_item>& traces) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return;
    }

    //<<"Timestamp \t Hostname\t DiskNumber \t Type \tOffset \t Size \t Response \t Time"<<endl;
    //int count = 1000;
    std::string line;
    while (getline(file,line) ) {
        //cout<<"round"<<count<<std::endl;
        //count--;
        std::vector<std::string > row;
        std::stringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ',')) { // Assuming the CSV file uses a comma as the delimiter
            //cout<<cell<<'\t';
            row.push_back(cell);
        }
        
        trace_item item;
        
        //bug!
        uint64_t total100ns=string2uin64(row[0]);
        item.rqtime=total100ns;
        struct  tm *timestamp;
        
        
        timestamp= ConvertFileTimeToLinuxTime(total100ns);
        item.timestamp=timestamp;
        //<<asctime(timestamp)<<endl;
        item.rqtime=total100ns;
    
        item.type= row[3]=="Write" ? Write: Read;
        item.offset=string2uin64(row[4]);
        item.size=static_cast<int> (string2uin64(row[5]));
        total100ns=string2uin64(row[6]);
        item.responseTime=total100ns;
        //item.display();
        traces.push_back(item);
        //cout<<"round success"<<endl;
    }

    file.close();
}

void xhdebug(std::string s)
{
    std::cout<<s;
}


void split_update(
                std::unordered_map<uint64_t,std::vector<std::vector<int>>> &stripe_update_info,//stripeid,每次更新的块
                std:: vector<trace_item> &traces,stripe_conf sp_conf,int interval_sec=2)
{
    
    int stripe_data_size=sp_conf.blocksize*sp_conf.k;
    uint64_t starttime=traces[0].rqtime;
    uint64_t interval_100ns=interval_sec*10000000ULL;
    uint64_t step_time=starttime + interval_100ns;
    int interval_count=0;
    uint64_t read_num=0;
    uint64_t first_write_num=0;
    uint64_t update_num=0;

    std::map<uint64_t,int> up_hash;

    std::unordered_map<uint64_t,std::set<int>> temp_sp_update_info;//stripeid->blk
    
    for(int i=0;i<traces.size();i++)
    {   
        if(traces[i].type==Read)
        {
            read_num++;
            continue;
        }
        
        if(traces[i].type==Write  )
        {
            if(up_hash.find(traces[i].offset)==up_hash.end())
            {
                up_hash[traces[i].offset]=1;//write num
                first_write_num++;
                continue;
            }  
            
            else{//同一offset的表示更新
                if(up_hash[traces[i].offset]==1){
                    first_write_num--;
                    update_num++;
                    up_hash[traces[i].offset]++;
                }
                else
                {
                    update_num++;
                    up_hash[traces[i].offset]++;
                }
            }
        }


        /*每个steptime内，将更新块缓存下，下个时间片写入*/
        if(traces[i].rqtime>step_time){
        //if(i%5==0){
            for(auto it=temp_sp_update_info.begin();it!=temp_sp_update_info.end();it++){
                if(it->second.size()>0){
                    std::vector<int> output(it->second.size());
                    std::copy(it->second.begin(), it->second.end(), output.begin());
                    stripe_update_info[it->first].push_back(output);
                } 
            }
            temp_sp_update_info.clear();
        }

        while(traces[i].rqtime>step_time){
            step_time+=interval_100ns;
            interval_count++;
        }

        int stripe_id=traces[i].offset/stripe_data_size;
        if(stripe_id>stripe_num) stripe_num=stripe_id;
        int start_block=traces[i].offset / sp_conf.blocksize;
        //int tttt=start_block % sp_conf.k;
        int end_block=(traces[i].offset + traces[i].size) / sp_conf.blocksize;
        //int uuuu=end_block % sp_conf.k;
        //int head_gap=traces[i].offset-start_block * sp_conf.blocksize;
        //int tail_gap=(end_block +1) * sp_conf.blocksize - (traces[i].offset+traces[i].size);
        
      
        while (start_block <= end_block)
        {
            /* code */
            /*
            xhdebug("block: " + to_string(start_block)+ " ");
            xhdebug("stpid " + to_string(stripe_id)+ " ");
            xhdebug("spblk  " + to_string((stripe_id)*sp_conf.k)+ " ");
            xhdebug("delta  " + to_string(start_block - (stripe_id)*sp_conf.k)+ " \n");
            */
            /*
            block: 45310
            stp+1 3776
            spblk  45312
            delta  10
            3775
            10 */
            /*判断是否已经在更新这个块 lazy update*/
            /*
            if(start_block < (stripe_id+1)*sp_conf.k ){//在条带范围内
                
                if(std::find(temp.begin(),temp.end(),start_block) == temp.end()) temp.push_back(start_block % sp_conf.k);
            }
            else{
                
                stripe_update_info[stripe_id].push_back(temp);

                xhdebug(to_string( stripe_id));
                for(int i=0;i<temp.size();i++) xhdebug(to_string(temp[i])+" ");
                xhdebug("\n");
                temp.clear();
                stripe_id++;
                temp.push_back(start_block % sp_conf.k);
                
            }
            start_block++;
            */
           if(start_block < (stripe_id+1)*sp_conf.k ){//在条带范围内
                
                temp_sp_update_info[stripe_id].insert(start_block % sp_conf.k);
                //<<"blockID "<<stripe_id<<" "<<start_block % sp_conf.k<<endl;
                    
            }
            else{
                stripe_id++;
                temp_sp_update_info[stripe_id].insert(start_block % sp_conf.k);
                //cout<<"blockID "<<stripe_id<<" "<<start_block % sp_conf.k<<endl;
            }
            start_block++;
        }
        /*
        xhdebug("set: \n");
        for(auto const & it:temp_sp_update_info){
            for(auto const & itttttt:it.second) std::cout<<itttttt<<' ';
            cout<<endl;
        }
        */
        //cout<<endl;

    }
    /*
    for(auto it=stripe_update_info.begin();it!=stripe_update_info.end();it++)
    {
        cout<<"stripe id:"<<it->first<<endl;
        for(int i=0;i<it->second.size();i++){
            for(int j=0;j<it->second[i].size();j++){
                cout<<it->second[i][j]<<' ';
            }
            cout<<endl;
        }
    }
    */
   for(auto it=temp_sp_update_info.begin();it!=temp_sp_update_info.end();it++){
        if(it->second.size()>0){
            std::vector<int> output(it->second.size());
            std::copy(it->second.begin(), it->second.end(), output.begin());
            stripe_update_info[it->first].push_back(output);
        } 
    }
    temp_sp_update_info.clear();
    std::cout<<"readnum: "<<read_num<<std::endl;
    std::cout<<"first_write: "<<first_write_num<<std::endl;
    std::cout<<"update_num: "<<update_num<<std::endl;
    
    return ;

}


bool update_ana(std::vector<trace_item> &traces,stripe_conf sp_conf,std::string ofname,std::fstream &of,int interval_sec=2)
{
    std::unordered_map<uint64_t,std::vector<std::vector<int>>> stripe_update_info;//stripeid->[[block_index,...],...]

    split_update(stripe_update_info,traces,sp_conf,interval_sec);
    std::cout<<"K"<<sp_conf.k<<"B"<<sp_conf.blocksize/1024<<"KB\n";
    std::cout<<"updated sp num: "<<stripe_update_info.size()<<std::endl;
    int max_u_b_n=-2;
    int min_u_b_n=sp_conf.k;
    int total=0;
    int up_cnt=0;
    std::vector<int> up_distri=std::vector<int> (sp_conf.k+1, 0);

    


    for(auto const & it : stripe_update_info ){
        for(auto const & ttt : (it.second)){
            int size=ttt.size();
            up_distri[size]++;
            //cout<<"up blk num "<<ttt.size()<<endl;
            if(max_u_b_n < size){
                //cout<<max_u_b_n<<endl;
                max_u_b_n=ttt.size();
                //cout<<max_u_b_n<<endl;
            } 
            if(ttt.size()<min_u_b_n){
                //cout<<min_u_b_n<<endl;
                min_u_b_n=ttt.size();
                //cout<<min_u_b_n<<endl;
            } 
            total+=ttt.size();
        }
        up_cnt+=it.second.size();
    }

    int avg_num=total/up_cnt;
    if(total%up_cnt !=0 ) avg_num++;

    std::lock_guard<std::mutex> guard(g_pages_mutex);
    std::cout<<"open out file"<<std::endl;
    //of<<"k="<<sp_conf.k<<" m="<<sp_conf.m<<" blocksize="<<sp_conf.blocksize/1024<<"KB"<<" time interal="<<interval_sec<<"s"<<endl;
    double temp_rate=0.0;
    int temp_sum=0;

    std::vector<double> up_dis_rates=std::vector<double>(up_distri.size(),0.0);
    for(int i=1;i<=up_distri.size();i++ ){
        temp_sum+=up_distri[i];
    }
    of<<"K"<<sp_conf.k<<"B"<<sp_conf.blocksize/1024<<"KB\t";
    for(int i=1;i<=up_distri.size();i++ ){
        temp_rate=(double) up_distri[i]/temp_sum;
        of<<std::to_string(temp_rate)<<'\t';
    }
    of<<std::endl;

    //统计单块更新时候每个块的更新次数
    std::unordered_map<uint64_t,std::unordered_map<int,int>> blk_up_num;
    for(auto const & it : stripe_update_info ){
        for(auto const & ttt : (it.second)){
            if (ttt.size()!=1)
            {
                continue;
                
            }
            blk_up_num[it.first][ttt[0]]++;
            
        }
    }
    std::unordered_map<uint64_t,std::unordered_map<int,double>> blk_up_distr;//一个条带中每个块更新所占比例
    
    for(auto const & it : blk_up_num ){
        int total=0;
        for(auto const & ttt : (it.second)){
           total+=ttt.second;
        }
        for(auto const & ttt : (it.second)){
            blk_up_distr[it.first][ttt.first]=ttt.second/((double)total);
        }
    }
     /* code */
    std::vector<double> blk_up_ddddddd;
    for(auto const & it : blk_up_distr ){
        for (auto const & ttt : (it.second)){
            blk_up_ddddddd.push_back(ttt.second);
        }
    }
    std::sort(blk_up_ddddddd.begin(),blk_up_ddddddd.end());
    double max_b_ratio=blk_up_ddddddd[blk_up_ddddddd.size()-1];
    double min_blk_ratio=blk_up_ddddddd[0];
    double middle_b_ratio=blk_up_ddddddd[blk_up_ddddddd.size()/2];

    std::cout<<"max_b_ratio:"<<max_b_ratio<<std::endl;
    std::cout<<"min_blk_ratio:"<<min_blk_ratio<<std::endl;
    std::cout<<"middle_b_ratio:"<<middle_b_ratio<<std::endl;

    return true;
}




void parse_arg(int argc,char** argv)
{
  partial_decoding = (std::string(argv[1]) == "true");
  if (std::string(argv[2]) == "OPPO_LRC")
  {
    encode_type = OppoProject::OPPO_LRC;
  }
  else if (std::string(argv[2]) == "Azure_LRC_1")
  {
    encode_type = OppoProject::Azure_LRC_1;
  }
  else if (std::string(argv[2]) == "RS")
  {
    encode_type = OppoProject::RS;
  }
  else
  {
    std::cout << "error: unknown encode_type" << std::endl;
    exit(-1);
  }
  if (std::string(argv[3]) == "Flat")
  {
    placement_type = OppoProject::Flat;
  }
  else if (std::string(argv[3]) == "Random")
  {
    placement_type = OppoProject::Random;
  }
  else if (std::string(argv[3]) == "Best_Placement")
  {
    placement_type = OppoProject::Best_Placement;
  }
  else
  {
    std::cout << "error: unknown placement_type" << std::endl;
    exit(-1);
  }
  k = std::stoi(std::string(argv[4]));
  real_l = std::stoi(std::string(argv[5]));
  b = std::ceil((double)k / (double)real_l);
  g_m = std::stoi(std::string(argv[6]));
  small_file_upper = std::stoi(std::string(argv[7]));
  blob_size_upper = std::stoi(std::string(argv[8]));
  block_size = std::stoi(std::string(argv[10]));
  block_size=block_size<<10;
  value_length=block_size*k;
  std::cout<<"k g l blocksize valuelen"<<k<<' '<<real_l<<' '<<g_m<<' '<<block_size<<' '<<value_length<<' '<<std::endl;

  if(std::string(argv[11]) == "RCW") up_func=OppoProject::RCW;
  else if(std::string(argv[11]) == "RMW") up_func=OppoProject::RMW;
  else if(std::string(argv[11]) == "AZCoordinated") up_func=OppoProject::AZCoordinated;
  else if(std::string(argv[11]) == "PURMWRCW") up_func=OppoProject::PURMWRCW;
  else if(std::string(argv[11]) == "PARIX") up_func=OppoProject::PARIX;
  min_tolerance=std::stoi(std::string(argv[13]));
  max_wait_time_ms=std::stoi(std::string(argv[14]));
  std::cerr<<"parse arg finish\n";

  return ;
}

int main(int argc,char** argv)
{
    parse_arg(argc,argv);
    std::string path("/home/hadoop/wxh/Documents/ECPU/MSR-Cambridge/");
    std::string trace_fname(argv[12]);
    std::cout<<"filename:"<<trace_fname<<std::endl;
    //cin >> fname;

    std::vector<trace_item> traces;
    int count=30;
    std::cout<<path+trace_fname<<std::endl;
    readCSVFileAndStore(path+trace_fname,traces);

    std::cout<<"read traces success"<<std::endl;

    sp_conf.k=k;
    sp_conf.l=real_l;
    sp_conf.m=g_m;
    sp_conf.blocksize=block_size;

    std::unordered_map<uint64_t,std::vector<std::vector<int>>> stripe_update_info;//stripeid->[[block_index,...],...]

    split_update(stripe_update_info,traces,sp_conf,2);

    stripe_num+=2;

    

    //set
    std::string client_ip, coordinator_ip;
  
    client_ip = "0.0.0.0";
    
    coordinator_ip = client_ip;
    
    OppoProject::Client client(client_ip, 44444, coordinator_ip + std::string(":55555"));
    std::cout<<"debug1"<<std::endl;
    /**测试**/
    std::cout << client.sayHelloToCoordinatorByGrpc("MMMMMMMM") << std::endl;
    /**测试**/
    
    /**设置编码参数的函数，咱就是说有用没用都给传过去存下来，
     * 现在的想法就是每次需要修改这个参数，都要调用一次这个函数来改**/
  
    if (client.SetParameterByGrpc({partial_decoding, encode_type, placement_type, k, real_l, g_m, b, small_file_upper, blob_size_upper}))
    {
      std::cout << "set parameter successfully!" << std::endl;
    }
    else
    {
      std::cout << "Failed to set parameter!" << std::endl;
    }

    if (std::string(argv[9]) == "random")
    {
        for(int i=0;i<stripe_num;i++)
        {
            std::string key=std::to_string(i);
            std::string value;
            OppoProject::random_generate_value(value,value_length);
            client.set(key, value, "00");
        }
        for(int i=0;i<5;i++)
        {
            std::string get_value;
            client.get(std::to_string(i), get_value);
            std::cerr<<get_value<<std::endl;
        }

    }
    auto start = std::chrono::high_resolution_clock::now();
    //update
    for(auto & ttt:stripe_update_info)
    {
        auto stripeid=ttt.first;
        auto & uuu=ttt.second;
        for(auto & vvv:uuu)
        {
            std::sort(vvv.begin(),vvv.end());
            std::string neww_value;
            OppoProject::random_generate_value(neww_value,vvv.size()*block_size);

            if(!client.update(std::to_string(stripeid),vvv.at(0)*block_size,neww_value.length(),neww_value,up_func,min_tolerance,max_wait_time_ms)) std::cout<<"update fail when update stripe:"<<stripeid<<"\n";
        }

    }
    auto end = std::chrono::high_resolution_clock::now();

    // 计算程序执行所用的时间
    std::chrono::duration<double> duration = end - start;

    // 输出程序执行所用的时间 //却输出m' t怎么搞
    std::cout<<k<<" "<<g_m<<" "<<block_size<<" "<<trace_fname<<" "<<std::string(argv[11]);
    std::cout << " 更新时间：" << duration.count() << " 秒" << std::endl;

    return 0;

}
