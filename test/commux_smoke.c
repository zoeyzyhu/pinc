/*
 * Simple smoke test for the CMake commux-only build.
 *
 * This test intentionally uses only pnetcdf_comm.h. In commux mode that header
 * must not require mpi.h or the legacy MPI-shaped pnetcdf.h API.
 */

#include <pnetcdf_comm.h>

#include <stdio.h>
#include <string.h>

static int
expect_true(int cond, const char *msg)
{
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 1;
    }
    return 0;
}

int
main(void)
{
    int nerrs = 0;
    int ncid = 1234;
    int err;

    nerrs += expect_true(PNETCDF_COMM_BACKEND == PNETCDF_COMM_BACKEND_COMMUX,
                         "expected commux communication backend");
    nerrs += expect_true(PNETCDF_ENABLE_COMMUX == 1,
                         "expected commux support to be enabled");
    nerrs += expect_true(strcmp(ncmpix_comm_backend(), "commux") == 0,
                         "backend name should be commux");

    err = ncmpix_create(PNC_COMM_SELF, "commux-smoke.nc", 0, PNC_INFO_NULL,
                        &ncid);
    nerrs += expect_true(err == NC_ENOTBUILT,
                         "ncmpix_create should report NC_ENOTBUILT for stub");
    nerrs += expect_true(ncid == -1,
                         "ncmpix_create should reset ncid to -1");

    ncid = 1234;
    err = ncmpix_open(PNC_COMM_SELF, "commux-smoke.nc", 0, PNC_INFO_NULL,
                      &ncid);
    nerrs += expect_true(err == NC_ENOTBUILT,
                         "ncmpix_open should report NC_ENOTBUILT for stub");
    nerrs += expect_true(ncid == -1,
                         "ncmpix_open should reset ncid to -1");

    if (nerrs == 0)
        printf("PASS: commux smoke test\n");

    return nerrs == 0 ? 0 : 1;
}
