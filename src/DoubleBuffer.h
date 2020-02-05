#pragma once
#include <mutex>

template <typename T>
class DoubleBuffer {
private:
	std::vector<T> _buffer0;
	std::vector<T> _buffer1;
	std::vector<T>* _inputPtr;
	std::vector<T>* _outputPtr;

	std::mutex _threadlock;

	void swap() {
		std::lock_guard<std::mutex> guard(_threadlock);
		std::vector<T>* tempPtr = _inputPtr;
		_inputPtr = _outputPtr;
		_outputPtr = tempPtr;
		if (_inputPtr != nullptr) {
			_inputPtr->clear();
		}
	}

public:
	DoubleBuffer() {
		_inputPtr = &_buffer0;
		_outputPtr = &_buffer1;
	}

	// copy constructor
	DoubleBuffer(const DoubleBuffer &that) {
		operator=(that);
	}

	// assignment operator
	DoubleBuffer& operator=(const DoubleBuffer &that) {
		std::lock_guard<std::mutex> guard(that->_threadlock);
		if (this != that) {
			std::lock_guard<std::mutex> guard(_threadlock);
			_buffer0 = that->_buffer0;
			_buffer1 = that->_buffer1;
			_inputPtr = _buffer0.data();
			_outputPtr = _buffer1.data();
		}
		return *this;
	}

	~DoubleBuffer() {
		std::lock_guard<std::mutex> guard(_threadlock);
		_inputPtr = nullptr;
		_outputPtr = nullptr;
	}

	void push_back(T data) {
		if (_inputPtr != nullptr) {
			std::lock_guard<std::mutex> guard(_threadlock);
			_inputPtr->push_back(data);
		}
	}

	//write() {
	//	if (_inputPtr != nullptr) {
	//		std::lock_guard<std::mutex> guard(_threadlock);
	//		_inputPtr->push_back(data);
	//	}
	//}

	//T read() {
	//	swap();
	//	std::lock_guard<std::mutex> guard(_threadlock);
	//	if (_outputPtr.size() > 0) {
	//		return _outputPtr.at(0);
	//	}
	//	else {
	//		return nullptr;
	//	}
	//}

	vector<T> get() {
		swap();
		return *_outputPtr;
	}

	void get(vector<T> &output) {
		swap();
		output = *_outputPtr;
	}
};