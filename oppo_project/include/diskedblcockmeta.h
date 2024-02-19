#include <vector>
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
