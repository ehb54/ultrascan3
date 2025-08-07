
#if __cplusplus >= 201103L
#define USE_RAND_RNG
#define USE_SPHERE_CENTERS_MODEL
#define USE_NANOFLANN_SORT_SCM
#include <chrono>
#include <future>

#include "../include/us_hydrodyn_zeno.h"
#undef R
extern bool *zeno_stop_flag;
extern US_Hydrodyn *zeno_us_hydrodyn;

#include <fstream>
class zeno_fout {
 public:
  zeno_fout(const char *fname) { ofs.open(fname); };
  ~zeno_fout() {
    if (ofs.is_open()) {
      ofs.close();
    }
  };
  std::ofstream ofs;
};
extern zeno_fout *zeno_cxx_fout;

#include "../include/us_zeno_cxx_nf.h"

#endif  //  __cplusplus >= 201103L
