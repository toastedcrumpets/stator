#include <stator/symbolic/symbolic.hpp>
#include <boost/numeric/interval.hpp> //filib++

namespace sym {
  using boost::numeric::interval;

  template<class Config = DefaultReprConfig, class ...Args>
  inline std::string repr(const sym::interval<Args...>& f)
  {
    return repr<Config>(f.lower()) + "..." + repr<Config>(f.upper());
  }

}