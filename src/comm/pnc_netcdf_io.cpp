/*
 * Minimal UCX-coordinated CDF-5 file API for snapy pnetcdf output.
 */

#include <pnetcdf_comm.h>
#include <pnetcdf_commux.h>

#include <netcdf.h>

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace {

struct FileState {
  int ncid = -1;
  int rank = 0;
  int size = 1;
  int next_dimid = 0;
  int next_varid = 0;
  int next_reqid = 1;
  std::map<int, int> req_status;
  std::string path;
  bool define_mode = false;
};

std::map<int, FileState> g_files;
int g_next_handle = 1000;

int ensure_comm(void) {
  int rank = 0;
  int err = pnc_commux_rank(&rank);
  if (err == PNC_COMMUX_SUCCESS) return NC_NOERR;
  if (std::getenv("RANK") == nullptr && std::getenv("WORLD_SIZE") == nullptr)
    return NC_NOERR;
  err = pnc_commux_init_env("ucx");
  if (err == PNC_COMMUX_SUCCESS) return NC_NOERR;
  return NC_EINVAL;
}

int rank(void) {
  int r = 0;
  if (pnc_commux_rank(&r) != PNC_COMMUX_SUCCESS) return 0;
  return r;
}

int size(void) {
  int s = 1;
  if (pnc_commux_size(&s) != PNC_COMMUX_SUCCESS) return 1;
  return s;
}

int barrier(void) {
  if (size() <= 1) return NC_NOERR;
  return pnc_commux_barrier() == PNC_COMMUX_SUCCESS ? NC_NOERR : NC_EINVAL;
}

FileState* file_state(int ncid) {
  auto it = g_files.find(ncid);
  if (it == g_files.end()) return nullptr;
  return &it->second;
}

std::vector<size_t> to_size_t(const PNC_Offset* values, int n) {
  std::vector<size_t> out(n);
  for (int i = 0; i < n; ++i) out[i] = static_cast<size_t>(values[i]);
  return out;
}

int var_ndims(int ncid, int varid) {
  int ndims = 0;
  if (nc_inq_varndims(ncid, varid, &ndims) != NC_NOERR) return 0;
  return ndims;
}

}  // namespace

extern "C" {

const char* ncmpix_strerror(int err) {
  if (err == NC_ENOTBUILT) return "feature was not built";
  return nc_strerror(err);
}

int ncmpix_create(PNC_Comm, const char* path, int cmode, PNC_Info, int* ncidp) {
  if (ncidp == nullptr) return NC_EINVAL;
  int err = ensure_comm();
  if (err != NC_NOERR) return err;

  FileState st;
  st.rank = rank();
  st.size = size();
  st.path = path != nullptr ? path : "";
  st.define_mode = true;
  if (st.path.empty()) return NC_EINVAL;

  if (st.rank == 0) {
    int mode = NC_CDF5;
    if (cmode & NC_NOCLOBBER)
      mode |= NC_NOCLOBBER;
    else
      mode |= NC_CLOBBER;
    err = nc_create(st.path.c_str(), mode, &st.ncid);
    if (err != NC_NOERR) return err;
    int old_fill = 0;
    nc_set_fill(st.ncid, NC_NOFILL, &old_fill);
  }
  if ((err = barrier()) != NC_NOERR) return err;

  int handle = g_next_handle++;
  g_files[handle] = st;
  *ncidp = handle;
  return NC_NOERR;
}

int ncmpix_open(PNC_Comm, const char* path, int omode, PNC_Info, int* ncidp) {
  if (ncidp == nullptr || path == nullptr) return NC_EINVAL;
  int err = ensure_comm();
  if (err != NC_NOERR) return err;

  FileState st;
  st.rank = rank();
  st.size = size();
  st.path = path;
  err = nc_open(path, omode, &st.ncid);
  if (err != NC_NOERR) return err;

  int handle = g_next_handle++;
  g_files[handle] = st;
  *ncidp = handle;
  return NC_NOERR;
}

int ncmpix_close(int ncid) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int err = NC_NOERR;
  if (st->ncid >= 0) err = nc_close(st->ncid);
  g_files.erase(ncid);
  int berr = barrier();
  return err != NC_NOERR ? err : berr;
}

int ncmpix_def_dim(int ncid, const char* name, PNC_Offset len, int* idp) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int id = st->next_dimid++;
  int err = NC_NOERR;
  if (st->rank == 0)
    err = nc_def_dim(st->ncid, name, static_cast<size_t>(len), &id);
  if (idp != nullptr) *idp = id;
  return err;
}

int ncmpix_def_var(int ncid, const char* name, nc_type xtype, int ndims,
                   const int* dimids, int* varidp) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int id = st->next_varid++;
  int err = NC_NOERR;
  if (st->rank == 0)
    err = nc_def_var(st->ncid, name, xtype, ndims, dimids, &id);
  if (varidp != nullptr) *varidp = id;
  return err;
}

int ncmpix_put_att_text(int ncid, int varid, const char* name, PNC_Offset len,
                        const char* value) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  if (st->rank != 0) return NC_NOERR;
  return nc_put_att_text(st->ncid, varid, name, static_cast<size_t>(len), value);
}

int ncmpix_put_att_int(int ncid, int varid, const char* name, nc_type xtype,
                       PNC_Offset len, const int* value) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  if (st->rank != 0) return NC_NOERR;
  return nc_put_att_int(st->ncid, varid, name, xtype, static_cast<size_t>(len),
                        value);
}

int ncmpix_put_att_float(int ncid, int varid, const char* name, nc_type xtype,
                         PNC_Offset len, const float* value) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  if (st->rank != 0) return NC_NOERR;
  return nc_put_att_float(st->ncid, varid, name, xtype, static_cast<size_t>(len),
                          value);
}

int ncmpix_enddef(int ncid) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int err = NC_NOERR;
  if (st->rank == 0) {
    err = nc_enddef(st->ncid);
    if (err == NC_NOERR) err = nc_close(st->ncid);
    st->ncid = -1;
  }
  if (err != NC_NOERR) return err;
  if ((err = barrier()) != NC_NOERR) return err;
  err = nc_open(st->path.c_str(), NC_WRITE, &st->ncid);
  if (err != NC_NOERR) return err;
  st->define_mode = false;
  return barrier();
}

int ncmpix_put_vara_float_all(int ncid, int varid, const PNC_Offset* start,
                              const PNC_Offset* count, const float* value) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int err = NC_NOERR;
  if (st->rank == 0) {
    int ndims = var_ndims(st->ncid, varid);
    auto s = to_size_t(start, ndims);
    auto c = to_size_t(count, ndims);
    err = nc_put_vara_float(st->ncid, varid, s.data(), c.data(), value);
  }
  int berr = barrier();
  return err != NC_NOERR ? err : berr;
}

int ncmpix_iput_vara_float(int ncid, int varid, const PNC_Offset* start,
                           const PNC_Offset* count, const float* value,
                           int* reqidp) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int ndims = var_ndims(st->ncid, varid);
  auto s = to_size_t(start, ndims);
  auto c = to_size_t(count, ndims);
  int req = st->next_reqid++;
  int meta_tag = 9000 + req * 2;
  int data_tag = meta_tag + 1;
  PNC_Offset nvals = 1;
  for (int i = 0; i < ndims; ++i) nvals *= count[i];
  PNC_Offset nbytes = nvals * static_cast<PNC_Offset>(sizeof(float));
  int status = NC_NOERR;
  if (st->rank == 0) {
    status = nc_put_vara_float(st->ncid, varid, s.data(), c.data(), value);
    for (int r = 1; r < st->size; ++r) {
      std::vector<PNC_Offset> meta(1 + 2 * ndims);
      if (pnc_commux_recv_bytes(meta.data(),
                                static_cast<int64_t>(meta.size() *
                                                     sizeof(PNC_Offset)),
                                r, meta_tag) != PNC_COMMUX_SUCCESS) {
        status = NC_EINVAL;
        continue;
      }
      int remote_ndims = static_cast<int>(meta[0]);
      if (remote_ndims != ndims) {
        status = NC_EINVAL;
        continue;
      }
      std::vector<size_t> rs(ndims), rc(ndims);
      PNC_Offset remote_vals = 1;
      for (int i = 0; i < ndims; ++i) {
        rs[i] = static_cast<size_t>(meta[1 + i]);
        rc[i] = static_cast<size_t>(meta[1 + ndims + i]);
        remote_vals *= meta[1 + ndims + i];
      }
      std::vector<float> remote(static_cast<size_t>(remote_vals));
      if (pnc_commux_recv_bytes(remote.data(),
                                static_cast<int64_t>(remote.size() *
                                                     sizeof(float)),
                                r, data_tag) != PNC_COMMUX_SUCCESS) {
        status = NC_EINVAL;
        continue;
      }
      int werr = nc_put_vara_float(st->ncid, varid, rs.data(), rc.data(),
                                   remote.data());
      if (status == NC_NOERR && werr != NC_NOERR) status = werr;
    }
  } else {
    std::vector<PNC_Offset> meta(1 + 2 * ndims);
    meta[0] = ndims;
    for (int i = 0; i < ndims; ++i) {
      meta[1 + i] = start[i];
      meta[1 + ndims + i] = count[i];
    }
    if (pnc_commux_send_bytes(meta.data(),
                              static_cast<int64_t>(meta.size() *
                                                   sizeof(PNC_Offset)),
                              0, meta_tag) != PNC_COMMUX_SUCCESS ||
        pnc_commux_send_bytes(value, static_cast<int64_t>(nbytes), 0,
                              data_tag) != PNC_COMMUX_SUCCESS) {
      status = NC_EINVAL;
    }
  }
  int berr = barrier();
  if (status == NC_NOERR && berr != NC_NOERR) status = berr;
  st->req_status[req] = status;
  if (reqidp != nullptr) *reqidp = req;
  return status;
}

int ncmpix_iput_var_float(int ncid, int varid, const float* value,
                          int* reqidp) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int status = NC_NOERR;
  for (int r = 0; r < st->size; ++r) {
    if (st->rank == r) status = nc_put_var_float(st->ncid, varid, value);
    int berr = barrier();
    if (status == NC_NOERR && berr != NC_NOERR) status = berr;
  }
  int req = st->next_reqid++;
  st->req_status[req] = status;
  if (reqidp != nullptr) *reqidp = req;
  return status;
}

int ncmpix_wait_all(int ncid, int num, int* reqids, int* statuses) {
  FileState* st = file_state(ncid);
  if (st == nullptr) return NC_EBADID;
  int err = NC_NOERR;
  for (int i = 0; i < num; ++i) {
    int status = NC_NOERR;
    auto it = st->req_status.find(reqids[i]);
    if (it != st->req_status.end()) {
      status = it->second;
      st->req_status.erase(it);
    }
    if (statuses != nullptr) statuses[i] = status;
    if (err == NC_NOERR && status != NC_NOERR) err = status;
  }
  int berr = barrier();
  return err != NC_NOERR ? err : berr;
}

}  // extern "C"
