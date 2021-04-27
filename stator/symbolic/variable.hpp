#pragma once

#include <type_traits>


namespace sym {
  namespace detail {

    namespace varname_detail {
      /*! \brief Converts a series of template argument digits
       (i.e. <1,3,2,4>) into a const char array with the string
       representation (i.e. "1324").
      */
      template<uint8_t... digits> struct to_chars { static const char value[]; };
      
      /*! \brief The actual implementation of the conversion. */
      template<uint8_t... digits> constexpr char to_chars<digits...>::value[] = {'X', ('0' + digits)..., 0};

      /*! \brief Recursively expand all digits into a template base 10 representation.
       */
      template<uintmax_t rem, uint8_t... digits>
      struct explode : explode<rem / 10, rem % 10, digits...> {};

      /*! \brief Halt the recursive base10 expansion once we run out
          of digits and hand it over to to_chars.
       */
      template<uint8_t... digits>
      struct explode<0, digits...> : to_chars<digits...> {};

      template<char v> struct char_to_string { static const char value[]; };
      template<char v> constexpr char char_to_string<v>::value[] = {v, 0};
    }
      
    /*! \brief Generates a string that can be used as a variable name. 
      
      This default implementation takes the passed char as the name.
     */
    template<size_t idx, char nom>
    struct varname : public varname_detail::char_to_string<nom> {};

    /*! \brief Generates a string that can be used as a variable name. 
      
      If the user doesn't supply a character this specialisation will
      be applied a single alphabetical name, a-Z, IFF the variable
      index is in the ASCII range, otherwise it gives X123 where 123
      is the variable index.
     */
    template<size_t v>
    struct varname<v, std::numeric_limits<char>::max()> :
      std::conditional<((v >= 'A') && (v <= 'Z')) || ((v >= 'a') && (v <= 'z')), varname_detail::char_to_string<v>, varname_detail::explode<v> >::type
    {};
    
    //! \brief The template argument tag for variable indexes.
    struct vidx_ID;
    //! \brief The template argument tag for variable character name (if used).
    struct vnom_ID;
  };

  /*! \brief Configuration template argument for \ref Var. Used to
      specify the variable index.
   */
  template<size_t idx>
  struct vidx : stator::orphan::value_conf_t<detail::vidx_ID, size_t, idx> {};

  /*! \brief Configuration template argument for \ref Var. Used to
      specify the variable index.
   */
  template<char nom>
  struct vnom : stator::orphan::value_conf_t<detail::vnom_ID, char, nom> {};
  
  /*!\brief Symbolic representation of a variable.

    This class is used to denote a variable.
  */
  template<typename ...Args> struct Var : SymbolicOperator {
    
    template<class Arg>
    auto operator=(const Arg& a) const -> STATOR_AUTORETURN(equal(*this, a));
    
    template<typename ...Args2>
    constexpr bool operator==(const Var<Args2...>& a) const { return getID() == a.getID(); }

    template<typename ...Args2>
    constexpr bool operator!=(const Var<Args2...>& a) const { return !(getID() == a.getID()); }
    
    constexpr size_t getID() const { return idx; }
    static constexpr const char* getName() { return name; }

  private:
    template<typename Var1, typename Var2>
    friend struct variable_in;
    
    static constexpr const auto idx = stator::orphan::get_value<vidx<'x'>, Args...>::value;
    static constexpr const auto nom = stator::orphan::get_value<vnom<std::numeric_limits<char>::max()>, Args...>::value;
    static constexpr const auto name = detail::varname<idx, nom>::value;
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
    typedef Var1 type;
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
  auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>() == Var<Args2...>(), Unity>::type
  { return Unity(); }

  /*! \brief Determine the derivative of a variable by another variable.

    If the variable is NOT the variable in which a derivative is
    being taken, then this overload should be selected to return
    Null.
  */
  template<class ...Args1, class ...Args2,
	   typename = typename std::enable_if<!IsDynamic<Var<Args1...> >::value && !IsDynamic<Var<Args2...>>::value>::type>
  auto derivative(Var<Args1...>, Var<Args2...>) -> typename std::enable_if<Var<Args1...>() != Var<Args2...>(), Null>::type
  { return Null(); }  

  template<class Config = DefaultReprConfig, class ...Args>
  inline std::string repr(const sym::Var<Args...>& v) {
    return v.getName();
  }
}
