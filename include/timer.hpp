#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

// template<typename T_precision = std::chrono::nanoseconds>
using namespace std::chrono;
class Timer {
 public: 
	Timer()
   :m_start{}
   ,m_end{}
  {}

  void start() {
    m_start = high_resolution_clock::now();
  }

  void end() {
    m_end = high_resolution_clock::now();
  }
  
  // in ms
  double duration() const {
    auto time = duration_cast<nanoseconds>(m_end - m_start);
    return double(time.count()) / 1000.0 / 1000.0;
  }

  // in ms
  double durationEnd() {
    end();
    auto time = duration_cast<nanoseconds>(m_end - m_start);
    return double(time.count()) / 1000.0 / 1000.0;
  }

  private:
  time_point<high_resolution_clock> m_start;
  time_point<high_resolution_clock> m_end;
};

#endif