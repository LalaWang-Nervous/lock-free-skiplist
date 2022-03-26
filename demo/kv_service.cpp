#include <iostream>
#include <sstream>
#include <mutex>
#include <pthread.h>
#include "../src/Skiplist.hpp"

std::mutex mtx;
void atomic_log(std::string s) {
    mtx.lock();
    std::cout << s << std::endl;
    mtx.unlock();
}

#define MAX_CLIENT_NUM 1000
pthread_t all_thread[MAX_CLIENT_NUM + 1];

class Server {
public:
    Server() {}
    ~Server() {}
public:
    void insert(int key, std::string& value){
        _list.insert(key, value);
    }
    bool erase(int key) {
        return _list.erase(key);
    }
    bool read(int key, std::string& value) {
        return _list.read(key, value);
    }
private:
    Skiplist<int, std::string> _list;
};

Server global_s;

void* random_write(void* p) {
    Server* s = &global_s;
    int num = 0;
    while(true) {
        int rand_key = rand() % 100;
        if(rand_key % 3 == 0) {
            rand_key = rand() % 100;
            bool ret = s->erase(rand_key);
            if(ret) {
                num--;
            }
            std::stringstream ss;
            ss << "write thread try to erase key=" << rand_key << ", ret= " << ret << std::endl;
            atomic_log(ss.str());
        } else {
            rand_key = rand() % 100;
            std::string rand_value = std::to_string(rand());
            s->insert(rand_key, rand_value);
            num++;
            std::stringstream ss;
            ss << "write thread insert key=" << rand_key << ", value= " << rand_value << std::endl;
            atomic_log(ss.str());
        }
    }
}

void* random_read(void* p) {
    Server* s = &global_s;
    long id = (long)p;
    while(true) {
        int rand_key = rand() % 100;
        std::string v;
        bool ret = s->read(rand_key, v);
        std::stringstream ss;
        ss << id << "-th reader try read key=" << rand_key \
        << ", ret=" << ret << ", value=" << v << std::endl;
        atomic_log(ss.str());
    }
}

int main() {
    int client_num = 100;

    pthread_t w_thread;
    int r = pthread_create(&w_thread, NULL, random_write, (void*)(&global_s));
    if (r) {
        std::cout << "Error:unable to create thread," << r << std::endl;
        exit(-1);
    }

    pthread_t r_thread[client_num];
    for(int i = 0; i < client_num; i++) {
        int r = pthread_create(&all_thread[i + 1], NULL, random_read, (void *)i);
        if (r) {
            std::cout << "Error:unable to create thread," << r << std::endl;
            exit(-1);
        }
    }

    void *ret;
    if (pthread_join(w_thread, &ret) !=0 )  {
        perror("join error");
        exit(-1);
    }

    for(auto i = 0; i < client_num; i++) {
        if (pthread_join(r_thread[i], &ret) !=0 )  {
            perror("join error");
            exit(-1);
        }
    }

    // never return
}

