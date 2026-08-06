// GRPY C++ port -- Phase 2 numeric core (Rotne-Prager-Yamakawa hydrodynamics).
//
// Faithful translation of the compute path in GRPY.f:
//   CALCRG2 -> HYDRO(HYDRO_RP assembly + INVFRI_TO_FRI inverse + T_RIGID_11 + AR)
//   -> INTRINSIC_HIGH -> BROWN_EW -> invert AR(1:6,1:6) -> CALCRCH -> CALCATRCH
//   -> CALCTAU. Fills the same quantities WRITESTDOUT consumes.
//
// Linear algebra via Eigen (SOMO's vendored copy). The big symmetric inverse
// (MATREV = DPOTRF/DPOTRI upper) maps to Eigen's LLT; DSYEV -> SelfAdjointEigenSolver
// (ascending eigenvalues); DGESV -> partialPivLu. Column-major (Eigen default) matches
// the Fortran storage. Indices are 0-based; Fortran 6*(I-1)+j maps to 6*i+(j-1).
#pragma once
#include <Eigen/Dense>
#include <array>
#include <cmath>
#include <vector>
#include "linalg.hpp"

namespace core {
using Eigen::MatrixXd;
using Eigen::Matrix3d;
using Eigen::Vector3d;

struct Bead { double x, y, z, r, mw; };

// 3rd/4th-order Cartesian tensors as plain arrays.
struct T3 { double d[3][3][3]; void zero() { std::fill(&d[0][0][0], &d[0][0][0] + 27, 0.0); } };
struct T4 { double d[3][3][3][3]; void zero() { std::fill(&d[0][0][0][0], &d[0][0][0][0] + 81, 0.0); } };

// ---- constant tensors -------------------------------------------------------
inline const Matrix3d& U() { static Matrix3d u = Matrix3d::Identity(); return u; }
inline double EPS(int i, int j, int k) {  // Levi-Civita
    if ((i==0&&j==1&&k==2)||(i==1&&j==2&&k==0)||(i==2&&j==0&&k==1)) return 1.0;
    if ((i==0&&j==2&&k==1)||(i==2&&j==1&&k==0)||(i==1&&j==0&&k==2)) return -1.0;
    return 0.0;
}
// Y2 basis (INIT_Y2): 5 symmetric-traceless 3x3 matrices, index 0..4.
inline const std::array<Matrix3d,5>& Y2() {
    static std::array<Matrix3d,5> y = []{
        std::array<Matrix3d,5> a; for (auto& m : a) m.setZero();
        const double s2 = 1.0/std::sqrt(2.0), s6 = 1.0/std::sqrt(6.0), s23 = std::sqrt(2.0/3.0);
        a[0](0,1)=s2;  a[0](1,0)=s2;
        a[1](1,2)=-s2; a[1](2,1)=-s2;
        a[2](0,0)=-s6; a[2](1,1)=-s6; a[2](2,2)=s23;
        a[3](0,2)=-s2; a[3](2,0)=-s2;
        a[4](0,0)=s2;  a[4](1,1)=-s2;
        return a;
    }();
    return y;
}

// ---- small tensor builders (CALC_*) ----------------------------------------
inline Matrix3d calc_RR(const Vector3d& rw) { return rw * rw.transpose(); }
inline Matrix3d calc_EPSR(const Vector3d& rw) {
    Matrix3d e; e.setZero();
    e(0,1)=rw(2); e(1,2)=rw(0); e(2,0)=rw(1);
    e(1,0)=-rw(2); e(2,1)=-rw(0); e(0,2)=-rw(1);
    return e;
}
inline T3 calc_UR(const Vector3d& rw) {  // t3UR(I,I,:) = rw
    T3 t; t.zero();
    for (int i=0;i<3;i++) for (int k=0;k<3;k++) t.d[i][i][k]=rw(k);
    return t;
}
inline T3 calc_RRR(const Vector3d& rw) {
    T3 t;
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int k=0;k<3;k++) t.d[i][j][k]=rw(i)*rw(j)*rw(k);
    return t;
}
inline T3 calc_EPSRR(const Vector3d& rw) {
    T3 t; Matrix3d e=calc_EPSR(rw);
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int k=0;k<3;k++) t.d[i][j][k]=e(i,j)*rw(k);
    return t;
}
// t4(I,J,K,L) = a(I,J)*b(K,L)
inline T4 t2at2b(const Matrix3d& a, const Matrix3d& b) {
    T4 t;
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int k=0;k<3;k++) for (int l=0;l<3;l++)
        t.d[i][j][k][l]=a(i,j)*b(k,l);
    return t;
}
// transpose a pair of indices (i1,i2 are 1-based like the Fortran)
inline T4 t4trans(const T4& o, int i1, int i2) {
    T4 t;
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int k=0;k<3;k++) for (int l=0;l<3;l++) {
        int a=i,b=j,c=k,e=l;
        if (i1==1&&i2==2) { a=j;b=i; }
        else if (i1==1&&i2==3) { a=k;c=i; }
        else if (i1==1&&i2==4) { a=l;e=i; }
        else if (i1==2&&i2==3) { b=k;c=j; }
        else if (i1==2&&i2==4) { b=l;e=j; }
        else if (i1==3&&i2==4) { c=l;e=k; }
        t.d[i][j][k][l]=o.d[a][b][c][e];
    }
    return t;
}
inline double mulT2aT4T2b(const Matrix3d& a, const T4& t, const Matrix3d& b) {
    double s=0;
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int k=0;k<3;k++) for (int l=0;l<3;l++)
        s += t.d[i][j][k][l]*a(i,j)*b(k,l);
    return s;
}
inline Vector3d mulT3T2(const T3& t, const Matrix3d& m) {
    Vector3d v(0,0,0);
    for (int i=0;i<3;i++) for (int j=0;j<3;j++) for (int p=0;p<3;p++) v(p)+=t.d[p][i][j]*m(i,j);
    return v;
}
inline T4 add(const T4&a,const T4&b,double s){ T4 t; for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)for(int l=0;l<3;l++) t.d[i][j][k][l]=a.d[i][j][k][l]+s*b.d[i][j][k][l]; return t; }
inline T4 scale(const T4&a,double s){ T4 t; for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)for(int l=0;l<3;l++) t.d[i][j][k][l]=s*a.d[i][j][k][l]; return t; }

inline T4 calc_t4D0(const Matrix3d& RR) {
    Matrix3d tmp = RR - U()/3.0;
    return scale(t2at2b(tmp,tmp), 1.5);
}
inline T4 calc_t4D1(const Matrix3d& RR) {
    T4 t; t.zero();
    t = add(t, t4trans(t2at2b(U(),RR),1,4), 1.0);
    t = add(t, t4trans(t2at2b(U(),RR),2,3), 1.0);
    t = add(t, t4trans(t2at2b(U(),RR),1,3), 1.0);
    t = add(t, t4trans(t2at2b(U(),RR),2,4), 1.0);
    t = add(t, t2at2b(RR,RR), -4.0);
    return scale(t,0.5);
}
inline T4 calc_t4D2(const Matrix3d& RR) {
    T4 t; t.zero();
    t = add(t, t4trans(t2at2b(U(),U()),2,3), 1.0);
    t = add(t, t4trans(t2at2b(U(),U()),1,3), 1.0);
    t = add(t, t2at2b(U(),U()), -1.0);
    t = add(t, t2at2b(RR,U()), 1.0);
    t = add(t, t2at2b(U(),RR), 1.0);
    t = add(t, t2at2b(RR,RR), -3.0);
    return scale(t,0.5);
}

// ---- pair tensors -----------------------------------------------------------
// Each returns the small block; R is the separation vector, aI/aJ the radii.
inline Matrix3d rp_TT(const Vector3d& R,double aI,double aJ){
    double P18=1.0/8.0, P23a2=(aI*aI+aJ*aJ)/3.0;
    double dist=R.norm(); Vector3d rw=R/dist; double d3=dist*dist*dist;
    Matrix3d RR=calc_RR(rw);
    return P18*((1.0/dist+P23a2/d3)*U() + (1.0/dist-3.0*P23a2/d3)*RR);
}
inline Matrix3d yama_TT(const Vector3d& R,double aI,double aJ){
    double M0=1.0/6.0; double dist=R.norm(); Vector3d rw=R/dist; Matrix3d RR=calc_RR(rw);
    double amin=std::min(aI,aJ), amax=std::max(aI,aJ);
    if (dist<=amax-amin) return (M0/amax)*U();
    double d2=dist*dist,d3=d2*dist,aij=aI-aJ;
    double pre=1.0/(6*32*aI*aJ);
    double pu=16*(aI+aJ)-std::pow(aij*aij+3*d2,2)/d3;
    double prr=3*std::pow(aij*aij-d2,2)/d3;
    return pre*(pu*U()+prr*RR);
}
inline Matrix3d rp_RR(const Vector3d& R){
    double P16=1.0/16.0; double dist=R.norm(); Vector3d rw=R/dist; double d3=dist*dist*dist;
    Matrix3d RR=calc_RR(rw);
    return -P16*(U()/d3-3.0*RR/d3);
}
inline Matrix3d yama_RR(const Vector3d& R,double aI,double aJ){
    double M0=1.0/8.0; double dist=R.norm(); Vector3d rw=R/dist; Matrix3d RR=calc_RR(rw);
    double amin=std::min(aI,aJ), amax=std::max(aI,aJ);
    if (dist<=amax-amin) return (M0/(amax*amax*amax))*U();
    double d2=dist*dist,d3=d2*dist,aij=aI-aJ;
    double aI2=aI*aI,aJ2=aJ*aJ,aI3=aI2*aI,aJ3=aJ2*aJ;
    double pre=1.0/(8.0*64.0*aI3*aJ3);
    double pu = 5*d3 - 27*dist*(aI2+aJ2) + 32*(aI3+aJ3)
              - 9*std::pow(aI2-aJ2,2)/dist - std::pow(aij,4)*(aI2+4*aI*aJ+aJ2)/d3;
    double prr= 3*std::pow(aij*aij-d2,2)*((aI2+4*aI*aJ+aJ2)-d2)/d3;
    return pre*(pu*U()+prr*RR);
}
inline Matrix3d rp_RT(const Vector3d& R){
    double P18=1.0/8.0; double dist=R.norm(); Vector3d rw=R/dist; double d2=dist*dist;
    return P18*calc_EPSR(rw)/d2;
}
inline Matrix3d yama_RT(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; Matrix3d E=calc_EPSR(rw);
    if (aI<=aJ) {
        if (dist<=aJ-aI) return Matrix3d::Zero();
        double pre=1.0/(16.0*8.0*aI*aI*aI*aJ);
        double pe=std::pow((aI-aJ)+dist,2)*(aJ*aJ+2*aJ*(aI+dist)-3*std::pow(aI-dist,2))/(dist*dist);
        return pre*pe*E;
    } else {
        if (dist<=aI-aJ) return (dist/(8.0*aI*aI*aI))*E;
        double pre=1.0/(16.0*8.0*aI*aI*aI*aJ);
        double pe=std::pow((aI-aJ)+dist,2)*(aJ*aJ+2*aJ*(aI+dist)-3*std::pow(aI-dist,2))/(dist*dist);
        return pre*pe*E;
    }
}
inline T4 rp_DD(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; double d5=std::pow(dist,5),d2=dist*dist;
    double aI2=aI*aI,aJ2=aJ*aJ; Matrix3d RR=calc_RR(rw);
    T4 D0=calc_t4D0(RR),D1=calc_t4D1(RR),D2=calc_t4D2(RR); D2=add(D2,D1,-1.0);
    T4 b; b.zero();
    b=add(b,D0, 3.0*(6.0*(aI2+aJ2)-5.0*d2)/(20.0*d5));
    b=add(b,D1,-3.0*(8.0*(aI2+aJ2)-5.0*d2)/(40.0*d5));
    b=add(b,D2, 3.0*(aI2+aJ2)/(20.0*d5));
    return b;
}
inline T4 yama_DD(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; double d3=std::pow(dist,3),d5=std::pow(dist,5);
    double aI2=aI*aI,aJ2=aJ*aJ,aI3=aI2*aI,aJ3=aJ2*aJ; Matrix3d RR=calc_RR(rw);
    T4 D0=calc_t4D0(RR),D1=calc_t4D1(RR),D2=calc_t4D2(RR); D2=add(D2,D1,-1.0);
    T4 b; b.zero();
    double amin=std::min(aI,aJ),amax=std::max(aI,aJ);
    if (dist<=amax-amin) {
        double c=3.0/(amax*amax*amax*20.0);
        b=add(b,D0,c); b=add(b,D1,c); b=add(b,D2,c); return b;
    }
    double a6=std::pow(aI-aJ,6),a4=std::pow(aI-aJ,4);
    b=add(b,D0, 3.0/(1280.0*aI3*aJ3)*( 3.0*a6*(aI2+6*aI*aJ+aJ2)/d5
        -10.0*a4*(aI2+4*aI*aJ+aJ2)/d3 +32.0*(aI3+aJ3) -30.0*(aI2+aJ2)*dist +5.0*d3 ));
    b=add(b,D1, 3.0/(1280.0*aI3*aJ3)*( -2.0*a6*(aI2+6*aI*aJ+aJ2)/d5
        +5.0*a4*(aI2+4*aI*aJ+aJ2)/d3 -15.0*std::pow(aI2-aJ2,2)/dist
        +32.0*(aI3+aJ3) -25.0*(aI2+aJ2)*dist +5.0*d3 ));
    b=add(b,D2, 3.0/(2560.0*aI3*aJ3)*( a6*(aI2+6*aI*aJ+aJ2)/d5
        -30.0*std::pow(aI2-aJ2,2)/dist +64.0*(aI3+aJ3) -40.0*(aI2+aJ2)*dist +5.0*d3 ));
    return b;
}
inline T3 rp_TD(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; double d4=std::pow(dist,4),d2=dist*dist;
    double aI2=aI*aI,aJ2=aJ*aJ; T3 UR=calc_UR(rw),RRR=calc_RRR(rw),c; c.zero();
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)
        c.d[i][j][k]=(-2.0*(5.0*aI2*aJ2+3.0*aJ2*aJ2)/(5.0*d4)*UR.d[i][j][k]
                      + aJ2*(5.0*aI2+3.0*aJ2-3.0*d2)/d4*RRR.d[i][j][k])/(8.0*aJ2);
    return c;
}
inline T3 yama_TD(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; double d2=dist*dist,d4=std::pow(dist,4);
    T3 UR=calc_UR(rw),RRR=calc_RRR(rw),c; c.zero();
    if (aI<=aJ && dist<=aJ-aI) { double f=-3.0*dist/(20.0*aJ*aJ*aJ);
        for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++) c.d[i][j][k]=f*UR.d[i][j][k]; return c; }
    if (aI>aJ && dist<=aI-aJ) return c; // zero
    double pre=1.0/(8.0*aI*aJ*aJ*aJ);
    double pur=(10.0*d2-24.0*aI*dist-15.0*(aJ*aJ-aI*aI)+std::pow(aJ-aI,5)*(aI+5.0*aJ)/d4)/40.0;
    double prrr=std::pow(std::pow(aI-aJ,2)-d2,2)*((aI-aJ)*(aI+5*aJ)-d2)/(16.0*d4);
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)
        c.d[i][j][k]=pre*(pur*UR.d[i][j][k]+prrr*RRR.d[i][j][k]);
    return c;
}
inline T3 rp_RD(const Vector3d& R){
    double dist=R.norm(); Vector3d rw=R/dist; T3 E=calc_EPSRR(rw),c;
    double f=-3.0/(8.0*std::pow(dist,3));
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++) c.d[i][j][k]=f*E.d[i][j][k];
    return c;
}
inline T3 yama_RD(const Vector3d& R,double aI,double aJ){
    double dist=R.norm(); Vector3d rw=R/dist; double d2=dist*dist; T3 E=calc_EPSRR(rw),c; c.zero();
    double amin=std::min(aI,aJ),amax=std::max(aI,aJ);
    if (dist<=amax-amin) return c; // zero
    double f=-15.0/(1280.0*aI*aI*aI*aJ*aJ*aJ*std::pow(dist,3))
             *std::pow(std::pow(aI-aJ,2)-d2,2)*((aI*aI+4*aI*aJ+aJ*aJ)-d2);
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++) c.d[i][j][k]=f*E.d[i][j][k];
    return c;
}
// Y2 projections
inline Eigen::Matrix<double,5,5> ddY2(const T4& cart){
    Eigen::Matrix<double,5,5> b;
    for(int i=0;i<5;i++)for(int j=0;j<5;j++) b(i,j)=mulT2aT4T2b(Y2()[i],cart,Y2()[j]);
    return b;
}
inline Eigen::Matrix<double,3,5> tdY2(const T3& cart){
    Eigen::Matrix<double,3,5> c;
    for(int i=0;i<5;i++) c.col(i)=mulT3T2(cart,Y2()[i]);
    return c;
}

// ---- assembly: ROTNE_PRAGER_YAMAKAWA ---------------------------------------
inline void assemble(const std::vector<Bead>& b, int N,
                     MatrixXd& APP, MatrixXd& APQ, MatrixXd& AQQ) {
    APP.setZero(6*N,6*N); APQ.setZero(6*N,5*N); AQQ.setZero(5*N,5*N);
    for (int i=0;i<N;i++) {
        double a=b[i].r;
        for (int j=0;j<3;j++) { APP(6*i+j,6*i+j)=1.0/(6.0*a); APP(6*i+3+j,6*i+3+j)=1.0/(8.0*a*a*a); }
        for (int j=0;j<5;j++) AQQ(5*i+j,5*i+j)=3.0/(20.0*a*a*a);
    }
    auto putP=[&](int r,int c,const Matrix3d& m){ APP.block<3,3>(r,c)=m; };
    for (int i=0;i<N;i++) for (int j=i+1;j<N;j++) {
        Vector3d R(b[i].x-b[j].x, b[i].y-b[j].y, b[i].z-b[j].z);
        double dist=R.norm(), aI=b[i].r, aJ=b[j].r;
        int pi=6*i, pj=6*j, qi=5*i, qj=5*j;
        if (dist>aI+aJ) {
            Matrix3d tt=rp_TT(R,aI,aJ); putP(pi,pj,tt); putP(pj,pi,tt);
            Matrix3d rr=rp_RR(R);       putP(pi+3,pj+3,rr); putP(pj+3,pi+3,rr);
            Matrix3d rt=rp_RT(R);
            putP(pi+3,pj,rt); putP(pj+3,pi,-rt); putP(pi,pj+3,rt); putP(pj,pi+3,-rt);
            APQ.block<3,5>(pi,qj)   = -tdY2(rp_TD(R,aI,aJ));
            APQ.block<3,5>(pj,qi)   = -tdY2(rp_TD(-R,aJ,aI));
            APQ.block<3,5>(pi+3,qj) = -tdY2(rp_RD(R));
            APQ.block<3,5>(pj+3,qi) = -tdY2(rp_RD(-R));
            Eigen::Matrix<double,5,5> dd=ddY2(rp_DD(R,aI,aJ));
            AQQ.block<5,5>(qi,qj)=dd; AQQ.block<5,5>(qj,qi)=dd;
        } else {
            Matrix3d tt=yama_TT(R,aI,aJ); putP(pi,pj,tt); putP(pj,pi,tt);
            Matrix3d rr=yama_RR(R,aI,aJ); putP(pi+3,pj+3,rr); putP(pj+3,pi+3,rr);
            Matrix3d rt=yama_RT(R,aI,aJ); putP(pi+3,pj,rt); putP(pj,pi+3,-rt);
            Matrix3d rtji=yama_RT(-R,aJ,aI); putP(pj+3,pi,rtji); putP(pi,pj+3,-rtji);
            APQ.block<3,5>(pi,qj)   = -tdY2(yama_TD(R,aI,aJ));
            APQ.block<3,5>(pj,qi)   = -tdY2(yama_TD(-R,aJ,aI));
            APQ.block<3,5>(pi+3,qj) = -tdY2(yama_RD(R,aI,aJ));
            APQ.block<3,5>(pj+3,qi) = -tdY2(yama_RD(-R,aJ,aI));
            Eigen::Matrix<double,5,5> dd=ddY2(yama_DD(R,aI,aJ));
            AQQ.block<5,5>(qi,qj)=dd; AQQ.block<5,5>(qj,qi)=dd;
        }
    }
}

// Direct assembly into tiled UPPER storage (never materialises the full matrix).
// Every pair block for i<j and every P-Q coupling lands in the upper triangle; we
// write only those. Tensor math stays in double, cast to the storage scalar S on
// store. Validated element-for-element against assemble() (which matches Fortran).
template <typename S>
inline void assemble_tiled(const std::vector<Bead>& b, int N, la::TiledUpperSPD<S>& M,
                           la::Parallel& par) {
    auto put = [&](int r0, int c0, const auto& blk) {           // off-diagonal block, fully upper
        for (int i = 0; i < blk.rows(); ++i)
            for (int j = 0; j < blk.cols(); ++j) M.at(r0 + i, c0 + j) = (S)blk(i, j);
    };
    const int Q = 6 * N;
    for (int i = 0; i < N; ++i) {                               // self-terms (O(N), serial)
        double a = b[i].r;
        for (int j = 0; j < 3; ++j) {
            M.at(6*i+j, 6*i+j)     = (S)(1.0/(6.0*a));
            M.at(6*i+3+j, 6*i+3+j) = (S)(1.0/(8.0*a*a*a));
        }
        for (int j = 0; j < 5; ++j) M.at(Q+5*i+j, Q+5*i+j) = (S)(3.0/(20.0*a*a*a));
    }
    // Pair blocks (O(N^2)) parallelized over the outer bead i. Each i writes bead i's
    // rows only, at distinct elements -> data-parallel-safe (disjoint writes, no resize).
    par.for_range(N, [&](int i) {
    for (int j = i+1; j < N; ++j) {
        Vector3d R(b[i].x-b[j].x, b[i].y-b[j].y, b[i].z-b[j].z);
        double dist = R.norm(), aI = b[i].r, aJ = b[j].r;
        int pi=6*i, pj=6*j, qi=Q+5*i, qj=Q+5*j;
        if (dist > aI+aJ) {                                     // Rotne-Prager
            put(pi,pj, rp_TT(R,aI,aJ));  put(pi+3,pj+3, rp_RR(R));
            Matrix3d rt=rp_RT(R);        put(pi+3,pj, rt);  put(pi,pj+3, rt);
            put(pi,   qj, (-tdY2(rp_TD(R,aI,aJ)).eval()));
            put(pj,   qi, (-tdY2(rp_TD(-R,aJ,aI)).eval()));
            put(pi+3, qj, (-tdY2(rp_RD(R)).eval()));
            put(pj+3, qi, (-tdY2(rp_RD(-R)).eval()));
            put(qi, qj, ddY2(rp_DD(R,aI,aJ)));
        } else {                                                // Yamakawa (overlapping)
            put(pi,pj, yama_TT(R,aI,aJ)); put(pi+3,pj+3, yama_RR(R,aI,aJ));
            put(pi+3,pj, yama_RT(R,aI,aJ));
            put(pi,pj+3, (-yama_RT(-R,aJ,aI)).eval());
            put(pi,   qj, (-tdY2(yama_TD(R,aI,aJ)).eval()));
            put(pj,   qi, (-tdY2(yama_TD(-R,aJ,aI)).eval()));
            put(pi+3, qj, (-tdY2(yama_RD(R,aI,aJ)).eval()));
            put(pj+3, qi, (-tdY2(yama_RD(-R,aJ,aI)).eval()));
            put(qi, qj, ddY2(yama_DD(R,aI,aJ)));
        }
    }
    });
}

// symmetric positive-definite inverse (MATREV 'M': DPOTRF/DPOTRI upper + symmetrize)
inline MatrixXd spd_inverse(const MatrixXd& Ain) {
    int n=Ain.rows();
    MatrixXd inv = Ain.selfadjointView<Eigen::Upper>().llt().solve(MatrixXd::Identity(n,n));
    for (int i=0;i<n;i++) for (int j=0;j<i;j++) inv(i,j)=inv(j,i);  // mirror upper->lower
    return inv;
}

// T_RIGID_11: build the 11N x 11 rigid-body projection
inline MatrixXd t_rigid_11(const std::vector<Bead>& b, int N, const Vector3d& RC) {
    MatrixXd T = MatrixXd::Zero(11*N, 11);
    for (int i=0;i<N;i++) {
        Vector3d cnf(b[i].x-RC(0), b[i].y-RC(1), b[i].z-RC(2));
        for (int m=0;m<3;m++) { T(6*i+m,m)=1.0; T(6*i+3+m,3+m)=1.0; }
        for (int k=0;k<5;k++) T(6*N+5*i+k, 6+k)=1.0;
        T(6*i+0,4)= cnf(2); T(6*i+1,3)=-cnf(2);
        T(6*i+1,5)= cnf(0); T(6*i+2,4)=-cnf(0);
        T(6*i+2,3)= cnf(1); T(6*i+0,5)=-cnf(1);
        for (int k=0;k<5;k++) T.block<3,1>(6*i,6+k) = Y2()[k]*cnf;
    }
    return T;
}

// HYDRO: assemble the 11N mobility M, then compute AR = Tᵀ·M⁻¹·T (11x11).
//
// The Fortran forms the full inverse M⁻¹ (DPOTRF+DPOTRI) purely to multiply it by
// the 11N x 11 projection T. That inverse is never needed on its own: we instead
// Cholesky-factor M once and SOLVE M·X = T for the 11 columns of T, then AR = Tᵀ·X.
// Mathematically identical (and more accurate), it drops DPOTRI (~2-3x less compute)
// and never materialises a second 11N x 11N matrix -- the memory that walls this at
// thousands of beads. The factorization is done IN PLACE (Eigen Ref) so M is not
// copied. (Phase 3 step 3 replaces this with a packed, blocked, threaded Cholesky
// that halves storage again and emits progress.)
inline Eigen::Matrix<double,11,11> hydro(const std::vector<Bead>& b, int N, const Vector3d& RC) {
    MatrixXd APP,APQ,AQQ; assemble(b,N,APP,APQ,AQQ);
    MatrixXd M(11*N,11*N);
    M.topLeftCorner(6*N,6*N)=APP;
    M.topRightCorner(6*N,5*N)=APQ;
    M.bottomLeftCorner(5*N,6*N)=APQ.transpose();
    M.bottomRightCorner(5*N,5*N)=AQQ;
    APP.resize(0,0); APQ.resize(0,0); AQQ.resize(0,0);   // free the block copies
    MatrixXd T = t_rigid_11(b,N,RC);
    Eigen::LLT<Eigen::Ref<MatrixXd>> llt(M);             // factor M in place (no copy)
    MatrixXd X = llt.solve(T);                           // X = M⁻¹T, 11N x 11
    return T.transpose() * X;                            // AR = Tᵀ M⁻¹ T
}

// Tiled HYDRO: assemble packed upper M in scalar S, factor (blocked/threaded, with
// progress), solve M X = T, AR = Tᵀ X. This is the memory-lean production path.
template <typename S>
inline Eigen::Matrix<double,11,11> hydro_tiled(const std::vector<Bead>& b, int N,
        const Vector3d& RC, la::Parallel& par, const la::Progress& prog = {},
        int tile = 256, const std::string& oocfile = "") {
    la::TiledUpperSPD<S> M(11*N, tile, oocfile);   // oocfile != "" -> disk-backed
    assemble_tiled<S>(b, N, M, par);
    MatrixXd Td = t_rigid_11(b, N, RC);
    M.factor(par, prog);
    Eigen::Matrix<S,Eigen::Dynamic,Eigen::Dynamic> Ts = Td.cast<S>();
    Eigen::Matrix<S,Eigen::Dynamic,Eigen::Dynamic> Xs = M.solve(Ts);
    return (Td.transpose() * Xs.template cast<double>());
}

// ---- pre/post processing ----------------------------------------------------
inline void calcrg2(const std::vector<Bead>& b,int N,Vector3d& RC,double& RG2){
    RC.setZero(); double tot=0;
    for (int i=0;i<N;i++){ double r3=std::pow(b[i].r,3);
        RC+=Vector3d(b[i].x,b[i].y,b[i].z)*r3; tot+=r3; }
    RC/=tot; RG2=0;
    for (int i=0;i<N;i++){ Vector3d si(b[i].x-RC(0),b[i].y-RC(1),b[i].z-RC(2));
        RG2+=(3.0/5.0*b[i].r*b[i].r + si.dot(si))*std::pow(b[i].r,3)/tot; }
}
inline double intrinsic_high(const Eigen::Matrix<double,11,11>& AR){
    MatrixXd ATT=AR.topLeftCorner(6,6);
    MatrixXd ATD=AR.block(0,6,6,5), ADD=AR.block(6,6,5,5);
    ATT=spd_inverse(ATT);
    ADD = ADD - ATD.transpose()*ATT*ATD;
    double mdd=0; for(int i=0;i<5;i++) mdd+=ADD(i,i);
    return mdd/10.0;
}
inline double brown_ew(const Eigen::Matrix<double,11,11>& AR){
    // Y2m basis with m=-2..2 mapped to index 0..4 (same 5 matrices as Y2()).
    const std::array<Matrix3d,5>& Y2m = Y2();
    MatrixXd ATT=AR.topLeftCorner(6,6); MatrixXd ATD=AR.block(0,6,6,5);
    ATT=spd_inverse(ATT);
    Matrix3d MRR=ATT.block(3,3,3,3); double trmrr=MRR.trace();
    // MRDS(3,5) = ATT(4:6,:) * ATD  (rows 3..5 of ATT, all 6 cols) * ATD(6x5)
    Eigen::Matrix<double,3,5> MRDS = ATT.block(3,0,3,6)*ATD;
    // MRD(m1,:,:) = sum_m2 MRDS(m1,m2) Y2m[m2]
    T3 MRD; MRD.zero();
    for(int m1=0;m1<3;m1++)for(int m2=0;m2<5;m2++)for(int i=0;i<3;i++)for(int j=0;j<3;j++)
        MRD.d[m1][i][j]+=MRDS(m1,m2)*Y2m[m2](i,j);
    Matrix3d H=Matrix3d::Zero();
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)for(int l=0;l<3;l++)
        H(i,j)+=EPS(i,k,l)*MRD.d[k][l][j]+EPS(j,k,l)*MRD.d[k][l][i];
    Eigen::Matrix<double,5,1> HS=Eigen::Matrix<double,5,1>::Zero();
    for(int m1=0;m1<5;m1++)for(int i=0;i<3;i++)for(int j=0;j<3;j++) HS(m1)+=Y2m[m1](i,j)*H(i,j);
    // EDE(i,j,k,l)=sum_pq EPS(i,k,p)EPS(j,l,q)MRR(p,q); LRR
    T4 LRR; LRR.zero();
    for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)for(int l=0;l<3;l++){
        double ede=0;
        for(int p=0;p<3;p++)for(int q=0;q<3;q++) ede+=EPS(i,k,p)*EPS(j,l,q)*MRR(p,q);
        LRR.d[i][j][k][l]=MRR(i,k)*U()(j,l)+U()(i,k)*MRR(j,l)+2.0*ede;
    }
    Eigen::Matrix<double,5,5> LRRS;
    for(int m1=0;m1<5;m1++)for(int m2=0;m2<5;m2++){
        double v=(m1==m2)?2.0*trmrr:0.0;
        for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)for(int l=0;l<3;l++)
            v-=Y2m[m1](i,j)*LRR.d[i][j][k][l]*Y2m[m2](k,l);
        LRRS(m1,m2)=v;
    }
    Eigen::Matrix<double,5,5> LRRSi = spd_inverse(LRRS);
    return (HS.array()*(LRRSi*HS).array()).sum()/10.0;
}
inline Vector3d calcrch(const Eigen::Matrix<double,6,6>& ATR){
    Matrix3d M; M.setZero();
    M(0,0)=ATR(3,5); M(0,1)=ATR(4,5); M(0,2)=-ATR(3,3)-ATR(4,4);
    M(1,0)=-ATR(3,4); M(1,1)=ATR(3,3)+ATR(5,5); M(1,2)=-ATR(5,4);
    M(2,0)=-ATR(4,4)-ATR(5,5); M(2,1)=ATR(4,3); M(2,2)=ATR(5,3);
    Vector3d rhs(ATR(4,0)-ATR(3,1), ATR(5,0)-ATR(3,2), ATR(5,1)-ATR(4,2));
    return M.partialPivLu().solve(rhs);
}
// EIJK for CALCATRCH (Levi-Civita, matches EIJK in source)
inline double EIJK(int i,int j,int k){ return EPS(i,j,k); }
inline void calcatrch(const Eigen::Matrix<double,11,11>& AR, const Vector3d& RCH,
                      Eigen::Matrix<double,6,6>& ATRCH, Matrix3d& EIV, Vector3d& DRR){
    ATRCH = AR.topLeftCorner(6,6);
    Eigen::Matrix<double,6,6> inv = spd_inverse(ATRCH);
    // TMP(:,:,2) accumulation, 1-based L,I,J,K,M,N mapped to 0-based
    Eigen::Matrix<double,6,6> T2 = Eigen::Matrix<double,6,6>::Zero();
    auto A=[&](int r,int c){ return inv(r,c); };
    for(int L=0;L<3;L++)for(int I=0;I<3;I++){
        for(int J=0;J<3;J++)for(int K=0;K<3;K++){
            for(int M=0;M<3;M++)for(int Nn=0;Nn<3;Nn++)
                T2(L,I)+=RCH(K)*EIJK(L,K,M)*A(3+M,3+Nn)*RCH(J)*EIJK(I,Nn,J);
            T2(3+I,L)+=RCH(J)*A(3+K,L)*EIJK(I,J,K);
            T2(L,3+I)+=A(L,3+J)*RCH(K)*EIJK(I,J,K);
            T2(3+L,3+I)+=A(3+L,3+J)*RCH(K)*EIJK(I,J,K);
        }
    }
    Eigen::Matrix<double,6,6> out = inv;
    out.block(0,0,3,3) = inv.block(0,0,3,3) - T2.block(0,0,3,3)
                         - T2.block(3,0,3,3) + T2.block(0,3,3,3);
    out.block(3,0,3,3) = inv.block(3,0,3,3) + T2.block(3,3,3,3);
    out.block(0,3,3,3) = out.block(3,0,3,3).transpose();
    // diagonalize rotational block
    Matrix3d ARR = out.block(3,3,3,3);
    Eigen::SelfAdjointEigenSolver<Matrix3d> es(ARR);
    DRR = es.eigenvalues();
    EIV = es.eigenvectors();
    Matrix3d EIVinv = EIV.inverse();
    ATRCH.block(3,3,3,3) = EIVinv*ARR*EIV;
    ATRCH.block(0,0,3,3) = EIVinv*out.block(0,0,3,3)*EIV;
    ATRCH.block(0,3,3,3) = EIVinv*out.block(0,3,3,3)*EIV;
    ATRCH.block(3,0,3,3) = EIVinv*out.block(3,0,3,3)*EIV;
}
inline void calctau(const Vector3d& DRR, double& DTAU, double tau[9]){
    DTAU=DRR.sum()/3.0;
    double delta=DRR(0)*DRR(0)+DRR(1)*DRR(1)+DRR(2)*DRR(2)
                 -DRR(0)*DRR(1)-DRR(0)*DRR(2)-DRR(1)*DRR(2);
    if (delta<=0) delta=0; delta=std::sqrt(delta);
    tau[1]=1.0/(6.0*DTAU+2.0*delta); tau[2]=1.0/(6.0*DTAU-2.0*delta);
    tau[3]=1.0/3.0/(DTAU+DRR(0)); tau[4]=1.0/3.0/(DTAU+DRR(1)); tau[5]=1.0/3.0/(DTAU+DRR(2));
    tau[6]=1.0/(3.0*DTAU-DRR(0)); tau[7]=1.0/(3.0*DTAU-DRR(1)); tau[8]=1.0/(3.0*DTAU-DRR(2));
}

}  // namespace core
