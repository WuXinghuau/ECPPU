#include "datanode.h"
#include "toolbox.h"


void DataNode::get_data_from_proxy() {
    for (;;) {
        try {
            asio::ip::tcp::socket socket(io_context);
            acceptor_for_get_data.accept(socket);

            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(socket, asio::buffer(int_buf, int_buf.size()));
            int key_size = OppoProject::bytes_to_int(int_buf);
            asio::read(socket, asio::buffer(int_buf, int_buf.size()));
            int value_size = OppoProject::bytes_to_int(int_buf);

            std::vector<unsigned char> key_buf(key_size);
            std::vector<unsigned char> value_buf(value_size);
            asio::read(socket, asio::buffer(key_buf, key_buf.size()));
            asio::read(socket, asio::buffer(value_buf, value_buf.size()));

            std::cout << key_size << " " << key_size << " " << value_size << std::endl;

            memcached_set(m_memcached, (const char *)key_buf.data(), key_buf.size(), (const char *)value_buf.data(), value_buf.size(), (time_t)0, (uint32_t)0);
            std::vector<char> finish(1);
            asio::write(socket, asio::buffer(finish, finish.size()));
            socket.shutdown(asio::ip::tcp::socket::shutdown_receive);
            socket.close();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            exit(-1);
        }
    }
}

void DataNode::send_data_to_proxy() {
    for (;;) {
        try {
            asio::ip::tcp::socket socket(io_context);
            acceptor_for_send_data.accept(socket);
            
            std::vector<unsigned char> int_buf(sizeof(int));
            asio::read(socket, asio::buffer(int_buf, int_buf.size()));
            int key_size = OppoProject::bytes_to_int(int_buf);

            std::vector<unsigned char> key_buf(key_size);
            asio::read(socket, asio::buffer(key_buf, key_buf.size()));

            asio::read(socket, asio::buffer(int_buf, int_buf.size()));
            int offset = OppoProject::bytes_to_int(int_buf);

            asio::read(socket, asio::buffer(int_buf, int_buf.size()));
            int lenth = OppoProject::bytes_to_int(int_buf);

            std::cout << key_size << " " << offset << " " << lenth << std::endl;

            memcached_return_t error;
            uint32_t flag;
            size_t value_size;
            char *value_ptr = memcached_get(m_memcached, (const char *)key_buf.data(), key_buf.size(), &value_size, &flag, &error);

            // printf("%d %s %p %d %ld\n", key_size, std::string((char *)key_buf.data(), key_size).c_str(), value_ptr, port, value_size);
            asio::write(socket, asio::buffer(value_ptr + offset, lenth));
            socket.shutdown(asio::ip::tcp::socket::shutdown_both);
            socket.close();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            exit(-1);
        }
    }
}

void DataNode::start() {
    ths.push_back(std::thread(std::bind(&DataNode::get_data_from_proxy, this)));
    ths.push_back(std::thread(std::bind(&DataNode::send_data_to_proxy, this)));
    ths[0].join();
    ths[1].join();
}