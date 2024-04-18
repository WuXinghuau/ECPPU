#ifndef DISKEDBLOCKMETA_H
#define DISKEDBLOCKMETA_H
#include <vector>

struct block_addr_len
{
    /* data */
    long long address;
    int length;
    block_addr_len()=default;
    block_addr_len(long long addr,int len):address(addr),length(len){};
};

struct datablock_meta
{
    /* data */
    int version;
    long long address;
    int length;
};

struct parity_block_meta
{
    /* data */
    std::vector<int> v_version;
    long long address;
    int length;
};


struct parity_delta_meta
{
    /* data */
    int dataidx;
    int data_version;
    int parityidx;
    long long address;
    int length;
};
struct parity_delta_version
{
    /* data */
    int dataidx;
    int data_version;
    int parityidx;
};
#endif