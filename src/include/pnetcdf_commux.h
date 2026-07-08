/*
 *  Copyright (C) 2026, Northwestern University and Argonne National Laboratory
 *  See COPYRIGHT notice in top-level directory.
 *
 * A small C ABI for the installed Python commux package.
 */

#ifndef H_PNETCDF_COMMUX
#define H_PNETCDF_COMMUX

#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_DLL
#define PINC_COMMUX_API __declspec(dllexport)
#else
#define PINC_COMMUX_API __declspec(dllimport)
#endif
#else
#if __GNUC__ >= 4
#define PINC_COMMUX_API __attribute__((visibility("default")))
#else
#define PINC_COMMUX_API
#endif
#endif

#define PINC_COMMUX_SUCCESS 0
#define PINC_COMMUX_ERR_RUNTIME -1
#define PINC_COMMUX_ERR_ARG -2

typedef enum PINC_CommuxReduceOp {
  PINC_COMMUX_SUM = 0,
  PINC_COMMUX_MIN = 1,
  PINC_COMMUX_MAX = 2
} PINC_CommuxReduceOp;

#if defined(__cplusplus)
extern "C" {
#endif

PINC_COMMUX_API const char *pinc_commux_last_error(void);

PINC_COMMUX_API int pinc_commux_init(const char *backend,
                                     const char *init_method, int rank,
                                     int world_size);

PINC_COMMUX_API int pinc_commux_init_env(const char *backend);

PINC_COMMUX_API int pinc_commux_finalize(void);

PINC_COMMUX_API int pinc_commux_rank(int *rankp);

PINC_COMMUX_API int pinc_commux_size(int *sizep);

PINC_COMMUX_API int pinc_commux_barrier(void);

PINC_COMMUX_API int pinc_commux_allreduce_int64(int64_t value,
                                                PINC_CommuxReduceOp op,
                                                int64_t *resultp);

PINC_COMMUX_API int pinc_commux_send_int64(int64_t value, int dst, int tag);

PINC_COMMUX_API int pinc_commux_recv_int64(int src, int tag, int64_t *valuep);

PINC_COMMUX_API int pinc_commux_send_bytes(const void *data, int64_t nbytes,
                                           int dst, int tag);

PINC_COMMUX_API int pinc_commux_recv_bytes(void *data, int64_t nbytes, int src,
                                           int tag);

PINC_COMMUX_API int pinc_commux_file_open(const char *path, int flags, int mode,
                                          int64_t *handlep);

PINC_COMMUX_API int pinc_commux_file_close(int64_t handle);

PINC_COMMUX_API int pinc_commux_file_pwrite(int64_t handle, const void *data,
                                            int64_t nbytes, int64_t offset);

PINC_COMMUX_API int pinc_commux_file_sync(int64_t handle);

#if defined(__cplusplus)
}
#endif

#endif /* H_PNETCDF_COMMUX */
