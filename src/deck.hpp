#pragma once

#include <forward_list>
#include <random>
#include <chrono>
#include <initializer_list>
#include "constants.hpp"

template<typename T>
class SizedForwardList {
	std::forward_list<T> list;
	uint8_t size;
public:
	void push_front(const T& t){ list.push_front(t); size += 1; };
	bool empty() const { return list.empty(); }
	void put_into(SizedForwardList<T>& other) {
		other.splice_after(other.cbefore_begin(), *this);
		other.size += size;
		size = 0;
	}
	void shuffle();
	T pop(){
		T ret = list.front();
		list.pop_front();
		size -= 1;
		return ret;
	}
};

template<typename T>
class Deck {
	SizedForwardList<T> drawPile, discardPile;
public:
    Deck(std::initializer_list<T> cards): drawPile(cards)  {
		drawPile.shuffle();
	}
	void discard(const T& card) {
		discardPile.push_front(card);
	}
	T draw() {
		if(drawPile.empty()) discardPile.put_into(drawPile);
		return drawPile.pop();
	}
};

template<typename T>
void SizedForwardList<T>::shuffle(){
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::forward_list<T> newlist;
	for(uint i=0; i<size; i++){
		std::uniform_int_distribution<int> distribution(0, size - i - 1);
		int r = distribution(generator) % (size-i);

		auto start = list.before_begin();
		for(int j = 0; j < r; j++) start++; //start shows predecessor of swapped Card
		list.splice_after(newlist.before_begin(), newlist, start);
	}
	list = std::move(newlist);
}

template<typename T>
Deck<T> SizedForwardList<T>::create(){

}