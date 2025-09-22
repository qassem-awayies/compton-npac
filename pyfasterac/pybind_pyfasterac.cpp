//
// Created by Mathieu GUIGUE on 16/09/2024.
//

#include "fastreader.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(pyfasterac, m) {
  m.doc() = "fasterac python module"; // optional module docstring
  m.def("fast_version", &fast_version, "A function which adds two numbers");

  py::class_<fastreader>(m, "fastreader")
      .def(py::init<const std::string &>())
      .def("get_next_event", &fastreader::get_next_event, "Get the next event in file")
      .def("get_event", &fastreader::get_event, "Get the event (group or individual)")
      .def("is_group", &fastreader::is_group, "Is event a group?")
      ;

  py::class_<event_data>(m, "event_data")
      .def_readwrite("label", &event_data::label)
      .def_readwrite("multiplicity", &event_data::multiplicity)
      .def_readwrite("time", &event_data::time)
      .def_readwrite("sub_events", &event_data::sub_events)
      ;

  py::class_<sub_event_data>(m, "sub_event_data")
      .def_readwrite("label", &sub_event_data::label)
      .def_readwrite("delta_t", &sub_event_data::delta_t)
      .def_readwrite("q", &sub_event_data::q)
      ;
}