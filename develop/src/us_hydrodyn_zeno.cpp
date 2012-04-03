#include "include/us_hydrodyn_zeno.h"

static US_Hydrodyn  * zeno_us_hydrodyn;
static QProgressBar * zeno_progress;

namespace zeno {

   using namespace fem::major_types;
   
   void
   system(...)
   {
      throw std::runtime_error(
                            "Missing function implementation: system");
   }
   
   struct common_errors
   {
      fem::integer_star_4 nerr;
      arr<fem::integer_star_4> errorlist;

      common_errors() :
         nerr(fem::zero<fem::integer_star_4>()),
         errorlist(dimension(100), fem::fill0)
      {}
   };
   
   struct common_naughty
   {
      fem::str<10> badword;

      common_naughty() :
         badword(fem::char0)
      {}
   };

   struct common_shush
   {
      bool silent;

      common_shush() :
         silent(fem::bool0)
      {}
   };

   struct common :
      fem::common,
      common_errors,
      common_naughty,
      common_shush
   {
      fem::variant_core common_cubit;
      fem::variant_core common_sell;
      fem::variant_core common_random;
      fem::cmn_sve show_errors_sve;
      fem::cmn_sve seeder_sve;
      fem::cmn_sve ran2_sve;
      fem::cmn_sve parse_sve;
      fem::cmn_sve mincube_sve;
      fem::cmn_sve jrand_sve;
      fem::cmn_sve doverellipsoid_sve;
      fem::cmn_sve program_zeno_sve;

      common(
             int argc,
             char const* argv[])
         :
         fem::common(argc, argv)
      {}
   };

   //C
   //C*************************************************
   //C
   void
   begin_output(
                common& cmn,
                fem::integer_star_4 const& nout,
                str_cref id,
                fem::integer_star_4 const& nelts,
                str_arr_cref<> /* map */,
                arr_cref<fem::integer_star_4> /* eltype */,
                int const& /* maxelts */,
                arr_cref<fem::integer_star_4> /* mx */,
                arr_cref<fem::real_star_4, 2> /* bv */)
   {
      common_write write(cmn);
      //C
      //C        Send initial information to body file
      //C
      write(nout, "('Body name:  ',a25)"), id;
      write(nout, "('Number of body elements: ',i7)"), nelts;
      write(nout, "(50('='))");
      //C
   }

   //C
   //C********************************************************
   //C
   void
   summer(
          arr_cref<fem::real_star_8> a,
          arr_cref<fem::real_star_8> b,
          arr_ref<fem::real_star_8> c)
   {
      a(dimension(2));
      b(dimension(2));
      c(dimension(2));
      //C
      c(2) = fem::pow2(a(2)) + fem::pow2(b(2));
      c(2) = fem::dsqrt(c(2));
      c(1) = a(1) + b(1);
      //C
   }

   //C
   //C********************************************************
   //C
   void
   multiply(
            arr_cref<fem::real_star_8> a,
            arr_cref<fem::real_star_8> b,
            arr_ref<fem::real_star_8> c)
   {
      a(dimension(2));
      b(dimension(2));
      c(dimension(2));
      //C
      c(2) = (fem::pow2(b(1))) * (fem::pow2(a(2))) + (fem::pow2(a(1))) * (
                                                                          fem::pow2(b(2)));
      c(2) = fem::dsqrt(c(2));
      c(1) = a(1) * b(1);
      //C
   }

   //C
   //C********************************************************
   //C
   void
   divide(
          arr_cref<fem::real_star_8> x1,
          arr_cref<fem::real_star_8> x2,
          arr_ref<fem::real_star_8> y)
   {
      x1(dimension(2));
      x2(dimension(2));
      y(dimension(2));
      //C        real*8 (a-h,o-z)
      //C
      float d1 = 1.0e0 / x2(1);
      float d2 = -x1(1) / (fem::pow2(x2(1)));
      //C
      y(2) = (fem::pow2(d1)) * (fem::pow2(x1(2))) + (fem::pow2(d2)) * (
                                                                       fem::pow2(x2(2)));
      y(2) = fem::dsqrt(y(2));
      y(1) = x1(1) / x2(1);
      //C
   }

   //C
   //C********************************************************
   //C
   void
   power(
         arr_cref<fem::real_star_8> x,
         arr_ref<fem::real_star_8> y,
         fem::real_star_8 const& exponent)
   {
      x(dimension(2));
      y(dimension(2));
      //C        real*8 (a-h,o-z)
      //C
      y(1) = fem::pow(x(1), exponent);
      fem::real_star_8 d = exponent * (fem::pow(x(1), (exponent - 1.0e0)));
      //C
      y(2) = (fem::pow2(d)) * (fem::pow2(x(2)));
      y(2) = fem::dsqrt(y(2));
      //C
   }

   //C
   //C********************************************************
   //C
   void
   derive(
          str_cref lunit,
          str_ref aunit,
          str_ref volunit,
          str_ref nounit)
   {
      //C
      nounit = "      ";
      aunit = lunit;
      volunit = lunit;
      //C
      if (aunit(2, 2) == " ") {
         aunit(2, 3) = "^2";
         volunit(2, 3) = "^3";
      }
      else {
         aunit(3, 4) = "^2";
         volunit(3, 4) = "^3";
      }
      //C
   }

   //C
   //C********************************************************
   //C
   void
   loadconstants(
                 arr_ref<fem::real_star_8> q1,
                 arr_ref<fem::real_star_8> q2,
                 arr_ref<fem::real_star_8> pi,
                 arr_ref<fem::real_star_8> six,
                 arr_ref<fem::real_star_8> three,
                 arr_ref<fem::real_star_8> four,
                 arr_ref<fem::real_star_8> boltz,
                 arr_ref<fem::real_star_8> celsius,
                 arr_ref<fem::real_star_8> pre1,
                 arr_ref<fem::real_star_8> pre2)
   {
      q1(dimension(2));
      q2(dimension(2));
      pi(dimension(2));
      six(dimension(2));
      three(dimension(2));
      four(dimension(2));
      boltz(dimension(2));
      celsius(dimension(2));
      pre1(dimension(2));
      pre2(dimension(2));
      //C
      q1(1) = 1.0e0;
      q1(2) = 0.01e0;
      //C
      q2(1) = 0.79e0;
      q2(2) = 0.04e0;
      //C
      pi(1) = 3.14159265358979f;
      pi(2) = 0.0e0;
      //C
      six(1) = 6.0e0;
      six(2) = 0.0e0;
      //C
      three(1) = 3.0e0;
      three(2) = 0.0e0;
      //C
      four(1) = 4.0e0;
      four(2) = 0.0e0;
      //C
      boltz(1) = 1.38065e-23;
      boltz(2) = 1.0e-23 / 1.0e5;
      //C
      celsius(1) = 273.15e0;
      celsius(2) = 0.0e0;
      //C
      pre1(1) = fem::dsqrt(1.0e0 / (4.0e0 * pi(1)));
      pre2(1) = fem::dsqrt(1.0e0 / pi(1)) * (2.0e0 / pi(1));
      pre1(2) = 0.0e0;
      pre2(2) = 0.0e0;
      //C
   }

   //C
   //C********************************************************
   //C
   void
   parameters(
              bool const& bt,
              arr_ref<fem::real_star_8> temp,
              bool const& bc,
              arr_ref<fem::real_star_8> eta,
              str_ref tunit,
              str_ref vunit,
              arr_cref<fem::real_star_8> celsius)
   {
      temp(dimension(2));
      eta(dimension(2));
      celsius(dimension(2));
      //C
      //C  must convert temp to kelvin, eta to centipoise
      //C
      arr_1d<2, fem::real_star_8> t1(fem::fill0);
      arr_1d<2, fem::real_star_8> t2(fem::fill0);
      if (bt) {
         t1(1) = temp(1);
         t1(2) = temp(2);
         if (tunit == "C     ") {
            t2(1) = celsius(1);
            t2(2) = celsius(2);
            tunit = "K     ";
         }
         else if (tunit == "K     ") {
            t2(1) = 0.0e0;
            t2(2) = 0.0e0;
         }
         summer(t1, t2, temp);
      }
      //C
      if (bc) {
         t1(1) = eta(1);
         t1(2) = eta(2);
         if (vunit == "cp    ") {
            eta(1) = t1(1);
            eta(2) = t1(2);
         }
         else if (vunit == "p     ") {
            t2(1) = 100.0e0;
            t2(2) = 0.0e0;
            multiply(t1, t2, eta);
            vunit = "cp    ";
         }
      }
      //C
   }

   //C
   //C********************************************************
   //C
   void
   dotout(
          str_cref name,
          str_ref line)
   {
      int jj = fem::index(name, "@");
      line = name;
      line(jj, jj) = " ";
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, jj + 1, 30) {
         if (fem::mod(i, 2) == 1) {
            line(i, i) = " ";
         }
         if (fem::mod(i, 2) == 0) {
            line(i, i) = ".";
         }
      }
      //C
   }

   //C
   //C********************************************************
   //C
   void
   writeint(
            common& cmn,
            fem::integer_star_4 const& npop,
            fem::integer_star_4 const& mz,
            str_cref name)
   {
      common_write write(cmn);
      //C
      fem::str<30> line = fem::char0;
      dotout(name, line);
      write(npop, "(a30,i12)"), line, mz;
      //C
   }

   //C
   //C********************************************************
   //C
   void
   writeno(
           common& cmn,
           fem::integer_star_4 const& npop,
           fem::real_star_8 const& rl,
           str_cref name,
           str_cref lunit)
   {
      common_write write(cmn);
      //C
      fem::str<30> line = fem::char0;
      dotout(name, line);
      write(npop, "(a30,g15.6,4x,a6)"), line, rl, lunit;
      //C
   }

   //C
   //C********************************************************
   //C
   void
   compvisc(
            arr_cref<fem::real_star_8> temp,
            arr_ref<fem::real_star_8> eta,
            str_ref vunit)
   {
      temp(dimension(2));
      eta(dimension(2));
      //C
      //C        Ref.  CRC handbook of chemistry and physics,
      //C        55th edition p. F49
      //C
      fem::real_star_8 tc = temp(1) - 273.15e0;
      //C
      fem::real_star_8 tm = fem::zero<fem::real_star_8>();
      fem::real_star_8 den = fem::zero<fem::real_star_8>();
      fem::real_star_8 z = fem::zero<fem::real_star_8>();
      fem::real_star_8 tp = fem::zero<fem::real_star_8>();
      fem::real_star_8 num = fem::zero<fem::real_star_8>();
      if (tc <= 20.0e0) {
         tm = tc - 20.0e0;
         den = 998.333e0 + 8.1855e0 * tm + 0.00585e0 * (fem::pow2(tm));
         z = 1301.0e0 / den - 3.30233e0;
         eta(1) = fem::pow(10.0e0, z);
         eta(1) = eta(1) * 100.0e0;
         eta(2) = 0.001e0;
      }
      else {
         tm = tc - 20.0e0;
         tp = tc + 105.0e0;
         num = -1.3272e0 * tm - 0.001053f * tm * tm;
         z = num / tp;
         eta(1) = 1.002e0 * fem::pow(10.0e0, z);
         eta(2) = 0.0001e0;
      }
      //C
      vunit = "cp    ";
   }

   //C
   //C********************************************************
   //C
   void
   conventional(
                arr_cref<fem::real_star_8> t1,
                arr_ref<fem::real_star_8> etam,
                str_cref lunit,
                str_cref munit,
                str_ref intunit)
   {
      t1(dimension(2));
      etam(dimension(2));
      //C
      //C:  possibilities for lunit are  m,cm,nm,A,L
      //C:  possibilities for munit are  kg,g,Da,kDa
      //C
      //C        on entry, t1 contains the [eta]M value, but in units
      //C        lunit**3/munit.
      //C
      //C        This subroutine does the conversion to cm**3/gm
      //C        and returns the result in etam
      //C
      if (lunit == "L     ") {
         intunit(1, 4) = "L^3/";
         intunit(5, 6) = munit(1, 2);
         return;
      }
      //C
      arr_1d<2, fem::real_star_8> conv(fem::fill0);
      if (lunit == "m     ") {
         conv(1) = 1.0e6;
      }
      else if (lunit == "cm    ") {
         conv(1) = 1.0e0;
      }
      else if (lunit == "nm    ") {
         conv(1) = 1.0e-21;
      }
      else if (lunit == "A     ") {
         conv(1) = 1.0e-24;
      }
      else {
         FEM_STOP("bad lunit in conventional");
      }
      conv(2) = 0.0e0;
      //C
      arr_1d<2, fem::real_star_8> t2(fem::fill0);
      multiply(t1, conv, t2);
      //C
      if (munit == "kg    ") {
         conv(1) = 1.0e-3;
         conv(2) = 0.0e0;
      }
      else if (munit == "g     ") {
         conv(1) = 1.0e0;
         conv(2) = 0.0e0;
      }
      else if (munit == "Da    ") {
         conv(1) = 6.02214e23;
         conv(2) = 1.0e18;
      }
      else {
         conv(1) = 6.02214e20;
         conv(2) = 1.0e15;
      }
      //C
      multiply(t2, conv, etam);
      //C
      intunit = "cm^3/g";
      //C
   }

   //C
   //C********************************************************
   //C
   void
   diffuse(
           arr_cref<fem::real_star_8> t1,
           arr_ref<fem::real_star_8> d,
           str_cref lunit,
           str_ref dunit)
   {
      t1(dimension(2));
      d(dimension(2));
      //C
      //C        on entry, t1 contains the diffusion constant in
      //C        units J/(cp lunit)
      //C
      //C        This converts the diffusion constant to (cm^2/s)
      //C        and returns the result in d
      //C
      arr_1d<2, fem::real_star_8> conv(fem::fill0);
      if (lunit == "m     ") {
         conv(1) = 1.0e7;
      }
      else if (lunit == "cm    ") {
         conv(1) = 1.0e9;
      }
      else if (lunit == "nm    ") {
         conv(1) = 1.0e16;
      }
      else if (lunit == "A     ") {
         conv(1) = 1.0e17;
      }
      else {
         FEM_STOP("bad lunit in diffuse");
      }
      conv(2) = 0.0e0;
      //C
      multiply(t1, conv, d);
      //C
      dunit = "cm^2/s";
      //C
   }

   //C
   //C********************************************************
   //C
   void
   pack20(
          str_ref longname)
   {
      //C
      int nwk = 1;
      fem::str<1> b1 = fem::char0;
      fem::str<1> b2 = fem::char0;
      while (nwk < 20) {
         if (nwk == 0) {
            nwk = 1;
         }
         b1 = longname(nwk, nwk);
         b2 = longname(nwk + 1, nwk + 1);
         if (b1 == " " && b2 != " ") {
            longname(nwk, nwk) = b2;
            longname(nwk + 1, nwk + 1) = b1;
            nwk = nwk - 1;
         }
         else {
            nwk++;
         }
      }
      //C
   }

   //C
   //C*********************************************************
   //C
   void
   pack49(
          str_ref longname)
   {
      //C
      int nwk = 1;
      fem::str<1> b1 = fem::char0;
      fem::str<1> b2 = fem::char0;
      while (nwk < 49) {
         if (nwk == 0) {
            nwk = 1;
         }
         b1 = longname(nwk, nwk);
         b2 = longname(nwk + 1, nwk + 1);
         if (b1 == " " && b2 != " ") {
            longname(nwk, nwk) = b2;
            longname(nwk + 1, nwk + 1) = b1;
            nwk = nwk - 1;
         }
         else {
            nwk++;
         }
      }
      //C
   }

   //C
   //C*********************************************************
   //C
   void
   shiftleft(
             str_ref string)
   {
      //C
      int idx = fem::index(string, ".");
      if (idx == 0) {
         FEM_STOP("shiftleft error 1");
      }
      if (idx == 1) {
         FEM_STOP("shiftleft error 2");
      }
      //C
      fem::str<1> b1 = string(idx - 1, idx - 1);
      fem::str<1> b2 = string(idx, idx);
      string(idx - 1, idx - 1) = b2;
      string(idx, idx) = b1;
      //C
   }

   //C
   //C********************************************************
   //C
   void
   shiftright(
              str_ref string)
   {
      //C
      int idx = fem::index(string, ".");
      if (idx == 0) {
         FEM_STOP("shiftright error 1");
      }
      if (idx == 20) {
         FEM_STOP("shiftright error 2");
      }
      //C
      fem::str<1> b1 = string(idx + 1, idx + 1);
      fem::str<1> b2 = string(idx, idx);
      string(idx + 1, idx + 1) = b2;
      string(idx, idx) = b1;
      //C
   }

   //C
   //C********************************************************
   //C
   void
   writeyes(
            common& cmn,
            fem::integer_star_4 const& npop,
            arr_cref<fem::real_star_8> tx,
            str_cref name,
            str_cref u,
            bool const& tell_all)
   {
      tx(dimension(2));
      common_write write(cmn);
      //C
      //C  Operate on a copy
      arr_1d<2, fem::real_star_8> t(fem::fill0);
      t(1) = tx(1);
      t(2) = tx(2);
      //C
      fem::str<6> blanks = "      ";
      //C
      int k1 = fem::int0;
      int k2 = fem::int0;
      int nex = fem::int0;
      if (t(2) == 0.0e0) {
         //C
         k1 = fem::nint(t(1));
         k2 = 0;
         nex = 0;
         //C
         while (k1 > 999999 || 99999 >= k1) {
            //C
            if (k1 > 999999) {
               t(1) = t(1) / 10.0e0;
               nex++;
            }
            else if (99999 >= k1) {
               t(1) = t(1) * 10.0e0;
               nex = nex - 1;
            }
            else {
               FEM_STOP("writeyes 1 error");
            }
            //C
            k1 = fem::nint(t(1));
         }
         //C
      }
      else {
         //C
         nex = 0;
         //C
         while (t(2) < 0.95e0 || 9.5f <= t(2)) {
            if (t(2) < 0.95e0) {
               t(1) = t(1) * 10.0e0;
               t(2) = t(2) * 10.0e0;
               nex = nex - 1;
            }
            else if (9.5f <= t(2)) {
               t(1) = t(1) / 10.0e0;
               t(2) = t(2) / 10.0e0;
               nex++;
            }
            else {
               FEM_STOP("writeyes error");
            }
         }
         //C
         k1 = fem::nint(t(1));
         k2 = fem::nint(t(2));
         //C
      }
      //C
      fem::str<20> string = fem::char0;
      write(string, "(i10,'.(',i7,')')"), k1, k2;
      pack20(string);
      int ndes = fem::int0;
      if (string(1, 1) == "-") {
         ndes = 3;
      }
      else {
         ndes = 2;
      }
      //C
      int look = fem::index(string, ".");
      if (look == 0) {
         FEM_STOP("writeyes error 2");
      }
      while (look != ndes) {
         if (look > ndes) {
            shiftleft(string);
            nex++;
         }
         else {
            shiftright(string);
            nex = nex - 1;
         }
         look = fem::index(string, ".");
      }
      //C
      fem::str<30> line = fem::char0;
      dotout(name, line);
      //C
      fem::str<1> byte = fem::char0;
      if (nex < 0) {
         byte = "-";
         nex = fem::iabs(nex);
      }
      else {
         byte = "+";
      }
      //C
      fem::str<49> longname = fem::char0;
      if (nex == 0) {
         write(longname, "(a20,22x,'@',a6)"), string, blanks;
      }
      else if (nex >= 10) {
         write(longname, "(a20,'E',a1,i20,'@',a6)"), string, byte, nex, blanks;
      }
      else {
         write(longname, "(a20,'E',a1,i20.2,'@',a6)"), string, byte, nex, blanks;
      }
      //C
      //C                20 + 1 +1+ 20+  1 + 6 = 49
      //C
      pack49(longname);
      //C
      int jq = fem::index(longname, ".");
      if (longname(jq + 1, jq + 1) == "(") {
         longname(jq, jq) = " ";
         pack49(longname);
      }
      //C
      int jj = fem::index(longname, "@");
      longname(jj, jj) = " ";
      fem::str<49> copy = fem::char0;
      if (longname(1, 1) == "-") {
      }
      else {
         copy(2, 49) = longname(1, 48);
         copy(1, 1) = " ";
         longname = copy;
      }
      longname(19, 24) = u;
      //C
      write(npop, "(a30,1x,a49)"), line, longname;
      if (tell_all) {
         write(npop, "(2g20.8)"), tx;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   poplar(
          common& cmn,
          fem::integer_star_4 const& nzno,
          fem::integer_star_4 const& mz,
          fem::integer_star_4 const& ms,
          fem::integer_star_4 const& mi,
          bool const& bl,
          bool const& bk,
          bool const& bs,
          bool const& bi,
          bool const& bt,
          bool const& bm,
          bool const& bw,
          bool const& bc,
          bool const& bz,
          fem::real_star_8 const& rl,
          fem::real_star_8 const& eps,
          arr_cref<fem::real_star_8> cap,
          arr_cref<fem::real_star_8> a11,
          arr_cref<fem::real_star_8> a12,
          arr_cref<fem::real_star_8> a13,
          arr_cref<fem::real_star_8> a21,
          arr_cref<fem::real_star_8> a22,
          arr_cref<fem::real_star_8> a23,
          arr_cref<fem::real_star_8> a31,
          arr_cref<fem::real_star_8> a32,
          arr_cref<fem::real_star_8> a33,
          arr_cref<fem::real_star_8> rk,
          arr_cref<fem::real_star_8> surf,
          arr_cref<fem::real_star_8> rg2surf,
          arr_cref<fem::real_star_8> v,
          arr_cref<fem::real_star_8> rg2int,
          arr_ref<fem::real_star_8> temp,
          arr_cref<fem::real_star_8> mass,
          arr_ref<fem::real_star_8> eta,
          str_cref lunit,
          str_ref tunit,
          str_cref munit,
          str_ref vunit,
          bool const& tell_all)
   {
      cap(dimension(2));
      a11(dimension(2));
      a12(dimension(2));
      a13(dimension(2));
      a21(dimension(2));
      a22(dimension(2));
      a23(dimension(2));
      a31(dimension(2));
      a32(dimension(2));
      a33(dimension(2));
      rk(dimension(2));
      surf(dimension(2));
      rg2surf(dimension(2));
      v(dimension(2));
      rg2int(dimension(2));
      temp(dimension(2));
      mass(dimension(2));
      eta(dimension(2));
      //C
      //C        post-processor for zeno.f
      //C
      //C  Square root of surface are
      //C  Russell and Rayleigh aprox. to C
      //C
      fem::integer_star_4 npop = nzno;
      //C
      bool bu = !(lunit == "L     ");
      //C
      arr_1d<2, fem::real_star_8> q1(fem::fill0);
      arr_1d<2, fem::real_star_8> q2(fem::fill0);
      arr_1d<2, fem::real_star_8> pi(fem::fill0);
      arr_1d<2, fem::real_star_8> six(fem::fill0);
      arr_1d<2, fem::real_star_8> three(fem::fill0);
      arr_1d<2, fem::real_star_8> four(fem::fill0);
      arr_1d<2, fem::real_star_8> boltz(fem::fill0);
      arr_1d<2, fem::real_star_8> celsius(fem::fill0);
      arr_1d<2, fem::real_star_8> pre1(fem::fill0);
      arr_1d<2, fem::real_star_8> pre2(fem::fill0);
      loadconstants(q1, q2, pi, six, three, four, boltz, celsius, pre1, pre2);
      //C
      fem::str<6> aunit = fem::char0;
      fem::str<6> volunit = fem::char0;
      fem::str<6> nounit = fem::char0;
      derive(lunit, aunit, volunit, nounit);
      //C
      parameters(bt, temp, bc, eta, tunit, vunit, celsius);
      //C  must convert temp to kelvin, eta to centipoise
      //C
      fem::str<30> name = fem::char0;
      if (bl) {
         name = "launch radius@";
         writeno(cmn, npop, rl, name, lunit);
      }
      //C
      if (bk) {
         name = "skin thickness@";
         writeno(cmn, npop, eps, name, lunit);
      }
      //C
      if (bt) {
         name = "temperature@";
         writeyes(cmn, npop, temp, name, tunit, tell_all);
      }
      //C
      if (bm) {
         name = "mass@";
         writeyes(cmn, npop, mass, name, munit, tell_all);
      }
      //C
      bool bv = false;
      if (bc) {
         name = "solvent viscosity (supplied)@";
         writeyes(cmn, npop, eta, name, vunit, tell_all);
         bv = true;
      }
      //C
      if ((!bc) && bt && bw) {
         compvisc(temp, eta, vunit);
         bv = true;
         name = "solvent viscosity (computed)@";
         writeyes(cmn, npop, eta, name, vunit, tell_all);
      }
      //C
      if (bz) {
         name = "zeno m.c. steps@";
         writeint(cmn, npop, mz, name);
         name = "capacitance, C@";
         writeyes(cmn, npop, cap, name, lunit, tell_all);
         name = "polarizability 11@";
         writeyes(cmn, npop, a11, name, volunit, tell_all);
         name = "polarizability 12@";
         writeyes(cmn, npop, a12, name, volunit, tell_all);
         name = "polarizability 13@";
         writeyes(cmn, npop, a13, name, volunit, tell_all);
         name = "polarizability 21@";
         writeyes(cmn, npop, a21, name, volunit, tell_all);
         name = "polarizability 22@";
         writeyes(cmn, npop, a22, name, volunit, tell_all);
         name = "polarizability 23@";
         writeyes(cmn, npop, a23, name, volunit, tell_all);
         name = "polarizability 31@";
         writeyes(cmn, npop, a31, name, volunit, tell_all);
         name = "polarizability 32@";
         writeyes(cmn, npop, a32, name, volunit, tell_all);
         name = "polarizability 33@";
         writeyes(cmn, npop, a33, name, volunit, tell_all);
      }
      //C
      fem::real_star_8 exponent = fem::zero<fem::real_star_8>();
      arr_1d<2, fem::real_star_8> rgsurf(fem::fill0);
      arr_1d<2, fem::real_star_8> sqsurf(fem::fill0);
      arr_1d<2, fem::real_star_8> cruss(fem::fill0);
      arr_1d<2, fem::real_star_8> cray(fem::fill0);
      if (bs) {
         name = "surface m.c. steps@";
         writeint(cmn, npop, ms, name);
         name = "RK@";
         writeyes(cmn, npop, rk, name, lunit, tell_all);
         name = "surface area@";
         writeyes(cmn, npop, surf, name, aunit, tell_all);
         exponent = 1.0e0 / 2.0e0;
         power(rg2surf, rgsurf, exponent);
         name = "Rg (surface)@";
         writeyes(cmn, npop, rgsurf, name, lunit, tell_all);
         exponent = 1.0e0 / 2.0e0;
         power(surf, sqsurf, exponent);
         multiply(sqsurf, pre1, cruss);
         multiply(sqsurf, pre2, cray);
         name = "R(Russell)@";
         writeyes(cmn, npop, cruss, name, lunit, tell_all);
         name = "R(Rayleigh)@";
         writeyes(cmn, npop, cray, name, lunit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> rgint(fem::fill0);
      if (bi) {
         name = "interior m.c. steps@";
         writeint(cmn, npop, mi, name);
         name = "volume@";
         writeyes(cmn, npop, v, name, volunit, tell_all);
         exponent = 1.0e0 / 2.0e0;
         power(rg2int, rgint, exponent);
         name = "Rg (interior)@";
         writeyes(cmn, npop, rgint, name, lunit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> rh(fem::fill0);
      arr_1d<2, fem::real_star_8> t1(fem::fill0);
      arr_1d<2, fem::real_star_8> trace(fem::fill0);
      arr_1d<2, fem::real_star_8> vh(fem::fill0);
      if (bz) {
         multiply(q1, cap, rh);
         name = "Rh@";
         writeyes(cmn, npop, rh, name, lunit, tell_all);
         summer(a11, a22, t1);
         summer(t1, a33, trace);
         name = "Tr(alpha)@";
         writeyes(cmn, npop, trace, name, volunit, tell_all);
         multiply(q2, trace, t1);
         divide(t1, three, vh);
         name = "Vh@";
         writeyes(cmn, npop, vh, name, volunit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> t2(fem::fill0);
      arr_1d<2, fem::real_star_8> c0(fem::fill0);
      if (bi) {
         multiply(three, v, t1);
         divide(t1, four, t2);
         divide(t2, pi, t1);
         exponent = 1.0f / 3.0f;
         power(t1, c0, exponent);
         name = "C0@";
         writeyes(cmn, npop, c0, name, lunit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> sig11(fem::fill0);
      arr_1d<2, fem::real_star_8> sig22(fem::fill0);
      arr_1d<2, fem::real_star_8> sig33(fem::fill0);
      arr_1d<2, fem::real_star_8> signorm(fem::fill0);
      arr_1d<2, fem::real_star_8> sigma(fem::fill0);
      arr_1d<2, fem::real_star_8> etav(fem::fill0);
      if (bz && bi) {
         divide(a11, three, t1);
         divide(t1, v, sig11);
         name = "sig 11@";
         writeyes(cmn, npop, sig11, name, nounit, tell_all);
         divide(a22, three, t1);
         divide(t1, v, sig22);
         name = "sig 22@";
         writeyes(cmn, npop, sig22, name, nounit, tell_all);
         divide(a33, three, t1);
         divide(t1, v, sig33);
         name = "sig 33@";
         writeyes(cmn, npop, sig33, name, nounit, tell_all);
         summer(sig11, sig22, signorm);
         name = "sig(normal)@";
         writeyes(cmn, npop, signorm, name, nounit, tell_all);
         name = "sig(parallel)@";
         writeyes(cmn, npop, sig33, name, nounit, tell_all);
         summer(signorm, sig33, sigma);
         name = "sigma@";
         writeyes(cmn, npop, sigma, name, nounit, tell_all);
         divide(vh, v, etav);
         name = "[eta](V)@";
         writeyes(cmn, npop, etav, name, nounit, tell_all);
         divide(cap, rgint, t1);
         name = "C/Rg(int)@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(rgint, rh, t1);
         name = "Rg(int)/Rh@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(cap, c0, t1);
         name = "C/C0@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(v, vh, t1);
         name = "V/Vh@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
      }
      //C
      if (bz && bs) {
         divide(cap, rk, t1);
         name = "C/RK@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(cap, rgsurf, t1);
         name = "C/Rg(surf)@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(rgsurf, rh, t1);
         name = "Rg(surf)/Rh@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(cruss, cap, t1);
         name = "R(Russell)/C@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
         divide(cray, cap, t1);
         name = "R(Rayleigh)/C@";
         writeyes(cmn, npop, t1, name, nounit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> ssig(fem::fill0);
      if (bi && bs) {
         multiply(six, six, t1);
         multiply(t1, pi, t2);
         exponent = 1.0e0 / 3.0e0;
         power(t2, t1, exponent);
         divide(surf, t1, t2);
         exponent = 2.0e0 / 3.0e0;
         power(v, t1, exponent);
         divide(t2, t1, ssig);
         name = "sphericity@";
         writeyes(cmn, npop, ssig, name, nounit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> etam(fem::fill0);
      fem::str<6> intunit = fem::char0;
      if (bz && bm) {
         divide(vh, mass, t1);
         conventional(t1, etam, lunit, munit, intunit);
         name = "[eta](M)@";
         writeyes(cmn, npop, etam, name, intunit, tell_all);
      }
      //C
      arr_1d<2, fem::real_star_8> d(fem::fill0);
      fem::str<6> dunit = fem::char0;
      if (bz && bt && bv && bu) {
         multiply(boltz, temp, t1);
         divide(t1, six, t2);
         divide(t2, pi, t1);
         divide(t1, eta, t2);
         divide(t2, rh, t1);
         diffuse(t1, d, lunit, dunit);
         name = "D@";
         writeyes(cmn, npop, d, name, dunit, tell_all);
      }
      //C
      cmn.io.close(npop);
      //C
   }

   struct show_errors_save
   {
      fem::variant_bindings cubit_bindings;
      fem::variant_bindings sell_bindings;
   };

   //C
   //C********************************************************
   //C
   void
   show_errors(
               common& cmn,
               fem::integer_star_4 const& nout)
   {
      FEM_CMN_SVE(show_errors);
      common_write write(cmn);
      // COMMON errors
      fem::integer_star_4& nerr = cmn.nerr;
      arr_cref<fem::integer_star_4> errorlist(cmn.errorlist, dimension(100));
      // COMMON naughty
      fem::str<10>& badword = cmn.badword;
      //
      common_variant cubit(cmn.common_cubit, sve.cubit_bindings);
      common_variant sell(cmn.common_sell, sve.sell_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<fem::integer_star_4> ncube;
            mbr<fem::real_star_4> ferr;
            cubit.allocate(), ncube, ferr;
         }
         {
            mbr<fem::integer_star_4> nell;
            mbr<fem::real_star_4> rerr;
            sell.allocate(), nell, rerr;
         }
      }
      fem::integer_star_4 const& ncube = cubit.bind<fem::integer_star_4>();
      fem::real_star_4 const& ferr = cubit.bind<fem::real_star_4>();
      fem::integer_star_4 const& nell = sell.bind<fem::integer_star_4>();
      /* fem::real_star_4 const& rerr */ sell.bind<fem::real_star_4>();
      static const char* format_100 = "(' ')";
      static const char* format_205 = "(50('='))";
      //C
      //C        Send error list to report file
      //C
      int ntotal = nerr + ncube + nell;
      //C
      if (ntotal == 0) {
      }
      else {
         write(nout, format_205);
         write(nout, "('ERRORS DETECTED')");
         write(nout, format_100);
      }
      //C
      if (ntotal == 0) {
         return;
      }
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, nerr) {
         if (errorlist(i) == 1) {
            write(nout, "('  Unrecognized word in body file: ',a10)"), badword;
            write(nout, format_100);
         }
         if (errorlist(i) == 2) {
            write(nout, "('  Max number of elements exceeded.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 3) {
            write(nout, "('  Read past end of body file.')");
            write(nout, "('  The last command is probably missing')");
            write(nout, "('  some arguments.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 4) {
            write(nout, "('  Trying to read this string as numeric: ',a10)"), badword;
            write(nout, format_100);
         }
         if (errorlist(i) == 5) {
            write(nout, "('  No elements detected on body file.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 6) {
            //C                error code of 6 has been taken
            //C                from the list
            FEM_STOP("should not be here");
         }
         if (errorlist(i) == 7) {
            write(nout, "('  Body element of type: ',a10)"), badword;
            write(nout, "('  is inconsistent with INTERIOR calculation.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 8) {
            write(nout, "('  Attempt to form lens from non-overlapping spheres.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 9) {
            write(nout, "('  Bad UNITS specification:  ',a10)"), badword;
            write(nout, format_100);
         }
         if (errorlist(i) == 10) {
            write(nout, "('  The following string is not a valid predicate,')");
            write(nout, "('  although you have placed it in the position of')");
            write(nout, "('  a predicate:  ',a10)"), badword;
            write(nout, format_100);
         }
         if (errorlist(i) == 11) {
            write(nout, "('  Unrecognized units for temp, mass, or viscosity')");
            write(nout, "('  command: ',a10)");
            write(nout, format_100);
         }
         if (errorlist(i) == 12) {
            write(nout, "('  Viscosity and solvent commands both found.')");
            write(nout, "('  The second is being ignored.')");
            write(nout, format_100);
         }
         if (errorlist(i) == 13) {
            write(nout,
                  "('  Unrecognized modifier for solvent command: ',a10)"),
               badword;
            write(nout, format_100);
         }
      }
      //C
      if (ncube == 1) {
         write(nout, "('  Point found inside cube.  Only a cause')");
         write(nout, "('  for concern if the following number is')");
         write(nout, "('  not close to 1:',g20.8)"), ferr;
         write(nout, format_100);
      }
      //C
      if (nell == 1) {
         write(nout, "('  Overstretched ellipsoid.  Only a cause')");
         write(nout, "('  for concern if the following number is')");
         write(nout, "('  not close to 1:',g20.8)");
         write(nout, format_100);
      }
      //C
      write(nout, format_205);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   report(
          common& cmn,
          str_cref /* id */,
          str_cref actions,
          arr_cref<fem::integer_star_4> m1,
          fem::integer_star_4 const& /* nelts */,
          fem::real_star_4 const& tol,
          float const& rlaunch,
          fem::real_star_4 const& cap,
          fem::real_star_4 const& delta_cap,
          arr_cref<fem::real_star_4, 2> alpha_bongo,
          arr_cref<fem::real_star_4, 2> delta_bongo,
          fem::real_star_4 const& volume,
          fem::real_star_4 const& delta_volume,
          fem::real_star_4 const& surface,
          fem::real_star_4 const& delta_surface,
          fem::real_star_4 const& rg2int,
          fem::real_star_4 const& delta_rg2int,
          fem::real_star_4 const& rg2surf,
          fem::real_star_4 const& delta_rg2surf,
          fem::real_star_4 const& kirk,
          fem::real_star_4 const& delta_kirk,
          bool const& zeno_done,
          bool const& kirk_done,
          bool const& rg_done,
          bool const& launch_done,
          bool const& tol_given,
          fem::integer_star_4 const& nout,
          str_cref unitcode,
          int const& mz,
          int const& mi,
          int const& ms,
          bool const& bt,
          bool const& bm,
          bool const& bw,
          bool const& bc,
          arr_ref<fem::real_star_8> temp,
          str_ref tunit,
          arr_cref<fem::real_star_8> mass,
          str_cref munit,
          arr_ref<fem::real_star_8> visc,
          str_ref vunit,
          bool const& tell_all)
   {
      m1(dimension(3));
      alpha_bongo(dimension(3, 3));
      delta_bongo(dimension(3, 3));
      temp(dimension(2));
      mass(dimension(2));
      visc(dimension(2));
      common_write write(cmn);
      //C
      //C        Generate program output to report file
      //C
      fem::real_star_4 pi = 3.14159265f;
      //C
      fem::str<6> lunit = unitcode;
      lunit(3, 6) = "    ";
      //C
      show_errors(cmn, nout);
      //C
      write(nout, "('JOB SUMMARY:')");
      write(nout, "('       Actions      Checked if    Monte Carlo')");
      write(nout, "('       requested    completed     size')");
      write(nout, "(50('-'))");
      //C
      //C        Actions      Checked if    Monte Carlo
      //C        requested    completed     size
      //C==================================================
      //C            z            *          1000000
      //C000000000111111111122222222223333333333444
      //C123456789012345678901234567890123456789012
      //C
      int i = fem::int0;
      bool do_something = fem::bool0;
      fem::str<1> b1 = fem::char0;
      fem::str<1> b2 = fem::char0;
      int nzip = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         do_something = false;
         if (actions(i, i) == "z") {
            b1 = "z";
            b2 = " ";
            if (zeno_done) {
               b2 = "*";
            }
            nzip = m1(i);
            do_something = true;
         }
         else if (actions(i, i) == "i") {
            b1 = "i";
            b2 = " ";
            if (rg_done) {
               b2 = "*";
            }
            nzip = m1(i);
            do_something = true;
         }
         else if (actions(i, i) == "s") {
            b1 = "s";
            b2 = " ";
            if (kirk_done) {
               b2 = "*";
            }
            nzip = m1(i);
            do_something = true;
         }
         if (do_something) {
            write(nout, "(11x,a1,12x,a1,7x,i10)"), b1, b2, nzip;
         }
      }
      write(nout, "(50('='))");
      //C
      bool bl = launch_done;
      fem::real_star_8 rl = fem::dble(rlaunch);
      bool bk = tol_given;
      fem::real_star_8 eps = fem::dble(tol);
      bool bz = zeno_done;
      arr_1d<2, fem::real_star_8> capd(fem::fill0);
      int j = fem::int0;
      arr_2d<3, 3, fem::real_star_4> alpha_paper(fem::fill0);
      arr_2d<3, 3, fem::real_star_4> delta_paper(fem::fill0);
      arr_1d<2, fem::real_star_8> a11(fem::fill0);
      arr_1d<2, fem::real_star_8> a12(fem::fill0);
      arr_1d<2, fem::real_star_8> a13(fem::fill0);
      arr_1d<2, fem::real_star_8> a21(fem::fill0);
      arr_1d<2, fem::real_star_8> a22(fem::fill0);
      arr_1d<2, fem::real_star_8> a23(fem::fill0);
      arr_1d<2, fem::real_star_8> a31(fem::fill0);
      arr_1d<2, fem::real_star_8> a32(fem::fill0);
      arr_1d<2, fem::real_star_8> a33(fem::fill0);
      if (zeno_done) {
         capd(1) = fem::dble(cap);
         capd(2) = fem::dble(delta_cap);
         FEM_DO_SAFE(i, 1, 3) {
            FEM_DO_SAFE(j, 1, 3) {
               alpha_paper(i, j) = alpha_bongo(i, j) * 4.0f * pi;
               delta_paper(i, j) = delta_bongo(i, j) * 4.0f * pi;
            }
         }
         a11(1) = fem::dble(alpha_paper(1, 1));
         a12(1) = fem::dble(alpha_paper(1, 2));
         a13(1) = fem::dble(alpha_paper(1, 3));
         a21(1) = fem::dble(alpha_paper(2, 1));
         a22(1) = fem::dble(alpha_paper(2, 2));
         a23(1) = fem::dble(alpha_paper(1, 3));
         a31(1) = fem::dble(alpha_paper(3, 1));
         a32(1) = fem::dble(alpha_paper(3, 2));
         a33(1) = fem::dble(alpha_paper(3, 3));
         a11(2) = fem::dble(delta_paper(1, 1));
         a12(2) = fem::dble(delta_paper(1, 2));
         a13(2) = fem::dble(delta_paper(1, 3));
         a21(2) = fem::dble(delta_paper(2, 1));
         a22(2) = fem::dble(delta_paper(2, 2));
         a23(2) = fem::dble(delta_paper(2, 3));
         a31(2) = fem::dble(delta_paper(3, 1));
         a32(2) = fem::dble(delta_paper(3, 2));
         a33(2) = fem::dble(delta_paper(3, 3));
      }
      //C
      bool bs = kirk_done;
      arr_1d<2, fem::real_star_8> rk(fem::fill0);
      arr_1d<2, fem::real_star_8> surf(fem::fill0);
      arr_1d<2, fem::real_star_8> rg2surfd(fem::fill0);
      if (kirk_done) {
         rk(1) = fem::dble(kirk);
         rk(2) = fem::dble(delta_kirk);
         surf(1) = fem::dble(surface);
         surf(2) = fem::dble(delta_surface);
         rg2surfd(1) = fem::dble(rg2surf);
         rg2surfd(2) = fem::dble(delta_rg2surf);
      }
      //C
      bool bi = rg_done;
      arr_1d<2, fem::real_star_8> v(fem::fill0);
      arr_1d<2, fem::real_star_8> rg2intd(fem::fill0);
      if (rg_done) {
         v(1) = fem::dble(volume);
         v(2) = fem::dble(delta_volume);
         rg2intd(1) = fem::dble(rg2int);
         rg2intd(2) = fem::dble(delta_rg2int);
      }
      //C
      poplar(cmn, nout, mz, ms, mi, bl, bk, bs, bi, bt, bm, bw, bc, bz,
             rl, eps, capd, a11, a12, a13, a21, a22, a23, a31, a32, a33, rk,
             surf, rg2surfd, v, rg2intd, temp, mass, visc, lunit, tunit, munit,
             vunit, tell_all);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   pythag0(
           arr_cref<fem::real_star_4> x,
           fem::real_star_4& d)
   {
      x(dimension(3));
      //C
      //C        Compute distance between x and origin by the pythagorean theorem
      //C
      d = fem::sqrt(fem::pow2(x(1)) + fem::pow2(x(2)) + fem::pow2(x(3)));
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxcube(
           arr_cref<fem::real_star_4> v1,
           float const& side,
           fem::real_star_4& dd)
   {
      v1(dimension(3));
      //C
      //C        Given a cube with low corner at v1 and high corner at v2,
      //C        Return in dd the maximum distance between all points on the surface of
      //C        the cube and the origin.
      //C
      int ix = fem::int0;
      int iy = fem::int0;
      int iz = fem::int0;
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      float d = fem::float0;
      FEM_DO_SAFE(ix, 0, 1) {
         FEM_DO_SAFE(iy, 0, 1) {
            FEM_DO_SAFE(iz, 0, 1) {
               v2(1) = v1(1) + fem::ffloat(ix) * side;
               v2(2) = v1(2) + fem::ffloat(iy) * side;
               v2(3) = v1(3) + fem::ffloat(iz) * side;
               pythag0(v2, d);
               if (ix + iy + iz == 0) {
                  dd = d;
               }
               else {
                  dd = fem::amax1(dd, d);
               }
            }
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxsphere(
             arr_cref<fem::real_star_4> c,
             fem::real_star_4 const& r,
             fem::real_star_4& dd)
   {
      c(dimension(3));
      //C
      //C        Given a sphere with center at an arbitrary point c and with radius r.
      //C        Return in dd the maximum distance between all points on the surface of
      //C        the sphere and the origin.
      //C
      fem::real_star_4 d1 = fem::zero<fem::real_star_4>();
      pythag0(c, d1);
      dd = d1 + r;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxtriangle(
               arr_cref<fem::real_star_4> v1,
               arr_cref<fem::real_star_4> v2,
               arr_cref<fem::real_star_4> v3,
               fem::real_star_4& dd)
   {
      v1(dimension(3));
      v2(dimension(3));
      v3(dimension(3));
      //C
      //C        Given a triangle with vertices at v1, v2, v3.
      //C        Return in dd the maximum distance between all points on the surface
      //C        of the triangle and the origin.
      //C
      //C        Assumption:  One of the three vertices is at the maximum.
      //C
      fem::real_star_4 d1 = fem::zero<fem::real_star_4>();
      pythag0(v1, d1);
      fem::real_star_4 d2 = fem::zero<fem::real_star_4>();
      pythag0(v2, d2);
      fem::real_star_4 d3 = fem::zero<fem::real_star_4>();
      pythag0(v3, d3);
      fem::real_star_4 dd1 = fem::amax1(d1, d2);
      dd = fem::amax1(dd1, d3);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   rotate(
          arr_ref<fem::real_star_4> a,
          arr_cref<fem::real_star_4, 2> t,
          arr_cref<fem::real_star_4> b)
   {
      a(dimension(3));
      t(dimension(3, 3));
      b(dimension(3));
      //C        t is a rotation matrix.  Return in a the product:
      //C        a = t.b
      //C
      int i = fem::int0;
      int j = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         a(i) = 0.0f;
         FEM_DO_SAFE(j, 1, 3) {
            a(i) += t(i, j) * b(j);
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   scalar_product(
                  fem::real_star_4 const& a_scalar,
                  arr_cref<fem::real_star_4> b_vector,
                  arr_ref<fem::real_star_4> c)
   {
      b_vector(dimension(3));
      c(dimension(3));
      //C
      //C        c = a_scalar * b_vector
      //C
      c(1) = a_scalar * b_vector(1);
      c(2) = a_scalar * b_vector(2);
      c(3) = a_scalar * b_vector(3);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxdisk(
           arr_cref<fem::real_star_4> c,
           arr_cref<fem::real_star_4> /* n */,
           fem::real_star_4 const& r,
           arr_cref<fem::real_star_4, 2> t,
           fem::real_star_4& dd)
   {
      c(dimension(3));
      t(dimension(3, 3));
      //C
      //C        Given a disk centered at c, with unit normal n and radius r.
      //C        This returns the maximum distance of points on the disk away
      //C        from the origin.
      //C
      arr_1d<3, fem::real_star_4> oi(fem::fill0);
      scalar_product(-1.0f, c, oi);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, oi);
      //C
      float qq = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      dd = fem::pow2(q(3)) + fem::pow2((qq + r));
      dd = fem::sqrt(dd);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxcylinder(
               arr_cref<fem::real_star_4> c,
               arr_cref<fem::real_star_4> /* n */,
               fem::real_star_4 const& r,
               fem::real_star_4 const& l,
               arr_cref<fem::real_star_4, 2> t,
               fem::real_star_4& dd)
   {
      c(dimension(3));
      t(dimension(3, 3));
      //C
      //C        Given a cylinder centered at c, with unit normal n, radius r,
      //C                length l.
      //C        This returns the maximum distance of points on the cylinder
      //C        away from the origin.
      //C
      arr_1d<3, fem::real_star_4> oi(fem::fill0);
      scalar_product(-1.0f, c, oi);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, oi);
      //C
      float rr = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      float zz = q(3);
      float dd1 = fem::sqrt(fem::pow2((rr + r)) + fem::pow2((zz - l / 2.0f)));
      float dd2 = fem::sqrt(fem::pow2((rr + r)) + fem::pow2((zz + l / 2.0f)));
      dd = fem::amax1(dd1, dd2);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxtorus(
            arr_cref<fem::real_star_4> c,
            arr_cref<fem::real_star_4> /* n */,
            fem::real_star_4 const& r1,
            fem::real_star_4 const& r2,
            arr_cref<fem::real_star_4, 2> t,
            fem::real_star_4& dd)
   {
      c(dimension(3));
      t(dimension(3, 3));
      //C
      //C        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
      //C
      //C        Given a torus centered at c, with unit normal n, radii r1 and r2.
      //C        This returns the maximum distance of points on the torus away
      //C        from the origin.
      //C
      //C                  x                         x
      //C             x       x                 x       x
      //C            x         x               x         x   ___
      //C            x         x               x         x    |
      //C             x       x                 x       x     |  r2
      //C                 x                         x        ---
      //C
      //C                 |---------2 * r1 ---------|
      //C
      arr_1d<3, fem::real_star_4> oi(fem::fill0);
      scalar_product(-1.0f, c, oi);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, oi);
      float qq = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      //C
      float sig = q(3) / (qq - r1);
      float dod = fem::sqrt(fem::pow2(r2) / (1.0f + fem::pow2(sig)));
      float x1 = r1 + dod;
      float x2 = r1 - dod;
      float y1 = sig * dod;
      float y2 = -sig * dod;
      float d1 = fem::pow2((qq - x1)) + fem::pow2((q(3) - y1));
      float d2 = fem::pow2((qq - x2)) + fem::pow2((q(3) - y2));
      //C
      sig = q(3) / (qq + r1);
      dod = fem::sqrt(fem::pow2(r2) / (1.0f + fem::pow2(sig)));
      x1 = -r1 + dod;
      x2 = -r1 - dod;
      y1 = sig * dod;
      y2 = -sig * dod;
      float d3 = fem::pow2((qq - x1)) + fem::pow2((q(3) - y1));
      float d4 = fem::pow2((qq - x2)) + fem::pow2((q(3) - y2));
      //C
      dd = fem::amax1(d1, d2);
      dd = fem::amax1(dd, d3);
      dd = fem::amax1(dd, d4);
      //C
      dd = fem::sqrt(dd);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   vector_difference(
                     arr_cref<fem::real_star_4> a,
                     arr_cref<fem::real_star_4> b,
                     arr_ref<fem::real_star_4> c)
   {
      a(dimension(3));
      b(dimension(3));
      c(dimension(3));
      //C
      //C        c = a - b
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         c(i) = a(i) - b(i);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   pythag(
          arr_cref<fem::real_star_4> x,
          arr_cref<fem::real_star_4> y,
          float& d)
   {
      x(dimension(3));
      y(dimension(3));
      //C
      //C        Compute distance between two points x and y by the
      //C        pythagorian theorem
      //C
      d = 0.0f;
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         d += fem::pow2((x(i) - y(i)));
      }
      d = fem::sqrt(d);
   }

   //C
   //C*************************************************
   //C
   void
   dotproduct(
              arr_cref<fem::real_star_4> a,
              arr_cref<fem::real_star_4> b,
              fem::real_star_4& c)
   {
      a(dimension(3));
      b(dimension(3));
      //C
      //C        c = a . b
      //C
      c = 0.0f;
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         c += a(i) * b(i);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   normalize(
             arr_cref<fem::real_star_4> n1,
             arr_ref<fem::real_star_4> n)
   {
      n1(dimension(3));
      n(dimension(3));
      //C
      float z = fem::float0;
      dotproduct(n1, n1, z);
      z = fem::sqrt(z);
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         n(i) = n1(i) / z;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   sphereaway(
              arr_cref<fem::real_star_4> c,
              fem::real_star_4 const& rc,
              arr_ref<fem::real_star_4> p)
   {
      c(dimension(3));
      p(dimension(3));
      //C
      //C        There is a sphere centered at (cx,cy,cz) with radius rc.  Return in
      //C        p the coordinates of the point on the surface of the sphere that is
      //C        farthest from the origin.
      //C
      float cc = fem::float0;
      pythag0(c, cc);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      if (cc == 0.0f) {
         p(1) = 0.0f;
         p(2) = 0.0f;
         p(3) = rc;
      }
      else {
         normalize(c, n);
         scalar_product(cc + rc, n, p);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxlens(
           arr_cref<fem::real_star_4> c,
           arr_cref<fem::real_star_4> d,
           fem::real_star_4 const& rc,
           fem::real_star_4 const& rd,
           arr_cref<fem::real_star_4, 2> t,
           fem::real_star_4& ddm)
   {
      c(dimension(3));
      d(dimension(3));
      t(dimension(3, 3));
      //C
      //C        LENS[(cx,cy,cz),(dx,dy,dz),rc,rd]
      //C        real*4 c(3),d(3),rc,rd
      //C        Defined as the intersection of two spheres, centered
      //C        respectively at c and d,
      //C        having radii respectively rc, rd
      //C
      //C        Determine the maximum distance of points in the lens
      //C        away from the origin
      //C
      arr_1d<3, fem::real_star_4> cm(fem::fill0);
      sphereaway(c, rc, cm);
      arr_1d<3, fem::real_star_4> dm(fem::fill0);
      sphereaway(d, rd, dm);
      float cmd = fem::float0;
      pythag(cm, d, cmd);
      float dmc = fem::float0;
      pythag(dm, c, dmc);
      //C
      if (cmd < rd) {
         pythag0(cm, ddm);
         return;
      }
      else if (dmc < rc) {
         pythag0(dm, ddm);
         return;
      }
      //C
      float rcd = fem::float0;
      pythag(c, d, rcd);
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      vector_difference(d, c, n1);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      normalize(n1, n);
      //C
      float x = (fem::pow2(rc) + fem::pow2(rcd) - fem::pow2(rd)) / (2.0f * rcd);
      float y = fem::sqrt(fem::pow2(rc) - fem::pow2(x));
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> diskc(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         diskc(i) = c(i) + x * n(i);
      }
      //C
      maxdisk(diskc, n, y, t, ddm);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   sort(
        arr_cref<fem::real_star_4> a,
        arr_ref<fem::real_star_4> a2s,
        int& ndif)
   {
      a(dimension(3));
      a2s(dimension(3));
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         a2s(i) = fem::pow2(a(i));
      }
      //C
      ndif = 3;
      int nwk = 1;
      //C
      float z = fem::float0;
      while (nwk < ndif) {
         if (nwk == 0) {
            nwk = 1;
         }
         if (a2s(nwk) < a2s(nwk + 1)) {
            nwk++;
         }
         else if (a2s(nwk) > a2s(nwk + 1)) {
            z = a2s(nwk);
            a2s(nwk) = a2s(nwk + 1);
            a2s(nwk + 1) = z;
            nwk = nwk - 1;
         }
         else {
            a2s(nwk + 1) = a2s(ndif);
            ndif = ndif - 1;
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   eval(
        fem::real_star_4 const& z,
        arr_cref<fem::real_star_4> a,
        arr_cref<fem::real_star_4> p,
        arr_ref<fem::real_star_4> x,
        fem::real_star_4& t)
   {
      a(dimension(3));
      p(dimension(3));
      x(dimension(3));
      //C
      int i = fem::int0;
      float q = fem::float0;
      FEM_DO_SAFE(i, 1, 3) {
         q = 1.0f + z / (fem::pow2(a(i)));
         q = 1.0f / q;
         x(i) = q * p(i);
      }
      //C
      t = 0.0f;
      FEM_DO_SAFE(i, 1, 3) {
         t += fem::pow2((x(i) / a(i)));
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   deval(
         fem::real_star_8 const& z,
         arr_cref<fem::real_star_8> a,
         arr_cref<fem::real_star_8> p,
         arr_ref<fem::real_star_8> x,
         fem::real_star_8& t)
   {
      a(dimension(3));
      p(dimension(3));
      x(dimension(3));
      //C
      int i = fem::int0;
      fem::real_star_8 q = fem::zero<fem::real_star_8>();
      FEM_DO_SAFE(i, 1, 3) {
         q = 1.0e0 + z / (fem::pow2(a(i)));
         q = 1.0e0 / q;
         x(i) = q * p(i);
      }
      //C
      t = 0.0e0;
      FEM_DO_SAFE(i, 1, 3) {
         t += fem::pow2((x(i) / a(i)));
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   converge(
            float& zlow,
            float& zhih,
            arr_cref<fem::real_star_4> a,
            arr_cref<fem::real_star_4> p,
            arr_ref<fem::real_star_4> x,
            float& alam)
   {
      a(dimension(3));
      p(dimension(3));
      x(dimension(3));
      //C
      fem::real_star_8 dhih = fem::dble(zhih);
      fem::real_star_8 dlow = fem::dble(zlow);
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_8> da(fem::fill0);
      arr_1d<3, fem::real_star_8> dp(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         da(i) = fem::dble(a(i));
         dp(i) = fem::dble(p(i));
      }
      //C
      arr_1d<3, fem::real_star_8> dx(fem::fill0);
      fem::real_star_8 dthih = fem::zero<fem::real_star_8>();
      deval(dhih, da, dp, dx, dthih);
      fem::real_star_8 dtlow = fem::zero<fem::real_star_8>();
      deval(dlow, da, dp, dx, dtlow);
      //C
      fem::real_star_8 dm = fem::zero<fem::real_star_8>();
      fem::real_star_8 dt = fem::zero<fem::real_star_8>();
      while ((dhih - dlow) * 2.0e0 / (fem::dabs(dhih) + fem::dabs(
                                                                  dlow)) > 1.0e-12) {
         dm = 0.5e0 * (dhih + dlow);
         deval(dm, da, dp, dx, dt);
         if ((dthih - 1.0e0) * (dt - 1.0e0) > 0.0e0) {
            dhih = dm;
            dthih = dt;
         }
         else {
            dlow = dm;
            dtlow = dt;
         }
      }
      //C
      zhih = fem::sngl(dhih);
      zlow = fem::sngl(dlow);
      //C
      alam = (zhih + zlow) / 2.0f;
      float t = fem::float0;
      eval(alam, a, p, x, t);
   }

   //C
   //C*************************************************
   //C
   void
   above(
         arr_cref<fem::real_star_4> a,
         arr_cref<fem::real_star_4> p,
         float& alam,
         arr_ref<fem::real_star_4> x,
         float const& sing,
         float const& ccc)
   {
      a(dimension(3));
      p(dimension(3));
      x(dimension(3));
      float add = fem::float0;
      float z = fem::float0;
      float t = fem::float0;
      float zhih = fem::float0;
      float ztry = fem::float0;
      float zlow = fem::float0;
      float zz = fem::float0;
      //C
      add = 1.0f;
      //C
   statement_1:
      z = sing + add * ccc;
      eval(z, a, p, x, t);
      if (t < 1.0f) {
         zhih = z;
         goto statement_2;
      }
      add = add * 1.5f;
      goto statement_1;
      //C
   statement_2:
      //C
      ztry = zhih;
      //C
   statement_3:
      ztry = (ztry + sing) / 2.0f;
      eval(ztry, a, p, x, t);
      if (t > 1.0f) {
         zlow = ztry;
         goto statement_4;
      }
      goto statement_3;
      //C
   statement_4:
      //C
      if (ccc < 0.0f) {
         zz = zlow;
         zlow = zhih;
         zhih = zz;
      }
      //C
      converge(zlow, zhih, a, p, x, alam);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   eval2(
         fem::real_star_4 const& z,
         arr_cref<fem::real_star_4> a,
         arr_cref<fem::real_star_4> p,
         arr_ref<fem::real_star_4> x,
         fem::real_star_4& t)
   {
      a(dimension(3));
      p(dimension(3));
      x(dimension(3));
      //C
      int i = fem::int0;
      float q = fem::float0;
      FEM_DO_SAFE(i, 1, 3) {
         q = 1.0f + z / (fem::pow2(a(i)));
         q = 1.0f / q;
         x(i) = q * p(i);
      }
      //C
      t = 0.0f;
      float den = fem::float0;
      FEM_DO_SAFE(i, 1, 3) {
         den = fem::pow2((fem::pow2(a(i)) + z));
         t += x(i) * p(i) / den;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   between(
           arr_cref<fem::real_star_4> a,
           arr_cref<fem::real_star_4> p,
           float& alam1,
           float& alam2,
           arr_ref<fem::real_star_4> x1,
           arr_ref<fem::real_star_4> x2,
           float const& sing1,
           float const& sing2,
           int& ns)
   {
      a(dimension(3));
      p(dimension(3));
      x1(dimension(3));
      x2(dimension(3));
      float z = fem::float0;
      arr_1d<3, fem::real_star_4> x(fem::fill0);
      float t = fem::float0;
      float zlow = fem::float0;
      int k1 = fem::int0;
      float zhih = fem::float0;
      float ztest = fem::float0;
      float zmid = fem::float0;
      float zc = fem::float0;
      float z1 = fem::float0;
      //C
      z = (sing1 + sing2) / 2.0f;
      eval2(z, a, p, x, t);
      //C
      if (t > 0.0f) {
         zlow = z;
         k1 = 1;
      }
      else {
         zhih = z;
         k1 = 2;
      }
      //C
      if (k1 == 1) {
         //C
      statement_1:
         ztest = (zlow + sing2) / 2.0f;
         eval2(ztest, a, p, x, t);
         if (t < 0.0f) {
            zhih = ztest;
            goto statement_10;
         }
         zlow = ztest;
         goto statement_1;
         //C
      }
      else {
         //C
      statement_2:
         ztest = (zhih + sing1) / 2.0f;
         eval2(ztest, a, p, x, t);
         if (t > 0.0f) {
            zlow = ztest;
            goto statement_10;
         }
         zhih = ztest;
         goto statement_2;
         //C
      }
      //C
   statement_10:
      //C
      while ((zhih - zlow) * 2.0f / (fem::abs(zhih) + fem::abs(zlow)) > 1.0e-5f) {
         //C
         zmid = (zhih + zlow) / 2.0f;
         eval2(zmid, a, p, x, t);
         if (t < 0.0f) {
            zhih = zmid;
         }
         else {
            zlow = zmid;
         }
         //C
      }
      //C
      zc = 0.5f * (zhih + zlow);
      eval(zc, a, p, x, t);
      if (t > 1.0f) {
         ns = 0;
         return;
      }
      //C
      ns = 2;
      z1 = zc;
      //C
   statement_12:
      //C
      eval(z1, a, p, x, t);
      if (t > 1.0f) {
         goto statement_13;
      }
      z1 = (z1 + sing1) / 2.0f;
      //C
      goto statement_12;
      //C
   statement_13:
      //C
      converge(z1, zc, a, p, x1, alam1);
      //C
      z1 = zc;
      //C
   statement_20:
      //C
      eval(z1, a, p, x, t);
      if (t > 1.0f) {
         goto statement_21;
      }
      z1 = (z1 + sing2) / 2.0f;
      //C
      goto statement_20;
      //C
   statement_21:
      //C
      converge(zc, z1, a, p, x2, alam2);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   exellipsoid(
               arr_cref<fem::real_star_4> c,
               arr_cref<fem::real_star_4> /* n1 */,
               arr_cref<fem::real_star_4> /* n2 */,
               fem::real_star_4 const& aa,
               fem::real_star_4 const& bb,
               fem::real_star_4 const& cc,
               arr_cref<fem::real_star_4, 2> t,
               arr_cref<fem::real_star_4> po,
               fem::integer_star_4 const& minmax,
               fem::real_star_4& d)
   {
      c(dimension(3));
      t(dimension(3, 3));
      po(dimension(3));
      //C
      arr_1d<3, fem::real_star_4> pomc(fem::fill0);
      vector_difference(po, c, pomc);
      arr_1d<3, fem::real_star_4> p(fem::fill0);
      rotate(p, t, pomc);
      //C
      arr_1d<3, fem::real_star_4> a(fem::fill0);
      a(1) = aa;
      a(2) = bb;
      a(3) = cc;
      //C
      //C        The determination of extremal points on the surface of
      //C        an ellipsoid is difficult when any component of p is
      //C        close to zero, because the problem becomes degenerate.
      //C
      //C        We deal with the problem here by making any very small
      //C        coordinates not so small.  The justification for this
      //C        adjustment is different in each of two different
      //C        cases:  1.  All three coordinates near zero, and 2. Only
      //C        one or two coordinates near zero.
      //C
      //C        When all three coordinates are near zero, we change
      //C        them to a small positive value epsilon.  This increases
      //C        the maximum distance by an amount of order
      //C        epsilon.  We are obviously at a
      //C        point inside the ellipsoid, and so we are attempting
      //C        to set the radius of the launch sphere.  Therefore,
      //C        the only effect of this manipulation is to make
      //C        the launch sphere slightly larger.
      //C
      //C        If we are not on the interior of the ellipsoid trying
      //C        to set the launch sphere radius, then we must be on
      //C        the exterior and trying to compute the minimum distance
      //C        to the surface from an exterior point. Therefore,
      //C        the second case above only occurs when we are
      //C        on the outside looking in.  Then, if
      //C        we change only one or two zero coordinates by a small
      //C        amount epsilon,
      //C        the effect on the distance measurement is of order
      //C        epsilon**2 (it's like we are rotating a lever arm)
      //C        and therefore negligible.
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         if (fem::abs(p(i)) / a(i) < 1.0e-6f) {
            p(i) = 1.0e-4f * a(i);
         }
      }
      //C
      int nsol = 0;
      arr_1d<3, fem::real_star_4> a2s(fem::fill0);
      int ndif = fem::int0;
      sort(a, a2s, ndif);
      //C       a2s contains squared and sorted a's:
      //C       ndif = 3:  a2s(1) < a2s(2) < a2s(3)
      //C       ndif = 2:  a2s(1) < a2s(2) and a2s(3) is irrelevant
      //C       ndif = 1:  sphere, a2s(1) is only relevant entry
      //C
      float alam = fem::float0;
      arr_1d<3, fem::real_star_4> x(fem::fill0);
      arr_2d<6, 3, fem::real_star_4> xsol(fem::fill0);
      float alam1 = fem::float0;
      float alam2 = fem::float0;
      arr_1d<3, fem::real_star_4> x1(fem::fill0);
      arr_1d<3, fem::real_star_4> x2(fem::fill0);
      int ns = fem::int0;
      if (ndif == 3) {
         //C
         above(a, p, alam, x, -a2s(1), +1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
         between(a, p, alam1, alam2, x1, x2, -a2s(2), -a2s(1), ns);
         if (ns == 2) {
            xsol(nsol + 1, 1) = x1(1);
            xsol(nsol + 1, 2) = x1(2);
            xsol(nsol + 1, 3) = x1(3);
            xsol(nsol + 2, 1) = x2(1);
            xsol(nsol + 2, 2) = x2(2);
            xsol(nsol + 2, 3) = x2(3);
            nsol += 2;
         }
         //C
         between(a, p, alam1, alam2, x1, x2, -a2s(3), -a2s(2), ns);
         if (ns == 2) {
            xsol(nsol + 1, 1) = x1(1);
            xsol(nsol + 1, 2) = x1(2);
            xsol(nsol + 1, 3) = x1(3);
            xsol(nsol + 2, 1) = x2(1);
            xsol(nsol + 2, 2) = x2(2);
            xsol(nsol + 2, 3) = x2(3);
            nsol += 2;
         }
         //C
         above(a, p, alam, x, -a2s(3), -1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
      }
      else if (ndif == 2) {
         //C
         above(a, p, alam, x, -a2s(1), +1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
         between(a, p, alam1, alam2, x1, x2, -a2s(2), -a2s(1), ns);
         if (ns == 2) {
            xsol(nsol + 1, 1) = x1(1);
            xsol(nsol + 1, 2) = x1(2);
            xsol(nsol + 1, 3) = x1(3);
            xsol(nsol + 2, 1) = x2(1);
            xsol(nsol + 2, 2) = x2(2);
            xsol(nsol + 2, 3) = x2(3);
            nsol += 2;
         }
         //C
         above(a, p, alam, x, -a2s(2), -1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
      }
      else {
         //C
         above(a, p, alam, x, -a2s(1), +1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
         above(a, p, alam, x, -a2s(1), -1.0f);
         nsol++;
         xsol(nsol, 1) = x(1);
         xsol(nsol, 2) = x(2);
         xsol(nsol, 3) = x(3);
         //C
      }
      //C
      float dd = fem::float0;
      float ss = fem::float0;
      int j = fem::int0;
      FEM_DO_SAFE(i, 1, nsol) {
         dd = 0.0f;
         ss = 0.0f;
         FEM_DO_SAFE(j, 1, 3) {
            dd += fem::pow2((p(j) - xsol(i, j)));
            ss += fem::pow2((xsol(i, j) / a(j)));
         }
         dd = fem::sqrt(dd);
         if (i == 1) {
            d = dd;
         }
         else {
            if (minmax ==  - 1) {
               d = fem::amin1(dd, d);
            }
            else {
               d = fem::amax1(dd, d);
            }
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   maxellipsoid(
                arr_cref<fem::real_star_4> c,
                arr_cref<fem::real_star_4> n1,
                arr_cref<fem::real_star_4> n2,
                fem::real_star_4 const& aa,
                fem::real_star_4 const& bb,
                fem::real_star_4 const& cc,
                arr_cref<fem::real_star_4, 2> t,
                fem::real_star_4& dd)
   {
      c(dimension(3));
      n1(dimension(3));
      n2(dimension(3));
      t(dimension(3, 3));
      //C
      //C        return in dd the maximum distance of points in this
      //C        ellipsoid away from the origin
      //C
      arr_1d<3, fem::real_star_4> p(fem::fill0);
      p(1) = 0.0f;
      p(2) = 0.0f;
      p(3) = 0.0f;
      //C
      int minmax = 1;
      exellipsoid(c, n1, n2, aa, bb, cc, t, p, minmax, dd);
      //C
      //C        dd always comes out slightly low, so lets do this
      //C        to compensate.  It only means we are using a
      //C        slightly larger launch sphere.
      //C
      dd = dd * 1.001f;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   do_launch(
             int const& maxelts,
             fem::integer_star_4 const& nelts,
             arr_cref<fem::integer_star_4> eltype,
             arr_cref<fem::real_star_4, 2> bv,
             float& rlaunch,
             arr_cref<fem::real_star_4, 3> rotations)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      //C
      //C        Determine launch radius
      //C
      rlaunch = 0.0f;
      //C
      int i = fem::int0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> v1(fem::fill0);
      float side = fem::float0;
      float dd = fem::float0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      float r = fem::float0;
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      arr_1d<3, fem::real_star_4> v3(fem::fill0);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      int k = fem::int0;
      arr_2d<3, 3, fem::real_star_4> t(fem::fill0);
      float al = fem::float0;
      float r1 = fem::float0;
      float r2 = fem::float0;
      arr_1d<3, fem::real_star_4> d(fem::fill0);
      float rc = fem::float0;
      float rd = fem::float0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n2(fem::fill0);
      float aa = fem::float0;
      float bb = fem::float0;
      float cc = fem::float0;
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         //C  cubes
         if (eltype(i) == 8) {
            FEM_DO_SAFE(j, 1, 3) {
               v1(j) = bv(i, j);
               side = bv(i, 4);
            }
            maxcube(v1, side, dd);
            rlaunch = fem::amax1(dd, rlaunch);
         }
         //C
         //C spheres
         if (eltype(i) == 1) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
            }
            r = bv(i, 4);
            maxsphere(c, r, dd);
            rlaunch = fem::amax1(dd, rlaunch);
            //C
         }
         //C
         //C triangles
         if (eltype(i) == 2) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               v1(j) = bv(i, j);
               v2(j) = bv(i, j + 3);
               v3(j) = bv(i, j + 6);
            }
            maxtriangle(v1, v2, v3, dd);
            rlaunch = fem::amax1(dd, rlaunch);
            //C
         }
         //C
         //C  disks
         if (eltype(i) == 3) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r = bv(i, 7);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            maxdisk(c, n, r, t, dd);
            rlaunch = fem::amax1(dd, rlaunch);
         }
         //C
         //C  cylinders
         if (eltype(i) == 4) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r = bv(i, 7);
            al = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            maxcylinder(c, n, r, al, t, dd);
            rlaunch = fem::amax1(dd, rlaunch);
         }
         //C
         //C  tori
         if (eltype(i) == 5) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r1 = bv(i, 7);
            r2 = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            maxtorus(c, n, r1, r2, t, dd);
            rlaunch = fem::amax1(dd, rlaunch);
            //C
         }
         //C
         //C  lenses
         if (eltype(i) == 6) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               d(j) = bv(i, j + 3);
            }
            rc = bv(i, 7);
            rd = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            maxlens(c, d, rc, rd, t, dd);
            rlaunch = fem::amax1(dd, rlaunch);
            //C
         }
         //C
         //C  ellipsoids
         if (eltype(i) == 7) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n1(j) = bv(i, j + 3);
               n2(j) = bv(i, j + 6);
            }
            aa = bv(i, 10);
            bb = bv(i, 11);
            cc = bv(i, 12);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            maxellipsoid(c, n1, n2, aa, bb, cc, t, dd);
            rlaunch = fem::amax1(dd, rlaunch);
            //C
         }
         //C
      }
      //C
   }

   struct seeder_save
   {
      fem::variant_bindings random_bindings;
   };

   //C
   //C*************************************************
   //C
   void
   seeder(
          common& cmn,
          str_cref dateline)
   {
      FEM_CMN_SVE(seeder);
      common_read read(cmn);
      common_variant random(cmn.common_random, sve.random_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<int> idum;
            random.allocate(), idum;
         }
      }
      int& idum = random.bind<int>();
      //C
      //C        This program uses the UNIX date command to generate a
      //C        seed for
      //C        the sequence of random numbers
      //C
      fem::str<8> shore = dateline(9, 10);
      shore(3, 4) = dateline(12, 13);
      shore(5, 6) = dateline(15, 16);
      shore(7, 8) = dateline(18, 19);
      read(shore, star), idum;
      idum = -idum;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   gettime(
           common& cmn,
           str_cref dfl,
           str_ref start)
   {
      common_read read(cmn);
      //C
      cmn.io.open(82, dfl)
         .status("old");
      read(82, "(a28)"), start;
      cmn.io.close(82);
   }

   //C
   //C*************************************************
   //C
   void
   setup(
         common& cmn,
         str_ref id,
         fem::integer_star_4& nin,
         fem::integer_star_4& nout,
         arr_ref<fem::integer_star_4> m1,
         str_ref actions)
   {
      m1(dimension(3));
      common_read read(cmn);
      //C
      //C        Parse the invocation string, and initialize random numbers
      //C
      //C        Get the body ID:
      cmn.getarg(1, id);
      //C
      //C        Get the action codes
      int nac = cmn.iargc() - 1;
      int i = fem::int0;
      fem::str<30> ac = fem::char0;
      fem::str<30> pix = fem::char0;
      int mult = fem::int0;
      int nzip = fem::int0;
      FEM_DO_SAFE(i, 1, nac) {
         cmn.getarg(i + 1, ac);
         if (ac(1, 1) == "z") {
            actions(i, i) = "z";
         }
         else if (ac(1, 1) == "i") {
            actions(i, i) = "i";
         }
         else if (ac(1, 1) == "s") {
            actions(i, i) = "s";
         }
         else {
            FEM_STOP("bad action code");
         }
         ac(1, 1) = " ";
         pix = ac;
         //C
         mult = 1;
         nzip = fem::index(pix, "t");
         if (nzip != 0) {
            mult = 1000 * mult;
            pix(nzip, nzip) = " ";
         }
         //C
         nzip = fem::index(pix, "m");
         if (nzip != 0) {
            mult = 1000000 * mult;
            pix(nzip, nzip) = " ";
         }
         //C
         nzip = fem::index(pix, "b");
         if (nzip != 0) {
            mult = 1000000000 * mult;
            pix(nzip, nzip) = " ";
         }
         //C
         read(pix, star), m1(i);
         m1(i) = m1(i) * mult;
         //C
      }
      //C
      FEM_DO_SAFE(i, nac + 1, 3) {
         actions(i, i) = ".";
      }
      //C
      int nsp = fem::index(id, " ");
      fem::str<25> fin = id;
      fem::str<25> fout = id;
      fin(nsp, nsp + 3) = ".bod";
      fout(nsp, nsp + 3) = ".zno";
      fem::str<25> dfl = id;
      dfl(nsp, nsp + 3) = ".dfl";
      //C
      fem::str<32> com = "date > ";
      //C                   1234567
      com(8, 32) = dfl;
      system(com);
      //C
      fem::str<28> start = fem::char0;
      gettime(cmn, dfl, start);
      seeder(cmn, start);
      //C
      nin = 20;
      nout = 99;
      // std::cout << QString( "Input file: %1\n").arg( (char *)&fin ).ascii();
      cmn.io.open(nin, fin)
         .status("old");
      // std::cout << QString( "Output file: %1\n").arg( (char *)&fout ).ascii();
      cmn.io.open(nout, fout)
         .status("unknown");
      //C
   }

   //C
   //C********************************************************
   //C
   void
   pack80(
          str_ref line)
   {
      //C
      int nz = 0;
      //C
      fem::str<80> copy = fem::char0;
      while (line(1, 1) == " " && nz < 90) {
         copy(1, 79) = line(2, 80);
         copy(80, 80) = " ";
         line = copy;
         nz++;
      }
      //C
   }

   //C
   //C********************************************************
   //C
   void
   floatstring(
               common& cmn,
               str_ref tline,
               arr_ref<fem::real_star_8> t1)
   {
      t1(dimension(2));
      common_read read(cmn);
      //C
      int j1 = fem::index(tline, "E");
      int j2 = fem::int0;
      int j3 = fem::int0;
      if (j1 == 0) {
         j2 = fem::index(tline, "e");
         if (j2 == 0) {
            j3 = fem::index(tline, " ");
            tline(j3, j3 + 1) = "e0";
         }
      }
      else {
         tline(j1, j1) = "e";
      }
      //C
      fem::real_star_8 pre = fem::zero<fem::real_star_8>();
      if (tline(1, 1) == "-") {
         pre = -1.0e0;
         tline(1, 1) = " ";
      }
      else if (tline(1, 1) == "+") {
         pre = +1.0e0;
         tline(1, 1) = " ";
      }
      else {
         pre = 1.0e0;
      }
      //C
      pack80(tline);
      //C
      j1 = fem::index(tline, "e");
      if (j1 == 0) {
         FEM_STOP("e should be there by now");
      }
      //C
      int i = fem::int0;
      fem::str<80> cline = fem::char0;
      FEM_DO_SAFE(i, 1, j1) {
         cline(i, i) = " ";
      }
      FEM_DO_SAFE(i, j1 + 1, 80) {
         cline(i, i) = tline(i, i);
         tline(i, i) = " ";
      }
      tline(j1, j1) = " ";
      //C
      int nex = fem::int0;
      read(cline, star), nex;
      //C
      int nj = fem::index(tline, ".");
      if (nj == 0) {
         nj = fem::index(tline, " ");
         tline(nj, nj) = ".";
      }
      //C
      int ndot = fem::index(tline, ".");
      if (ndot == 0) {
         FEM_STOP("dot should be there by now");
      }
      //C
      nj = fem::index(tline, " ") - 1;
      //C
      fem::real_star_8 eval = fem::zero<fem::real_star_8>();
      if (nj > ndot) {
         eval = fem::pow(10.0e0, (ndot - nj));
      }
      else if (nj == ndot) {
         eval = 1.0e0;
      }
      else {
         FEM_STOP("dotty");
      }
      //C
      fem::real_star_8 val = 0.0e0;
      fem::real_star_8 dig = fem::zero<fem::real_star_8>();
      fem::real_star_8 place = fem::zero<fem::real_star_8>();
      FEM_DO_SAFE(i, 1, nj) {
         if (i != ndot) {
            read(tline(i, i), star), dig;
            if (i < ndot) {
               place = fem::pow(10.0e0, (ndot - i - 1));
            }
            else {
               place = fem::pow(10.0e0, (ndot - i));
            }
            val += dig * place;
         }
      }
      //C
      t1(1) = pre * val * (fem::pow(10.0e0, nex));
      t1(2) = 0.5e0 * eval * (fem::pow(10.0e0, nex));
      //C
   }

   //C
   //C*************************************************
   //C
   void
   packspaces(
              str_ref buffer,
              int& next)
   {
      //C
      //C        pack leading spaces from the string "buffer"
      //C
      int ngo = 0;
      //C
      fem::str<80> copy = fem::char0;
      while (buffer(1, 1) == " ") {
         copy = buffer;
         buffer(1, 79) = copy(2, 80);
         buffer(80, 80) = " ";
         ngo++;
         if (ngo == 80) {
            next = 0;
            return;
         }
      }
      //C
      next = fem::index(buffer, " ");
   }

   //C
   //C*************************************************
   //C
   void
   cleartabs(
             str_ref buffer)
   {
      //C
      //C        Convert tab-characters in input buffer to spaces
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 80) {
         if (fem::ichar(buffer(i, i)) == 9) {
            buffer(i, i) = " ";
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   nextstring(
              common& cmn,
              fem::integer_star_4 const& nin,
              bool& nobuffer,
              str_ref buffer,
              str_ref string,
              bool& atend)
   {
      common_read read(cmn);
      common_write write(cmn);
      int next = fem::int0;
      int i = fem::int0;
      static const char* format_980 = "(a80)";
      //C
      //C        Interpret the next input string
      //C
      atend = false;
      //C
   statement_1:
      if (nobuffer) {
         try {
            read(nin, format_980), buffer;
         }
         catch (fem::read_end const&) {
            goto statement_20;
         }
         cleartabs(buffer);
         if (!cmn.silent) {
            write(6, format_980), buffer;
         }
         nobuffer = false;
      }
      //C
      if (buffer(1, 1) == "*") {
         nobuffer = true;
         goto statement_1;
      }
      //C
      packspaces(buffer, next);
      if (next == 0) {
         nobuffer = true;
         goto statement_1;
      }
      //C
      FEM_DO_SAFE(i, 1, 80) {
         string(i, i) = " ";
      }
      string(1, next - 1) = buffer(1, next - 1);
      FEM_DO_SAFE(i, 1, next - 1) {
         buffer(i, i) = " ";
      }
      //C
      return;
      //C
   statement_20:
      atend = true;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   testword(
            common& cmn,
            str_cref string,
            int& ntype,
            bool& bailout)
   {
      common_read read(cmn);
      // COMMON errors
      fem::integer_star_4& nerr = cmn.nerr;
      arr_ref<fem::integer_star_4> errorlist(cmn.errorlist, dimension(100));
      // COMMON naughty
      fem::str<10>& badword = cmn.badword;
      //
      //C
      //C        Is string a recognized word?  If so encode its value in ntype
      //C
      //C------------------------------------------------------
      //C        The following are all of the recognized words.  If
      //C        The string is not equal to one of these words,
      //C        we have to abort the job.
      //C------------------------------------------------------
      //C
      //C        Word            Length  Internal code
      //C        ---------       ------  -------------
      //C        ELLIPSOID          9       07
      //C        ellipsoid          9       07
      //C        VISCOSITY          9       24
      //C        viscosity          9       24
      //C        TRIANGLE           8       02
      //C        triangle           8       02
      //C        CYLINDER           8       04
      //C        cylinder           8       04
      //C        SOLVENT            7       27
      //C        solvent            7       27
      //C        SPHERE             6       01
      //C        sphere             6       01
      //C        UNITS              5       10
      //C        units              5       10
      //C        TORUS              5       05
      //C        torus              5       05
      //C        WATER              5       28
      //C        water              5       28
      //C        DISK               4       03
      //C        disk               4       03
      //C        LENS               4       06
      //C        lens               4       06
      //C        CUBE               4       08
      //C        cube               4       08
      //C       MASS               4       19
      //C       mass               4       19
      //C        TEMP               4       16
      //C        temp               4       16
      //C        kDa                3       21
      //C        TO                 2       05
      //C        to                 2       05
      //C        ST                 2       09
      //C        st                 2       09
      //C       cm                 2        12
      //C       nm                 2       13
      //C        Da                 2       20
      //C        kg                 2       23
      //C        cp                 2       26
      //C        S                  1       01
      //C        s                  1       01
      //C        T                  1       02
      //C        t                  1       02
      //C        D                  1       03
      //C        d                  1       03
      //C        C                  1       17
      //C        E                  1       07
      //C        e                  1       07
      //C       m                   1       11
      //C       A                   1       14
      //C       L                   1       15
      //C       K                   1       18
      //C        g                  1       22
      //C       p                  1       25
      //C------------------------------------------------------
      //C
      bailout = false;
      //C
      fem::str<15> cn = "SsTtDdCEemALKgp";
      fem::str<15> c10 = "000000100111122";
      fem::str<15> c1 = "112233777145825";
      fem::str<18> cc2 = "TOtoSTstcmnmDakgcp";
      fem::str<18> ccl = "050509091213202326";
      //C
      int nlen = fem::index(string, " ") - 1;
      //C
      if (nlen > 9) {
         nerr++;
         errorlist(nerr) = 1;
         badword(1, 10) = string(1, 10);
         bailout = true;
         return;
      }
      //C
      if (nlen == 9) {
         if (string(1, 9) == "ELLIPSOID") {
            ntype = 7;
         }
         else if (string(1, 9) == "ellipsoid") {
            ntype = 7;
         }
         else if (string(1, 9) == "VISCOSITY") {
            ntype = 24;
         }
         else if (string(1, 9) == "viscosity") {
            ntype = 24;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 9) = string(1, 9);
            badword(10, 10) = " ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 8) {
         if (string(1, 8) == "TRIANGLE") {
            ntype = 2;
         }
         else if (string(1, 8) == "triangle") {
            ntype = 2;
         }
         else if (string(1, 8) == "CYLINDER") {
            ntype = 4;
         }
         else if (string(1, 8) == "cylinder") {
            ntype = 4;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 8) = string(1, 8);
            badword(9, 10) = "  ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 7) {
         if (string(1, 7) == "SOLVENT") {
            ntype = 27;
         }
         else if (string(1, 7) == "solvent") {
            ntype = 27;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 7) = string(1, 7);
            badword(8, 10) = "   ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 6) {
         if (string(1, 6) == "SPHERE") {
            ntype = 1;
         }
         else if (string(1, 6) == "sphere") {
            ntype = 1;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 6) = string(1, 6);
            badword(7, 10) = "    ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 5) {
         if (string(1, 5) == "TORUS") {
            ntype = 5;
         }
         else if (string(1, 5) == "torus") {
            ntype = 5;
         }
         else if (string(1, 5) == "UNITS") {
            ntype = 10;
         }
         else if (string(1, 5) == "units") {
            ntype = 10;
         }
         else if (string(1, 5) == "WATER") {
            ntype = 28;
         }
         else if (string(1, 5) == "water") {
            ntype = 28;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 5) = string(1, 5);
            badword(6, 10) = "     ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 4) {
         if (string(1, 4) == "DISK") {
            ntype = 3;
         }
         else if (string(1, 4) == "disk") {
            ntype = 3;
         }
         else if (string(1, 4) == "LENS") {
            ntype = 6;
         }
         else if (string(1, 4) == "lens") {
            ntype = 6;
         }
         else if (string(1, 4) == "CUBE") {
            ntype = 8;
         }
         else if (string(1, 4) == "cube") {
            ntype = 8;
         }
         else if (string(1, 4) == "MASS") {
            ntype = 19;
         }
         else if (string(1, 4) == "mass") {
            ntype = 19;
         }
         else if (string(1, 4) == "TEMP") {
            ntype = 16;
         }
         else if (string(1, 4) == "temp") {
            ntype = 16;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 4) = string(1, 4);
            badword(5, 10) = "      ";
            bailout = true;
         }
         return;
      }
      //C
      if (nlen == 3) {
         if (string(1, 3) == "kDa") {
            ntype = 21;
         }
         else {
            nerr++;
            errorlist(nerr) = 1;
            badword(1, 3) = string(1, 3);
            badword(4, 10) = "       ";
            bailout = true;
            return;
         }
      }
      //C
      //C        cc2 = 'TOtoSTstcmnmDakgcp'
      //C        ccl = '050509091213202326'
      //C
      int j = fem::int0;
      if (nlen == 2) {
         FEM_DOSTEP(j, 1, 17, 2) {
            if (string(1, 2) == cc2(j, j + 1)) {
               read(ccl(j, j + 1), star), ntype;
               return;
            }
         }
         nerr++;
         errorlist(nerr) = 1;
         badword(1, 2) = string(1, 2);
         badword(3, 10) = "        ";
         bailout = true;
         return;
      }
      //C
      //C        cn  = 'SsTtDdCEemALKgp'
      //C        c10 = '000000100111122'
      //C        c1  = '112233777145825'
      //C
      int n1 = fem::int0;
      int n2 = fem::int0;
      if (nlen == 1) {
         FEM_DO_SAFE(j, 1, 15) {
            if (string(1, 1) == cn(j, j)) {
               read(c10(j, j), star), n1;
               read(c1(j, j), star), n2;
               ntype = 10 * n1 + n2;
               return;
            }
         }
         nerr++;
         errorlist(nerr) = 1;
         badword(1, 1) = string(1, 1);
         badword(2, 10) = "         ";
         bailout = true;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   cross_product(
                 arr_cref<fem::real_star_4> a,
                 arr_cref<fem::real_star_4> b,
                 arr_ref<fem::real_star_4> c)
   {
      a(dimension(3));
      b(dimension(3));
      c(dimension(3));
      //C
      //C        c = a x b
      //C
      c(3) = a(1) * b(2) - a(2) * b(1);
      c(1) = a(2) * b(3) - a(3) * b(2);
      c(2) = a(3) * b(1) - a(1) * b(3);
      //C
   }

   struct ran2_save
   {
      fem::variant_bindings random_bindings;

      static const int ntab = 32;

      int idum2;
      arr<int> iv;
      int iy;

      ran2_save() :
         idum2(fem::int0),
         iv(dimension(ntab), fem::fill0),
         iy(fem::int0)
      {}
   };

   const int ran2_save::ntab;

   //C
   //C****************************************************************
   //C
   //C        This is the numerical recipes subroutine ran2
   //C        (From Numerical Recipes in Fortran 77, 2nd ed., p. 272)
   //C
   //C        Long period ( > 2e18) random number generator of L'Ecuyer with
   //C        Bays-Durham shuffle and added safeguards.  Returns a uniform
   //C        random deviate between 0.0 and 1.0 (exclusive of the endpoint
   //C        values).  Call with idum a negative integer to initialize;
   //C        thereafter, do not alter idum between successive deviates in
   //C        a sequence.  rnmx should approximate the largest floating value
   //C        that is less than 1.
   //C
   float
   ran2(
        common& cmn)
   {
      float return_value = fem::float0;
      FEM_CMN_SVE(ran2);
      common_variant random(cmn.common_random, sve.random_bindings);
      // SAVE
      int& idum2 = sve.idum2;
      const int ntab = 32;
      arr_ref<int> iv(sve.iv, dimension(ntab));
      int& iy = sve.iy;
      //
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<int> idum;
            random.allocate(), idum;
         }
      }
      int& idum = random.bind<int>();
      if (is_called_first_time) {
         idum2 = 123456789;
         fem::data((values, ntab*datum(0))), iv;
         iy = 0;
      }
      //C
      //C  Initialize
      int j = fem::int0;
      const int iq1 = 53668;
      int k = fem::int0;
      const int ia1 = 40014;
      const int ir1 = 12211;
      const int im1 = 2147483563;
      if (idum <= 0) {
         //C
         //C  Be sure to prevent idum = 0
         idum = fem::max(-idum, 1);
         idum2 = idum;
         //C
         //C  Load the shuffle table
         FEM_DOSTEP(j, ntab + 8, 1, -1) {
            //C     (after 8 warm-ups)
            //C
            k = idum / iq1;
            idum = ia1 * (idum - k * iq1) - k * ir1;
            if (idum < 0) {
               idum += im1;
            }
            if (j <= ntab) {
               iv(j) = idum;
            }
            //C
         }
         //C
         iy = iv(1);
         //C
      }
      //C
      //C  Start here when not
      k = idum / iq1;
      //C      initializing
      //C
      //C  Compute idum=mod(ia1*dum,
      idum = ia1 * (idum - k * iq1) - k * ir1;
      //C      im1) without overflows
      //C      by Schrage's method
      if (idum < 0) {
         idum += im1;
      }
      //C
      const int iq2 = 52774;
      k = idum2 / iq2;
      //C  Likewise for idum2
      const int ia2 = 40692;
      const int ir2 = 3791;
      idum2 = ia2 * (idum2 - k * iq2) - k * ir2;
      //C
      const int im2 = 2147483399;
      if (idum2 < 0) {
         idum2 += im2;
      }
      //C
      //C  Will be in range 1:ntab
      const int imm1 = im1 - 1;
      const int ndiv = 1 + imm1 / ntab;
      j = 1 + iy / ndiv;
      //C
      //C        Now idum is shuffled, idum and idum2 are combined to generate output
      iy = iv(j) - idum2;
      iv(j) = idum;
      if (iy < 1) {
         iy += imm1;
      }
      //C
      //C  Do not output endpoint values
      const float am = 1.0f / im1;
      const float eps = 1.2e-7f;
      const float rnmx = 1.0f - eps;
      return_value = fem::min(am * iy, rnmx);
      //C
      return return_value;
   }

   //C
   //C*************************************************
   //C
   void
   sphere(
          common& cmn,
          arr_ref<fem::real_star_4> rt,
          fem::real_star_4 const& r1)
   {
      rt(dimension(3));
      int j = fem::int0;
      float dd = fem::float0;
      //C
      //C        Get a point distributed randomly on the
      //C        surface of sphere of radius r1
      //C
   statement_1:
      //C
      FEM_DO_SAFE(j, 1, 3) {
         rt(j) = ran2(cmn) * 2.0f - 1.0f;
      }
      //C
      dd = fem::pow2(rt(1)) + fem::pow2(rt(2)) + fem::pow2(rt(3));
      if (dd > 1.0f) {
         goto statement_1;
      }
      dd = fem::sqrt(dd);
      FEM_DO_SAFE(j, 1, 3) {
         rt(j) = r1 * rt(j) / dd;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   zeerot(
          common& cmn,
          arr_cref<fem::real_star_4> n3,
          arr_ref<fem::real_star_4, 2> t)
   {
      n3(dimension(3));
      t(dimension(3, 3));
      arr_1d<3, fem::real_star_4> nx(fem::fill0);
      float dx3 = fem::float0;
      float beta = fem::float0;
      float alpha = fem::float0;
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n2(fem::fill0);
      //C
      //C        Upon entry, n3 is a normalized vector.  This returns a
      //C        rotation matrix in t that rotates n into the positive z-axis.
      //C
      //C        Proceed by taking an arbitrary normalized vector, but throw it
      //C        away and start over if it is too close to n3.  The second axis
      //C        is taken as a linear combination of these two vectors
      //C
   statement_1:
      sphere(cmn, nx, 1.0f);
      dotproduct(nx, n3, dx3);
      if (1.0f - fem::abs(dx3) <= 0.1f) {
         goto statement_1;
      }
      beta = fem::sqrt(1.0f / (1.0f - fem::pow2(dx3)));
      alpha = -beta * dx3;
      FEM_DO_SAFE(i, 1, 3) {
         n1(i) = alpha * n3(i) + beta * nx(i);
      }
      cross_product(n3, n1, n2);
      //C
      FEM_DO_SAFE(i, 1, 3) {
         t(1, i) = n1(i);
         t(2, i) = n2(i);
         t(3, i) = n3(i);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   xyzrot(
          arr_cref<fem::real_star_4> n1,
          arr_cref<fem::real_star_4> n2,
          arr_cref<fem::real_star_4> n3,
          arr_ref<fem::real_star_4, 2> t)
   {
      n1(dimension(3));
      n2(dimension(3));
      n3(dimension(3));
      t(dimension(3, 3));
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         t(1, i) = n1(i);
         t(2, i) = n2(i);
         t(3, i) = n3(i);
      }
      //C
   }

   struct parse_save
   {
      arr<fem::integer_star_4> nvals;

      parse_save() :
         nvals(dimension(9), fem::fill0)
      {}
   };

   //C
   //C*************************************************
   //C
   void
   parse(
         common& cmn,
         int const& maxelts,
         fem::integer_star_4 const& nin,
         fem::integer_star_4& nelts,
         arr_ref<fem::integer_star_4> eltype,
         arr_ref<fem::real_star_4, 2> bv,
         float& tol,
         arr_ref<fem::real_star_4, 3> rotations,
         bool& tolset,
         str_ref unitcode,
         bool& bailout,
         bool& bt,
         bool& bm,
         bool& bw,
         bool& bc,
         arr_ref<fem::real_star_8> temp,
         str_ref tunit,
         arr_ref<fem::real_star_8> mass,
         str_ref munit,
         arr_ref<fem::real_star_8> visc,
         str_ref vunit)
   {
      FEM_CMN_SVE(parse);
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      temp(dimension(2));
      mass(dimension(2));
      visc(dimension(2));
      common_read read(cmn);
      fem::integer_star_4& nerr = cmn.nerr;
      arr_ref<fem::integer_star_4> errorlist(cmn.errorlist, dimension(100));
      fem::str<10>& badword = cmn.badword;
      //
      arr_ref<fem::integer_star_4> nvals(sve.nvals, dimension(9));
      if (is_called_first_time) {
         static const int values[] = {
            4, 9, 7, 8, 8, 8, 12, 4, 1
         };
         fem::data_of_type<int>(FEM_VALUES_AND_SIZE),
            nvals;
      }
      bool nobuffer = fem::bool0;
      fem::str<80> buffer = fem::char0;
      fem::str<80> string = fem::char0;
      bool atend = fem::bool0;
      int ntype = fem::int0;
      int kk = fem::int0;
      int i = fem::int0;
      int ntype2 = fem::int0;
      int jj = fem::int0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      arr_2d<3, 3, fem::real_star_4> t(fem::fill0);
      int k = fem::int0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      arr_1d<3, fem::real_star_4> d(fem::fill0);
      float rc = fem::float0;
      float rd = fem::float0;
      float dc = fem::float0;
      arr_1d<3, fem::real_star_4> nx(fem::fill0);
      arr_1d<3, fem::real_star_4> ny(fem::fill0);
      arr_1d<3, fem::real_star_4> nz(fem::fill0);
      //C
      //C        Parse the input file
      //C
      //C------------------------------------------------------
      //C        SPHERE, sphere, S, s    (Internal code 1)
      //C        cx, cy, cz, r
      //C------------------------------------------------------
      //C        TRIANGLE, triangle, T, t  (Internal code 2)
      //C        v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z
      //C------------------------------------------------------
      //C        DISK, disk, D, d   (Internal code 3)
      //C        cx,cy,cz,nx,ny,nz,r
      //C------------------------------------------------------
      //C        CYLINDER, cylinder    (Internal code 4)
      //C        cx,cy,cz,nx,ny,nz,r,l
      //C------------------------------------------------------
      //C        TORUS, torus, TO, to     (Internal code 5)
      //C        cx,cy,cz,nx,ny,nz,r1,r2
      //C------------------------------------------------------
      //C        LENS, lens         (Internal code 6)
      //C        cx,cy,cz,dx,dy,dz,rc,rd
      //C------------------------------------------------------
      //C        ELLIPSOID, ellipsoid, E, e  (Internal code 7)
      //C        cx,cy,cz,n1x,n1y,n1z,n2x,n2y,n2z,aa,bb,cc
      //C------------------------------------------------------
      //C        CUBE, cube                (Internal code 8)
      //C        cx,cy,cz,s
      //C------------------------------------------------------
      //C        ST, st         (Internal code 9)
      //C        tol
      //C------------------------------------------------------
      //C        UNITS, units    (Internal code 10)
      //C
      //C       Modifier      Internal
      //C         string         code        Meaning
      //C       -------       --------      -------
      //C           m             11         meters
      //C           cm            12         centimeters
      //C           nm            13         nanometers
      //C           A             14         Angstroms
      //C           L             15         generic or unspecified
      //C------------------------------------------------------
      //C        TEMP, temp    (Internal code 16)
      //C
      //C        First modifier:   number
      //C
      //C        Second
      //C        modifier      Internal
      //C        string          code         Meaning
      //C        -------       --------       -------
      //C           C            17           Celcius
      //C           K            18           Kelvin
      //C------------------------------------------------------
      //C        MASS, mass    (Internal code 19)
      //C
      //C        First modifier:  number
      //C
      //C        Second
      //C        modifier      Internal
      //C        string          code         Meaning
      //C        -------       --------       -------
      //C           Da           20           daltons
      //C           kDa          21           kilodaltons
      //C           g            22           grams
      //C           kg           23           kilograms
      //C------------------------------------------------------
      //C        VISCOSITY, viscosity (internal code 24)
      //C
      //C        Fist modifier:  number
      //C
      //C        Second
      //C        modifier      Internal
      //C        string          code         Meaning
      //C        -------       --------       -------
      //C          p              25           poise
      //C         cp              26           centipoise
      //C------------------------------------------------------
      //C
      nelts = 0;
      tolset = false;
      unitcode = "L ";
      bt = false;
      bm = false;
      bw = false;
      bc = false;
      //C
      nobuffer = true;
      nextstring(cmn, nin, nobuffer, buffer, string, atend);
      //C
      while (!atend) {
         testword(cmn, string, ntype, bailout);
         if (bailout) {
            return;
         }
         //C
         if (ntype <= 8) {
            //C
            nelts++;
            if (nelts > maxelts) {
               nerr++;
               errorlist(nerr) = 2;
               bailout = true;
               return;
            }
            eltype(nelts) = ntype;
            kk = nvals(ntype);
            //C
            FEM_DO_SAFE(i, 1, kk) {
               nextstring(cmn, nin, nobuffer, buffer, string, atend);
               if (atend) {
                  nerr++;
                  errorlist(nerr) = 3;
                  bailout = true;
                  return;
               }
               try {
                  read(string, star), bv(nelts, i);
               }
               catch (fem::io_err const&) {
                  goto statement_999;
               }
            }
            //C
         }
         else if (ntype == 9) {
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            try {
               read(string, star), tol;
            }
            catch (fem::io_err const&) {
               goto statement_999;
            }
            tolset = true;
            //C
         }
         else if (ntype == 10) {
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            testword(cmn, string, ntype2, bailout);
            if (bailout) {
               return;
            }
            if (ntype2 < 11 || ntype2 > 15) {
               nerr++;
               badword = string(1, 10);
               errorlist(nerr) = 9;
               bailout = true;
               return;
            }
            if (ntype2 == 11) {
               unitcode = "m ";
            }
            if (ntype2 == 12) {
               unitcode = "cm";
            }
            if (ntype2 == 13) {
               unitcode = "nm";
            }
            if (ntype2 == 14) {
               unitcode = "A ";
            }
            if (ntype2 == 15) {
               unitcode = "L ";
            }
            //C
         }
         else if (ntype == 16) {
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            floatstring(cmn, string, temp);
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            testword(cmn, string, ntype2, bailout);
            if (bailout) {
               return;
            }
            if (ntype2 < 17 || ntype2 > 18) {
               nerr++;
               errorlist(nerr) = 11;
               badword = string(1, 10);
               bailout = true;
               return;
            }
            if (ntype2 == 17) {
               tunit = "C     ";
            }
            if (ntype2 == 18) {
               tunit = "K     ";
            }
            bt = true;
            //C
         }
         else if (ntype == 19) {
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            floatstring(cmn, string, mass);
            nextstring(cmn, nin, nobuffer, buffer, string, atend);
            if (atend) {
               nerr++;
               errorlist(nerr) = 3;
               bailout = true;
               return;
            }
            testword(cmn, string, ntype2, bailout);
            if (bailout) {
               return;
            }
            if (ntype2 < 20 || ntype2 > 23) {
               nerr++;
               errorlist(nerr) = 11;
               badword = string(1, 10);
               bailout = true;
               return;
            }
            if (ntype2 == 20) {
               munit = "Da    ";
            }
            if (ntype2 == 21) {
               munit = "kDa   ";
            }
            if (ntype2 == 22) {
               munit = "g     ";
            }
            if (ntype2 == 23) {
               munit = "kg    ";
            }
            bm = true;
            //C
         }
         else if (ntype == 24) {
            if (!bw) {
               nextstring(cmn, nin, nobuffer, buffer, string, atend);
               if (atend) {
                  nerr++;
                  errorlist(nerr) = 3;
                  bailout = true;
                  return;
               }
               floatstring(cmn, string, visc);
               nextstring(cmn, nin, nobuffer, buffer, string, atend);
               if (atend) {
                  nerr++;
                  errorlist(nerr) = 3;
                  bailout = true;
                  return;
               }
               testword(cmn, string, ntype2, bailout);
               if (bailout) {
                  return;
               }
               if (ntype2 < 25 || ntype2 > 26) {
                  nerr++;
                  errorlist(nerr) = 11;
                  badword = string(1, 10);
                  bailout = true;
                  return;
               }
               if (ntype2 == 25) {
                  vunit = "p     ";
               }
               if (ntype2 == 26) {
                  vunit = "cp    ";
               }
               bc = true;
            }
            else {
               nerr++;
               errorlist(nerr) = 12;
               FEM_DO_SAFE(jj, 1, 2) {
                  if (atend) {
                     nerr++;
                     errorlist(nerr) = 3;
                     bailout = true;
                     return;
                  }
               }
               bc = false;
            }
            //C
         }
         else if (ntype == 27) {
            if (!bc) {
               nextstring(cmn, nin, nobuffer, buffer, string, atend);
               if (atend) {
                  nerr++;
                  errorlist(nerr) = 3;
                  bailout = true;
                  return;
               }
               testword(cmn, string, ntype2, bailout);
               if (bailout) {
                  return;
               }
               if (ntype2 != 28) {
                  nerr++;
                  errorlist(nerr) = 13;
                  bailout = true;
                  return;
               }
               else {
                  bw = true;
               }
            }
            else {
               bw = false;
            }
            //C
         }
         else {
            nerr++;
            badword = string(1, 10);
            errorlist(nerr) = 10;
            bailout = true;
            return;
            //C
         }
         //C
         nextstring(cmn, nin, nobuffer, buffer, string, atend);
      }
      //C
      if (nelts == 0) {
         nerr++;
         errorlist(nerr) = 5;
         bailout = true;
         return;
      }
      //C
      //C        Normalize all input vectors that need it
      //C        and create all needed rotation matrices
      //C
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         if (eltype(i) == 3 || eltype(i) == 4 || eltype(i) == 5) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               n1(j) = bv(i, j + 3);
            }
            normalize(n1, n);
            zeerot(cmn, n, t);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  rotations(i, j, k) = t(j, k);
               }
            }
            FEM_DO_SAFE(j, 1, 3) {
               bv(i, j + 3) = n(j);
            }
            //C
            //C lens
         }
         else if (eltype(i) == 6) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               d(j) = bv(i, j + 3);
            }
            rc = bv(i, 7);
            rd = bv(i, 8);
            vector_difference(d, c, n1);
            pythag(d, c, dc);
            if (dc > rc + rd) {
               nerr++;
               errorlist(nerr) = 8;
               bailout = true;
            }
            normalize(n1, n);
            zeerot(cmn, n, t);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  rotations(i, j, k) = t(j, k);
               }
            }
            //C
         }
         else if (eltype(i) == 7) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               n1(j) = bv(i, j + 3);
            }
            normalize(n1, nx);
            FEM_DO_SAFE(j, 1, 3) {
               bv(i, j + 3) = nx(j);
            }
            FEM_DO_SAFE(j, 1, 3) {
               n1(j) = bv(i, j + 6);
            }
            normalize(n1, ny);
            FEM_DO_SAFE(j, 1, 3) {
               bv(i, j + 6) = ny(j);
            }
            cross_product(nx, ny, nz);
            xyzrot(nx, ny, nz, t);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  rotations(i, j, k) = t(j, k);
               }
            }
            //C
         }
      }
      //C
      return;
      //C
   statement_999:
      nerr++;
      badword = string(1, 10);
      errorlist(nerr) = 4;
      bailout = true;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   mean20(
          arr_cref<fem::real_star_4> xar,
          fem::real_star_4& x,
          fem::real_star_4& delta)
   {
      xar(dimension(20));
      //C
      fem::real_star_4 s1 = 0.0f;
      fem::real_star_4 s2 = 0.0f;
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 20) {
         s1 += xar(i);
         s2 += fem::pow2(xar(i));
      }
      //C
      x = s1 / 20.0f;
      s1 = s1 / 20.0f;
      s2 = s2 / 20.0f;
      s2 = s2 - fem::pow2(s1);
      delta = fem::sqrt(s2 / 20.0f);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   tally(
         arr_cref<fem::integer_star_4, 2> khitp,
         arr_cref<fem::integer_star_4, 2> khite,
         arr_cref<fem::real_star_8, 3> vp,
         arr_cref<fem::real_star_8, 3> ve,
         arr_cref<fem::real_star_8> sum,
         arr_ref<fem::real_star_4, 2> aa,
         arr_ref<fem::real_star_4, 2> daa,
         fem::real_star_4& cap,
         fem::real_star_4& delta_cap,
         float const& r1)
   {
      khitp(dimension(3, 20));
      khite(dimension(3, 20));
      vp(dimension(3, 3, 20));
      ve(dimension(3, 3, 20));
      sum(dimension(20));
      aa(dimension(3, 3));
      daa(dimension(3, 3));
      //C
      //C        convert raw statistics on random walk trajectories into
      //C        polarizability tensor and capacitance
      //C
      int i = fem::int0;
      int j = fem::int0;
      arr_2d<3, 20, fem::real_star_8> tt(fem::fill0);
      arr_2d<3, 20, fem::real_star_8> tu(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         FEM_DO_SAFE(j, 1, 20) {
            tt(i, j) = fem::dble(khitp(i, j) + khite(i, j)) / sum(j);
            tu(i, j) = fem::dble(khitp(i, j) - khite(i, j)) / sum(j);
         }
      }
      //C
      int k = fem::int0;
      arr_3d<3, 3, 20, fem::real_star_8> tv(fem::fill0);
      arr_3d<3, 3, 20, fem::real_star_8> tw(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 20) {
               tv(i, j, k) = (vp(i, j, k) + ve(i, j, k)) / sum(k);
               tw(i, j, k) = (vp(i, j, k) - ve(i, j, k)) / sum(k);
            }
         }
      }
      //C
      arr_1d<20, fem::real_star_4> capar(fem::fill0);
      FEM_DO_SAFE(k, 1, 20) {
         capar(k) = tt(1, k) * r1;
      }
      //C
      arr_3d<3, 3, 20, fem::real_star_4> aar(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 20) {
               aar(i, j, k) = fem::sngl(tw(i, j, k) - tu(j, k) * tv(i, j,
                                                                    k) / tt(j, k));
            }
         }
      }
      //C
      FEM_DO_SAFE(i, 1, 3) {
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 20) {
               aar(i, j, k) = 3.0f * r1 * r1 * aar(i, j, k);
            }
         }
      }
      //C
      mean20(capar, cap, delta_cap);
      //C
      FEM_DO_SAFE(i, 1, 3) {
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 20) {
               capar(k) = aar(i, j, k);
            }
            mean20(capar, aa(i, j), daa(i, j));
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   accume(
          arr_cref<fem::real_star_4> t,
          arr_cref<fem::integer_star_4> kk,
          arr_ref<fem::integer_star_4, 2> khitp,
          arr_ref<fem::integer_star_4, 2> khite,
          arr_ref<fem::real_star_8, 3> vp,
          arr_ref<fem::real_star_8, 3> ve,
          int const& loop)
   {
      t(dimension(3));
      kk(dimension(3));
      khitp(dimension(3, 20));
      khite(dimension(3, 20));
      vp(dimension(3, 3, 20));
      ve(dimension(3, 3, 20));
      //C
      //C        Accumulate statistics on random walks
      //C
      //C        It's probably not necessary at present, but
      //C        just in case this ever gets run with very
      //C        large numbers of trajectories, accumulate
      //C        statistics in double precision
      //C
      int j = fem::int0;
      int jj = fem::int0;
      FEM_DO_SAFE(j, 1, 3) {
         if (kk(j) ==  + 1) {
            khitp(j, loop)++;
            FEM_DO_SAFE(jj, 1, 3) {
               vp(jj, j, loop) += fem::dble(t(jj));
            }
         }
         else {
            khite(j, loop)++;
            FEM_DO_SAFE(jj, 1, 3) {
               ve(jj, j, loop) += fem::dble(t(jj));
            }
         }
      }
      //C
   }

   struct mincube_save
   {
      fem::variant_bindings cubit_bindings;
   };

   //C
   //C*************************************************
   //C
   void
   mincube(
           common& cmn,
           arr_cref<fem::real_star_4> v1,
           fem::real_star_4 const& side,
           arr_cref<fem::real_star_4> p,
           fem::real_star_4& d)
   {
      FEM_CMN_SVE(mincube);
      v1(dimension(3));
      p(dimension(3));
      common_variant cubit(cmn.common_cubit, sve.cubit_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<int> ncube;
            mbr<float> ferr;
            cubit.allocate(), ncube, ferr;
         }
      }
      int& ncube = cubit.bind<int>();
      float& ferr = cubit.bind<float>();
      //C
      //C        Given a cube with low corner at v1 and with length of side = side,
      //C        also given an arbitrary point p outside the cube
      //C        Return in d the minimum distance from p to the cube
      //C
      float s2 = side / 2.0f;
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         c(i) = v1(i) + s2;
      }
      //C
      arr_1d<3, fem::real_star_4> pr(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         pr(i) = p(i) - c(i);
      }
      //C
      FEM_DO_SAFE(i, 1, 3) {
         pr(i) = fem::abs(pr(i));
      }
      //C
      int nwk = 1;
      float ss = fem::float0;
      while (nwk < 3) {
         if (pr(nwk) <= pr(nwk + 1)) {
            nwk++;
         }
         else {
            ss = pr(nwk);
            pr(nwk) = pr(nwk + 1);
            pr(nwk + 1) = ss;
            nwk = nwk - 1;
         }
         if (nwk == 0) {
            nwk = 1;
         }
      }
      //C
      if (pr(3) < s2) {
         if (ncube == 0) {
            ncube = 1;
            ferr = pr(3) / s2;
         }
         else {
            ferr = fem::amin1(ferr, pr(3) / s2);
         }
         d = 0.0f;
         return;
      }
      //C
      if (pr(2) < s2) {
         d = pr(3) - s2;
         return;
      }
      //C
      if (pr(1) < s2) {
         d = fem::pow2((pr(3) - s2)) + fem::pow2((pr(2) - s2));
         d = fem::sqrt(d);
         return;
      }
      //C
      d = fem::pow2((pr(1) - s2)) + fem::pow2((pr(2) - s2)) + fem::pow2((
                                                                         pr(3) - s2));
      d = fem::sqrt(d);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   minsphere(
             arr_cref<fem::real_star_4> c,
             fem::real_star_4 const& r,
             arr_cref<fem::real_star_4> p,
             fem::real_star_4& d)
   {
      c(dimension(3));
      p(dimension(3));
      //C
      //C        Given a sphere with center at an arbitrary point c and with radius r.
      //C        Also given an arbitrary point p.
      //C        Return in d the minimum distance between all points on the sphere
      //C        and the point p.
      //C
      fem::real_star_4 r1 = fem::zero<fem::real_star_4>();
      pythag(c, p, r1);
      d = fem::abs(r1 - r);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   minedge(
           arr_cref<fem::real_star_4> v1,
           arr_cref<fem::real_star_4> v2,
           arr_cref<fem::real_star_4> p,
           fem::real_star_4& d)
   {
      v1(dimension(3));
      v2(dimension(3));
      p(dimension(3));
      //C
      //C        Given two points v1 and v2 defining a line segment in space
      //C        Also given an arbitrary point p
      //C
      //C        Then return in d the minimum distance between the point p and the
      //C        line segment
      //C
      arr_1d<3, fem::real_star_4> p1(fem::fill0);
      vector_difference(p, v1, p1);
      arr_1d<3, fem::real_star_4> b(fem::fill0);
      vector_difference(v2, v1, b);
      float tb2 = fem::float0;
      dotproduct(p1, b, tb2);
      float b2 = fem::float0;
      dotproduct(b, b, b2);
      float t = tb2 / b2;
      //C
      arr_1d<3, fem::real_star_4> tb(fem::fill0);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      if (t < 0.0f) {
         pythag(v1, p, d);
      }
      else if (t > 1.0f) {
         pythag(v2, p, d);
      }
      else {
         scalar_product(t, b, tb);
         vector_difference(p1, tb, n);
         pythag0(n, d);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   mintriangle(
               arr_cref<fem::real_star_4> v1,
               arr_cref<fem::real_star_4> v2,
               arr_cref<fem::real_star_4> v3,
               arr_cref<fem::real_star_4> p,
               fem::real_star_4& d)
   {
      v1(dimension(3));
      v2(dimension(3));
      v3(dimension(3));
      p(dimension(3));
      //C
      //C        Given a triangle with vertices at v1, v2, v3.
      //C        Also given an arbitrary point p.
      //C        Return in d the minimum distance between all points on the
      //C                surface of the triangle and the point p.
      //C
      arr_1d<3, fem::real_star_4> b1(fem::fill0);
      vector_difference(v2, v1, b1);
      arr_1d<3, fem::real_star_4> b2(fem::fill0);
      vector_difference(v3, v1, b2);
      arr_1d<3, fem::real_star_4> b3(fem::fill0);
      cross_product(b1, b2, b3);
      arr_1d<3, fem::real_star_4> p1(fem::fill0);
      vector_difference(p, v1, p1);
      float dp1b3 = fem::float0;
      dotproduct(p1, b3, dp1b3);
      float db3b3 = fem::float0;
      dotproduct(b3, b3, db3b3);
      float t3 = dp1b3 / db3b3;
      arr_1d<3, fem::real_star_4> t3b3(fem::fill0);
      scalar_product(t3, b3, t3b3);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      vector_difference(p1, t3b3, q);
      float d11 = fem::float0;
      dotproduct(b1, b1, d11);
      float d12 = fem::float0;
      dotproduct(b1, b2, d12);
      float d22 = fem::float0;
      dotproduct(b2, b2, d22);
      float q1 = fem::float0;
      dotproduct(q, b1, q1);
      float q2 = fem::float0;
      dotproduct(q, b2, q2);
      float t1 = (q1 * d22 - q2 * d12) / (d11 * d22 - d12 * d12);
      float t2 = (d11 * q2 - q1 * d12) / (d11 * d22 - d12 * d12);
      float d1 = fem::float0;
      float d2 = fem::float0;
      float d3 = fem::float0;
      float dx = fem::float0;
      if (t1 >= 0.0f && t2 >= 0.0f && (t1 + t2) <= 1.0f) {
         pythag0(t3b3, d);
      }
      else {
         minedge(v1, v2, p, d1);
         minedge(v1, v3, p, d2);
         minedge(v2, v3, p, d3);
         dx = fem::amin1(d1, d2);
         d = fem::amin1(dx, d3);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   mindisk(
           arr_cref<fem::real_star_4> c,
           arr_cref<fem::real_star_4> n,
           fem::real_star_4 const& r,
           arr_cref<fem::real_star_4, 2> /* tr */,
           arr_cref<fem::real_star_4> p,
           fem::real_star_4& d)
   {
      c(dimension(3));
      n(dimension(3));
      p(dimension(3));
      //C
      //C        Given a disk centered at c, with unit normal n and radius r.
      //C        Also given an arbitrary point p.
      //C        Return in d the minimum distance between the point p and the disk.
      //C
      arr_1d<3, fem::real_star_4> pmc(fem::fill0);
      vector_difference(p, c, pmc);
      float t = fem::float0;
      dotproduct(pmc, n, t);
      arr_1d<3, fem::real_star_4> ta(fem::fill0);
      scalar_product(t, n, ta);
      arr_1d<3, fem::real_star_4> nn(fem::fill0);
      vector_difference(pmc, ta, nn);
      float quid = fem::float0;
      pythag0(nn, quid);
      //C
      if (quid <= r) {
         d = fem::abs(t);
      }
      else {
         d = fem::pow2(t) + fem::pow2((quid - r));
         d = fem::sqrt(d);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   mincylinder(
               arr_cref<fem::real_star_4> c,
               arr_cref<fem::real_star_4> /* n */,
               fem::real_star_4 const& r,
               fem::real_star_4 const& l,
               arr_cref<fem::real_star_4, 2> t,
               arr_cref<fem::real_star_4> p,
               fem::real_star_4& d)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p(dimension(3));
      //C
      //C        Given a cylinder centered at c, with unit normal n, radius r,
      //C                length l.
      //C        This returns the minimum distance between points on the cylinder
      //C        and the arbitrary point p.
      //C
      arr_1d<3, fem::real_star_4> pmc(fem::fill0);
      vector_difference(p, c, pmc);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, pmc);
      float ro = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      float dp = fem::abs(ro - r);
      //C
      float zz = fem::float0;
      if (fem::abs(q(3)) > l / 2.0f) {
         zz = fem::abs(q(3)) - l / 2.0f;
         d = fem::sqrt(fem::pow2(zz) + fem::pow2(dp));
      }
      else {
         d = dp;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   mintorus(
            arr_cref<fem::real_star_4> c,
            arr_cref<fem::real_star_4> /* n */,
            fem::real_star_4 const& r1,
            fem::real_star_4 const& r2,
            arr_cref<fem::real_star_4, 2> t,
            arr_cref<fem::real_star_4> p,
            fem::real_star_4& d)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p(dimension(3));
      //C
      //C        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
      //C
      //C        Given a torus centered at c, with unit normal n, radii r1 and r2.
      //C        This returns the minimum distance of points on the torus away
      //C        from an arbitrary point p.
      //C
      //C                  x                         x
      //C             x       x                 x       x
      //C            x         x               x         x   ___
      //C            x         x               x         x    |
      //C             x       x                 x       x     |  r2
      //C                 x                         x        ---
      //C
      //C                 |--------2 * r1 ----------|
      //C
      arr_1d<3, fem::real_star_4> pmc(fem::fill0);
      vector_difference(p, c, pmc);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, pmc);
      //C
      float qq = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      float sig = q(3) / (qq - r1);
      float dod = fem::sqrt(fem::pow2(r2) / (1.0f + fem::pow2(sig)));
      float x1 = r1 + dod;
      float x2 = r1 - dod;
      float y1 = sig * dod;
      float y2 = -sig * dod;
      float d1 = fem::pow2((qq - x1)) + fem::pow2((q(3) - y1));
      float d2 = fem::pow2((qq - x2)) + fem::pow2((q(3) - y2));
      //C
      sig = q(3) / (qq + r1);
      dod = fem::sqrt(fem::pow2(r2) / (1.0f + fem::pow2(sig)));
      x1 = -r1 + dod;
      x2 = -r1 - dod;
      y1 = sig * dod;
      y2 = -sig * dod;
      float d3 = fem::pow2((qq - x1)) + fem::pow2((q(3) - y1));
      float d4 = fem::pow2((qq - x2)) + fem::pow2((q(3) - y2));
      //C
      d = fem::amin1(d1, d2);
      d = fem::amin1(d, d3);
      d = fem::amin1(d, d4);
      //C
      d = fem::sqrt(d);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   sphereby(
            arr_cref<fem::real_star_4> p,
            arr_cref<fem::real_star_4> c,
            fem::real_star_4 const& rc,
            arr_ref<fem::real_star_4> s)
   {
      p(dimension(3));
      c(dimension(3));
      s(dimension(3));
      //C
      //C        There is a sphere centered at (cx,cy,cz) with radius rc.  Return in
      //C        s the coordinates of the point on the surface of the sphere that is
      //C        nearest the point p.
      //C
      float cmp = fem::float0;
      vector_difference(c, p, cmp);
      float cc = fem::float0;
      pythag0(cmp, cc);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      normalize(cmp, n);
      arr_1d<3, fem::real_star_4> v(fem::fill0);
      scalar_product(cc - rc, n, v);
      //C
      s(1) = p(1) + v(1);
      s(2) = p(2) + v(2);
      s(3) = p(3) + v(3);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   minlens(
           arr_cref<fem::real_star_4> c,
           arr_cref<fem::real_star_4> d,
           fem::real_star_4 const& rc,
           fem::real_star_4 const& rd,
           arr_cref<fem::real_star_4, 2> t,
           arr_cref<fem::real_star_4> p,
           fem::real_star_4& dm)
   {
      c(dimension(3));
      d(dimension(3));
      t(dimension(3, 3));
      p(dimension(3));
      //C
      //C        LENS[(cx,cy,cz),(dx,dy,dz),rc,rd]
      //C        real*4 c(3),d(3),rc,rd
      //C        Defined as the intersection of two spheres, centered
      //C        respectively at c and d,
      //C        having radii respectively rc, rd
      //C
      //C        p is an external point
      //C
      //C        return in dm the minimum distance between p and the lens
      //C
      arr_1d<3, fem::real_star_4> cmi(fem::fill0);
      sphereby(p, c, rc, cmi);
      arr_1d<3, fem::real_star_4> dmi(fem::fill0);
      sphereby(p, d, rd, dmi);
      //C
      float cmd = fem::float0;
      pythag(cmi, d, cmd);
      float dmc = fem::float0;
      pythag(dmi, c, dmc);
      //C
      if (cmd < rd) {
         pythag(cmi, p, dm);
         return;
      }
      else if (dmc < rc) {
         pythag(dmi, p, dm);
         return;
      }
      //C
      float rcd = fem::float0;
      pythag(c, d, rcd);
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      vector_difference(d, c, n1);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      normalize(n1, n);
      //C
      float x = (fem::pow2(rc) + fem::pow2(rcd) - fem::pow2(rd)) / (2.0f * rcd);
      float y = fem::sqrt(fem::pow2(rc) - fem::pow2(x));
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> diskc(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         diskc(i) = c(i) + x * n(i);
      }
      //C
      mindisk(diskc, n, y, t, p, dm);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   minellipsoid(
                arr_cref<fem::real_star_4> c,
                arr_cref<fem::real_star_4> n1,
                arr_cref<fem::real_star_4> n2,
                float const& aa,
                float const& bb,
                float const& cc,
                arr_cref<fem::real_star_4, 2> t,
                arr_cref<fem::real_star_4> p,
                float& d)
   {
      c(dimension(3));
      n1(dimension(3));
      n2(dimension(3));
      t(dimension(3, 3));
      p(dimension(3));
      //C
      //C        return in d the minimum distance between the ellipsoid and
      //C        the point p
      //C
      int minmax = -1;
      exellipsoid(c, n1, n2, aa, bb, cc, t, p, minmax, d);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   distance(
            common& cmn,
            int const& maxelts,
            arr_cref<fem::integer_star_4> eltype,
            arr_cref<fem::real_star_4, 2> bv,
            fem::integer_star_4 const& nelts,
            arr_cref<fem::real_star_4, 3> rotations,
            arr_cref<fem::real_star_4> p,
            float& ds)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      p(dimension(3));
      //C
      //C        Compute distance from point p to the surface of the body
      //C
      int i = fem::int0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> v1(fem::fill0);
      float side = fem::float0;
      float d = fem::float0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      float r = fem::float0;
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      arr_1d<3, fem::real_star_4> v3(fem::fill0);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      int k = fem::int0;
      arr_2d<3, 3, fem::real_star_4> t(fem::fill0);
      float al = fem::float0;
      float r1 = fem::float0;
      float r2 = fem::float0;
      arr_1d<3, fem::real_star_4> dp(fem::fill0);
      float rc = fem::float0;
      float rd = fem::float0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n2(fem::fill0);
      float aa = fem::float0;
      float bb = fem::float0;
      float cc = fem::float0;
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         //C Cubes
         if (eltype(i) == 8) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               v1(j) = bv(i, j);
            }
            side = bv(i, 4);
            mincube(cmn, v1, side, p, d);
            //C
            //C Spheres
         }
         else if (eltype(i) == 1) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
            }
            r = bv(i, 4);
            minsphere(c, r, p, d);
            //C
            //C Triangles
         }
         else if (eltype(i) == 2) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               v1(j) = bv(i, j);
               v2(j) = bv(i, j + 3);
               v3(j) = bv(i, j + 6);
            }
            mintriangle(v1, v2, v3, p, d);
            //C
            //C Disks
         }
         else if (eltype(i) == 3) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r = bv(i, 7);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            mindisk(c, n, r, t, p, d);
            //C
            //C Cylinders
         }
         else if (eltype(i) == 4) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r = bv(i, 7);
            al = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            mincylinder(c, n, r, al, t, p, d);
            //C
            //C Tori
         }
         else if (eltype(i) == 5) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r1 = bv(i, 7);
            r2 = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            mintorus(c, n, r1, r2, t, p, d);
            //C
            //C Lenses
         }
         else if (eltype(i) == 6) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               dp(j) = bv(i, j + 3);
            }
            rc = bv(i, 7);
            rd = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            minlens(c, dp, rc, rd, t, p, d);
            //C
            //C  Ellipsoids
         }
         else if (eltype(i) == 7) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n1(j) = bv(i, j + 3);
               n2(j) = bv(i, j + 6);
            }
            aa = bv(i, 10);
            bb = bv(i, 11);
            cc = bv(i, 12);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            minellipsoid(c, n1, n2, aa, bb, cc, t, p, d);
            //C
         }
         //C
         if (i == 1) {
            ds = d;
         }
         else {
            ds = fem::amin1(ds, d);
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   greensphere(
               common& cmn,
               arr_ref<fem::real_star_4> p,
               fem::real_star_4 const& r,
               fem::real_star_4 const& r0,
               bool& gone)
   {
      p(dimension(3));
      //C
      //C        Upon entry, p contains the coordinate of the
      //C        point, r contains the radius of the launch
      //C        sphere, and r0 contains the distance of p
      //C        from the origin.  It is known that p lies
      //C        outside the launch sphere.
      //C
      //C        This determines the probability that the
      //C        walker escapes to infinity without ever
      //C        returning to the launch sphere, and lets the
      //C        walker escape with that probablity.  Otherwise,
      //C        it returns the walker to a new point on the
      //C        launch sphere.
      //C
      //C        Upon exit, gone = .true. if the walker has
      //C        escaped to infinity.  Otherwise, gone = .false.,
      //C        and p contains the coordinates of the new
      //C        point on the launch sphere.
      //C
      float alpha = r / r0;
      gone = ran2(cmn) > alpha;
      if (gone) {
         return;
      }
      //C
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         p(i) = p(i) / r0;
      }
      //C
      //C        Pull off the numbers needed to transform the p-vector
      //C        to the plus z-axis:
      //C
      float cost = p(3);
      //C
      //C        Minor correction for round-off errors:
      if (cost > 1.0f) {
         cost = 1.0f;
      }
      if (cost <  - 1.0f) {
         cost = -1.0f;
      }
      //C
      float thetr = fem::acos(cost);
      float phir = fem::atan2(p(2), p(1));
      //C
      //C        With modern fortran implementations, atan2
      //C        has a range of 2*pi, so further correction of
      //C        phir is not necessary
      //C
      //C        For the time being, assume a new coordinate system for
      //C        which the radius vector is on the z-axis (thetr and phir
      //C        will permit us to transform back later)
      //C
      float s = ran2(cmn);
      //C
      float cx = fem::float0;
      float t1 = fem::float0;
      float t2 = fem::float0;
      float t3 = fem::float0;
      if (s == 0.0f) {
         cx = -1.0f;
      }
      else {
         t1 = (1.0f + fem::pow2(alpha)) / (2.0f * alpha);
         t2 = fem::pow2((1.0f - fem::pow2(alpha)));
         t3 = fem::pow2((1.0f - alpha + 2.0f * alpha * s));
         cx = t1 - t2 / (2.0f * alpha * t3);
      }
      //C
      //C        Minor correction for roundoff errors:
      if (cx > 1.0f) {
         cx = 1.0f;
      }
      if (cx <  - 1.0f) {
         cx = -1.0f;
      }
      //C
      float sx = fem::sqrt(1.0f - fem::pow2(cx));
      float pip = 2.0f * 3.14159265f * ran2(cmn);
      float sp = fem::sin(pip);
      float cp = fem::cos(pip);
      //C
      //C        This is the new position of the diffusor in the 2nd
      //C        coordinate system
      //C
      p(1) = r * sx * cp;
      p(2) = r * sx * sp;
      p(3) = r * cx;
      //C
      //C        Now transform to the first coordinate system, first
      //C        by a rotation about y-axis through thetr
      //C
      float sint = fem::sin(thetr);
      float zn = p(3) * cost - p(1) * sint;
      float xn = p(3) * sint + p(1) * cost;
      p(1) = xn;
      p(3) = zn;
      //C
      //C        and then by a rotation about z-axis through phir
      //C
      float cosp = fem::cos(phir);
      float sinp = fem::sin(phir);
      //C
      xn = p(1) * cosp - p(2) * sinp;
      float yn = p(2) * cosp + p(1) * sinp;
      p(1) = xn;
      p(2) = yn;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   park(
        common& cmn,
        int const& maxelts,
        arr_cref<fem::integer_star_4> eltype,
        arr_cref<fem::real_star_4, 2> bv,
        arr_cref<fem::real_star_4, 3> rotations,
        fem::integer_star_4 const& nelts,
        arr_ref<fem::real_star_4> p,
        float const& r,
        float const& r2,
        bool& hit,
        float const& tol)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      p(dimension(3));
      bool firstpass = fem::bool0;
      float r0 = fem::float0;
      bool gone = fem::bool0;
      fem::real_star_4 ds = fem::zero<fem::real_star_4>();
      arr_1d<3, fem::real_star_4> d(fem::fill0);
      int i = fem::int0;
      //C
      //C--------------------------------------------------------
      //C
      //C        Upon entry, the random walker sits at a point p on
      //C        the launch sphere.  r is the radius of the launch
      //C        sphere, and r2 is its square.
      //C
      //C        This subroutine lets the walker drift down onto the
      //C        object or else out to infinity.  If it is lost to
      //C        infinity, hit is returned as .false., and if it
      //C        drifts onto the body, hit is returned as .true. and
      //C        p is returned with the point at which the walker hits.
      //C
      //C        This is the algorithm obeyed by the subroutine:
      //C
      //C        1.  If the point p lies inside the launch sphere, proceed
      //C                to step 2.  If it lies outside the launch sphere,
      //C                move it onto the launch sphere using the charge-
      //C                outside-a-sphere Green's function, which may also
      //C                move the point off to infinity.  If the point gets
      //C                moved off to infinity, set hit = .false. and
      //C                return.
      //C
      //C        2.  The walker is now on or inside the launch sphere,
      //C                but outside the body.  A call to distance returns
      //C                the distance to the body, ds.  If ds is less than
      //C                tol, the program assumes that the walker has
      //C                adsorbed and so hit is set equal to .true. and
      //C                we return.  Otherwise, we jump to the surface
      //C                of the sphere that is centered on the current
      //C                point and has radius ds.
      //C
      //C        3.  Loop back to step 1.
      //C
      //C--------------------------------------------------------
      //C
      firstpass = true;
      //C
   statement_1:
      //C
      //C  STEP 1:
      //C
      r0 = fem::pow2(p(1)) + fem::pow2(p(2)) + fem::pow2(p(3));
      //C
      if (r0 > r2) {
         //C
         //C  No need to call
         if (firstpass) {
            //C  greensphere, on first
            //C  pass it could only be
            //C  due to round-off that
            //C  we are here.
            gone = false;
         }
         else {
            r0 = fem::sqrt(r0);
            greensphere(cmn, p, r, r0, gone);
         }
         //C
         if (gone) {
            hit = false;
            return;
         }
         //C
      }
      //C
      //C  STEP 2:
      //C
      distance(cmn, maxelts, eltype, bv, nelts, rotations, p, ds);
      //C
      if (ds < tol) {
         hit = true;
         return;
      }
      //C
      sphere(cmn, d, ds);
      FEM_DO_SAFE(i, 1, 3) {
         p(i) += d(i);
      }
      //C
      //C  STEP 3:
      //C
      firstpass = false;
      goto statement_1;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   reinit(
          arr_ref<fem::integer_star_4, 2> khitp,
          arr_ref<fem::integer_star_4, 2> khite,
          arr_ref<fem::real_star_8, 3> vp,
          arr_ref<fem::real_star_8, 3> ve,
          arr_ref<fem::real_star_8> sum)
   {
      khitp(dimension(3, 20));
      khite(dimension(3, 20));
      vp(dimension(3, 3, 20));
      ve(dimension(3, 3, 20));
      sum(dimension(20));
      //C
      //C        initialize statistical registers
      //C
      int k = fem::int0;
      int i = fem::int0;
      int j = fem::int0;
      FEM_DO_SAFE(k, 1, 20) {
         FEM_DO_SAFE(i, 1, 3) {
            FEM_DO_SAFE(j, 1, 3) {
               vp(i, j, k) = 0.0e0;
               ve(i, j, k) = 0.0e0;
            }
            khite(i, k) = 0;
            khitp(i, k) = 0;
         }
         sum(k) = 0.0e0;
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   charge(
          common& cmn,
          arr_cref<fem::real_star_4> rt,
          float const& r1,
          arr_ref<fem::integer_star_4> kk)
   {
      rt(dimension(3));
      kk(dimension(3));
      //C
      //C        Assign three charges to random walker, kk(i) = i-th charge
      //C
      int i = fem::int0;
      float x = fem::float0;
      float p = fem::float0;
      FEM_DO_SAFE(i, 1, 3) {
         x = rt(i) / r1;
         p = 0.5f * (1.0f + x);
         if (ran2(cmn) < p) {
            kk(i) = +1;
         }
         else {
            kk(i) = -1;
         }
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   blizzard(
            common& cmn,
            int const& maxelts,
            arr_cref<fem::integer_star_4> eltype,
            arr_cref<fem::real_star_4, 2> bv,
            fem::integer_star_4 const& nelts,
            fem::integer_star_4 const& m1,
            fem::real_star_4& tol,
            float const& rlaunch,
            arr_cref<fem::real_star_4, 3> rotations,
            fem::real_star_4& cap,
            arr_ref<fem::real_star_4, 2> alpha_bongo,
            bool& tol_given,
            bool& zeno_done,
            fem::integer_star_4 const& /* nout */,
            fem::real_star_4& delta_cap,
            arr_ref<fem::real_star_4, 2> delta_bongo,
            fem::integer_star_4& mz)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      alpha_bongo(dimension(3, 3));
      delta_bongo(dimension(3, 3));
      common_write write(cmn);
      // COMMON shush
      bool& silent = cmn.silent;
      //
      //C
      //C        Generate random walk trajectories to do path-integration
      //C
      zeno_done = false;
      //C
      if (!tol_given) {
         tol = rlaunch / 1.0e6f;
         tol_given = true;
      }
      //C
      float r = rlaunch;
      float r2 = fem::pow2(r);
      int mtdo = m1;
      //C
      // int i = fem::int0;
      //C
      arr_2d<3, 20, fem::integer_star_4> khitp(fem::fill0);
      arr_2d<3, 20, fem::integer_star_4> khite(fem::fill0);
      arr_3d<3, 3, 20, fem::real_star_8> vp(fem::fill0);
      arr_3d<3, 3, 20, fem::real_star_8> ve(fem::fill0);
      arr_1d<20, fem::real_star_8> sum(fem::fill0);
      reinit(khitp, khite, vp, ve, sum);
      //C
      if (!silent) {
         write(6, "('ZENO CALCULATION')");
         zeno_us_hydrodyn->editor->append( "ZENO calculation start\n" );
      }
      if (!silent) {
         write(6, "(108('='))");
      }
      int mout = 0;
      //C
      int jax = fem::int0;
      int loop = fem::int0;
      arr_1d<3, fem::real_star_4> rt(fem::fill0);
      arr_1d<3, fem::integer_star_4> kk(fem::fill0);
      bool hit = fem::bool0;
      int need = fem::int0;
      FEM_DO_SAFE(jax, 1, mtdo) {
         loop = fem::mod(jax, 20) + 1;
         sum(loop) += 1.0e0;
         //C
         sphere(cmn, rt, r);
         charge(cmn, rt, r, kk);
         park(cmn, maxelts, eltype, bv, rotations, nelts, rt, r, r2, hit, tol);
         //C
         if (hit) {
            accume(rt, kk, khitp, khite, vp, ve, loop);
         }
         //C
         need = fem::nint(108.0f * fem::ffloat(jax) / fem::ffloat(mtdo));
         while (mout < need) {
            if (!silent) {
               std::cout << "X" << std::flush;
               if ( zeno_progress )
               {
                  zeno_progress->setProgress( zeno_progress->progress() + 1 );
                  qApp->processEvents();
                  if ( zeno_us_hydrodyn->stopFlag )
                  {
                     // somehow abort
                     return;
                  }
               }
            }
            mout++;
         }
      }
      if (!silent) {
         write(6, "(' ')");
      }
      //C
      arr_2d<3, 3, fem::real_star_4> aa(fem::fill0);
      arr_2d<3, 3, fem::real_star_4> daa(fem::fill0);
      tally(khitp, khite, vp, ve, sum, aa, daa, cap, delta_cap, r);
      //C
      int ii = fem::int0;
      int jj = fem::int0;
      FEM_DO_SAFE(ii, 1, 3) {
         FEM_DO_SAFE(jj, 1, 3) {
            alpha_bongo(ii, jj) = aa(ii, jj);
            delta_bongo(ii, jj) = daa(ii, jj);
         }
      }
      //C
      zeno_done = true;
      mz = mtdo;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   inner_sphere(
                common& cmn,
                arr_ref<fem::real_star_4> rt,
                fem::real_star_4 const& r1)
   {
      rt(dimension(3));
      int j = fem::int0;
      float dd = fem::float0;
      //C
      //C        Get a point distributed randomly inside a
      //C        sphere of radius r1
      //C
   statement_1:
      //C
      FEM_DO_SAFE(j, 1, 3) {
         rt(j) = ran2(cmn) * 2.0f - 1.0f;
      }
      //C
      dd = fem::pow2(rt(1)) + fem::pow2(rt(2)) + fem::pow2(rt(3));
      if (dd > 1.0f) {
         goto statement_1;
      }
      FEM_DO_SAFE(j, 1, 3) {
         rt(j) = r1 * rt(j);
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   insidecube(
              arr_cref<fem::real_star_4> v1,
              fem::real_star_4 const& side,
              arr_cref<fem::real_star_4> p,
              bool& inside)
   {
      v1(dimension(3));
      p(dimension(3));
      //C
      //C        Given a cube with low corner at v1 and with length of side = side,
      //C        also given an arbitrary point p
      //C        report in "inside" whether or not p is inside the cube
      //C
      float s2 = side / 2.0f;
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         c(i) = v1(i) + s2;
      }
      //C
      arr_1d<3, fem::real_star_4> pr(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         pr(i) = p(i) - c(i);
      }
      //C
      FEM_DO_SAFE(i, 1, 3) {
         pr(i) = fem::abs(pr(i));
      }
      //C
      int nwk = 1;
      float ss = fem::float0;
      while (nwk < 3) {
         if (pr(nwk) <= pr(nwk + 1)) {
            nwk++;
         }
         else {
            ss = pr(nwk);
            pr(nwk) = pr(nwk + 1);
            pr(nwk + 1) = ss;
            nwk = nwk - 1;
         }
         if (nwk == 0) {
            nwk = 1;
         }
      }
      //C
      inside = pr(3) < s2;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   insidesphere(
                arr_cref<fem::real_star_4> c,
                fem::real_star_4 const& r,
                arr_cref<fem::real_star_4> p,
                bool& inside)
   {
      c(dimension(3));
      p(dimension(3));
      //C
      //C        Given a sphere with center at an arbitrary point c and with radius r.
      //C        Also given an arbitrary point p.
      //C        return inside=.true. if p is inside the sphere.
      //C
      fem::real_star_4 r1 = fem::zero<fem::real_star_4>();
      pythag(c, p, r1);
      inside = r1 < r;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   insidetorus(
               arr_cref<fem::real_star_4> c,
               arr_cref<fem::real_star_4> /* n */,
               fem::real_star_4 const& r1,
               fem::real_star_4 const& r2,
               arr_cref<fem::real_star_4, 2> t,
               arr_cref<fem::real_star_4> p,
               bool& inside)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p(dimension(3));
      //C
      //C        TORUS[(cx,cy,cz),(nx,ny,nz),r1,r2]
      //C
      //C        Given a torus centered at c, with unit normal n, radii r1 and r2.
      //C        This returns inside as true if p is insde the torus.
      //C
      //C                  x                         x
      //C             x       x                 x       x
      //C            x         x               x         x   ___
      //C            x         x               x         x    |
      //C             x       x                 x       x     |  r2
      //C                 x                         x        ---
      //C
      //C                 |--------2 * r1 ----------|
      //C
      arr_1d<3, fem::real_star_4> pmc(fem::fill0);
      vector_difference(p, c, pmc);
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      rotate(q, t, pmc);
      //C
      float qq = fem::sqrt(fem::pow2(q(1)) + fem::pow2(q(2)));
      //C
      float d1 = fem::pow2((qq - r1)) + fem::pow2(q(3));
      d1 = fem::sqrt(d1);
      if (d1 < r2) {
         inside = true;
         return;
      }
      //C
      d1 = fem::pow2((qq + r1)) + fem::pow2(q(3));
      d1 = fem::sqrt(d1);
      if (d1 < r2) {
         inside = true;
         return;
      }
      //C
      inside = false;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   insidelens(
              arr_cref<fem::real_star_4> c,
              arr_cref<fem::real_star_4> d,
              fem::real_star_4 const& rc,
              fem::real_star_4 const& rd,
              arr_cref<fem::real_star_4> p,
              bool& inside)
   {
      c(dimension(3));
      d(dimension(3));
      p(dimension(3));
      //C
      //C        LENS[(cx,cy,cz),(dx,dy,dz),rc,rd]
      //C        real*4 c(3),d(3),rc,rd
      //C        Defined as the intersection of two spheres, centered
      //C        respectively at c and d,
      //C        having radii respectively rc, rd
      //C
      //C        p is an arbitrary point
      //C
      //C        return inside = .true. if p is simultaneously inside both
      //C
      float cpd = fem::float0;
      pythag(c, p, cpd);
      float dpd = fem::float0;
      pythag(d, p, dpd);
      //C
      inside = (cpd < rc) && (dpd < rd);
      //C
   }

   //C
   //C*************************************************
   //C
   void
   insideellipsoid(
                   arr_cref<fem::real_star_4> c,
                   arr_cref<fem::real_star_4> /* n1 */,
                   arr_cref<fem::real_star_4> /* n2 */,
                   fem::real_star_4 const& aa,
                   fem::real_star_4 const& bb,
                   fem::real_star_4 const& cc,
                   arr_cref<fem::real_star_4, 2> t,
                   arr_cref<fem::real_star_4> po,
                   bool& inside)
   {
      c(dimension(3));
      t(dimension(3, 3));
      po(dimension(3));
      //C
      //C        return inside=.true. if point p is on the interior of the
      //C        ellipsoid
      //C
      arr_1d<3, fem::real_star_4> pomc(fem::fill0);
      vector_difference(po, c, pomc);
      arr_1d<3, fem::real_star_4> p(fem::fill0);
      rotate(p, t, pomc);
      //C
      float zz = fem::pow2((p(1) / aa)) + fem::pow2((p(2) / bb)) +
         fem::pow2((p(3) / cc));
      //C
      inside = zz < 1.0f;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   inbody(
          arr_cref<fem::real_star_4> rt,
          int const& maxelts,
          arr_cref<fem::integer_star_4> eltype,
          arr_cref<fem::real_star_4, 2> bv,
          fem::integer_star_4 const& nelts,
          arr_cref<fem::real_star_4, 3> rotations,
          bool& inside,
          bool& early,
          str_ref mess)
   {
      rt(dimension(3));
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      //C
      //C        Returns inside as true if the point rt lies inside the body
      //C
      inside = false;
      early = false;
      //C
      int i = fem::int0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      float side = fem::float0;
      float r = fem::float0;
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      float r1 = fem::float0;
      float r2 = fem::float0;
      int k = fem::int0;
      arr_2d<3, 3, fem::real_star_4> t(fem::fill0);
      arr_1d<3, fem::real_star_4> d(fem::fill0);
      float rc = fem::float0;
      float rd = fem::float0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n2(fem::fill0);
      float aa = fem::float0;
      float bb = fem::float0;
      float cc = fem::float0;
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         //C  Cubes
         if (eltype(i) == 8) {
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
            }
            side = bv(i, 4);
            insidecube(c, side, rt, inside);
            if (inside) {
               return;
            }
         }
         //C
         //C  Spheres
         if (eltype(i) == 1) {
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
            }
            r = bv(i, 4);
            insidesphere(c, r, rt, inside);
            if (inside) {
               return;
            }
         }
         //C
         if (eltype(i) == 2) {
            mess = "TRIANGLE  ";
            early = true;
            return;
         }
         //C
         if (eltype(i) == 3) {
            mess = "DISK      ";
            early = true;
            return;
         }
         //C
         if (eltype(i) == 4) {
            mess = "CYLINDER  ";
            early = true;
            return;
         }
         //C
         //C  Tori
         if (eltype(i) == 5) {
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n(j) = bv(i, j + 3);
            }
            r1 = bv(i, 7);
            r2 = bv(i, 8);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            insidetorus(c, n, r1, r2, t, rt, inside);
            if (inside) {
               return;
            }
         }
         //C
         //C  Lenses
         if (eltype(i) == 6) {
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               d(j) = bv(i, j + 3);
            }
            rc = bv(i, 7);
            rd = bv(i, 8);
            insidelens(c, d, rc, rd, rt, inside);
            if (inside) {
               return;
            }
         }
         //C
         //C Ellipsoids
         if (eltype(i) == 7) {
            //C
            FEM_DO_SAFE(j, 1, 3) {
               c(j) = bv(i, j);
               n1(j) = bv(i, j + 3);
               n2(j) = bv(i, j + 6);
            }
            aa = bv(i, 10);
            bb = bv(i, 11);
            cc = bv(i, 12);
            FEM_DO_SAFE(j, 1, 3) {
               FEM_DO_SAFE(k, 1, 3) {
                  t(j, k) = rotations(i, j, k);
               }
            }
            insideellipsoid(c, n1, n2, aa, bb, cc, t, rt, inside);
            //C
         }
         //C
      }
      //C
   }

   //C
   //C*************************************************
   //C
   void
   dorg(
        common& cmn,
        int const& maxelts,
        arr_cref<fem::integer_star_4> eltype,
        arr_cref<fem::real_star_4, 2> bv,
        fem::integer_star_4 const& nelts,
        fem::real_star_4 const& rlaunch,
        arr_cref<fem::real_star_4, 3> rotations,
        fem::integer_star_4 const& m1,
        fem::real_star_4& rg2int,
        fem::real_star_4& delta_rg2int,
        fem::real_star_4& volume,
        fem::real_star_4& delta_volume,
        bool& rg_done,
        int const& /* nout */,
        fem::integer_star_4& mi)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      common_write write(cmn);
      bool& silent = cmn.silent;
      fem::integer_star_4& nerr = cmn.nerr;
      arr_ref<fem::integer_star_4> errorlist(cmn.errorlist, dimension(100));
      //
      int i = fem::int0;
      int mout = fem::int0;
      int k = fem::int0;
      arr_1d<20, fem::real_star_8> trials(fem::fill0);
      arr_1d<20, fem::real_star_8> sm1(fem::fill0);
      arr_1d<20, fem::real_star_8> rg2sum(fem::fill0);
      arr_1d<20, fem::real_star_8> rg2norm(fem::fill0);
      int loop = fem::int0;
      arr_1d<3, fem::real_star_4> rt1(fem::fill0);
      bool inside = fem::bool0;
      bool early = fem::bool0;
      fem::str<10> mess = fem::char0;
      arr_1d<3, fem::real_star_4> rt2(fem::fill0);
      int jj = fem::int0;
      int need = fem::int0;
      float pi = fem::float0;
      float volsphere = fem::float0;
      arr_1d<20, fem::real_star_4> rad(fem::fill0);
      //C
      //C        numerical integration of the radius of gyration
      //C        and the volume
      //C
      rg_done = false;
      //C
      //C
      if (!silent) {
         write(6, "('INTERIOR CALCULATION')");
         zeno_us_hydrodyn->editor->append( "ZENO interior calculation start\n" );
      }
      if (!silent) {
         write(6, "(108('='))");
      }
      //C
      mout = 0;
      //C
      FEM_DO_SAFE(k, 1, 20) {
         trials(k) = 0.0e0;
         sm1(k) = 0.0e0;
         rg2sum(k) = 0.0e0;
         rg2norm(k) = 0.0e0;
      }
      //C
      FEM_DO_SAFE(i, 1, m1) {
         loop = fem::mod(i, 20) + 1;
         sm1(loop) += 1.0e0;
      statement_1:
         inner_sphere(cmn, rt1, rlaunch);
         //C
         //C          rt1 is the trial point
         //C
         inbody(rt1, maxelts, eltype, bv, nelts, rotations, inside, early, mess);
         //C
         if (early) {
            nerr++;
            errorlist(nerr) = 7;
            cmn.badword = mess;
            return;
         }
         //C
         trials(loop) += 1.0e0;
         if (!inside) {
            goto statement_1;
         }
         //C
         sm1(loop) += 1.0e0;
      statement_2:
         inner_sphere(cmn, rt2, rlaunch);
         //C
         //C          rt2 is the second trial point
         //C
         inbody(rt2, maxelts, eltype, bv, nelts, rotations, inside, early, mess);
         //C
         trials(loop) += 1.0e0;
         if (!inside) {
            goto statement_2;
         }
         //C
         //C          At this point, we have two points, rt1 and rt2, and we know that
         //C          both of them lie inside the body.
         //C
         FEM_DO_SAFE(jj, 1, 3) {
            rg2sum(loop) += fem::pow2(fem::dble(rt1(jj) - rt2(jj)));
         }
         rg2norm(loop) += 2.0e0;
         //C
         need = fem::nint(108.0f * fem::ffloat(i) / fem::ffloat(m1));
         while (mout < need) {
            if (!silent) {
               std::cout << "X" << std::flush;
               if ( zeno_progress )
               {
                  zeno_progress->setProgress( zeno_progress->progress() + 1 );
                  qApp->processEvents();
                  if ( zeno_us_hydrodyn->stopFlag )
                  {
                     // somehow abort
                     return;
                  }
               }
            }
            mout++;
         }
         //C
      }
      //C
      if (!silent) {
         write(6, "(' ')");
      }
      //C
      pi = 3.14159265f;
      volsphere = (4.0f / 3.0f) * pi * (fem::pow3(rlaunch));
      FEM_DO_SAFE(i, 1, 20) {
         rad(i) = volsphere * fem::sngl(sm1(i) / trials(i));
      }
      mean20(rad, volume, delta_volume);
      //C
      FEM_DO_SAFE(i, 1, 20) {
         rad(i) = fem::sngl(rg2sum(i) / rg2norm(i));
      }
      mean20(rad, rg2int, delta_rg2int);
      //C
      rg_done = true;
      mi = m1;
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   trisurf(
           arr_cref<fem::real_star_4> v1,
           arr_cref<fem::real_star_4> v2,
           arr_cref<fem::real_star_4> v3,
           float& sa)
   {
      v1(dimension(3));
      v2(dimension(3));
      v3(dimension(3));
      //C
      //C        Compute surface area of triangle
      //C
      arr_1d<3, fem::real_star_4> p23(fem::fill0);
      vector_difference(v2, v3, p23);
      arr_1d<3, fem::real_star_4> p21(fem::fill0);
      vector_difference(v2, v1, p21);
      float top = fem::float0;
      dotproduct(p23, p21, top);
      float bot = fem::float0;
      dotproduct(p21, p21, bot);
      float tt = top / bot;
      float h = fem::sqrt(bot);
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      FEM_DO_SAFE(i, 1, 3) {
         q(i) = tt * v1(i) + (1.0f - tt) * v2(i);
      }
      arr_1d<3, fem::real_star_4> av(fem::fill0);
      vector_difference(v3, q, av);
      float a = fem::float0;
      pythag0(av, a);
      sa = a * h / 2.0f;
   }

   //C
   //C**********************************************************************
   //C
   //C  as > bs > cs
   void
   sort3(
         float& as,
         float& bs,
         float& cs)
   {
      //C
      //C        sort the three axes
      //C
      arr_1d<3, fem::real_star_4> a(fem::fill0);
      a(1) = as;
      a(2) = bs;
      a(3) = cs;
      //C
      int nwk = 1;
      float save = fem::float0;
      while (nwk < 3) {
         if (nwk == 0) {
            nwk = 1;
         }
         if (a(nwk) >= a(nwk + 1)) {
            nwk++;
         }
         else {
            save = a(nwk);
            a(nwk) = a(nwk + 1);
            a(nwk + 1) = save;
            nwk = nwk - 1;
         }
      }
      //C
      as = a(1);
      bs = a(2);
      cs = a(3);
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   ellsurf(
           float const& aa,
           float const& bb,
           float const& cc,
           float& sa)
   {
      //C
      //C        Compute surface area of ellipsoid
      //C
      //C        Sort the eigenvalues:
      float as = aa;
      float bs = bb;
      float cs = cc;
      //C  as > bs > cs
      sort3(as, bs, cs);
      //C
      //C        Evaluate surface integral by double simpsons
      //C
      float pi = 3.14159265f;
      //C
      float phi = fem::float0;
      float t1 = fem::float0;
      float t2 = fem::float0;
      if (as == bs && bs == cs) {
         sa = 4.0f * pi * cs * cs;
         return;
      }
      else if (as == bs) {
         phi = fem::asin(fem::sqrt(1.0f - fem::pow2((cs / as))));
         t1 = fem::alog(fem::tan(pi / 4.0f + phi / 2.0f));
         t1 = t1 * as * cs * cs / fem::sqrt(fem::pow2(as) - fem::pow2(cs));
         t2 = fem::sqrt(fem::pow2(as) - 2.0f * fem::pow2(cs) + fem::pow4(
                                                                         cs) / fem::pow2(as));
         t2 = t2 * as;
         sa = 2.0f * pi * (fem::pow2(cs) + t1 + t2);
         return;
      }
      else if (bs == cs) {
         phi = fem::asin(fem::sqrt(1.0f - fem::pow2((cs / as))));
         t1 = cs * as * as / fem::sqrt(fem::pow2(as) - fem::pow2(cs));
         sa = 2.0f * pi * (fem::pow2(cs) + t1 * phi);
         return;
      }
      //C
      sa = 0.0f;
      float hphi = 2.0f * pi / 1000.0f;
      int iphi = fem::int0;
      float sum = fem::float0;
      float htheta = fem::float0;
      int itheta = fem::int0;
      float theta = fem::float0;
      float p = fem::float0;
      float rr = fem::float0;
      FEM_DO_SAFE(iphi, 0, 1000) {
         phi = fem::ffloat(iphi) * hphi;
         sum = 0.0f;
         htheta = pi / 1000.0f;
         FEM_DO_SAFE(itheta, 0, 1000) {
            theta = fem::ffloat(itheta) * htheta;
            p = fem::pow2((bs * cs * fem::sin(theta) * fem::cos(phi)));
            p += fem::pow2((as * cs * fem::sin(theta) * fem::sin(phi)));
            p += fem::pow2((as * bs * fem::cos(theta)));
            p = fem::sin(theta) * fem::sqrt(p);
            if (itheta == 0) {
               rr = 1.0f;
            }
            else if (itheta == 1000) {
               rr = 1.0f;
            }
            else if (fem::mod(itheta, 2) == 1) {
               rr = 4.0f;
            }
            else {
               rr = 2.0f;
            }
            sum += rr * p;
         }
         sum = htheta * sum / 3.0f;
         if (iphi == 0) {
            rr = 1.0f;
         }
         else if (iphi == 1000) {
            rr = 1.0f;
         }
         else if (fem::mod(iphi, 2) == 1) {
            rr = 4.0f;
         }
         else {
            rr = 2.0f;
         }
         sa += rr * sum;
      }
      sa = hphi * sa / 3.0f;
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   carea(
         int const& maxelts,
         arr_cref<fem::integer_star_4> eltype,
         arr_cref<fem::real_star_4, 2> bv,
         fem::integer_star_4 const& nelts,
         arr_ref<fem::real_star_4> saar,
         fem::real_star_4& total)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      saar(dimension(maxelts));
      //C
      float pi = 3.14159265f;
      total = 0.0f;
      //C
      int i = fem::int0;
      float side = fem::float0;
      float sa = fem::float0;
      float r = fem::float0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> v1(fem::fill0);
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      arr_1d<3, fem::real_star_4> v3(fem::fill0);
      float al = fem::float0;
      float r1 = fem::float0;
      float r2 = fem::float0;
      float aa = fem::float0;
      float bb = fem::float0;
      float cc = fem::float0;
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         //C  Cubes
         if (eltype(i) == 8) {
            side = bv(i, 4);
            sa = 6.0f * side * side;
         }
         //C
         //C  Spheres
         if (eltype(i) == 1) {
            r = bv(i, 4);
            sa = 4.0f * pi * r * r;
         }
         //C
         //C  Triangles
         if (eltype(i) == 2) {
            FEM_DO_SAFE(j, 1, 3) {
               v1(j) = bv(i, j);
               v2(j) = bv(i, j + 3);
               v3(j) = bv(i, j + 6);
            }
            trisurf(v1, v2, v3, sa);
         }
         //C
         //C  Disks
         if (eltype(i) == 3) {
            r = bv(i, 7);
            sa = pi * r * r;
         }
         //C
         //C  Cylinders
         if (eltype(i) == 4) {
            r = bv(i, 7);
            al = bv(i, 8);
            sa = 2.0f * pi * al * r;
         }
         //C
         //C  Tori
         if (eltype(i) == 5) {
            r1 = bv(i, 7);
            r2 = bv(i, 8);
            sa = 4.0f * pi * pi * r1 * r2;
         }
         //C
         //C  Lenses
         if (eltype(i) == 6) {
            FEM_STOP("lenssurf not complete");
            //C            do j = 1,3
            //C              c(j) = bv(i,j)
            //C              d(j) = bv(i,j+3)
            //C            end do
            //C            rc = bv(i,7)
            //C            rd = bv(i,8)
            //C            call lenssurf(c,d,rc,rd,sa)
         }
         //C
         //C Ellipsoids
         if (eltype(i) == 7) {
            aa = bv(i, 10);
            bb = bv(i, 11);
            cc = bv(i, 12);
            ellsurf(aa, bb, cc, sa);
         }
         //C
         saar(i) = sa;
         total += sa;
         //C
      }
      //C
   }

   struct jrand_save
   {
      fem::variant_bindings random_bindings;
   };

   //C
   //C*************************************************
   //C
   int
   jrand(
         common& cmn,
         int const& k1,
         int const& k2)
   {
      int return_value = fem::int0;
      FEM_CMN_SVE(jrand);
      common_variant random(cmn.common_random, sve.random_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<fem::integer_star_4> seed;
            random.allocate(), seed;
         }
      }
      /* fem::integer_star_4 const& seed */ random.bind<fem::integer_star_4>();
      float rr = fem::float0;
      //C
      if (k2 < k1) {
         FEM_STOP("args out of order in jrand");
      }
      if (k1 < 0) {
         FEM_STOP("neg. arg in jrand");
      }
      if (k2 == k1) {
         return_value = k1;
      }
      else {
      statement_1:
         rr = ran2(cmn);
         rr = fem::ffloat(k2 - k1 + 1) * rr + fem::ffloat(k1);
         return_value = fem::fint(rr);
         if (return_value < k1 || return_value > k2) {
            goto statement_1;
         }
      }
      return return_value;
   }

   //C
   //C**********************************************************************
   //C
   void
   dovercube(
             common& cmn,
             arr_cref<fem::real_star_4> c,
             float const& side,
             arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      p1(dimension(3));
      //C
      //C        Generate point distributed over a cube
      //C
      int idir = jrand(cmn, 1, 3);
      //C
      int kk = fem::int0;
      int is = fem::int0;
      FEM_DO_SAFE(kk, 1, 3) {
         if (kk != idir) {
            p1(kk) = side * (ran2(cmn) - 0.5f);
         }
         else {
            is = jrand(cmn, 1, 2);
            if (is == 1) {
               p1(kk) = -side / 2.0f;
            }
            else {
               p1(kk) = side / 2.0f;
            }
         }
         p1(kk) += c(kk);
      }
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   doversphere(
               common& cmn,
               arr_cref<fem::real_star_4> c,
               float const& r,
               arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      p1(dimension(3));
      //C
      //C        Generate a point distributed randomly over sphere
      //C
      sphere(cmn, p1, r);
      int i = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         p1(i) += c(i);
      }
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   dovertriangle(
                 common& cmn,
                 arr_cref<fem::real_star_4> p1,
                 arr_cref<fem::real_star_4> p2,
                 arr_cref<fem::real_star_4> p3,
                 arr_ref<fem::real_star_4> tt)
   {
      p1(dimension(3));
      p2(dimension(3));
      p3(dimension(3));
      tt(dimension(3));
      arr_1d<3, fem::real_star_4> p23(fem::fill0);
      arr_1d<3, fem::real_star_4> p21(fem::fill0);
      float top = fem::float0;
      float bot = fem::float0;
      float tat = fem::float0;
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> q(fem::fill0);
      arr_1d<3, fem::real_star_4> av(fem::fill0);
      float a = fem::float0;
      float th = fem::float0;
      float tl = fem::float0;
      float ts = fem::float0;
      float randy = fem::float0;
      arr_1d<3, fem::real_star_4> t1(fem::fill0);
      float aa1 = fem::float0;
      float bb1 = fem::float0;
      arr_1d<3, fem::real_star_4> p31(fem::fill0);
      float cc1 = fem::float0;
      float aa2 = fem::float0;
      float bb2 = fem::float0;
      float cc2 = fem::float0;
      float gamma2 = fem::float0;
      float gamma3 = fem::float0;
      //C
      //C        Generate a point distributed randomly over a triangle
      //C
      //C nm
      vector_difference(p2, p3, p23);
      //C nm
      vector_difference(p2, p1, p21);
      //C nm**2
      dotproduct(p23, p21, top);
      //C nm**2
      dotproduct(p21, p21, bot);
      //C nm**0
      tat = top / bot;
      FEM_DO_SAFE(i, 1, 3) {
         //C nm
         q(i) = tat * p1(i) + (1.0f - tat) * p2(i);
      }
      //C        av = altitude vector normal to (p1...p2) side
      //C nm
      vector_difference(p3, q, av);
      //C        a = altitude normal to (p1...p2) side
      //C nm
      pythag0(av, a);
      //C
      //C nm**0
      th = fem::amax1(1.0f, tat);
      //C nm**0
      tl = fem::amin1(0.0f, tat);
      //C
   statement_1:
      //C
      //C nm**0
      ts = tl + ran2(cmn) * (th - tl);
      randy = ran2(cmn);
      FEM_DO_SAFE(i, 1, 3) {
         //C nm
         tt(i) = ts * p1(i) + (1.0f - ts) * p2(i) + randy * av(i);
      }
      //C
      //C nm
      vector_difference(tt, p1, t1);
      //C nm**2
      dotproduct(t1, p21, aa1);
      //C nm**2
      bb1 = bot;
      //C nm
      vector_difference(p3, p1, p31);
      //C nm**2
      dotproduct(p31, p21, cc1);
      //C nm**2
      dotproduct(t1, p31, aa2);
      //C nm**2
      bb2 = cc1;
      //C nm**2
      dotproduct(p31, p31, cc2);
      //C nm**0
      gamma2 = (aa1 * cc2 - cc1 * aa2) / (bb1 * cc2 - cc1 * bb2);
      if (gamma2 < 0.0f) {
         goto statement_1;
      }
      //C nm**0
      gamma3 = (bb1 * aa2 - aa1 * bb2) / (bb1 * cc2 - cc1 * bb2);
      if (gamma3 < 0.0f) {
         goto statement_1;
      }
      if (gamma2 + gamma3 > 1.0f) {
         goto statement_1;
      }
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   backtransform(
                 arr_cref<fem::real_star_4> c,
                 arr_cref<fem::real_star_4, 2> t,
                 arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p1(dimension(3));
      //C
      //C        Since it is the reverse rotation, use the
      //C        transpose
      //C
      int i = fem::int0;
      arr_1d<3, fem::real_star_4> v(fem::fill0);
      int j = fem::int0;
      FEM_DO_SAFE(i, 1, 3) {
         v(i) = 0.0f;
         FEM_DO_SAFE(j, 1, 3) {
            v(i) += t(j, i) * p1(j);
         }
      }
      //C
      FEM_DO_SAFE(i, 1, 3) {
         p1(i) = v(i) + c(i);
      }
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   doverdisk(
             common& cmn,
             arr_cref<fem::real_star_4> c,
             arr_cref<fem::real_star_4> /* n */,
             fem::real_star_4 const& r,
             arr_cref<fem::real_star_4, 2> t,
             arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p1(dimension(3));
      float xx = fem::float0;
      //C
      //C        generate point distributed randomly over circular disk
      //C
   statement_1:
      //C
      p1(1) = 2.0f * r * (ran2(cmn) - 0.5f);
      p1(2) = 2.0f * r * (ran2(cmn) - 0.5f);
      xx = fem::pow2(p1(1)) + fem::pow2(p1(2));
      xx = fem::sqrt(xx);
      if (xx > r) {
         goto statement_1;
      }
      //C
      p1(3) = 0.0f;
      //C
      backtransform(c, t, p1);
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   dovercylinder(
                 common& cmn,
                 arr_cref<fem::real_star_4> c,
                 arr_cref<fem::real_star_4> /* n */,
                 fem::real_star_4 const& r,
                 fem::real_star_4 const& al,
                 arr_cref<fem::real_star_4, 2> t,
                 arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p1(dimension(3));
      //C
      //C        distribute a point randomly over cylinder
      //C
      p1(3) = al * (ran2(cmn) - 0.5f);
      float theta = 2.0f * 3.14159265f * ran2(cmn);
      p1(1) = r * fem::cos(theta);
      p1(2) = r * fem::sin(theta);
      //C
      backtransform(c, t, p1);
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   dovertorus(
              common& cmn,
              arr_cref<fem::real_star_4> c,
              arr_cref<fem::real_star_4> /* n */,
              fem::real_star_4 const& r1,
              fem::real_star_4 const& r2,
              arr_cref<fem::real_star_4, 2> t,
              arr_ref<fem::real_star_4> p1)
   {
      c(dimension(3));
      t(dimension(3, 3));
      p1(dimension(3));
      float pi = fem::float0;
      float theta = fem::float0;
      arr_1d<3, fem::real_star_4> p2(fem::fill0);
      float rstretch = fem::float0;
      float rmax = fem::float0;
      float probkeep = fem::float0;
      float phi = fem::float0;
      float rtest = fem::float0;
      //C
      //C        distribute a point randomly over torus
      //C
      pi = 3.14159265f;
      //C
   statement_1:
      theta = ran2(cmn) * 2.0f * pi;
      //C
      //C        Generate a point on a circle in the x-z plane
      p2(1) = r2 * fem::cos(theta) + r1;
      p2(2) = 0.0f;
      p2(3) = r2 * fem::sin(theta);
      //C
      //C        Record stretching data
      rstretch = p2(1);
      rmax = r1 + r2;
      probkeep = rstretch / rmax;
      //C
      //C        Rotate about z-axis through random angle phi
      phi = ran2(cmn) * 2.0f * pi;
      //C
      p1(1) = fem::cos(phi) * p2(1) - fem::sin(phi) * p2(2);
      p1(2) = fem::sin(phi) * p2(1) + fem::cos(phi) * p2(2);
      p1(3) = p2(3);
      //C
      rtest = ran2(cmn);
      if (rtest > probkeep) {
         goto statement_1;
      }
      //C
      backtransform(c, t, p1);
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   makepolar(
             arr_cref<fem::real_star_4> p,
             float& theta,
             float& phi)
   {
      p(dimension(3));
      //C
      //C        convert unit vector in p to spherical-polar coordinates
      //C
      if (p(3) <  - 1.0f) {
         theta = 3.14159265f;
         phi = 0.0f;
      }
      else if (p(3) > 1.0f) {
         theta = 0.0f;
         phi = 0.0f;
      }
      else {
         theta = fem::acos(p(3));
         phi = fem::atan2(p(2), p(1));
      }
      //C
   }
   //C
   //C**********************************************************************

   struct doverellipsoid_save
   {
      fem::variant_bindings sell_bindings;
   };

   //C
   //C**********************************************************************
   //C
   void
   doverellipsoid(
                  common& cmn,
                  arr_cref<fem::real_star_4> c,
                  arr_cref<fem::real_star_4> /* n1 */,
                  arr_cref<fem::real_star_4> /* n2 */,
                  fem::real_star_4 const& aa,
                  fem::real_star_4 const& bb,
                  fem::real_star_4 const& cc,
                  arr_cref<fem::real_star_4, 2> t,
                  arr_ref<fem::real_star_4> p1)
   {
      FEM_CMN_SVE(doverellipsoid);
      c(dimension(3));
      t(dimension(3, 3));
      p1(dimension(3));
      common_variant sell(cmn.common_sell, sve.sell_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<int> nell;
            mbr<float> rerr;
            sell.allocate(), nell, rerr;
         }
      }
      int& nell = sell.bind<int>();
      float& rerr = sell.bind<float>();
      float as = fem::float0;
      float bs = fem::float0;
      float cs = fem::float0;
      float stretchmax = fem::float0;
      arr_1d<3, fem::real_star_4> p2(fem::fill0);
      float theta = fem::float0;
      float phi = fem::float0;
      arr_1d<3, fem::real_star_4> am(fem::fill0);
      arr_1d<3, fem::real_star_4> ak(fem::fill0);
      float stretch1 = fem::float0;
      float stretch2 = fem::float0;
      float stretch = fem::float0;
      float probkeep = fem::float0;
      float rtest = fem::float0;
      //C
      //C        distribute a point randomly over ellipsoid
      //C
      //C        Sort the eigenvalues:
      as = aa;
      bs = bb;
      cs = cc;
      //C  as > bs > cs
      sort3(as, bs, cs);
      stretchmax = fem::pow2((as * bs)) + (fem::pow2((fem::pow2(as) -
                                                      fem::pow2(bs)))) / 4.0f;
      stretchmax = fem::sqrt(stretchmax);
      //C
   statement_1:
      //C
      sphere(cmn, p2, 1.0f);
      p1(1) = aa * p2(1);
      p1(2) = bb * p2(2);
      p1(3) = cc * p2(3);
      makepolar(p2, theta, phi);
      am(1) = fem::cos(theta) * fem::cos(phi);
      am(2) = fem::cos(theta) * fem::sin(phi);
      am(3) = -fem::sin(theta);
      ak(1) = -fem::sin(phi);
      ak(2) = fem::cos(phi);
      ak(3) = 0.0f;
      am(1) = aa * am(1);
      am(2) = bb * am(2);
      am(3) = cc * am(3);
      ak(1) = aa * ak(1);
      ak(2) = bb * ak(2);
      ak(3) = cc * ak(3);
      stretch1 = fem::pow2(am(1)) + fem::pow2(am(2)) + fem::pow2(am(3));
      stretch2 = fem::pow2(ak(1)) + fem::pow2(ak(2)) + fem::pow2(ak(3));
      stretch = fem::sqrt(stretch1 * stretch2);
      //C
      if (stretch > stretchmax) {
         if (nell == 0) {
            rerr = stretch / stretchmax;
            nell = 1;
         }
         else if (nell == 1) {
            rerr = fem::amax1(rerr, stretch / stretchmax);
         }
      }
      //C
      probkeep = stretch / stretchmax;
      //C
      rtest = ran2(cmn);
      if (rtest > probkeep) {
         goto statement_1;
      }
      //C
      backtransform(c, t, p1);
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   doverlens(
             arr_cref<fem::real_star_4> /* c */,
             arr_cref<fem::real_star_4> /* d */,
             fem::real_star_4 const& /* rc */,
             fem::real_star_4 const& /* rd */,
             arr_cref<fem::real_star_4> /* p1 */)
   {
      //C
      //C        distribute a point randomly over a lens
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   getsurface(
              common& cmn,
              int const& maxelts,
              arr_cref<fem::integer_star_4> eltype,
              arr_cref<fem::real_star_4, 2> bv,
              fem::integer_star_4 const& nelts,
              arr_cref<fem::real_star_4> saar,
              fem::real_star_4 const& total,
              arr_ref<fem::real_star_4> p1,
              arr_ref<fem::real_star_8> trials,
              arr_cref<fem::real_star_4, 3> rotations,
              int const& loop)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      saar(dimension(maxelts));
      p1(dimension(3));
      trials(dimension(20));
      rotations(dimension(maxelts, 3, 3));
      float zip = fem::float0;
      float sum = fem::float0;
      int i = fem::int0;
      int kdo = fem::int0;
      int j = fem::int0;
      arr_1d<3, fem::real_star_4> c(fem::fill0);
      float side = fem::float0;
      float r = fem::float0;
      arr_1d<3, fem::real_star_4> v1(fem::fill0);
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      arr_1d<3, fem::real_star_4> v3(fem::fill0);
      arr_1d<3, fem::real_star_4> n(fem::fill0);
      int k = fem::int0;
      arr_2d<3, 3, fem::real_star_4> t(fem::fill0);
      float al = fem::float0;
      float r1 = fem::float0;
      float r2 = fem::float0;
      arr_1d<3, fem::real_star_4> d(fem::fill0);
      float rc = fem::float0;
      float rd = fem::float0;
      arr_1d<3, fem::real_star_4> n1(fem::fill0);
      arr_1d<3, fem::real_star_4> n2(fem::fill0);
      float aa = fem::float0;
      float bb = fem::float0;
      float cc = fem::float0;
      bool inside = fem::bool0;
      //C
      //C        Generate a point, p1, distributed uniformly over the surface
      //C
   statement_100:
      //C
      trials(loop) += 1.0e0;
      //C
      zip = ran2(cmn) * total;
      sum = 0.0f;
      //C
      FEM_DO_SAFE(i, 1, nelts) {
         sum += saar(i);
         if (zip < sum) {
            kdo = i;
            goto statement_1;
         }
      }
      kdo = nelts;
   statement_1:
      //C
      i = kdo;
      //C
      //C  Cubes
      if (eltype(kdo) == 8) {
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
         }
         side = bv(i, 4);
         dovercube(cmn, c, side, p1);
      }
      //C
      //C  Spheres
      if (eltype(kdo) == 1) {
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
         }
         r = bv(i, 4);
         doversphere(cmn, c, r, p1);
      }
      //C
      //C  Triangles
      if (eltype(kdo) == 2) {
         FEM_DO_SAFE(j, 1, 3) {
            v1(j) = bv(i, j);
            v2(j) = bv(i, j + 3);
            v3(j) = bv(i, j + 6);
         }
         dovertriangle(cmn, v1, v2, v3, p1);
      }
      //C
      //C  Disks
      if (eltype(kdo) == 3) {
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
            n(j) = bv(i, j + 3);
         }
         r = bv(i, 7);
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 3) {
               t(j, k) = rotations(i, j, k);
            }
         }
         doverdisk(cmn, c, n, r, t, p1);
      }
      //C
      //C  Cylinders
      if (eltype(kdo) == 4) {
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
            n(j) = bv(i, j + 3);
         }
         r = bv(i, 7);
         al = bv(i, 8);
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 3) {
               t(j, k) = rotations(i, j, k);
            }
         }
         dovercylinder(cmn, c, n, r, al, t, p1);
      }
      //C
      //C  Tori
      if (eltype(kdo) == 5) {
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
            n(j) = bv(i, j + 3);
         }
         r1 = bv(i, 7);
         r2 = bv(i, 8);
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 3) {
               t(j, k) = rotations(i, j, k);
            }
         }
         dovertorus(cmn, c, n, r1, r2, t, p1);
      }
      //C
      //C  Lenses
      if (eltype(kdo) == 6) {
         //C            do j = 1,3
         //C              c(j) = bv(i,j)
         //C              d(j) = bv(i,j+3)
         //C            end do
         //C            rc = bv(i,7)
         //C            rd = bv(i,8)
         doverlens(c, d, rc, rd, p1);
         FEM_STOP("doverlens not ready");
      }
      //C
      //C Ellipsoids
      if (eltype(kdo) == 7) {
         //C
         FEM_DO_SAFE(j, 1, 3) {
            c(j) = bv(i, j);
            n1(j) = bv(i, j + 3);
            n2(j) = bv(i, j + 6);
         }
         aa = bv(i, 10);
         bb = bv(i, 11);
         cc = bv(i, 12);
         FEM_DO_SAFE(j, 1, 3) {
            FEM_DO_SAFE(k, 1, 3) {
               t(j, k) = rotations(i, j, k);
            }
         }
         doverellipsoid(cmn, c, n1, n2, aa, bb, cc, t, p1);
         //C
      }
      //C
      //C        p1 is now a point distributed over the surface
      //C        of the elements.  But we must remove it if it
      //C        is inside any other element.
      //C
      FEM_DO_SAFE(i, 1, nelts) {
         //C
         if (i != kdo) {
            //C
            //C  Cubes
            if (eltype(i) == 8) {
               FEM_DO_SAFE(j, 1, 3) {
                  c(j) = bv(i, j);
               }
               side = bv(i, 4);
               insidecube(c, side, p1, inside);
               if (inside) {
                  goto statement_100;
               }
            }
            //C
            //C  Spheres
            if (eltype(i) == 1) {
               FEM_DO_SAFE(j, 1, 3) {
                  c(j) = bv(i, j);
               }
               r = bv(i, 4);
               insidesphere(c, r, p1, inside);
               if (inside) {
                  goto statement_100;
               }
            }
            //C
            //C  Tori
            if (eltype(i) == 5) {
               FEM_DO_SAFE(j, 1, 3) {
                  c(j) = bv(i, j);
                  n(j) = bv(i, j + 3);
               }
               r1 = bv(i, 7);
               r2 = bv(i, 8);
               FEM_DO_SAFE(j, 1, 3) {
                  FEM_DO_SAFE(k, 1, 3) {
                     t(j, k) = rotations(i, j, k);
                  }
               }
               insidetorus(c, n, r1, r2, t, p1, inside);
               if (inside) {
                  goto statement_100;
               }
            }
            //C
            //C  Lenses
            if (eltype(i) == 6) {
               FEM_DO_SAFE(j, 1, 3) {
                  c(j) = bv(i, j);
                  d(j) = bv(i, j + 3);
               }
               rc = bv(i, 7);
               rd = bv(i, 8);
               insidelens(c, d, rc, rd, p1, inside);
               if (inside) {
                  goto statement_100;
               }
            }
            //C
            //C Ellipsoids
            if (eltype(i) == 7) {
               FEM_DO_SAFE(j, 1, 3) {
                  c(j) = bv(i, j);
                  n1(j) = bv(i, j + 3);
                  n2(j) = bv(i, j + 6);
               }
               aa = bv(i, 10);
               bb = bv(i, 11);
               cc = bv(i, 12);
               FEM_DO_SAFE(j, 1, 3) {
                  FEM_DO_SAFE(k, 1, 3) {
                     t(j, k) = rotations(i, j, k);
                  }
               }
               insideellipsoid(c, n1, n2, aa, bb, cc, t, p1, inside);
               if (inside) {
                  goto statement_100;
               }
            }
            //C
         }
      }
      //C
   }

   //C
   //C**********************************************************************
   //C
   void
   captain(
           common& cmn,
           int const& maxelts,
           arr_cref<fem::integer_star_4> eltype,
           arr_cref<fem::real_star_4, 2> bv,
           fem::integer_star_4 const& nelts,
           fem::integer_star_4 const& m1do,
           arr_cref<fem::real_star_4, 3> rotations,
           bool& kirk_done,
           fem::integer_star_4 const& /* nout */,
           arr_ref<fem::real_star_4> saar,
           fem::real_star_4& kirk,
           fem::real_star_4& delta_kirk,
           fem::real_star_4& surface,
           fem::real_star_4& delta_surface,
           fem::real_star_4& rg2surf,
           fem::real_star_4& delta_rg2surf,
           fem::integer_star_4& ms)
   {
      eltype(dimension(maxelts));
      bv(dimension(maxelts, 12));
      rotations(dimension(maxelts, 3, 3));
      saar(dimension(maxelts));
      common_write write(cmn);
      // COMMON shush
      bool& silent = cmn.silent;
      //
      //C
      kirk_done = false;
      //C
      fem::real_star_4 total = fem::zero<fem::real_star_4>();
      carea(maxelts, eltype, bv, nelts, saar, total);
      int i = fem::int0;
      arr_1d<20, fem::real_star_8> sum1(fem::fill0);
      arr_1d<20, fem::real_star_8> sum2(fem::fill0);
      arr_1d<20, fem::real_star_8> trials(fem::fill0);
      arr_1d<20, fem::real_star_8> successes(fem::fill0);
      arr_1d<20, fem::real_star_8> rg2sum(fem::fill0);
      arr_1d<20, fem::real_star_8> rg2norm(fem::fill0);
      FEM_DO_SAFE(i, 1, 20) {
         sum1(i) = 0.0e0;
         sum2(i) = 0.0e0;
         trials(i) = 0.0e0;
         successes(i) = 0.0e0;
         rg2sum(i) = 0.0e0;
         rg2norm(i) = 0.0e0;
      }
      //C
      //C
      if (!silent) {
         write(6, "('SURFACE CALCULATION')");
         zeno_us_hydrodyn->editor->append( "ZENO surface calculation start\n" );
      }
      if (!silent) {
         write(6, "(108('='))");
      }
      //C
      int mout = 0;
      //C
      int loop = fem::int0;
      arr_1d<3, fem::real_star_4> v1(fem::fill0);
      arr_1d<3, fem::real_star_4> v2(fem::fill0);
      float rr2 = fem::float0;
      int j = fem::int0;
      float rr = fem::float0;
      int need = fem::int0;
      FEM_DO_SAFE(i, 1, m1do) {
         loop = fem::mod(i, 20) + 1;
         getsurface(cmn, maxelts, eltype, bv, nelts, saar, total, v1,
                    trials, rotations, loop);
         getsurface(cmn, maxelts, eltype, bv, nelts, saar, total, v2,
                    trials, rotations, loop);
         successes(loop) += 2.0e0;
         rr2 = 0.0f;
         FEM_DO_SAFE(j, 1, 3) {
            rr2 += fem::pow2((v1(j) - v2(j)));
         }
         rr = fem::sqrt(rr2);
         sum2(loop) += 1.0e0 / fem::dble(rr);
         sum1(loop) += 1.0e0;
         rg2sum(loop) += fem::dble(rr2);
         rg2norm(loop) += 2.0e0;
         //C
         need = fem::nint(108.0f * fem::ffloat(i) / fem::ffloat(m1do));
         while (mout < need) {
            if (!silent) {
               std::cout << "X" << std::flush;
               if ( zeno_progress )
               {
                  zeno_progress->setProgress( zeno_progress->progress() + 1 );
                  qApp->processEvents();
                  if ( zeno_us_hydrodyn->stopFlag )
                  {
                     // somehow abort
                     return;
                  }
               }
            }
            mout++;
         }
         //C
      }
      if (!silent) {
         write(6, "(' ')");
      }
      //C
      int k = fem::int0;
      arr_1d<20, fem::real_star_4> rad(fem::fill0);
      FEM_DO_SAFE(k, 1, 20) {
         rad(k) = total * fem::sngl(successes(k) / trials(k));
      }
      mean20(rad, surface, delta_surface);
      //C
      FEM_DO_SAFE(i, 1, 20) {
         rad(i) = fem::sngl(rg2sum(i) / rg2norm(i));
      }
      mean20(rad, rg2surf, delta_rg2surf);
      //C
      FEM_DO_SAFE(k, 1, 20) {
         rad(k) = fem::sngl(sum2(k) / sum1(k));
      }
      float rho = fem::float0;
      float delta_rho = fem::float0;
      mean20(rad, rho, delta_rho);
      kirk = 1.0f / rho;
      delta_kirk = delta_rho / (fem::pow2(rho));
      //C
      kirk_done = true;
      ms = m1do;
      //C
   }

   //C
   //C*************************************************
   //C
   void
   pushon(
          str_cref byte,
          str_ref rs)
   {
      //C
      //C        push a single byte onto the left side of a string
      //C
      fem::str<15> copy = rs(1, 14);
      copy(1, 1) = byte;
      rs = copy;
      //C
   }

   //C
   //C**********************************************************************
   //C
   //C        FIXED TO HERE
   //C
   void
   lenssurf(
            arr_cref<fem::real_star_4> /* c */,
            arr_cref<fem::real_star_4> /* d */,
            float const& /* rc */,
            float const& /* rd */,
            float const& /* sa */)
   {
      //C
      //C        Compute surface area of lens
      //C
   }

   struct program_zeno_save
   {
      fem::variant_bindings cubit_bindings;
      fem::variant_bindings sell_bindings;
   };

   void
   program_zeno(
                int argc,
                char const* argv[])
   {
      common cmn(argc, argv);
      FEM_CMN_SVE(program_zeno);
      common_write write(cmn);
      bool& silent = cmn.silent;
      //
      common_variant cubit(cmn.common_cubit, sve.cubit_bindings);
      common_variant sell(cmn.common_sell, sve.sell_bindings);
      if (is_called_first_time) {
         using fem::mbr; // member of variant common or equivalence
         {
            mbr<fem::integer_star_4> ncube;
            mbr<fem::real_star_4> ferr;
            cubit.allocate(), ncube, ferr;
         }
         {
            mbr<fem::integer_star_4> nell;
            mbr<fem::real_star_4> rerr;
            sell.allocate(), nell, rerr;
         }
      }
      fem::integer_star_4& ncube = cubit.bind<fem::integer_star_4>();
      /* fem::real_star_4 const& ferr */ cubit.bind<fem::real_star_4>();
      fem::integer_star_4& nell = sell.bind<fem::integer_star_4>();
      /* fem::real_star_4 const& rerr */ sell.bind<fem::real_star_4>();
      bool tell_all = fem::bool0;
      arr_1d<8, fem::str<9> > map(fem::fill0);
      arr_1d<8, fem::integer_star_4> mx(fem::fill0);
      bool launch_done = fem::bool0;
      bool zeno_done = fem::bool0;
      bool kirk_done = fem::bool0;
      bool rg_done = fem::bool0;
      bool bailout = fem::bool0;
      fem::str<25> id = fem::char0;
      fem::integer_star_4 nin = fem::zero<fem::integer_star_4>();
      fem::integer_star_4 nout = fem::zero<fem::integer_star_4>();
      arr_1d<3, fem::integer_star_4> m1(fem::fill0);
      fem::str<3> actions = fem::char0;
      const int maxelts = 60000;
      fem::integer_star_4 nelts = fem::zero<fem::integer_star_4>();
      arr<fem::integer_star_4> eltype(dimension(maxelts), fem::fill0);
      arr<fem::real_star_4, 2> bv(dimension(maxelts, 12), fem::fill0);
      fem::real_star_4 tol = fem::zero<fem::real_star_4>();
      arr<fem::real_star_4, 3> rotations(dimension(maxelts, 3, 3), fem::fill0);
      bool tol_given = fem::bool0;
      fem::str<2> unitcode = fem::char0;
      bool bt = fem::bool0;
      bool bm = fem::bool0;
      bool bw = fem::bool0;
      bool bc = fem::bool0;
      arr_1d<2, fem::real_star_8> temp(fem::fill0);
      fem::str<6> tunit = fem::char0;
      arr_1d<2, fem::real_star_8> mass(fem::fill0);
      fem::str<6> munit = fem::char0;
      arr_1d<2, fem::real_star_8> visc(fem::fill0);
      fem::str<6> vunit = fem::char0;
      int i = fem::int0;
      float rlaunch = fem::float0;
      int m1do = fem::int0;
      fem::real_star_4 rg2int = fem::zero<fem::real_star_4>();
      fem::real_star_4 delta_rg2int = fem::zero<fem::real_star_4>();
      fem::real_star_4 volume = fem::zero<fem::real_star_4>();
      fem::real_star_4 delta_volume = fem::zero<fem::real_star_4>();
      fem::integer_star_4 mi = fem::zero<fem::integer_star_4>();
      fem::real_star_4 cap = fem::zero<fem::real_star_4>();
      arr_2d<3, 3, fem::real_star_4> alpha_bongo(fem::fill0);
      fem::real_star_4 delta_cap = fem::zero<fem::real_star_4>();
      arr_2d<3, 3, fem::real_star_4> delta_bongo(fem::fill0);
      fem::integer_star_4 mz = fem::zero<fem::integer_star_4>();
      arr<fem::real_star_4> saar(dimension(maxelts), fem::fill0);
      fem::real_star_4 kirk = fem::zero<fem::real_star_4>();
      fem::real_star_4 delta_kirk = fem::zero<fem::real_star_4>();
      fem::real_star_4 surface = fem::zero<fem::real_star_4>();
      fem::real_star_4 delta_surface = fem::zero<fem::real_star_4>();
      fem::real_star_4 rg2surf = fem::zero<fem::real_star_4>();
      fem::real_star_4 delta_rg2surf = fem::zero<fem::real_star_4>();
      fem::integer_star_4 ms = fem::zero<fem::integer_star_4>();
      //C
      //C        Author:  Marc L. Mansfield
      //C
      //C  Perform three different computations on bodies
      //C
      //C  Zeno computation
      //C        Computes the capacitance and polarizability of
      //C        bodies by the "Zeno" algorithm, and then uses
      //C        these to estimate transport properties.
      //C
      //C  Interior computation
      //C        Computes the volume and the radius of gyration of the
      //C        interior -- by a Monte Carlo integration -- by generating
      //C        a set of points through the interior.
      //C
      //C  Surface computation
      //C        Computes the surface area, the radius of gyration
      //C        of the surface, and the Kirkwood-hydrodynamic radius
      //C        (i.e., the hydrodynamic radius as evaluated by the
      //C        Kirkwood double sum <1/Rij> approximation) --  by Monte
      //C        Carlo integration --  by generating
      //C        surface points.
      //C
      //C========================================================
      //C
      //C        References:
      //C
      //C        Mansfield, Douglas, Garbozci
      //C        "Intrinsic viscosity and the electrical
      //C        polarizability of arbitrarily shaped objects,"
      //C        Physical Review E, 64, 061401 (2001).
      //C
      //C        Kang, Mansfield, Douglas
      //C        "Numerical path integration technique for the calculation
      //C        of transport properties of proteins,"
      //C        Physical Review E, 69, 031918 (2004).
      //C
      //C========================================================
      //C
      //C        This is the command line:
      //C
      //C        ./zeno <identifier> <action_code1> <action_code2> <action_code3>
      //C
      //C        Before issuing this command, you need to set
      //C        up the body file, which specifies the body.
      //C        The body file must be created with the name
      //C        <identifier>.bod
      //C
      //C        <identifier> identifies the model.  Input (the
      //C        specification for the model) is read in on the
      //C        file <identifier>.bod and a report of the
      //C        calculation is sent to <identifier>.zno
      //C
      //C        You specify between 0 and 3 action_codes, i.e.,
      //C        <action_code1>, <action_code2> and <action_code3>
      //C        are optional, but if you don't specify any, the
      //C        program won't do anything.
      //C
      //C========================================================
      //C
      //C        Summary of the action codes:
      //C
      //C        Each action code is a character string, with three
      //C        parts, a one-byte prefix, a set of digits in the
      //C        middle, and optionally, a suffix.
      //C        The prefix is one of three bytes:
      //C
      //C        z  Do the Zeno computation on the body
      //C        i  Do the interior integation on the body
      //C        s  Do the surface computation on the body
      //C
      //C        Allowed suffixes are any of the three bytes:
      //C
      //C        t = thousand
      //C        m = million
      //C        b = billion
      //C
      //C        but these suffixes are optional.
      //C
      //C        A few examples will be used to explain the action
      //C        codes:
      //C
      //C        z100t   requests that the zeno calculation be performed
      //C                with 100 thousand random walkers
      //C
      //C        i1b     requests that the interior computation
      //C                be performed with one billion points
      //C                in the interior of the body
      //C
      //C        s5000000
      //C                requests that the surface computation
      //C                be performed with five
      //C                million pairs of points distributed over the
      //C                surface
      //C
      //C        See below for limitations on both the surface and the
      //C        interior calculations.
      //C
      //C        You can specify as few as zero (which will result in
      //C        no action being taken by the program), or as many as
      //C        three action codes with each call.
      //C
      //C========================================================
      //C
      //C        The body is taken as the union of body elements.
      //C        Each body element requires a set of values that specify
      //C        size, shape, position, and spatial orientation.
      //C
      //C        All the following body elements are now implemented.
      //C        Also shown are variables for the values needed to
      //C        specify the shape.
      //C
      //C        SPHERE  cx cy cz   r
      //C        TRIANGLE  v1x v1y v1z   v2x v2y v2z   v3x v3y v3z
      //C        DISK  cx cy cz   nx ny nz   r
      //C        CYLINDER  cx cy cz   nx ny nz   r   l
      //C        TORUS   cx cy cz   nx ny nz   r1   r2
      //C        LENS   cx cy cz   dx dy dz   rc   rd
      //C        ELLIPSOID  cx cy cz   n1x n1y n1z   n2x n2y n2z   aa bb cc
      //C        CUBE  cx cy cz   s
      //C
      //C========================================================
      //C
      //C        The complete body is specified by supplying a "body file."
      //C        This is a file with the name:
      //C
      //C                <identifier>.bod
      //C
      //C        where <identifier> represents a character string that
      //C        identifies the model.
      //C
      //C        The data in the body file consists of a series of strings.
      //C        The strings are delimited by spaces or by carriage returns.
      //C        A single line is assumed to be 80 characters or less, so
      //C        do not put in more than 80 characters between carriage
      //C        returns.
      //C
      //C        Each string is either a "predicate" or a "modifier."
      //C        The modifiers belong to, or modify, the predicate, and
      //C        each predicate along with its modifiers constitutes
      //C        a single "command."
      //C
      //C        The modifiers to each predicate come in a specific order
      //C        following that predicate, and each predicate requires
      //C        a specific number of modifiers.
      //C
      //C        There are no punctuation marks flagging the end of
      //C        one command or the beginning of another.  The command
      //C        is defined as a valid predicate followed by the correct
      //C        number of modifiers.  However, for ease in reading,
      //C        the user will probably want to design the body file
      //C        with carriage returns between commands.
      //C
      //C        Usually (but not always) the modifiers are strings
      //C        representing numbers.  Such strings are called "numeric
      //C        strings," or simply "numbers."  A valid numeric
      //C        string is any character string that can be interpreted
      //C        by the fortran internal-read, free-format command:
      //C
      //C        read(string,*) value
      //C
      //C        To process the file, the program looks at the first
      //C        string on the file.  This string must a valid predicate.
      //C        If it is not, then the program aborts.  Then, the program
      //C        takes the next N strings, where N is the number of
      //C        modifiers required for this particular predicate.  The
      //C        program aborts if it has trouble interpreting any of
      //C        the modifiers.  Assuming these N strings are interpreted
      //C        successfully, the program repeats, reading a predicate
      //C        and its modifiers, until it encounters the end of the
      //C        file.
      //C
      //C        The following paragraphs summarize each command-type,
      //C        giving the valid predicate or predicates, and the valid
      //C        modifiers.  And they also briefly explain the action
      //C        of the command.  Full definitions of each modifer are
      //C        given later.  Most of the commands
      //C        add a body-element to the growing body.  Most of the
      //C        commands have several synonymous predicates.  E.g.,
      //C        the four strings "SPHERE", "sphere", "S", "s" are
      //C        all valid predicates for the "ADD A SPHERE" command.
      //C        The order of the modifiers given in the following
      //C        paragraphs must be followed in the body file.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A SPHERE
      //C
      //C        Valid predicates:
      //C        SPHERE, sphere, S, s    (Internal code 1)
      //C
      //C        Modifiers:  Four numbers.
      //C        cx, cy, cz, r
      //C
      //C        Action:  Adds a sphere to the list of body elements.
      //C
      //C        See below for definitions of cx, cy, cz, r.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A TRIANGLE
      //C
      //C        Valid predicates:
      //C        TRIANGLE, triangle, T, t  (Internal code 2)
      //C
      //C        Modifiers:  Nine numbers.
      //C        v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z
      //C
      //C        Action:  Adds a triangle to the list of body elements.
      //C
      //C        See below for definitions of v1x,v1y,v1z,v2x,
      //C                v2y,v2z,v3x,v3y,v3z.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A CIRCULAR DISK
      //C
      //C        Valid predicates:
      //C        DISK, disk, D, d   (Internal code 3)
      //C
      //C        Modifiers:  Seven numbers.
      //C        cx,cy,cz,nx,ny,nz,r
      //C
      //C        Action:  Adds a circular disk to the list of
      //C        body elements.
      //C
      //C        See below for definitions of cx,cy,cz,nx,ny,nz,r.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD AN OPEN CYLINDER
      //C
      //C        Valid predicates:
      //C        CYLINDER, cylinder  (Internal code 4)
      //C
      //C        Modifiers:  Eight numbers.
      //C        cx,cy,cz,nx,ny,nz,r,l
      //C
      //C        Action:  Adds an open cylinder to the list of
      //C        body elements.
      //C
      //C        See below for definitions of cx,cy,cz,nx,ny,nz,r,l.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A TORUS
      //C
      //C        Valid predicates:
      //C        TORUS, torus, TO, to     (Internal code 5)
      //C
      //C        Modifiers:  Eight numbers.
      //C        cx,cy,cz,nx,ny,nz,r1,r2
      //C
      //C        Action:  Adds a torus to the list of body elements.
      //C
      //C        See below for definitions of cx,cy,cz,nx,ny,nz,r1,r2.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A LENS
      //C
      //C        Valid predicates:
      //C        LENS, lens         (Internal code 6)
      //C
      //C        Modifiers:  Eight numbers.
      //C        cx,cy,cz,dx,dy,dz,rc,rd
      //C
      //C        Action:  Adds a lens to the list of body elements.
      //C
      //C        See below for definitions of cx,cy,cz,dx,dy,dz,rc,rd.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD AN ELLIPSOID
      //C
      //C        Valid predicates:
      //C        ELLIPSOID, ellipsoid, E, e  (Internal code 7)
      //C
      //C        Modifiers:  Twelve numbers.
      //C        cx,cy,cz,n1x,n1y,n1z,n2x,n2y,n2z,aa,bb,cc
      //C
      //C        Action:  Adds an ellipsoid to the list of body elements.
      //C
      //C        See below for definitions of cx,cy,cz,n1x,n1y,n1z,
      //C                        n2x,n2y,n2z,aa,bb,cc.
      //C
      //C------------------------------------------------------
      //C
      //C        ADD A CUBE
      //C
      //C        Valid predicates:
      //C        CUBE, cube                (Internal code 8)
      //C
      //C        Modifiers:  Four numbers.
      //C        cx,cy,cz,s
      //C
      //C        Action:  Adds a cube to the list of body elements.
      //C
      //C        See below for definitions of cx,cy,cz,s.
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY VALUE OF SKIN THICKNESS
      //C
      //C        Valid predicates:
      //C        ST, st         (Internal code 9)
      //C
      //C        Modifiers:  One number, the value of the
      //C        skin thickness.
      //C
      //C        Action:  Specifies the value of the skin thickness,
      //C        epsilon.
      //C
      //C        Note:  The skin thickness is only used in the zeno
      //C        calculation.  You can use this command to assign a
      //C        value for the skin thickness.  However, the command
      //C        is optional, if not given then as default, the
      //C        program uses the value of 1.0e-6 times the launch
      //C        radius.
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY LENGTH UNITS
      //C
      //C        Valid predicates:
      //C        UNITS, units    (Internal code 10)
      //C
      //C        Modifiers:  This command takes a single alphabetic
      //C        string as a modifier.  It does not take number
      //C        modifiers.  Only one of the following
      //C        five strings will be accepted as the modifier.
      //C
      //C       Modifier      Internal
      //C         string         code        Meaning
      //C       -------       --------      -------
      //C           m             11         meters
      //C           cm            12         centimeters
      //C           nm            13         nanometers
      //C           A             14         Angstroms
      //C           L             15         generic or unspecified
      //C
      //C        Action:  Specifies the length units in which all
      //C        values are expressed.
      //C
      //C        Note:  This command is optional.  If it is not found
      //C        in the body file, the value "L" will be assigned for
      //C        units, indicating generic or unspecified length units.
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY TEMPERATURE
      //C
      //C        This command is optional.  It needs to be present if
      //C        you want the program to compute the diffusion coefficient
      //C        from the Stokes-Einstein formula.
      //C
      //C        Valid predicates:
      //C        TEMP, temp              (Internal code 16)
      //C
      //C        Modifiers:  The predicate must be followed by two modifiers,
      //C        the first is a number, the second gives the temperature units.
      //C        Valid temperature unit codes:
      //C
      //C        C       Celcius (Internal code 17)
      //C        K       Kelvin  (Internal code 18)
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY MASS
      //C
      //C        This command is optional.  It needs to be present if you want
      //C        the program to compute the intrinsic viscosity in conventional
      //C        units.
      //C
      //C        Valid predicates:
      //C        MASS, mass              (Internal code 19)
      //C
      //C        Modifiers:  The predicate must be followed by two modifiers,
      //C        The first is a number, and the second gives the mass units.
      //C        Valid mass unit codes:
      //C
      //C        Da              Daltons  (Internal code 20)
      //C        kDa             kilodaltons  (internal code 21)
      //C        g               grams  (Internal code 22)
      //C        kg              kilograms  (internal code 23)
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY SOLVENT VISCOSITY
      //C
      //C        This command is optional.  Either it or the "SPECIFY SOLVENT = WATER"
      //C        command needs to be present if you want program to calculate the
      //C        diffusion coefficient by the Stokes-Einstein formula.
      //C
      //C        Valid predicates:
      //C        VISCOSITY, viscosity    (internal code 24)
      //C
      //C        Modifiers:  The predicate must be followed by two modifiers.
      //C        The first is a number, the second gives the viscosity units.
      //C        Valid viscosity units codes:
      //C
      //C        p       poise           (Internal code 25)
      //C        cp      centipoise      (Internal code 26)
      //C
      //C------------------------------------------------------
      //C
      //C        SPECIFY SOLVENT = WATER
      //C
      //C        This command is optional.  If you want the program to calculate
      //C        the diffusion coefficient by the Stokes-Einstein formula, it will
      //C        need the solvent viscosity.  You can enter the viscosity using the
      //C        "SPECIFY SOLVENT VISCOSITY" command, or you can issue this command,
      //C        and it will use the standard formula for the viscosity of pure
      //C        water as a function of temperature.  But for this to work, you
      //C        will also have to specify the temperature.
      //C
      //C        Valid predicates:
      //C        SOLVENT, solvent        (Internal code 27)
      //C
      //C        Valid modifiers:
      //C        WATER, water            (Internal code 28)
      //C
      //C        At some future date, it may be possible to include other solvents
      //C        in this command.
      //C
      //C------------------------------------------------------
      //C
      //C        The order of the commands is not important:  Body
      //C        elements may be added in any order, while either of
      //C        the two commands that do not add body elements, i.e.,
      //C        specifying skin thickness, or specifying units, can
      //C        be interspersed in any order among the body-element
      //C        commands.
      //C
      //C        The predicates ST or st do not specify a body element,
      //C        but rather the skin thickness of the model.
      //C        The skin thickness needs to be assigned if you
      //C        call for the zeno calculation.  If the skin thickness
      //C        is not set, then the zeno calculation will be skipped,
      //C        even if you request it.
      //C
      //C        The body file can also contain comments.  These
      //C        are any lines in the file that begin with an asterisk, *.
      //C
      //C        The end of the body file is its physical end.
      //C        The program continues processing the body file until
      //C        it hits the end.
      //C
      //C        Example of a body file (the | symbol shows
      //C        the left edge of each line):
      //C
      //C============================================
      //C  |*  This body consists of 5 spheres
      //C  |*  Blank lines are OK:
      //C  |
      //C  |*  This line inserts a sphere of radius 1 at the origin
      //C  |SPHERE 0 0 0 1
      //C  |
      //C  |*  The next line inserts a sphere of radius 2 tangent
      //C  |*  to the first sphere with center on the x axis
      //C  |S 3 0 0 2
      //C  |
      //C  |*  Carriage returns are permissible during the specification
      //C  |*  of any one element:
      //C  |sphere -3 0 0
      //C  |            2
      //C  |
      //C  |*  You can also run different elements together on
      //C  |*  the same line
      //C  |
      //C  |     S 1 1 1 1   s -1 -1 -1 1
      //C  |* This gives the skin thickness
      //C  |  st   0.00001
      //C  |
      //C  !* This command establishes nanometers as the length unit
      //C  |  unit nm
      //C==========================================
      //C
      //C        Following is a detailed specification of each type of body element.
      //C
      //C=====================================================================
      //C
      //C        SPHERE  cx cy cz  r
      //C
      //C        real*4 c(3),r
      //C
      //C        A sphere centered at c=(cx,cy,cz) with radius r.
      //C
      //C=====================================================================
      //C
      //C        TRIANGLE  v1x v1y v1z   v2x v2y v2z   v3x v3y v3z
      //C
      //C        real*4 v1(3),v2(3),v3(3)
      //C
      //C        v1, v2, v3 are the three vertices of the triangle.
      //C
      //C=====================================================================
      //C
      //C        DISK  cx cy cz   nx ny nz  r
      //C
      //C        real*4 c(3),n(3),r
      //C
      //C        A disk centered at c=(cx,cy,cz).
      //C
      //C        The vector n=(nx,ny,nz) is the "unit normal," i.e., it points
      //C        in the direction normal to the plane of the disk.  The
      //C        vector n may be unnormalized on entry; it is automatically
      //C        normalized upon input.
      //C
      //C        The radius of the disk is r.
      //C
      //C====================================================================
      //C
      //C        CYLINDER  cx cy cz   nx ny nz  r l
      //C
      //C        real*4 c(3),n(3),r,l
      //C
      //C        The cylinder is open; i.e., a tin can with both lids cut off.
      //C
      //C        To do a closed cylinder, combine this with two disks.
      //C
      //C        c is the geometric center of the cylinder.  n is a vector
      //C        pointing in the direction of the cylinder axis.  n
      //C        may be supplied unnormalized -- it will
      //C        be normalized automatically.
      //C        r is the radius, l is the total length; so it extends from
      //C        -l/2 to l/2.
      //C
      //C=====================================================================
      //C
      //C        TORUS  cx cy cz   nx ny nz  r1 r2
      //C
      //C        real*4 c(3),n(3),r1,r2
      //C
      //C        A torus centered at c.   n is a vector pointing in the
      //C        direction of the symmetry axis.  n may be supplied
      //C        unnormalized -- it will be
      //C        automatically normalized upon input.
      //C
      //C        r1 and r2 define the dimensions of the torus:
      //C
      //C                  x                         x
      //C             x       x                 x       x
      //C            x         x               x         x   ___
      //C            x         x               x         x    |
      //C             x       x                 x       x     |  r2
      //C                 x                         x        ---
      //C
      //C                 |-----------2 * r1 -------|
      //C
      //C======================================================================
      //C
      //C        LENS  cx cy cz   dx dy dz  rc rd
      //C
      //C        real*4 c(3),d(3),rc,rd
      //C
      //C        Defined as the intersection of two spheres, centered
      //C        respectively at c and d, having radii respectively rc, rd
      //C
      //C=====================================================================
      //C
      //C        ELLIPSOID  cx cy cz   n1x n1y n1z   n2x n2y n2z  aa bb cc
      //C
      //C        real*4 c(3),n1(3),n2(3),aa,bb,cc
      //C
      //C        Center of ellipsoid is at c;
      //C        n1 is a vector parallel to the aa axis;
      //C        n2 is a vector parallel to the bb axis;
      //C        n1 and n2 may be entered as unnormalized; they are normalized
      //C        automatically  upon input.  It is assumed that n1 and n2 are
      //C        orthogonal; unpredictable results will occur if not.
      //C
      //C        The third axis, n3, is calculated by taking
      //C        the cross product of n1 and n2.  Therefore, it is not
      //C        entered.  aa is the length of the
      //C        semi-axis along n1, bb along n2, and cc along n3.
      //C
      //C        So for example, if (cx,cy,cz) were the origin,
      //C        if n1, n2, and n3 were respectively the x, y, and z axes,
      //C        then aa, bb, cc are defined such that this
      //C
      //C        (x/aa)**2 + (y/bb)**2 + (z/cc)**2 = 1
      //C
      //C        is the equation of the ellipsoid.
      //C
      //C====================================================================
      //C
      //C        CUBE  cx cy cz  s
      //C
      //C        real*4 c(3),s
      //C
      //C        Lower corner at (cx,cy,cz), s is the side
      //C
      //C        Currently we only do cubes that are oriented parallel
      //C        to the cartesian coordinates.
      //C
      //C====================================================================
      //C
      //C        ZENO CALCULATIONS
      //C
      //C        You call for a zeno calculation on the body whenever you use
      //C        the z prefix on any of the action_codes.
      //C
      //C        The electrostatic capacitance and the electrostatic
      //C        polarizability are calculated according to the numerical
      //C        path integration approach described in:
      //C
      //C        Mansfield, Douglas, Garbozci
      //C        "Intrinsic viscosity and the electrical
      //C        polarizability of arbitrarily shaped objects,"
      //C        Physical Review E, 64, 061401 (2001).
      //C
      //C        Kang, Mansfield, Douglas
      //C        "Numerical path integration technique for the calculation
      //C        of transport properties of proteins,"
      //C        Physical Review E, 69, 031918 (2004).
      //C
      //C====================================================================
      //C
      //C        INTERIOR CALCULATIONS
      //C
      //C        You call for an interior calculation if you use the
      //C        i prefix on any of the action_codes.
      //C
      //C        The interior calculation is done by generating a set
      //C        of points on the interior of the body, and doing a numerical
      //C        integration over this set.  It is therefore limited to bodies
      //C        that are unions of space filling elements:  spheres, cubes,
      //C        tori, lenses, and ellipsoids.  If you call for the interior
      //C        calculation given bodies formed from other elements, the program
      //C        omits the calculation.
      //C        This calculation determines the volume of the body, and the
      //C        radius of gyration of the interior.
      //C
      //C        Of course, you can create a space-filling body using a union
      //C        of triangles distributed over the surface, but since it is
      //C        difficult to determine a priori that the body so built is
      //C        space-filling, we just won't try it here.  Bottom line:
      //C        Only bodies built up from spheres, tori, cubes, lenses,
      //C        and ellipsoids will be treated with the interior
      //C        calculation.
      //C
      //C        The set of points is obtained by generating random points inside the
      //C        launch sphere, and retaining only those that are also inside the
      //C        body.
      //C
      //C======================================================================
      //C
      //C        SURFACE CALCULATION
      //C
      //C        You call for a surface calculation by supplying the prefix s
      //C        on one of the action_codes.
      //C
      //C        The surface calculation works by generating a set of points
      //C        distributed randomly over the surface.  It provides an
      //C        estimate of the surface area, the radius of gyration of the
      //C        surface, and an estimate of the hydrodynamic radius using
      //C        the Kirkwood double sum formula.
      //C
      //C        The capacitance-hydrodynamic radius analogy results from
      //C        pre-averaging the Oseen tensor over orientations.  The
      //C        well-known formula of Kirkwood for the diffusion constant
      //C        approximates the hydrodynamic radius as the harmonic
      //C        mean distance between arbitrary pairs of points on the
      //C        surface.
      //C
      //C        For the purposes
      //C        of this calculation, a point is defined to be on the surface
      //C        if it is on the surface of one of the body elements, and
      //C        not inside any other space-filling body element.
      //C
      //C        Therefore, if your body is constructed so that any of the
      //C        above points are not accessible to the solvent, the
      //C        results will not be trustworthy.  An example of when this
      //C        could happen is if you use a triangular grid to set up a
      //C        closed surface, and then you put something like a sphere
      //C        so that it is partly outside and partly inside the triangular
      //C        grid.  Those inside points will be counted as part of
      //C        the surface.
      //C
      //C======================================================================
      //C
      //C        ERROR ESTIMATES
      //C
      //C        Significant figures of mass, viscosity, temperature
      //C
      //C======================================================================
      //C
      //C        POST-PROCESSING
      //C
      //C======================================================================
      //C
      //C        SOME NOTES ON UNITS, DEFINITIONS, AND CONVENTIONS
      //C
      //C        It is assumed that all parameters supplied by the .bod
      //C        file are in a consistent set of length units.
      //C
      //C        You have the option of specifying these units as either
      //C        meters, centimeters, nanometers, or Angstroms using
      //C        the "specify units" command.  However, if you omit
      //C        this command, the program just assumes generic units.
      //C
      //C        The capacitance is reported in length units, such that
      //C        the capacitance of a sphere equals its radius.
      //C
      //C        The polarizability tensor is reported in volume units.
      //C
      //C        The form of the polarizability tensor calculated internally
      //C        by this program will be called alpha.bongo
      //C
      //C        The form of the polarizability tensor that is output
      //C        will be called alpha.paper.  alpha.paper is the
      //C        definition that coincides with the Mansfield, Douglas,
      //C        Garbozci paper.
      //C
      //C        alpha.bongo     = polarizability tensor calculated
      //C                          internally
      //C                          by this program (units: volume)
      //C
      //C        alpha.paper     = polarizability tensor as defined
      //C                          in Mansfield-Douglas-Garbozci
      //C                          paper and as output (units: volume)
      //C
      //C        pi = 3.14159265...
      //C
      //C        Then:
      //C
      //C        alpha.paper = 4 * pi * alpha.bongo
      //C
      //C===========================================================
      //C
      silent = false;
      ncube = 0;
      nell = 0;
      cmn.nerr = 0;
      tell_all = false;
      //C
      //C        Names of the elements:
      map(1) = "sphere   ";
      map(2) = "triangle ";
      map(3) = "disk     ";
      map(4) = "cylinder ";
      map(5) = "torus    ";
      map(6) = "lens     ";
      map(7) = "ellipsoid";
      map(8) = "cube     ";
      //C
      //C        Number of floating point values needed to accompany each
      //C        type of element:
      mx(1) = 4;
      mx(2) = 9;
      mx(3) = 7;
      mx(4) = 8;
      mx(5) = 8;
      mx(6) = 8;
      mx(7) = 12;
      mx(8) = 4;
      //C
      //C        actions !  Codes for the actions to take:
      //C                !  z = zeno calculation, r = radius of gyration
      //C                !  calculation, k = kirkwood calculation, s = skip
      //C
      //C        eltype  !  A code for each element type, 1 = sphere, 2 = triangle,
      //C                !  3 =  disk, 4 = cylinder, 5 = torus, 6 = lens,
      //C                !  7 = ellipsoid, 8 = cube
      //C
      //C        nelts   !  The total number of body elements
      //C
      //C        bv      !  "Body values,"  bv(i,j),j=1,12 are all the
      //C                !  floating point numbers needed to specify the
      //C                !  body
      //C
      //C        id      !  character string that identifies the body
      //C                !  it is also used to make up the input and output
      //C                !  file names
      //C
      //C        nin     !  file unit for the input/body file
      //C
      //C        nout    !  file unit for the output/report file
      //C
      //C        m1(i)   !  Total number of Monte carlo steps to be
      //C                !  used in the i-th computation.
      //C
      //C        rotations  !  For disks, cylinders, tori, and lenses;
      //C                   !  this stores
      //C                   !  a rotation matrix required to rotate the
      //C                   !  axis of the element into the z-axis.
      //C                   !  For ellipsoids, it stores a rotation matrix
      //C                   !     required to rotate n1 to x-axis, n2 to
      //C                   !     y-axis, n3 to z-axis
      //C
      //C        saar(j)   !  The surface area of the j-th element
      //C
      //C        tol   !  Skin thickness
      //C
      //C        cap,delta_cap   !  The capacitance and its uncertainty
      //C
      //C        alpha_bongo,delta_bongo
      //C  The polarizability tensor, and its
      //C  uncertainty
      //C
      //C        volume,delta_volume
      //C  The volume and its uncertainty
      //C
      //C        surface,delta_surface
      //C  The surface area and its uncertainty
      //C
      //C        rg2int, delta_rg2int
      //C  The square radius of gyration of the
      //C  interior, and its uncertainty
      //C
      //C        rg2surf, delta_rg2surf
      //C  The square radius of gyration of the
      //C  surface, and its uncertainty
      //C
      //C        kirk, delta_kirk
      //C  The kirkwood estimate of the hydrodynamic
      //C  radius
      //C
      //C        zeno_done       !  The zeno calculation terminated successfully
      //C
      //C        kirk_done       !  The surface calculation terminated successfully
      //C
      //C        rg_done         !  The interior calculation terminated successfully
      //C
      //C        launch_done     !  The launch sphere has been generated
      //C
      //C        tol_given       !  The skin thickness was supplied in the bod file
      //C
      //C        errorlist       !  A list of error codes that are accumulated
      //C  during execution -- at the end, a list of errors
      //C  will be included in the report file
      //C
      //C        nerr            !  Physical length of the list of codes in errorlist
      //C
      //C        ncube,ferr      !  Sometimes, when using the cube body element,
      //C                        !  some points can be found "slightly" inside the
      //C                        !  cube.  If these values gets set -- the user
      //C                        !  will be warned by an error that this has happened,
      //C                        !  and the user will be told how far inside the
      //C                        !  cube the offending point(s) were found.
      //C
      //C        nell,rerr       !  I have put in a trap to check for overstretching
      //C  of ellipsoids, because I was not completely
      //C  confident of the stretching equations.  If these
      //C  values get set it is an indication that over-
      //C  stretching has happened.  The user will be
      //C  warned about this with an error statement.
      //C
      launch_done = false;
      zeno_done = false;
      kirk_done = false;
      rg_done = false;
      bailout = false;
      //C
      //C        Parse the invocation string:
      setup(cmn, id, nin, nout, m1, actions);
      //C
      //C        Parse the body file:
      parse(cmn, maxelts, nin, nelts, eltype, bv, tol, rotations, tol_given,
            unitcode, bailout, bt, bm, bw, bc, temp, tunit, mass, munit, visc,
            vunit);
      cmn.io.close(nin);
      if (bailout) {
         goto statement_99999;
      }
      //C
      //C        Begin output file:
      begin_output(cmn, nout, id, nelts, map, eltype, maxelts, mx, bv);
      //C
      //C        if either 'i' or 'z' action is called for,
      //C        then we need to compute the launch radius:
      //C
      FEM_DO_SAFE(i, 1, 3) {
         if (actions(i, i) == "i" || actions(i, i) == "z") {
            if (!launch_done) {
               do_launch(maxelts, nelts, eltype, bv, rlaunch, rotations);
               launch_done = true;
            }
         }
      }
      //C
      if (launch_done) {
         if (!silent) {
            write(6, "('Launch radius = ',g15.7)"), rlaunch;
         }
      }
      //C
      FEM_DO_SAFE(i, 1, 3) {
         m1do = m1(i);
         //C
         if (actions(i, i) == "i") {
            //C
            //C            Do the interior integrations:
            //C
            dorg(cmn, maxelts, eltype, bv, nelts, rlaunch, rotations, m1do,
                 rg2int, delta_rg2int, volume, delta_volume, rg_done, nout,
                 mi);
            //C
         }
         else if (actions(i, i) == "z") {
            //C
            //C            Do the zeno integrations:
            //C
            blizzard(cmn, maxelts, eltype, bv, nelts, m1do, tol, rlaunch,
                     rotations, cap, alpha_bongo, tol_given, zeno_done, nout,
                     delta_cap, delta_bongo, mz);
            //C
         }
         else if (actions(i, i) == "s") {
            //C
            //C            Do the surface integrations:
            //C
            captain(cmn, maxelts, eltype, bv, nelts, m1do, rotations,
                    kirk_done, nout, saar, kirk, delta_kirk, surface,
                    delta_surface, rg2surf, delta_rg2surf, ms);
            //C
         }
         //C
      }
      //C
   statement_99999:
      //C
      report(cmn, id, actions, m1, nelts, tol, rlaunch, cap, delta_cap,
             alpha_bongo, delta_bongo, volume, delta_volume, surface,
             delta_surface, rg2int, delta_rg2int, rg2surf, delta_rg2surf,
             kirk, delta_kirk, zeno_done, kirk_done, rg_done, launch_done,
             tol_given, nout, unitcode, mz, mi, ms, bt, bm, bw, bc, temp,
             tunit, mass, munit, visc, vunit, tell_all);
      //C
      cmn.io.close(nout);
      //C
      FEM_STOP(0);
   }

} // namespace zeno

int
zeno_main(
          int argc,
          char const* argv[])
{
   for ( int i = 0; i < argc; i++ )
   {
      printf( "zeno_main: arg %d <%s>\n", i, argv[ i ] );
   }
   zeno_progress->setTotalSteps( 108 * 3 );
   return fem::main_with_catch(
                               argc, argv,
                               zeno::program_zeno);
}

US_Hydrodyn_Zeno::US_Hydrodyn_Zeno( 
                                   hydro_options *         options,
                                   hydro_results *         results,
                                   US_Hydrodyn *           us_hydrodyn
                                   )
{
   this->options     = options;
   this->results     = results;
   this->us_hydrodyn = us_hydrodyn;
   zeno_us_hydrodyn  = us_hydrodyn;
   zeno_progress     = us_hydrodyn->progress;
}

bool US_Hydrodyn_Zeno::test()
{
   int argc = 5;
   const char *argv0 = "us_zeno";
   const char *argv1 = "zenotest";
   const char *argv2 = "i1t";
   const char *argv3 = "s1t";
   const char *argv4 = "z1t";
   const char *argv[ 5 ] = { argv0, argv1, argv2, argv3, argv4 };

   if ( !QFile::exists(  QString( "%1.bod" ).arg( argv1 ) ) )
   {
      cout << 
         QString( "error: %1 %2 does not exist" )
         .arg( QDir::current().path() )
         .arg( QString( "%1.bod" ).arg( argv1 ) );
      return false;
   }
   zeno_main( argc, argv );
   return true;
}

bool US_Hydrodyn_Zeno::run(
                           QString                 filename,
                           vector < PDB_atom > *   bead_model,
                           bool                    keep_files
                           )
{
   this->filename    = filename;
   this->bead_model  = bead_model;
   this->keep_files  = keep_files;

   error_msg = "";

   if ( !QDir::setCurrent( QFileInfo( filename ).filePath() ) )
   {
      // cout << QString( "current dir is %1\n" ).arg( QDir::current().path() );
      // error_msg = QString( "Error: can not change to directory: %1" ).arg( QFileInfo( filename ).dirPath() );
      // return false;
   }

   cout << QString( "current dir is %1\n" ).arg( QDir::current().path() );

   if ( options->unit != -9 &&
        options->unit != -10 )
   {
      error_msg = QString( "Error: units must be A(10) or nm(9) and value is %1" ).arg( -options->unit );
      return false;
   }

   // create .bod file
   // assume overwrite, maybe check in us_hydrodyn call

   QString outname = filename;
   if ( !outname.contains( QRegExp( ".bod$" ) ) )
   {
      outname += ".bod";
   } else {
      filename.replace( QRegExp( ".bod$" ), "" );
   }

   QFile fout ( outname );
   if ( !fout.open( IO_WriteOnly ) )
   {
      error_msg = QString( "Error: can not create file: %1" ).arg( outname );
      return false;
   }
    
   // output bead model

   QTextStream tso( &fout );

   double sum_mass = 0e0;

   for ( unsigned int i = 0; i < bead_model->size(); i++ )
   {
      tso << 
         QString( "S %1 %2 %3 %4\n" )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 0 ] )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 1 ] )
         .arg( (*bead_model)[ i ].bead_coordinate.axis[ 2 ] )
         .arg( (*bead_model)[ i ].bead_computed_radius )
         ;
      sum_mass += (*bead_model)[ i ].bead_ref_mw;
   }

   // additional info

   tso << QString( "temp      %1 C\n"  ).arg( options->temperature       );
   // tso << QString( "solvent   %1\n"    ).arg( options->solvent_name      );
   tso << QString( "viscosity %1 cp\n"    ).arg( options->solvent_viscosity );
   tso << QString( "mass      %1 Da\n" ).arg( options->mass_correction ?
                                              options->mass : sum_mass );
   tso << QString( "units     %1\n"    ).arg( options->unit == -9 ?
                                              "nm" : "A" );

   // add skin thickness
   if ( options->zeno_surface_thickness > 0.0 )
   {
      tso << QString( "st        %1\n"    ).arg( options->zeno_surface_thickness );
   }

   fout.close();
   cout << QString( "created: %1\n" ).arg( outname );

   int argc = 0;
   const char *argv[ 5 ];

   argv[ argc++ ] = "us_zeno";

   char buf_name[ QFileInfo( filename ).fileName().length() + 1 ];
   strncpy( buf_name, QFileInfo( filename ).fileName().ascii(), QFileInfo( filename ).fileName().length() );
   buf_name[ QFileInfo( filename ).fileName().length() ] = 0;
   argv[ argc++ ] = buf_name;

   cout << QString ( " zeno <%1> <%2> <%3>\n" )
      .arg( options->zeno_zeno_steps )
      .arg( options->zeno_interior_steps )
      .arg( options->zeno_surface_steps );


   QString qs_zeno     = QString( "" ).sprintf( "z%ut", options->zeno_zeno_steps     );
   char buf_zeno[ qs_zeno.length() + 1 ];
   strncpy( buf_zeno, qs_zeno.ascii(), qs_zeno.length() );
   buf_zeno[ qs_zeno.length() ] = 0;

   QString qs_interior = QString( "" ).sprintf( "i%ut", options->zeno_interior_steps );
   char buf_interior[ qs_interior.length() + 1 ];
   strncpy( buf_interior, qs_interior.ascii(), qs_interior.length() );
   buf_interior[ qs_interior.length() ] = 0;

   QString qs_surface  = QString( "" ).sprintf( "s%ut", options->zeno_surface_steps  );
   char buf_surface[ qs_surface.length() + 1 ];
   strncpy( buf_surface, qs_surface.ascii(), qs_surface.length() );
   buf_surface[ qs_surface.length() ] = 0;

   int progress_steps = 0;

   if ( options->zeno_zeno )
   {
      progress_steps += 108;
      argv[ argc++ ] = buf_zeno;
   }
   if ( options->zeno_interior )
   {
      progress_steps += 108;
      argv[ argc++ ] = buf_interior;
   }
   if ( options->zeno_surface )
   {
      progress_steps += 108;
      argv[ argc++ ] = buf_surface;
   }

   zeno_progress->setProgress( 0, progress_steps );
   zeno_main( argc, argv );
   zeno_progress->setProgress( 1, 1 );

   if ( !us_hydrodyn->stopFlag )
   {
      us_hydrodyn->last_hydro_res = QFileInfo( filename + ".zno" ).fileName();
   }

   return true;
}

bool US_Hydrodyn::calc_zeno()
{
   cout << "calc zeno\n";

   if ( !hydro.zeno_zeno &&
        !hydro.zeno_interior &&
        !hydro.zeno_surface )
   {
      editor_msg( "dark red", tr( "No Zeno methods selected.  Select in SOMO->Hydrodynamic Calculations Zeno" ) );
      return false;
   }

   US_Hydrodyn_Zeno uhz( &hydro, &results, this );

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   //   puts("calc hydro (supc)");
   display_default_differences();
   editor->append("\nBegin hydrodynamic calculations (Zeno) \n\n");

   int models_to_proc = 0;
   int first_model_no = 0;
   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append(QString("Model %1 will be included\n").arg(current_model + 1));
            bead_model = bead_models[current_model];
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }

   chdir(somo_dir);

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      pb_show_zeno_results->setEnabled(false);
      progress->reset();
      return false;
   }
   le_bead_model_suffix->setText(bead_model_suffix);

   for (current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++) {
      if (lb_model->isSelected(current_model)) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            bead_model = bead_models[current_model];
            bool result = 
               uhz.run( 
                       QString( somo_dir 
                                + QDir::separator() 
                                + project 
                                + QString("_%1").arg(current_model + 1) 
                                + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" ) ),
                       &bead_models[ current_model ], 
                       true );
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_calc_hydro->setEnabled(true);
               pb_calc_zeno->setEnabled(true);
               pb_bead_saxs->setEnabled(true);
               pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
               pb_show_zeno_results->setEnabled(false);
               progress->reset();
               return false;
            }
            if ( !result )
            {
               editor_msg( "red", "ZENO computation failed" );
               editor_msg( "red", tr( uhz.error_msg ) );
               pb_calc_hydro->setEnabled(true);
               pb_calc_zeno->setEnabled(true);
               pb_bead_saxs->setEnabled(true);
               pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
               pb_show_zeno_results->setEnabled(false);
               progress->reset();
               return false;
            }
         }
      }
   }

   chdir(somo_tmp_dir);

   pb_show_zeno_results->setEnabled( true );
   pb_calc_hydro->setEnabled(true);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );

   pb_stop_calc->setEnabled(false);
   editor->append("Calculate hydrodynamics (Zeno) completed\n");
   if ( advanced_config.auto_show_hydro ) 
   {
      show_zeno();
   }
   play_sounds(1);
   qApp->processEvents();
   return true;
}

void US_Hydrodyn::show_zeno()
{
   cout << "show zeno\n";
}

void US_Hydrodyn::show_zeno_options()
{
   if ( hydro_zeno_widget)
   {
      if ( hydro_zeno_window->isVisible() )
      {
         hydro_zeno_window->raise();
      }
      else
      {
         hydro_zeno_window->show();
      }
      return;
   }
   else
   {
      hydro_zeno_window = new US_Hydrodyn_Hydro_Zeno( &hydro, &hydro_zeno_widget, this );
      hydro_zeno_window->show();
   }
}
