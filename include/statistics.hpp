#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include "averager.hpp"
#include "timer.hpp"

#include <map>

class Statistics {
 public:
  Statistics()
  {}

  void addTimer(std::string const& name) {
    m_timers[name] = Timer{};
    addAverager(name);
  }

  void addAverager(std::string const& name) {
    m_averages[name] = Averager<double>{};
  }

  void start(std::string const& name) {
    m_timers.at(name).start();
  }

  void stop(std::string const& name) {
    m_averages.at(name).add(m_timers.at(name).durationEnd());
  }
  double stopValue(std::string const& name) {
    return m_timers.at(name).durationEnd();
  }

  void add(std::string const& name, double value) {
    m_averages.at(name).add(value);
  }

  double get(std::string const& name) {
    return m_averages.at(name).get();
  }

 private:
  std::map<std::string, Averager<double>> m_averages; 
  std::map<std::string, Timer> m_timers; 
};

#endif