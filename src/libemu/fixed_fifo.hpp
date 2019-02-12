#pragma once
#include <array>
#include <stdexcept>

namespace psycris {
	template <typename T, uint8_t S>
	class fixed_fifo {
	  public:
		uint8_t size() const { return _size; }

		uint8_t capacity() const { return S; }

		bool empty() const { return _size == 0; }

		bool full() const { return _size == capacity(); }

	  public:
		void push(T v) {
			if (full()) {
				throw std::runtime_error("fifo full");
			}
			_data[(_head + _size) % capacity()] = std::forward<T>(v);
			_size++;
		}

		T pop() {
			if (empty()) {
				throw std::runtime_error("fifo empty");
			}
			T r = std::move(_data[_head]);
			_head = (_head + 1) % capacity();
			_size--;
			return r;
		}

	  private:
		std::array<T, S> _data;
		uint8_t _head = 0;
		uint8_t _size = 0;
	};
}