// Proof that grpy::Solver runs in-process on SOMO's threading backend (QtParallel
// over QThreadPool) and delivers progress via a callback -- exactly the shape the
// SOMO adapter uses. Link QtCore + QtConcurrent.
#include "grpy_api.hpp"
#include "parallel_qt.hpp"
#include <cstdio>

int main() {
    std::vector<grpy::Bead> beads = { {0,0,0,20,0}, {0,50,0,30,0} };
    grpy::PhysParams p; p.temperature_C=20; p.eta=0.01; p.rho=1.0; p.vbar=0.9;
    p.units=1e-8; p.mw=1e3; p.input_label="GRPY";

    la::QtParallel par(8);                 // <- SOMO passes config numThreads here
    grpy::Solver solver(par);
    int last = -1;
    grpy::Results r = solver.run(beads, p, [&](int pct, const char* stage){
        if (pct != last) { std::printf("  progress %3d%%  %s\n", pct, stage); last = pct; }
    });
    std::printf("Dr=%.3e  s=%.3e  Dt=%.3e  eta0=%.3e  report=%zu bytes\n",
                r.rotational_diffusion, r.sedimentation, r.translational_diffusion,
                r.intrinsic_viscosity_zero, r.report.size());
    std::printf("%s\n", r.report.size() > 1000 ? "QT IN-PROCESS OK" : "FAIL");
    return 0;
}
