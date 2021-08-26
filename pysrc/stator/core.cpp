#include <stator/string.hpp>
#include <stator/symbolic/ad.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
using namespace pybind11::literals;


py::object to_python(const sym::Expr&);
  
struct ToPythonVisitor : public sym::detail::VisitorHelper<ToPythonVisitor, py::object> {
  template<class T>
  py::object apply(const T& v) {
    return py::cast(sym::Expr(v));
  }

  py::object apply(const sym::ArrayRT& v) {
    py::list out;
    for (const auto& itm : v)
      out.append(to_python(itm));
    return std::move(out);
  }

  py::object apply(const sym::Dict& v) {
    py::dict out;
    for (const auto& itm : v)
      out[to_python(itm.first)] = to_python(itm.second);
    return std::move(out);
  }
  
  py::object apply(const double& v) {
    return py::cast(v);
  }
  
  py::object result;
};

py::object to_python(const sym::Expr& b) {
  sym::Expr simple_b = simplify(b);
  ToPythonVisitor visitor;
  return simple_b->visit(visitor);
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

sym::Expr make_Expr(const py::list& l) {
  auto out_ptr = sym::ArrayRT::create();
  auto& out = *out_ptr;
  for (const auto& item : l)
    out.push_back(py::cast<sym::Expr>(item));
  return out;
}

PYBIND11_MODULE(core, m)
{
  
  py::class_<sym::Expr>(m, "Expr")
    .def(py::init<std::string>())
    .def(py::init<int>())
    .def(py::init<double>())
    .def(py::init([](const py::dict d){ return make_Expr(d); }))
    .def(py::init([](const py::list l){ return make_Expr(l); }))
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

  m.def("derivative", +[](const sym::Expr& l, const sym::Expr& r){ return sym::simplify(sym::derivative(l, r)); });
  m.def("simplify", static_cast<sym::Expr (*)(const sym::Expr&)>(&sym::simplify));
  m.def("sub", +[](const sym::Expr& l, const sym::Expr& r){ return to_python(sym::sub(l, r)); });
  m.def("sub", +[](const sym::Expr& l, const py::dict& r){ return to_python(sym::sub(l, make_Expr(r))); });
  
  py::implicitly_convertible<int, sym::Expr>();
  py::implicitly_convertible<double, sym::Expr>();
}
