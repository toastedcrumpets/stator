#include <stator/string.hpp>
#include <stator/symbolic/ad.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
using namespace pybind11::literals;


py::object to_python(const sym::Expr&);
  
struct ToPythonVisitor : public sym::detail::VisitorHelper<ToPythonVisitor> {
  template<class T> sym::Expr apply(const T& v) {
    result = py::cast(sym::Expr(v));
    return sym::Expr();
  }

  sym::Expr apply(const sym::List& v) {
    py::list out;
    for (const auto& itm : v)
      out.append(to_python(itm));
    result = out;
    return sym::Expr();
  }

  //sym::Expr apply(const sym::Dict& v) {
  //  py::dict out;
  //  for (const auto& itm : v) {
  //    sym::Expr var = py::cast<py::str>(itm.first);
  //    out[py::cast(sym::Expr())] = to_python(itm.second);
  //  }
  //  result = out;
  //  return sym::Expr();
  //}
  
  sym::Expr apply(const double& v) {
    result = py::cast(v);
    return sym::Expr();
  }
  
  //template<> struct RT_type_index<VarRT>                                  { static const int value = 1;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Sine>>            { static const int value = 2;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Cosine>>          { static const int value = 3;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Log>>             { static const int value = 4;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Exp>>             { static const int value = 5;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Absolute>>        { static const int value = 6;  };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Arbsign>>         { static const int value = 7;  };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Add, Expr>>      { static const int value = 8;  };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Subtract, Expr>> { static const int value = 9;  };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Multiply, Expr>> { static const int value = 10; };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Divide, Expr>>   { static const int value = 11; };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Power, Expr>>    { static const int value = 12; };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Equality, Expr>> { static const int value = 13; };
  //template<> struct RT_type_index<BinaryOp<Expr, detail::Array, Expr>>    { static const int value = 14; };
  //template<> struct RT_type_index<List>                                   { static const int value = 15; };
  //template<> struct RT_type_index<Dict>                                   { static const int value = 16; };
  //template<> struct RT_type_index<UnaryOp<Expr, detail::Negate>>          { static const int value = 17;  };
  
  py::object result;
};

py::object to_python(const sym::Expr& b) {
  sym::Expr simple_b = simplify(b);
  ToPythonVisitor visitor;
  simple_b->visit(visitor);
  return visitor.result;
}

sym::Expr make_Expr(const py::dict& d) {
  auto out_ptr = sym::Dict::create();
  auto& out = *out_ptr;
  for (const auto& item : d) {
    sym::Expr key = py::cast<sym::Expr>(item.first);
    sym::Expr value = py::cast<sym::Expr>(item.second);
    out[key] = value;
  }
  return out;
}


PYBIND11_MODULE(core, m)
{
  
  py::class_<sym::Expr>(m, "Expr")
    .def(py::init<std::string>())
    .def(py::init<int>())
    .def(py::init<double>())
    .def(py::init([](const py::dict d){ return make_Expr(d); }))
    .def("__repr__", +[](const sym::Expr& self) { return "Expr('"+sym::repr(self)+"')"; })
    .def("__str__", +[](const sym::Expr& self) { return sym::repr(self); })
    .def("latex", +[](const sym::Expr& self) { return sym::repr<stator::ReprConfig<stator::Latex_output> >(self); })
    .def("simplify", +[](const sym::Expr& self) { return sym::simplify(self); })
    .def("__add__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l+r); })
    .def("__radd__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l+r); })
    .def("__sub__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l-r); })
    .def("__rsub__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l-r); })
    .def("__mul__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l*r); })
    .def("__rmul__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l*r); })
    .def("__div__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l/r); })
    .def("__rdiv__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l/r); })
    .def("__truediv__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l/r); })
    .def("__rtruediv__", +[](const sym::Expr& l, const sym::Expr& r) { return to_python(l/r); })
    .def("__eq__", +[](const sym::Expr& l, const sym::Expr& r) { return l == r; })
    .def(py::hash(py::self))
    .def("equal", +[](const sym::Expr& l, const sym::Expr& r) { return equality(l, r); })
    .def("debug_form", +[](const sym::Expr& self) { return "Expr('"+sym::repr<stator::ReprConfig<stator::Debug_output>>(self)+"')"; })
    .def("to_python", &to_python)
    ;

  m.def("derivative", static_cast<sym::Expr (*)(const sym::Expr&, const sym::Expr&)>(&sym::derivative));
  m.def("simplify", static_cast<sym::Expr (*)(const sym::Expr&)>(&sym::simplify));
  m.def("sub", static_cast<sym::Expr (*)(const sym::Expr&, const sym::Expr&)>(&sym::sub));
  
  py::implicitly_convertible<int, sym::Expr>();
  py::implicitly_convertible<double, sym::Expr>();
}
