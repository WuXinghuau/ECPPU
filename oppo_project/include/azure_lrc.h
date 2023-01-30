#ifndef AZURELRC_H
#define AZURELRC_H
#include "jerasure.h"
#include "reed_sol.h"
#include "meta_definition.h"


namespace OppoProject
{
    bool lrc_make_matrix(int k, int g, int l, int *final_matrix);
    bool encode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type);
    bool decode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, std::shared_ptr<std::vector<int>> erasures, int blocksize, EncodeType encode_type);
    bool check_received_block(int k,int expect_block_number,std::shared_ptr<std::vector<int>> shards_idx_ptr, int shards_ptr_size=-1);
}
#endif