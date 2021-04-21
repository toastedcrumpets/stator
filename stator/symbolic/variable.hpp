#pragma once

namespace sym {
    namespace detail {
    struct vidx_ID;
  };

  template<char idx>
  struct vidx : stator::orphan::value_conf_t<detail::vidx_ID, char, idx> {};

  /*!\brief Symbolic representation of a variable.

    This class is used to denote a variable.
  */
  template<typename ...Args> struct Var : SymbolicOperator {
    static constexpr const auto idx = stator::orphan::get_value<vidx<'x'>, Args...>::value;

    template<class Arg>
    auto operator=(const Arg& a) const -> STATOR_AUTORETURN(equal(*this, a));

    template<typename ...Args2>
    bool operator==(const Var<Args2...>& a) const { return Var::idx == Var<Args2...>::idx; }
    
    std::string getID() const { return std::string(1, Var::idx); }
  };

  template<typename Var1, typename Var2>
  struct variable_in {
    static constexpr const bool value = Var1::idx == Var2::idx;
  };

  template<typename Var1, typename Var2>
  struct enable_if_var_in : std::enable_if<variable_in<Var1, Var2>::value> {
  };
  
  template<typename Var1, typename Var2>
  struct enable_if_var_not_in : std::enable_if<!variable_in<Var1, Var2>::value> {
  };

  template<typename Var1, typename Var2>
  struct variable_combine {
    typedef Var<vidx<Var1::idx> > type;
  };

  /*! \brief Determine the derivative of a symbolic expression.
    
    This default implementation gives all consants
    derivative of zero.
  */
  template<class T, class ...Args,
	     typename = typename std::enable_if<detail::IsConstant<T>::value>::type>
  Null derivative(const T&, Var<Args...>) { return Null(); }

  /*! \brief Determine the derivative of a variable.

    If the variable is the variable in which a derivative is being
    taken, then this overload should be selected to return
    Unity.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!IsDynamic<Var<Args1...> >::value && !IsDynamic<Var<Args2...>>::value>::type>
  auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>::idx == Var<Args2...>::idx, Unity>::type
  { return Unity(); }

  /*! \brief Determine the derivative of a variable by another variable.

    If the variable is NOT the variable in which a derivative is
    being taken, then this overload should be selected to return
    Null.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!IsDynamic<Var<Args1...> >::value && !IsDynamic<Var<Args2...>>::value>::type>
    auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>::idx != Var<Args2...>::idx, Null>::type
  { return Null(); }  
}
