# pinc

[![Continuous Integration](https://github.com/zoeyzyhu/pinc/actions/workflows/ci.yml/badge.svg)](https://github.com/zoeyzyhu/pinc/actions/workflows/ci.yml)

**pinc** is a parallel NetCDF I/O library that speaks
[Unidata's NetCDF](https://www.unidata.ucar.edu/software/netcdf) classic
formats (CDF-1/2/5) over a **UCX** transport instead of MPI. It is a fork of
[PnetCDF](https://parallel-netcdf.github.io) whose communication layer has been
abstracted so that ranks coordinate through
[**commux**](https://github.com/zoeyzyhu/commux) — a UCX tag-matching
`c10d` backend for `torch.distributed` — rather than MPI-IO. This lets NetCDF
parallel I/O run inside the PyTorch / `torch.distributed` (snapy) ecosystem
without an MPI runtime.

pinc is packaged as a CPU-only, Linux-only Python wheel (UCX builds only on
Linux) and installs alongside `commux` and `torch`.

## Relationship to commux

pinc does not vendor a transport of its own. Its comm backend is built on
commux:

```
NetCDF classic I/O  (PnetCDF core)
        |  generic PINC_Comm / PINC_Info / PINC_Datatype / PINC_Offset
        v
src/include/pnetcdf_comm.h(.in)      the backend seam (mpi | commux)
src/include/pnetcdf_commux.h         small C ABI (pinc_commux_* / PINC_COMMUX_*)
        v
src/comm/pinc_commux_runtime.cpp     owns a c10d TCPStore + commux ProcessGroupUCX
        v
libcommux (+ bundled UCX) and libtorch   from the installed commux / torch
```

At build time pinc's CMake locates the installed `commux` and `torch` Python
packages (plus system UCX headers and NetCDF). The wheel is thin: it links
`libcommux`/`libtorch` at runtime via `$ORIGIN`-relative rpaths into the
sibling `commux/` and `torch/` site-packages and vendors only `libnetcdf`.
`import pinc` imports `torch` first so `libtorch` is mapped before `libcommux`
loads.

torch is a hard dependency: `pinc_commux_runtime.cpp` uses `torch`/`c10d`
(`c10d::TCPStore`, `c10d::ReduceOp`, `commux::ProcessGroupUCX`) directly. pinc
is CPU-only, so the CPU build of torch is sufficient.

## Install

```console
pip install pinc
```

This pulls in `commux` and `torch` (CPU). Linux x86_64, Python 3.10–3.13.

## Python usage

```python
import pinc

pinc.init_env("commux")          # initialise the commux runtime from the env
print(pinc.rank(), pinc.size())
pinc.barrier()
total = pinc.allreduce_int64(pinc.rank(), pinc.ReduceOp.SUM)
pinc.finalize()

# packaged C headers / libs for building C consumers
print(pinc.include_dir, pinc.library_dir)
```

The Python module is a thin binding over the `pinc_commux_*` C ABI declared in
`src/include/pnetcdf_commux.h`.

## Build from source

Building the wheel needs the `commux` and `torch` packages importable (CMake
reads them), plus system UCX headers (`ucp/api/ucp.h`) and NetCDF:

```console
# Debian/Ubuntu
sudo apt-get install -y libucx-dev libnetcdf-dev pkg-config
pip install 'torch==2.10.0' --index-url https://download.pytorch.org/whl/cpu
pip install 'commux==0.2.1'

git clone https://github.com/zoeyzyhu/pinc.git
cd pinc
# --no-build-isolation so CMake can import commux/torch
pip install --no-build-isolation --no-deps .
```

The build is driven by `scikit-build-core`; select the backend with the CMake
option `PNETCDF_COMM_BACKEND` (`commux`, `mpi`, or `auto`).

## CI / CD / releases

* **ci.yml** — pre-commit lint checks and a Linux build + import smoke test on
  every push / PR to `main`.
* **cd.yml** — on PR merge, computes the next version from the latest tag (plus
  optional `release:major|minor|patch` labels), pushes the tag, and cuts a
  GitHub Release. No version-bump commit — the version comes from the git tag
  via `setuptools-scm`.
* **release.yml** — a manual *Publish to PyPI* run for a given tag that builds
  Linux wheels (cibuildwheel) and uploads them.

## Acknowledgements

pinc is derived from **PnetCDF**, a collaborative work of Northwestern
University and Argonne National Laboratory
([project page](https://parallel-netcdf.github.io),
[repository](https://github.com/Parallel-NetCDF/PnetCDF)). The upstream PnetCDF
license and copyright apply to the inherited sources; see
[COPYRIGHT](./COPYRIGHT) and [COPYING](./COPYING). PnetCDF has been supported by
the U.S. Department of Energy's Office of Science (SciDAC) and the Exascale
Computing Project.
