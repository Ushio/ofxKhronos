#pragma once

#include <cmath>
#include <chrono>
#include <atomic>

inline std::chrono::high_resolution_clock::time_point std_now() {
	return std::chrono::high_resolution_clock::now();
}

class Khronos {
public:
	struct Clock {
		double elapsed = 0.0;
		double scale = 1.0;
		int64_t check_point_microseconds = 0;

		double now(int64_t actual_microseconds) const {
			auto delta_microseconds = actual_microseconds - check_point_microseconds;
			auto delta = double(delta_microseconds) * 0.001 * 0.001;
			return elapsed + delta * scale;
		}
	};
	static_assert(sizeof(Clock) == 8 * 3, "Clock");

	Khronos() {
		_clock = Clock();
		_launch = std_now();
	}

	int64_t actualMicroseconds(std::chrono::high_resolution_clock::time_point time_point = std_now()) const {
		return std::chrono::duration_cast<std::chrono::microseconds>(time_point - _launch).count();
	}
	double now() const {
		Clock clock = _clock.load();
		return clock.now(actualMicroseconds());
	}

	void setScale(double scale) {
		Clock expected;
		Clock new_clock;
		do {
			expected = _clock.load();

			if (expected.scale == scale) {
				return;
			}

			auto act = actualMicroseconds();
			new_clock.elapsed = expected.now(act);
			new_clock.check_point_microseconds = act;
			new_clock.scale = scale;
		} while (!_clock.compare_exchange_weak(expected, new_clock));
	}
	double getScale() const {
		return _clock.load().scale;
	}
	void revise(double offsetSeconds) {
		auto micro = static_cast<int64_t>(offsetSeconds * 1000.0 * 1000.0);

		Clock expected;
		Clock new_clock;
		do {
			expected = _clock.load();
			new_clock = expected;
			new_clock.elapsed += offsetSeconds;
		} while (!_clock.compare_exchange_weak(expected, new_clock));
	}
private:
	std::chrono::high_resolution_clock::time_point _launch;
	std::atomic<Clock> _clock;
};

/**
ts: クライアントがリクエストを送った時間
Tr: サーバーがリクエストを受信した時間
Ts: サーバーがリクエストを送信した時間
tr: クライアントがリクエスト受信した時間
*/
struct NTP {
	double ts = 0.0;
	double Tr = 0.0;
	double Ts = 0.0;
	double tr = 0.0;

	// 行きと帰りの差
	double diffBothWays() const {
		double going = Tr - ts;
		double backing = Ts - tr;
		return std::abs(going - backing);
	}

	// 遅延を計算 
	double delay() const {
		return (Ts + Tr) * 0.5 - (ts + tr) * 0.5;
	}
};
