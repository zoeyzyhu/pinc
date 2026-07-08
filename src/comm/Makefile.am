#
# Copyright (C) 2026, Northwestern University and Argonne National Laboratory
# See COPYRIGHT notice in top-level directory.
#
# @configure_input@

AM_CPPFLAGS  = -I${top_srcdir}/src/include
AM_CPPFLAGS += -I${top_builddir}/src/include
AM_CPPFLAGS += @COMMUX_CPPFLAGS@

AM_CFLAGS = @C_VISIBILITY_FLAGS@

noinst_LTLIBRARIES = libpinccomm.la

if COMM_BACKEND_COMMUX
libpinccomm_la_SOURCES = pinc_comm_commux.c
else
libpinccomm_la_SOURCES = pinc_comm_mpi.c
endif

EXTRA_DIST = pinc_comm_mpi.c pinc_comm_commux.c pinc_commux_runtime.cpp

tests-local: all
