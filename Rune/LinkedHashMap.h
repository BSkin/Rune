#ifndef LINKED_HASH_MAP_H
#define LINKED_HASH_MAP_H

#include <unordered_map>
#include <list>
#include <algorithm>
using std::unordered_map;
using std::list;
using std::pair;

template <typename Key, typename Value>
class LinkedHashMap
{
public:
	LinkedHashMap() {
		hashMap = unordered_map<Key, Value>();
		linkedList = list<pair<Key, Value>>();
	}
	~LinkedHashMap() {
		hashMap.clear();
		linkedList.clear();
	}
	
	void add(Key k, Value v) {
		hashMap.insert(make_pair(k, v));
		linkedList.push_back(make_pair(k, v));
	}

	bool remove(Key k, Value v) {
		unordered_map<Key, Value>::iterator iter = hashMap.find(k);
		if (iter != hashMap.end()) {
			hashMap.erase(k);
			linkedList.remove(make_pair(k, v));
			return true;
		}
		return false;
	}
	bool remove(Key k) {
		unordered_map<Key, Value>::iterator iter = hashMap.find(k);
		if (iter != hashMap.end()) {
			linkedList.remove(make_pair(k, iter->second));
			hashMap.erase(k);
			return true;
		}
		return false;
	}
	bool remove(Value v) {
		for (list<pair<Key, Value>>::iterator iter = linkedList.begin(); iter != linkedList.end(); iter++) {
			if (iter->second == v) {
				hashMap.erase(iter->first);
				linkedList.erase(iter);
				return true;
			}
		}
		return false;
	}
	typename list<pair<Key, Value>>::iterator erase(typename list<pair<Key, Value>>::iterator iter) {
		unordered_map<Key, Value>::iterator hashIter = hashMap.find(iter->first);
		if (hashIter != hashMap.end()) {
			hashMap.erase(iter->first);
			return linkedList.erase(iter);
		}
		return typename list<pair<Key, Value>>::iterator();
	}

	int size() { return linkedList.size(); }
	typename list<pair<Key, Value>>::iterator begin() { return linkedList.begin(); }
	typename list<pair<Key, Value>>::iterator end() { return linkedList.end(); }
	Value getValue(Key k) {
		unordered_map<Key, Value>::iterator iter = hashMap.find(k);
		if (iter == hashMap.end()) return Value();
		return iter->second;
	}
	pair<Key, Value> back() { if (linkedList.size() == 0) return make_pair(Key(), Value()); return linkedList.back(); }
	Value backValue() { if (linkedList.size() == 0) return Value(); return linkedList.back().second; }

	void popBack() {
		if (linkedList.size() == 0) return;
		hashMap.erase(linkedList.back().first);
		linkedList.pop_back();
	}
private:
	unordered_map<Key, Value> hashMap;
	list<pair<Key, Value>> linkedList;
};

#endif