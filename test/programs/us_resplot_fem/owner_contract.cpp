// Compile the REAL declarations separately. A changed accessor signature must
// invalidate this harness rather than silently allowing stale doubles to pass.
#include "../../../programs/us_fematch/us_fematch.h"
#include "../../../programs/us_autoflow_analysis/us_autoflow_analysis.h"
#include "../../../programs/us_reporter_gmp/us_reporter_gmp.h"
#include <type_traits>

#define CHECK(Class, method, Result) \
    static_assert(std::is_same<decltype(&Class::method), Result (Class::*)()>::value, \
                  "Legacy residual owner accessor changed: " #Class "::" #method);
#define CHECK_OWNER(Class, prefix) \
    CHECK(Class, prefix##_editdata, US_DataIO::EditedData*) \
    CHECK(Class, prefix##_simdata, US_DataIO::RawData*) \
    CHECK(Class, prefix##_excllist, QList<int>*) \
    CHECK(Class, prefix##_ti_noise, US_Noise*) \
    CHECK(Class, prefix##_ri_noise, US_Noise*) \
    CHECK(Class, prefix##_resbmap, QPointer<US_ResidsBitmap>)
CHECK_OWNER(US_FeMatch, fem)
CHECK_OWNER(US_Analysis_auto, aa)
CHECK_OWNER(US_ReporterGMP, rg)
CHECK(US_Analysis_auto, aa_tripleInfo, QString)
CHECK(US_ReporterGMP, rg_tripleInfo, QString)
#undef CHECK_OWNER
#undef CHECK
