//
// Created by chenfeiwang on 2/19/22.
//
#include "../src/Skiplist.hpp"
#include <gtest/gtest.h>
#include <string>
#include <climits>

TEST(BaseSerializerTest, SerializeTestKey) {
    BasicSerializer bs;
    int test_num = rand() % INT_MAX;
    EXPECT_EQ(std::to_string(test_num), bs.serialize_key(test_num));
}

TEST(BaseSerializerTest, SerializeTestValue) {
    BasicSerializer bs;
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);
    std::string test_str =  str.substr(0, 16);
    EXPECT_EQ(test_str, bs.serialize_value(test_str));
}

TEST(BaseSerializerTest, DeserializeTestInt) {
    BasicSerializer bs;
    std::string str("0123456789");
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);
    std::string test_str =  str.substr(0, 4);
    int target_num = 1000 * (test_str[0] - '0') + 100 * (test_str[1] - '0') \
                        + 10 * (test_str[2] - '0') + (test_str[3] - '0');
    EXPECT_EQ(target_num, bs.deserialize_to_key(test_str));
}

TEST(BaseSerializerTest, DeserializeTestValue) {
    BasicSerializer bs;
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);
    std::string test_str =  str.substr(0, 16);
    EXPECT_EQ(test_str, bs.deserialize_to_value(test_str));
}

TEST(SkiplistTest, AddReadTest1) {
    Skiplist<int, int> list(nullptr);
    list.insert(1, 1996);
    int value;
    list.read(1, value);
    EXPECT_EQ(value, 1996);
}

TEST(SkiplistTest, AddReadTest2) {
    Skiplist<std::string, int> list(nullptr);
    list.insert("testKey", 1996);
    int value;
    list.read("testKey", value);
    EXPECT_EQ(value, 1996);
}

TEST(SkiplistTest, AddReadTest3) {
    Skiplist<int, std::string> list(nullptr);
    list.insert(1, "testValue");
    std::string value;
    list.read(1, value);
    EXPECT_EQ(value, "testValue");
}

TEST(SkiplistTest, AddReadTest4) {
    Skiplist<std::string, std::string> list(nullptr);
    list.insert("testkey", "testValue");
    std::string value;
    bool ret = list.read("testkey", value);
    EXPECT_TRUE(ret);
    EXPECT_EQ(value, "testValue");
}

TEST(SkiplistTest, AddReadTest5) {
    Skiplist<std::string, std::string> list(nullptr);
    list.insert("testkey", "testValue");
    std::string value;
    bool ret = list.read("testkey1", value);
    EXPECT_EQ(value, "");
    EXPECT_FALSE(ret);
}

TEST(SkiplistTest, AddReadTest6) {
    Skiplist<std::string, std::string> list(nullptr);
    list.insert("testkey", "testValue");
    list.insert("testkey", "testValue2");
    std::string value;
    bool ret = list.read("testkey", value);
    EXPECT_EQ(value, "testValue2");
    EXPECT_TRUE(ret);
}

TEST(SkiplistTest, AddReadTest7) {
    Skiplist<int, std::string> list(nullptr);
    list.insert(1, "testValue");
    list.insert(2, "testValue2");
    list.insert(3, "testValue3");
    list.insert(4, "testValue4");
    list.insert(5, "testValue5");
    list.insert(6, "testValue6");
    list.insert(7, "testValue7");
    std::string value;
    bool ret = list.read(1, value);
    EXPECT_EQ(value, "testValue");
    EXPECT_TRUE(ret);
    ret = list.read(2, value);
    EXPECT_EQ(value, "testValue2");
    ret = list.read(3, value);
    EXPECT_EQ(value, "testValue3");
    ret = list.read(4, value);
    EXPECT_EQ(value, "testValue4");
    ret = list.read(5, value);
    EXPECT_EQ(value, "testValue5");
    ret = list.read(6, value);
    EXPECT_EQ(value, "testValue6");
    ret = list.read(7, value);
    EXPECT_EQ(value, "testValue7");

}

TEST(SkiplistTest, AddEraseTest1) {
    Skiplist<std::string, std::string> list(nullptr);
    list.insert("testkey", "testValue");
    std::string value;
    list.read("testkey", value);
    EXPECT_EQ(value, "testValue");

    value.clear();
    list.erase("testkey");
    list.read("testkey", value);
    EXPECT_EQ(value, "");
}

TEST(SkiplistTest, AddEraseTest2) {
    Skiplist<std::string, std::string> list(nullptr);
    list.insert("testkey", "testValue");
    bool ret = list.erase("testkey2");
    EXPECT_FALSE(ret);
}

TEST(SkiplistTest, DumpLoadTest) {
    BasicSerializer bs;
    Skiplist<int, std::string> list(&bs);
    list.insert(1, "testValue1");
    list.insert(2, "testValue2");
    list.insert(3, "testValue3");
    list.insert(4, "testValue4");
    list.dump_to("./output/dump_test.json");

    Skiplist<int, std::string> list2(&bs);
    list2.load_from("./output/dump_test.json");
    bool ret;
    std::string value;
    ret = list2.read(1,value);
    EXPECT_TRUE(ret);
    EXPECT_EQ(value, "testValue1");
    ret = list2.read(2,value);
    EXPECT_TRUE(ret);
    EXPECT_EQ(value, "testValue2");
    ret = list2.read(3,value);
    EXPECT_TRUE(ret);
    EXPECT_EQ(value, "testValue3");
    ret = list2.read(4,value);
    EXPECT_TRUE(ret);
    EXPECT_EQ(value, "testValue4");
}

int main() {
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
