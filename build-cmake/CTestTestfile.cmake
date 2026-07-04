# CMake generated Testfile for 
# Source directory: /home/chengcli/scix/repos/pnetcdf
# Build directory: /home/chengcli/scix/repos/pnetcdf/build-cmake
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(commux_smoke "/home/chengcli/scix/repos/pnetcdf/build-cmake/commux_smoke")
set_tests_properties(commux_smoke PROPERTIES  _BACKTRACE_TRIPLES "/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;264;add_test;/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;0;")
add_test(commux_c_4proc "/home/chengcli/pyenv/bin/torchrun" "--nproc-per-node=4" "--no-python" "/home/chengcli/scix/repos/pnetcdf/build-cmake/commux_c_4proc")
set_tests_properties(commux_c_4proc PROPERTIES  _BACKTRACE_TRIPLES "/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;276;add_test;/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;0;")
add_test(ncmpix_cdf5_4proc "/home/chengcli/pyenv/bin/torchrun" "--nproc-per-node=4" "--no-python" "/home/chengcli/scix/repos/pnetcdf/build-cmake/ncmpix_cdf5_4proc")
set_tests_properties(ncmpix_cdf5_4proc PROPERTIES  _BACKTRACE_TRIPLES "/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;279;add_test;/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;0;")
add_test(commux_4proc "/home/chengcli/pyenv/bin/python3.11" "/home/chengcli/scix/repos/pnetcdf/test/commux_4proc.py")
set_tests_properties(commux_4proc PROPERTIES  SKIP_RETURN_CODE "77" _BACKTRACE_TRIPLES "/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;282;add_test;/home/chengcli/scix/repos/pnetcdf/CMakeLists.txt;0;")
