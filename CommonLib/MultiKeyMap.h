#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <iterator>
#include <string>

typedef uint64_t id_t;

template<typename K, typename V>
class MultiKeyMap
{
public:
	explicit MultiKeyMap() : keys(), vals() {
	}

	//template<typename Iter>
	//MultiKeyMap(Iter begin, Iter end) : keys(), vals() {
	//	for (auto it = begin; it != end; it++) {
	//		insert((*it).first.begin(), (*it).first.end(), (*it).second);
	//	}
	//}

	MultiKeyMap(std::initializer_list<std::pair<std::vector<K>, V>> l) : MultiKeyMap() {
		insert(l);
	}

	void insert(std::initializer_list<std::pair<std::vector<K>, V>> l) {
		for (auto it = l.begin(); it != l.end(); it++) {
			insert((*it).first, (*it).second);
		}
	}

	template<typename Iter>
	void insert(std::iterator_traits<Iter> keys_begin, std::iterator_traits<Iter> keys_end, V val) {
		auto id = nextID();
		for (auto it = keys_begin; it != keys_end; it++) {
			keys.insert({ *it, id });
		}
		vals.insert({ id, val });
	}

	void insert(K k, V v)
	{
		auto id = nextID();
		keys.insert({k, id});
		vals.insert({ id, v });
	}
	void insert(std::vector<K> ks, V v)
	{
		auto id = nextID();
		for (auto const k : ks) {
			keys.insert({ k, id });
		}
		vals.insert({ id, v });
	}

	bool add(K baseKey, K additionalKey);
	bool add(K baseKey, std::vector<K> additionalKeys);

	bool add(V val, K additionalKey);
	bool add(V val, std::vector<K> additionalKeys);



	V const* at(K key) const {
		auto id = keys.find(key);
		if (id == keys.end()) {
			return nullptr;
		}

		return &vals.at((*id).second);
	}


private:
	inline id_t nextID() {
		return next_id++;
	}

private:
	std::map<K, id_t> keys;
	std::map<id_t, V> vals;


	id_t next_id = 1;

};
//
//template<typename K, typename V>
//inline void MultiKeyMap<K, V>::insert(std::iterator keys_begin, std::iterator keys_end, V val)
//{
//}
