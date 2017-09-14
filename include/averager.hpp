#ifndef AVERAGER_HPP
#define AVERAGER_HPP

template<typename T>
class Averager {
 public: 
	Averager()
   :m_average{}
   ,m_max{}
   ,m_count{0}
  {}

  void add(T const& val) {
    m_average = (m_average * T(m_count) + val) / T(m_count + 1);
    m_count += 1;
    if (val > m_max) {
      m_max = val;
    }
  }

  T get() const {
    return m_average;
  }

  T max() const {
    return m_max;
  }

  private:
  T m_average;
  T m_max;
  unsigned long m_count;
};

#endif
