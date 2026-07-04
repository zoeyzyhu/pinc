/*
 *  Copyright (C) 2026, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 *
 * The installed commux package currently exposes a PyTorch c10d C++ process
 * group rather than a C ABI suitable for PnetCDF's C core. Configure detection
 * and linkage live in this initial backend hook; the C-facing transport
 * implementation can grow here without changing the public ncmpix_* API.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <pnetcdf_comm.h>

const char*
ncmpix_comm_backend(void)
{
    return "commux";
}

int
ncmpix_create(PNC_Comm comm, const char *path, int cmode, PNC_Info info,
              int *ncidp)
{
    (void)comm;
    (void)path;
    (void)cmode;
    (void)info;

    if (ncidp != 0) *ncidp = -1;
    return NC_ENOTBUILT;
}

int
ncmpix_open(PNC_Comm comm, const char *path, int omode, PNC_Info info,
            int *ncidp)
{
    (void)comm;
    (void)path;
    (void)omode;
    (void)info;

    if (ncidp != 0) *ncidp = -1;
    return NC_ENOTBUILT;
}
