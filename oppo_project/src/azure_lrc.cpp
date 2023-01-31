#include <azure_lrc.h>

void OppoProject::dfs(std::vector<int> temp, std::shared_ptr<std::vector<std::vector<int>>> ans, int cur, int n, int k)
{
    if (int(temp.size()) + (n - cur + 1) < k)
    {
        return;
    }
    if (temp.size() == k)
    {
        ans->push_back(temp);
        return;
    }
    temp.push_back(cur);
    dfs(temp, ans, cur + 1, n, k);
    temp.pop_back();
    dfs(temp, ans, cur + 1, n, k);
}

bool OppoProject::combine(std::shared_ptr<std::vector<std::vector<int>>> ans, int n, int k)
{
    std::vector<int> temp;
    dfs(temp, ans, 1, n, k);
    return true;
}

bool OppoProject::check_k_data(std::vector<int> erasures, int k)
{
    int flag = 1;
    for (int i = 0; i < k; i++)
    {
        if (std::find(erasures.begin(), erasures.end(), i) != erasures.end())
        {
            flag = 0;
        }
    }
    if (flag)
    {
        return true;
    }

    return false;
}
bool OppoProject::lrc_make_matrix(int k, int g, int l, int *final_matrix)
{
    int r = (k + l - 1) / l;
    int *matrix = NULL;

    matrix = reed_sol_vandermonde_coding_matrix(k, g + 1, 8);
    if (matrix == NULL)
    {
        std::cout << "matrix == NULL" << std::endl;
    }

    // final_matrix = (int *)malloc(sizeof(int) * k * (g + l));
    if (final_matrix == NULL)
    {
        std::cout << "final_matrix == NULL" << std::endl;
    }
    bzero(final_matrix, sizeof(int) * k * (g + l));

    for (int i = 0; i < g; i++)
    {
        for (int j = 0; j < k; j++)
        {
            final_matrix[i * k + j] = matrix[(i + 1) * k + j];
        }
    }

    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (i * r <= j && j < (i + 1) * r)
            {
                final_matrix[(i + g) * k + j] = 1;
            }
        }
    }

    free(matrix);
    return true;
}

bool OppoProject::encode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, int blocksize, EncodeType encode_type)
{
    int *matrix;
    matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
    if (encode_type == RS)
    {
        jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
    }
    else if (encode_type == Azure_LRC_1)
    {
        std::vector<int> new_matrix((m + l) * k, 0);
        lrc_make_matrix(k, m, l, new_matrix.data());
        jerasure_matrix_encode(k, m + l, 8, new_matrix.data(), data_ptrs, coding_ptrs, blocksize);

        // 生成全局校验块的局部校验块
        std::vector<int> last_matrix(m, 1);
        jerasure_matrix_encode(m, 1, 8, last_matrix.data(), coding_ptrs, &coding_ptrs[m + l], blocksize);
    }
    else if (encode_type == OPPO_LRC)
    {
        int r = (k + l - 1) / l;
        jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, blocksize);
        std::vector<int> az_g_number(l, 0);
        std::vector<int> az_data_number(l, 0);
        for (int i = 0; i < m; i++)
        {
            az_g_number[i % l] += 1;
        }
        for (int i = 0; i < l; i++)
        {
            az_data_number[i] = std::min(k - i * r, r);
        }
        int g_sum = 0;
        for (int i = 0; i < az_g_number.size(); i++)
        {
            std::vector<char *> vecotraz_g_number(az_data_number[i] + az_g_number[i]);
            char **new_data = (char **)vecotraz_g_number.data();

            for (int j = 0; j < az_data_number[i]; j++)
            {
                new_data[j] = data_ptrs[i * r + j];
            }
            for (int j = 0; j < az_g_number[i]; j++)
            {
                new_data[az_data_number[i] + j] = coding_ptrs[g_sum + j];
            }

            int shard_number_az = az_data_number[i] + az_g_number[i];
            std::vector<int> last_matrix(shard_number_az, 1);
            jerasure_matrix_encode(shard_number_az, 1, 8, last_matrix.data(), new_data, &coding_ptrs[m + i], blocksize);
            g_sum += az_g_number[i];
        }
    }
    free(matrix);
    return true;
}

bool OppoProject::decode(int k, int m, int l, char **data_ptrs, char **coding_ptrs, std::shared_ptr<std::vector<int>> erasures, int blocksize, EncodeType encode_type)
{

    if (encode_type == RS || encode_type == OPPO_LRC)
    {
        std::vector<int> matrix(m * k, 0);
        int *rs_matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
        memcpy(matrix.data(), rs_matrix, m * k * sizeof(int));
        jerasure_matrix_decode(k, m, 8, matrix.data(), 0, erasures->data(), data_ptrs, coding_ptrs, blocksize);
        free(rs_matrix);
    }
    else if (encode_type == Azure_LRC_1)
    {

        std::vector<int> matrix((m + l) * k, 0);
        lrc_make_matrix(k, m, l, matrix.data());
        if (check_k_data(*erasures, k))
        {
            return true;
        }
        if (jerasure_matrix_decode(k, m + l, 8, matrix.data(), 0, erasures->data(), data_ptrs, coding_ptrs, blocksize) == -1)
        {
            std::vector<int> new_erasures(m + l + 1, 1);
            int survival_number = k + m + l - erasures->size() + 1;
            std::vector<int> survival_index;
            auto part_new_erasure = std::make_shared<std::vector<std::vector<int>>>();
            for (int i = 0; i < int(erasures->size() - 1); i++)
            {
                new_erasures[i] = (*erasures)[i];
            }
            new_erasures[m + l] = -1;

            for (int i = 0; i < k + m + l; i++)
            {
                if (std::find(erasures->begin(), erasures->end(), i) == erasures->end())
                {
                    survival_index.push_back(i);
                }
            }
            if (survival_number > k)
            {
                combine(part_new_erasure, survival_index.size(), survival_number - k);
            }
            for (int i = 0; i < int(part_new_erasure->size()); i++)
            {
                for (int j = 0; j < int((*part_new_erasure)[i].size()); j++)
                {
                    new_erasures[erasures->size() - 1 + j] = survival_index[(*part_new_erasure)[i][j] - 1];
                }

                if (jerasure_matrix_decode(k, m + l, 8, matrix.data(), 0, new_erasures.data(), data_ptrs, coding_ptrs, blocksize) != -1)
                {
                    break;
                }
            }
        }
        else
        {
            return true;
        }
        std::cout << "cannot decode!!!!!!!!!!!!" << std::endl;
    }    
    return true;
}

bool OppoProject::check_received_block(int k, int expect_block_number, std::shared_ptr<std::vector<int>> shards_idx_ptr, int shards_ptr_size)
{
    if (shards_ptr_size != -1)
    {
        if (int(shards_idx_ptr->size()) != shards_ptr_size)
        {
            return false;
        }
    }

    if (int(shards_idx_ptr->size()) >= expect_block_number)
    {
        return true;
    }
    else if (int(shards_idx_ptr->size()) == k) // azure_lrc防误杀
    {
        for (int i = 0; i < k; i++)
        {
            // 没找到
            if (std::find(shards_idx_ptr->begin(), shards_idx_ptr->end(), i) == shards_idx_ptr->end())
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}
