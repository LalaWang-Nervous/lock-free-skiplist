//
// Created by chenfeiwang on 2/16/22.
//

#ifndef SKIPLIST_CHENFEI_SKIPLIST_HPP
#define SKIPLIST_CHENFEI_SKIPLIST_HPP

#include <string>
#include <vector>
#include <cassert>
#include <atomic>
#include <random>
#include <iostream>
#include <thread>
#include <fstream>
#include "../thirdparty/nlohmann_json/json.hpp"
#include "Serializers.hpp"

// default value of the max skiplist's height
#define DEFAULT_MAX_HEIGHT 32
// control the probability of increasing skiplist-node's height
// value is N, the has 1/N probability to increase height by one
#define DEFAULT_PROBABILITY_DENOMINATOR 4

static std::default_random_engine generator;
static std::uniform_int_distribution<int> distribution(0,INT32_MAX);

template<class Key, class Value>
class Skiplist {
private:
    class Node {
    public:
        const Key key;
        int height;
    public:
        Value value() { return _value; }
        void set_value(const Value& v) { _value = v; }
        Node* next(int level) {
            assert((level >= 0) && (level < height));
            return _next[level].load(std::memory_order_acquire);
        }
        void set_next(int level, Node* node) {
            assert((level >= 0) && (level < height));
            _next[level].store(node, std::memory_order_release);
        }
    private:
        Value _value;
        std::atomic<Node*>* _next;
    public:
        Node(const Key& k, const Value& v, int h): key(k), _value(v), height(h) {
            _next = (std::atomic<Node*>*)malloc(height * sizeof(std::atomic<Node*>));
            memset(_next, 0, height * sizeof(std::atomic<Node*>));
        };
        ~Node() {};
    };
public:
    // insert a new key value pair, if the key exists, change the value
    // or new a new node and insert
    void insert(const Key& key, const Value& value);
    // erase a key value pair, if the key does not exist, return false
    bool erase(const Key& key);
    // read value according to key, if the key does not exist, return false
    bool read(const Key& key, Value& value);
    // dump the skiplist to file
    bool dump_to(const std::string& path);
    // recover the skiplist from a pre-dumped file
    bool load_from(const std::string& path);
private:
    // the upper bound of this skiplist's height
    int _max_h;
    // increase node's height with probability (1/_pd)
    int _pd;
    // record all nodes generated by this skiplist
    // only when this skiplist is destroyed these nodes will be freed
    // erase a node only take it away from skiplist, but won't free it
    std::vector<Node*> _all_nodes;
    // the skiplist's current height, there may write and read concurrent,
    // so it needs to be atomic
    std::atomic<int> _cur_h;
    // dummy head node of skiplist
    Node* _head;
    // the serializer
    ISerializer<Key, Value>* _serializer;
private:
    // generate random height,
    // return 1 for probability of (1 - 1/_pd),
    // 2 for (1/_pd) * (1 - 1/_pd), 3 for (1/_pd)^2 * (1 - 1/_pd),.. and so on
    int random_height();
    // generate a new node for skiplist and record it
    Node* new_node(const Key& k, const Value& v, int height);
    // find a node whose key value greater or equal to input param key
    // if such node does not exist, return the last node
    // if the input param vec is not null,
    // it will record the last traverse node on each level during the find process
    Node* find_greater_or_equal(const Key& key,
                                std::vector<Node*>* vec);
    // get current skiplist's height
    int get_current_list_height() {
        return _cur_h.load(std::memory_order_acquire);
    }
    // set current skiplist's height
    void set_current_list_height(int h) {
        _cur_h.store(h, std::memory_order_release);
    }
    void _add(const Key& key, const Value& value, int height);
public:
    // to implement dump/load for skiplist
    // for template class type Key and Value
    // caller should implement ISerializer interface
    // to implement their corresponding serialized method
    Skiplist(int max_height, int probability_denominator, ISerializer<Key, Value>* s);
    Skiplist(ISerializer<Key, Value>* s = nullptr) : Skiplist(DEFAULT_MAX_HEIGHT,
                                                              DEFAULT_PROBABILITY_DENOMINATOR,
                                                              s) {}
    ~Skiplist();
    Skiplist(const Skiplist&) = delete;
    Skiplist& operator=(const Skiplist&) = delete;
};

template<class Key, class Value>
Skiplist<Key, Value>::Skiplist(int max_height,
                               int probability_denominator,
                               ISerializer<Key, Value>* s) :
                               _max_h(max_height),
                               _pd(probability_denominator),
                               _cur_h(1),
                               _serializer(s) {
    _head = new_node(Key(), Value(), _max_h);
}

template<class Key, class Value>
Skiplist<Key, Value>::~Skiplist() {
    for(auto& p : _all_nodes) {
        free(p);
    }
}

template<class Key, class Value>
void Skiplist<Key, Value>::_add(const Key &key, const Value &value, int height) {
    std::vector<Node*> need_update(_max_h, nullptr);
    Node* next = find_greater_or_equal(key, &need_update);

    if (next && (next->key == key)) {
        next->set_value(value);
        return;
    }

    if (height > get_current_list_height()) {
        for (int i = get_current_list_height(); i < height; i++) {
            need_update[i] = _head;
        }
        set_current_list_height(height);
    }

    Node* add_node = new_node(key, value, height);
    for (int i = 0; i < height; i++) {
        add_node->set_next(i, need_update[i]->next(i));
        need_update[i]->set_next(i, add_node);
    }
}

template<class Key, class Value>
void Skiplist<Key, Value>::insert(const Key &key, const Value &value) {
    int height = random_height();
    return _add(key, value, height);
}

template<class Key, class Value>
bool Skiplist<Key, Value>::erase(const Key &key) {
    std::vector<Node*> need_update(_max_h, nullptr);
    Node* ge = find_greater_or_equal(key, &need_update);

    if ((ge == nullptr) || (ge->key != key)) {
        return false;
    }

    int height = ge->height;
    for (int i = 0; i < height; i++) {
        need_update[i]->set_next(i, ge->next(i));
    }

    // update the skiplist's height
    int new_cur_height = 1;
    Node* p = _head->next(0);
    while(p) {
        new_cur_height = p->height > new_cur_height ? p->height : new_cur_height;
        p = p->next(0);
    }
    set_current_list_height(new_cur_height);

    return true;
}

template<class Key, class Value>
bool Skiplist<Key, Value>::read(const Key &key, Value &value) {
    Node* next = find_greater_or_equal(key, nullptr);
    if (next && next->key == key) {
        value = next->value();
        return true;
    }

    return false;
}

template<class Key, class Value>
bool Skiplist<Key, Value>::dump_to(const std::string &path) {
    Node* p = _head->next(0);
    nlohmann::json all_nodes;
    while(p) {
        nlohmann::json node;
        std::string k_str = _serializer->serialize_key(p->key);
        std::string v_str = _serializer->serialize_value(p->value());
        node["NODE_KEY"] = k_str;
        node["NODE_VALUE"] = v_str;
        node["NODE_HEIGHT"] = p->height;

        all_nodes.push_back(node);

        p = p->next(0);
    }

    std::ofstream o(path);
    o << all_nodes;
    o.close();

    return true;
}

template<class Key, class Value>
bool Skiplist<Key, Value>::load_from(const std::string &path) {
    std::ifstream i(path);
    nlohmann::json all_nodes;
    i >> all_nodes;
    for(auto it = all_nodes.begin(); it != all_nodes.end(); it++) {
        nlohmann::json node_json = *it;
        Key node_k = _serializer->deserialize_to_key(node_json["NODE_KEY"]);
        Value node_v = _serializer->deserialize_to_value(node_json["NODE_VALUE"]);
        int h = node_json["NODE_HEIGHT"];
        _add(node_k, node_v, h);
    }
}

template<class Key, class Value>
typename Skiplist<Key, Value>::Node*
Skiplist<Key, Value>::find_greater_or_equal(const Key &k,
                                            std::vector<Node*>* vec) {
    Node* p = _head;
    int level = get_current_list_height() - 1;
    while(true) {
        Node* next = p->next(level);
        if(next && (next->key < k)) {
            p = next;
        } else {
            if(vec) (*vec)[level] = p;
            if (level == 0) {
                return next;
            } else {
                level--;
            }
        }
    }
}

template<class Key, class Value>
typename Skiplist<Key, Value>::Node* Skiplist<Key, Value>::new_node(const Key &k,
                                                                    const Value &v,
                                                                    int height) {
    Node* n = new Node(k, v, height);
    // Attention:
    // std::vector push_back is not thread-safe,
    // so this Skiplist can't be written by multi-threads
    _all_nodes.push_back(n);
    return n;
}

template<class Key, class Value>
int Skiplist<Key, Value>::random_height() {
    int height = 1;
    while((height < _max_h) &&
          (distribution(generator) % _pd == 0)) {
        height++;
    }

    return height;
}

#endif //SKIPLIST_CHENFEI_SKIPLIST_HPP
