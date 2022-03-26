#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "../thirdparty/googletest/include/gtest/gtest.h"
#include "../src/Skiplist.hpp"

#define WRITE_TEST_COUNT 1000000

#define READ_NUM_THREADS 100
#define READ_TEST_COUNT 1000000

BasicSerializer serializer;
Skiplist<int, std::string> skipList(&serializer);

void *insertElementRand(void* threadid) {
    for (int i = 0; i < WRITE_TEST_COUNT; i++) {
        skipList.insert(rand(), "testStr");
    }
    pthread_exit(NULL);
}

void *insertElementOrder(void* threadid) {
    for (int i = 0; i < WRITE_TEST_COUNT; i++) {
        skipList.insert(i, "testStr");
    }
    pthread_exit(NULL);
}

void *readElement(void* threadid) {
    for (int i = 0; i < READ_TEST_COUNT; i++) {
        std::string str;
        skipList.read(rand(), str);
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    {
        std::cout << std::endl;
        std::cout << "[TEST INFO]" << std::endl;
        std::cout << "Test Insert Performance:" << std::endl;
        std::cout << "Key Type: int, Value Type: std::string" << std::endl;
        std::cout << "Key is fixed, Value is fixed." << std::endl;
        std::cout << "The number of insert operation: " << WRITE_TEST_COUNT << std::endl;

        pthread_t w_thread;
        std::cout << "[TEST BEGIN]" << std::endl;
        std::cout << "creating thread for insert..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        int rc = pthread_create(&w_thread, NULL, insertElementOrder, NULL);
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << std::endl;
            exit(-1);
        }

        void *ret;
        if (pthread_join(w_thread, &ret) !=0 )  {
            perror("join error");
            exit(-1);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        std::cout << "insert complete." << std::endl;
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "use " << elapsed.count() << " secs for " << WRITE_TEST_COUNT << " insert operation" << std::endl;
        std::cout << "QPS: " << (WRITE_TEST_COUNT / elapsed.count()) << std::endl;
    }

    {
        std::cout << std::endl;
        std::cout << "[TEST INFO]" << std::endl;
        std::cout << "Test Insert Performance:" << std::endl;
        std::cout << "Key Type : int, Value Type: std::string" << std::endl;
        std::cout << "Key is random generated, Value is fixed." << std::endl;
        std::cout << "The number of insert operation: " << WRITE_TEST_COUNT << std::endl;

        pthread_t w_thread;
        std::cout << "[TEST BEGIN]" << std::endl;
        std::cout << "creating thread for insert..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        int rc = pthread_create(&w_thread, NULL, insertElementRand, NULL);
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << std::endl;
            exit(-1);
        }

        void *ret;
        if (pthread_join(w_thread, &ret) !=0 )  {
            perror("join error");
            exit(-1);
        }

        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = finish - start;
        std::cout << "insert complete." << std::endl;
        std::cout << "use " << elapsed.count() << " secs for " << WRITE_TEST_COUNT << " insert operation" << std::endl;
        std::cout << "QPS: " << (WRITE_TEST_COUNT / elapsed.count()) << std::endl;

        // Test Read Performance for single-thread
        std::cout << std::endl;
        std::cout << "[TEST INFO]" << std::endl;
        std::cout << "Test Read Performance for single-thread:" << std::endl;
        std::cout << "Key Type : int, Value Type: std::string" << std::endl;
        std::cout << "Key is random generated" << std::endl;
        std::cout << "The number of read threads: " << 1 << std::endl;
        std::cout << "The number of read operation for each thread: " << READ_TEST_COUNT << std::endl;

        pthread_t r_thread;
        std::cout << "[TEST BEGIN]" << std::endl;
        std::cout << "creating threads for read..." << std::endl;
        start = std::chrono::high_resolution_clock::now();
        rc = pthread_create(&r_thread, NULL, readElement, NULL);
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << std::endl;
            exit(-1);
        }

        if (pthread_join(r_thread, &ret) !=0 )  {
            perror("pthread_create() error");
            exit(3);
        }

        finish = std::chrono::high_resolution_clock::now();
        elapsed = finish - start;
        std::cout << "read complete." << std::endl;
        std::cout << "single thread ";
        std::cout << "uses " << elapsed.count() << " secs for " << READ_TEST_COUNT << " read operation respectively" << std::endl;
        std::cout << "QPS: " << (READ_TEST_COUNT / elapsed.count()) << std::endl;


        std::cout << std::endl;
        std::cout << "[TEST INFO]" << std::endl;
        std::cout << "Test Read Performance for multi-threads:" << std::endl;
        std::cout << "Key Type : int, Value Type: std::string" << std::endl;
        std::cout << "Key is random generated" << std::endl;
        std::cout << "The number of read threads: " << READ_NUM_THREADS << std::endl;
        std::cout << "The number of read operation for each thread: " << READ_TEST_COUNT << std::endl;

        pthread_t r_threads[READ_NUM_THREADS];
        std::cout << "[TEST BEGIN]" << std::endl;
        std::cout << "creating threads for read..." << std::endl;
        start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < READ_NUM_THREADS; i++ ) {
            rc = pthread_create(&r_threads[i], NULL, readElement, NULL);
            if (rc) {
                std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        for(int i = 0; i < READ_NUM_THREADS; i++ ) {
            if (pthread_join(r_threads[i], &ret) !=0 )  {
                perror("pthread_create() error");
                exit(3);
            }
        }

        finish = std::chrono::high_resolution_clock::now();
        elapsed = finish - start;
        std::cout << "read complete." << std::endl;
        std::cout << READ_NUM_THREADS << " threads ";
        std::cout << "use " << elapsed.count() << " secs for " << READ_TEST_COUNT << " read operation respectively" << std::endl;
        std::cout << "QPS: " << (READ_TEST_COUNT / elapsed.count()) << std::endl;
    }

    return 0;
}
