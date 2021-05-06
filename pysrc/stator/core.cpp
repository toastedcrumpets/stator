#include <stator/string.hpp>
#include <stator/symbolic/ad.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(core, m)
{
  py::class_<sym::Expr>(m, "Expr")
    .def(py::init<std::string>())
    .def(py::init<int>())
    .def(py::init<double>())
    .def("__repr__", +[](const sym::Expr& self) { return "Expr('"+sym::repr(self)+"')"; })
    .def("__str__", +[](const sym::Expr& self) { return sym::repr(self); })
    .def("latex", +[](const sym::Expr& self) { return sym::repr<stator::ReprConfig<stator::Latex_output> >(self); })
    .def("simplify", +[](const sym::Expr& self) { return sym::simplify(self); })
    .def("__add__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l+r); })
    .def("__radd__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l+r); })
    .def("__sub__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l-r); })
    .def("__rsub__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l-r); })
    .def("__mul__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l*r); })
    .def("__rmul__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l*r); })
    .def("__div__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l/r); })
    .def("__rdiv__", +[](const sym::Expr& l, const sym::Expr& r) { return sym::Expr(l/r); })
    .def("__eq__", +[](const sym::Expr& l, const sym::Expr& r) { return l == r; })
    .def("debug_form", +[](const sym::Expr& self) { return "Expr('"+sym::repr<stator::ReprConfig<stator::Debug_output>>(self)+"')"; }) 
    .def(py::self / py::self)
    ;

  m.def("derivative", static_cast<sym::Expr (*)(const sym::Expr&, const sym::Expr&)>(&sym::derivative));
  m.def("simplify", static_cast<sym::Expr (*)(const sym::Expr&)>(&sym::simplify));
  m.def("subs", static_cast<sym::Expr (*)(const sym::Expr&)>(&sym::simplify));
  
  py::implicitly_convertible<int, sym::Expr>();
  py::implicitly_convertible<double, sym::Expr>();
}
