#pragma once

#include <vector>

template<typename K, typename V>
class NamedList
{
public:
	NamedList() = default;

	/*const V& NONE{};
	
	const V& find(const K& key) const {
		for (size_t i = 0; i < values.size(); i++) {
			if (values[i].first == key) {
				return values[i].second;
			}
		}
		return NONE;
	}*/

	bool find(const K& key) const {
		for (size_t i = 0; i < values.size(); i++) {
			if (values[i].first == key) {
				return true;
			}
		}
		return false;
	}

	const V& get(const K& key) const {
		for (size_t i = 0; i < values.size(); i++) {
			if (values[i].first == key) {
				return values[i].second;
			}
		}
		return NULL;
	}

	bool remove(const K& key);

	bool removeVal(const V& val);

	bool removeValAll(const V& val);

	void push(K key, V val) {
		values.push_back({ key, val });
	}

private:


private:
	std::vector<std::pair<K, V>> values;

};

