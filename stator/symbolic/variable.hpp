#pragma once

#include <type_traits>
#include <stator/hash.hpp>

namespace sym {
  struct VarBase {};

  typedef const char* conststr;
  
  constexpr char default_var_name[] = "x";
  
  /*!\brief Symbolic representation of a variable.

    This class is used to denote a variable.
  */
  template<conststr name = default_var_name, typename ...Args>
  struct Var : SymbolicOperator<Var<name, Args...>>, VarBase {
    template<class Arg>
    auto operator=(const Arg& a) const {
      return equality(*this, a);
    }
    
    template<const char* name2, typename ...Args2>
    constexpr bool operator==(const Var<name2, Args2...>&) const { return std::string_view(name) ==  name2; }

    template<const char* name2, typename ...Args2>
    constexpr bool operator!=(const Var<name2, Args2...>&) const { return std::string_view(name) !=  name2; }
    
    static constexpr std::string_view getName() { return name; }
  };

  template<typename Var1, typename Var2>
  struct variable_eq {
    static constexpr const bool value = false;
  };

  template<auto N1, auto N2, typename ...Args1, typename ...Args2>
  struct variable_eq<Var<N1, Args1...>, Var<N2, Args2...> > {
    static constexpr const bool value = Var<N1, Args1...>() == Var<N2, Args2...>();
  };
  
  template<typename Var1, typename Var2>
  struct enable_if_var_eq : std::enable_if<variable_eq<Var1, Var2>::value> {
  };
  
  template<typename Var1, typename Var2>
  struct enable_if_var_not_eq : std::enable_if<!variable_eq<Var1, Var2>::value> {
  };
  
  /*! \brief Determine the derivative of a symbolic expression.
    
    This default implementation gives all consants
    derivative of zero.
  */
  template<class T, auto N, typename ...Args,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
  Null derivative(const T&, const Var<N, Args...>&) { return Null(); }

  /*! \brief Determine the derivative of a variable by another variable.
  */
  template<auto N1, auto N2, class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!IsDynamic<Var<N1, Args1...> >::value && !IsDynamic<Var<N2, Args2...>>::value>::type>
  auto derivative(Var<N1, Args1...> x1, Var<N2, Args2...> x2)
  {
    if constexpr (x1 == x2)
      return Unity();
    else
      return Null();
  }

  template<class Config = DefaultReprConfig, auto N, class ...Args>
  inline std::string repr(const sym::Var<N, Args...>& v) {
    if (Config::Debug_output)
      return "Var<\"" + std::string(v.getName()) + "\">()";
    return std::string(v.getName());
  }

  /*! \brief Returns the binding powers for variables.

    Arguments that cannot be split, like variables have very high
    binding powers internally as they cannot be split.
  */
  template<auto N, class ...Args>
  std::pair<int, int> BP(const Var<N, Args...>& v)
  { return std::make_pair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()); }

  namespace detail {
    template<auto N, class ...Args>
    struct Type_index<Var<N, Args...> > { static const int value = 1;  };
  }
}

namespace std
{
  template<auto N, class ...Args> struct hash<sym::Var<N, Args...> >
  {
    std::size_t operator()(sym::Var<N, Args...> const& v) const noexcept
    {
      std::size_t seed = sym::detail::Type_index<sym::Var<N, Args...>>::value;
      stator::hash_combine(seed, std::hash<std::string>{}(v.getName()));
      return seed;
    }
  };
}
