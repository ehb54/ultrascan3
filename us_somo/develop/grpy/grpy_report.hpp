// GRPY report writer -- shared by the CLI and the in-process API so both emit the
// EXACT same report text (byte-identical to the Fortran GRPY, validated vs golden).
// Pure formatting + the physical-scalar display formulas; no I/O, no SOMO deps.
#pragma once
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ostream>
#include <string>

namespace grpy {

// Corrected physical constants (double precision; see Phase 0).
static constexpr double PI = 3.14159265358979324;
static constexpr double KB = 1.38064852e-16;
static constexpr double NA = 6.022140858e+23;

// Quantities the report is built from (filled by the pipeline; see grpy_api.hpp).
struct ReportData {
    double ATR[7][7]{};    // 6x6 mobility at origin (1-based)
    double ATRCH[7][7]{};  // 6x6 mobility at centre of resistance, principal frame
    double EIV[4][4]{};    // eigenvectors 3x3
    double RR[4]{}, RCH[4]{};
    double TAU[9]{};       // TAU(1..8)
    double DTAU = 0, MDD = 0, BR = 0, RG2 = 0;
    double UNITS = 1, ETA = 0.01, TK = 293.15, VBAR = 0, RHO = 1, MW = 0;
    std::string inputtype;
};

// ---- Fortran-format helpers (Aw right-justify, ESw.3 == %.3E) ----------------
namespace ff {
inline bool hp() { static const bool v = std::getenv("GRPY_HP") != nullptr; return v; }
inline std::string aw(const std::string& s, int w) {
    int n = (int)s.size();
    return n >= w ? s.substr(0, w) : std::string(w - n, ' ') + s;
}
inline std::string es(double v) {
    char buf[64]; std::snprintf(buf, sizeof buf, hp() ? "%24.15E" : "%11.3E", v);
    return std::string(buf);
}
inline void blank(std::ostream& o) { o << "\n"; }
inline void lstr(std::ostream& o, const std::string& s) { o << " " << s << "\n"; }
inline void f202(std::ostream& o, const std::string& s) { o << " " << aw(s, 70) << "\n"; }
inline void f200(std::ostream& o, const std::string& s) { o << " " << aw(s, 80) << "\n"; }
inline void f201(std::ostream& o, const std::string& a, const std::string& b) { o << " " << aw(a,50) << aw(b,20) << "\n"; }
inline void f100L(std::ostream& o, const std::string& s) { o << " " << aw(s, 70) << "\n"; }
inline void f100(std::ostream& o, const std::string& s, double v, const std::string& u) { o << " " << aw(s,70) << es(v) << aw(u,11) << "\n"; }
inline void f105(std::ostream& o, double a, double b, double c) { o << " " << es(a) << es(b) << es(c) << "\n"; }
inline void f106(std::ostream& o, const std::string& a, const std::string& b, const std::string& c) { o << " " << aw(a,11) << aw(b,11) << aw(c,11) << "\n"; }
inline void f102(std::ostream& o, const std::string& a, const std::string& b) { o << " " << aw(a,40) << aw(b,15) << "\n"; }
inline void f107(std::ostream& o, const std::string& s, double a, double b, double c) { o << " " << aw(s,11) << es(a) << es(b) << es(c) << "\n"; }
inline void f101(std::ostream& o, double a, double b, double c, double d, double e, double f) {
    o << " " << es(a) << es(b) << es(c) << aw(" ",3) << es(d) << es(e) << es(f) << "\n"; }
}  // namespace ff

// The 40-char progress bar, '\r'+bar, no newline (SOMO parses `NN% TASK:`).
inline void write_progress(std::ostream& o, int proc, const std::string& state) {
    char p3[8]; std::snprintf(p3, sizeof p3, "%3d", proc);
    o << "\r" << std::string(p3) << "%" << " TASK: " << ff::aw(state, 29);
    o.flush();
}

// Emit the full GRPY report (identical to the Fortran). Non-mutating.
inline void write_report(std::ostream& o, const ReportData& d) {
    using namespace ff;
    const double U = d.UNITS, ETA = d.ETA, TK = d.TK;
    const auto& TAU = d.TAU; const auto& ATR = d.ATR; const auto& ATRCH = d.ATRCH;
    double DTR[7][7]{};

    blank(o); f202(o, "GRPY program"); blank(o);
    f200(o, "Hydrodynamic properties of the macromolecule");
    f200(o, "based on the Generalized Rotne-Prager-Yamakawa method");
    blank(o); f201(o, "from the:", d.inputtype + " input file"); blank(o);

    f100(o, "Rotational diffusion coefficient:", d.DTAU * (KB*TK) / (std::pow(U,3)*PI*ETA), "[s^-1]" "    ");
    f100L(o, "Rotational relaxation times associated with");
    f100L(o, "the Brownian relaxation of a vector:");
    f100(o, "-> Relaxation time (1):", TAU[6] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (2):", TAU[7] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (3):", TAU[8] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100L(o, "Rotational relaxation times associated with");
    f100L(o, "the Brownian relaxation of a traceless symmetric tensor:");
    f100(o, "-> Relaxation time (1):", TAU[1] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (2):", TAU[2] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (3):", TAU[3] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (4):", TAU[4] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "-> Relaxation time (5):", TAU[5] * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "Harmonic mean (correlation) time:",
         5.0 / (1.0/TAU[1]+1.0/TAU[2]+1.0/TAU[3]+1.0/TAU[4]+1.0/TAU[5]) * (std::pow(U,3)*PI*ETA)/(KB*TK), "[s]" "       ");
    f100(o, "Sedimentation coefficient (Mw Dlt (1. - (vbar*rho))/(nA kB T)):",
         d.MW*(1.0-(d.VBAR*d.RHO))*1.0e+13/NA * ((ATRCH[1][1]+ATRCH[2][2]+ATRCH[3][3])/3.0)/(U*PI*ETA), "[Svedberg]");
    blank(o);
    f100(o, "High frequency intrinsic viscosity eta oo:", d.MDD*PI*NA/d.MW*std::pow(U,3), "[cm^3/g]" "  ");
    f100(o, "Zero frequency intrinsic viscosity eta 0:", (d.MDD+d.BR)*PI*NA/d.MW*std::pow(U,3), "[cm^3/g]" "  ");
    blank(o);
    f102(o, " calculated using the origin", ""); f102(o, " of the coordinate system:", "");
    f105(o, 0.0, 0.0, 0.0);
    f102(o, " as the reference point", ""); f102(o, " in the standard reference frame:", "");
    f107(o, " e1:", 1.0, 0.0, 0.0); f107(o, " e2:", 0.0, 1.0, 0.0); f107(o, " e3:", 0.0, 0.0, 1.0);
    blank(o);
    f100(o, "Translational diffusion coefficient:", ((ATR[1][1]+ATR[2][2]+ATR[3][3])/3.0)*KB*TK/(U*PI*ETA), "[cm^2/s]" "  ");
    blank(o); f102(o, " 6x6 diffusion matrix: ", "Dtt  Dtr"); blank(o); f102(o, "", "Drt  Drr"); blank(o);
    auto fillDTR = [&](const double M[7][7]) {
        for (int i=1;i<=3;++i) for (int j=1;j<=3;++j) DTR[i][j] = M[i][j]*KB*TK/(U*PI*ETA);
        for (int i=4;i<=6;++i) for (int j=4;j<=6;++j) DTR[i][j] = M[i][j]*KB*TK/(std::pow(U,3)*PI*ETA);
        for (int i=1;i<=3;++i) for (int j=4;j<=6;++j) DTR[i][j] = M[i][j]*KB*TK/(std::pow(U,2)*PI*ETA);
        for (int i=4;i<=6;++i) for (int j=1;j<=3;++j) DTR[i][j] = M[i][j]*KB*TK/(std::pow(U,2)*PI*ETA);
    };
    auto dumpDTR = [&]() {
        for (int i=1;i<=3;++i) f101(o, DTR[i][1],DTR[i][2],DTR[i][3],DTR[i][4],DTR[i][5],DTR[i][6]);
        blank(o);
        for (int i=4;i<=6;++i) f101(o, DTR[i][1],DTR[i][2],DTR[i][3],DTR[i][4],DTR[i][5],DTR[i][6]);
    };
    fillDTR(ATR); dumpDTR(); blank(o);
    f102(o, "calculated using the mobility center:", "");
    f106(o, "x [cm]", "y [cm]", "z [cm]");
    f105(o, d.RCH[1]*U, d.RCH[2]*U, d.RCH[3]*U);   // scaled for display, no mutation
    f102(o, " as the reference point", ""); f102(o, " in the reference frame:", "");
    f107(o, " e1:", d.EIV[1][1], d.EIV[2][1], d.EIV[3][1]);
    f107(o, " e2:", d.EIV[1][2], d.EIV[2][2], d.EIV[3][2]);
    f107(o, " e3:", d.EIV[1][3], d.EIV[2][3], d.EIV[3][3]);
    blank(o);
    f100(o, "Translational diffusion coefficient:", ((ATRCH[1][1]+ATRCH[2][2]+ATRCH[3][3])/3.0)*KB*TK/(U*PI*ETA), "[cm^2/s]" "  ");
    blank(o); f102(o, " 6x6 diffusion matrix: ", "Dtt  Dtr"); blank(o); f102(o, "", "Drt  Drr"); blank(o);
    fillDTR(ATRCH); dumpDTR(); blank(o);
    f100L(o, "Radius of the sphere with equal:"); blank(o);
    f100(o, "Translational diffusion coefficient:", U/((ATRCH[1][1]+ATRCH[2][2]+ATRCH[3][3])/3.0)/6.0, "[cm]" "      ");
    f100(o, "Rotational diffusion coefficient:", U/std::pow(8.0*d.DTAU, 1.0/3.0), "[cm]" "      ");
    f100L(o, "Rotational relaxation times associated with");
    f100L(o, "the Brownian relaxation of a vector:");
    f100(o, "-> Relaxation time (1):", U*std::pow(6.0*TAU[6]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (2):", U*std::pow(6.0*TAU[7]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (3):", U*std::pow(6.0*TAU[8]/8.0, 1.0/3.0), "[cm]" "      ");
    f100L(o, "Rotational relaxation times associated with");
    f100L(o, "the Brownian relaxation of a traceless symmetric tensor:");
    f100(o, "-> Relaxation time (1):", U*std::pow(6.0*TAU[1]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (2):", U*std::pow(6.0*TAU[2]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (3):", U*std::pow(6.0*TAU[3]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (4):", U*std::pow(6.0*TAU[4]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "-> Relaxation time (5):", U*std::pow(6.0*TAU[5]/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "Mean relaxation time:",
         U*std::pow(6.0*5.0/(1.0/TAU[1]+1.0/TAU[2]+1.0/TAU[3]+1.0/TAU[4]+1.0/TAU[5])/8.0, 1.0/3.0), "[cm]" "      ");
    f100(o, "High frequency intrinsic viscosity eta oo:", std::pow(3.0/10.0*d.MDD, 1.0/3.0)*U, "[cm]" "      ");
    f100(o, "Zero frequency intrinsic viscosity eta 0:", std::pow(3.0/10.0*(d.MDD+d.BR), 1.0/3.0)*U, "[cm]" "      ");
}

}  // namespace grpy
