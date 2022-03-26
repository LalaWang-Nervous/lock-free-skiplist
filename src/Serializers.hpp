//
// Created by wangchenfei01 on 2022/3/25.
//

#ifndef SKIPLIST_CHENFEI_SERIALIZERS_H
#define SKIPLIST_CHENFEI_SERIALIZERS_H

template<class Key, class Value>
class ISerializer {
public:
    ISerializer() {}
    virtual ~ISerializer() {}
public:
    virtual std::string serialize_key(const Key& key) = 0;
    virtual std::string serialize_value(const Value& value) = 0;
    virtual Key deserialize_to_key(const std::string& str) = 0;
    virtual Value deserialize_to_value(const std::string& str) = 0;
};

class BasicSerializer : public ISerializer<int, std::string> {
public:
    BasicSerializer() = default;
    ~BasicSerializer() override = default;
public:
    std::string serialize_key(const int& key) override {
        return std::to_string(key);
    }
    std::string serialize_value(const std::string& value) override {
        return value;
    }
    int deserialize_to_key(const std::string& str) override {
        return atoi(str.c_str());
    }
    std::string deserialize_to_value(const std::string& str) override {
        return str;
    }
};

#endif //SKIPLIST_CHENFEI_SERIALIZERS_H
