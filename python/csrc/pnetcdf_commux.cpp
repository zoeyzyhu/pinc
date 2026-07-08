#include <pnetcdf_commux.h>
#include <pybind11/pybind11.h>

#include <stdexcept>

namespace py = pybind11;

namespace {

void check(int err) {
  if (err != PINC_COMMUX_SUCCESS)
    throw std::runtime_error(pinc_commux_last_error());
}

}  // namespace

PYBIND11_MODULE(_pnetcdf_commux, m) {
  m.doc() = "Thin Python binding for the PnetCDF commux C ABI";

  py::enum_<PINC_CommuxReduceOp>(m, "ReduceOp")
      .value("SUM", PINC_COMMUX_SUM)
      .value("MIN", PINC_COMMUX_MIN)
      .value("MAX", PINC_COMMUX_MAX);

  m.def("init", [](const char* backend, const char* init_method, int rank,
                   int world_size) {
    check(pinc_commux_init(backend, init_method, rank, world_size));
  });
  m.def(
      "init_env",
      [](const char* backend) { check(pinc_commux_init_env(backend)); },
      py::arg("backend") = "ucx");
  m.def("finalize", []() { check(pinc_commux_finalize()); });
  m.def("rank", []() {
    int rank = -1;
    check(pinc_commux_rank(&rank));
    return rank;
  });
  m.def("size", []() {
    int size = -1;
    check(pinc_commux_size(&size));
    return size;
  });
  m.def("barrier", []() { check(pinc_commux_barrier()); });
  m.def("allreduce_int64", [](int64_t value, PINC_CommuxReduceOp op) {
    int64_t result = 0;
    check(pinc_commux_allreduce_int64(value, op, &result));
    return result;
  });
  m.def("send_int64", [](int64_t value, int dst, int tag) {
    check(pinc_commux_send_int64(value, dst, tag));
  });
  m.def("recv_int64", [](int src, int tag) {
    int64_t value = 0;
    check(pinc_commux_recv_int64(src, tag, &value));
    return value;
  });
}
