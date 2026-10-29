// tinytest.h -- zero-dependency test harness (CHECK/REQUIRE + auto-registered TEST cases).
#pragma once
#include <cstdio>
#include <string>
#include <vector>
#include <functional>

namespace tinytest {
struct Case { std::string name; std::function<void()> fn; };
inline std::vector<Case>& cases(){ static std::vector<Case> c; return c; }
inline int& checks(){ static int n=0; return n; }
inline int& fails(){ static int n=0; return n; }
struct Reg { Reg(const std::string& n, std::function<void()> f){ cases().push_back({n,std::move(f)}); } };
struct Abort {};
inline int run(){
    int failed_cases=0;
    for (auto& c : cases()){
        int before=fails();
        try { c.fn(); } catch (Abort&) {}
        catch (std::exception& e){ ++fails(); std::printf("  [EXC] %s: %s\n", c.name.c_str(), e.what()); }
        if (fails()>before){ std::printf("FAIL  %s\n", c.name.c_str()); ++failed_cases; }
        else                std::printf("ok    %s\n", c.name.c_str());
    }
    std::printf("\n%d checks, %d failures across %zu cases\n", checks(), fails(), cases().size());
    return failed_cases==0 ? 0 : 1;
}
} // namespace tinytest

#define TT_CAT2(a,b) a##b
#define TT_CAT(a,b) TT_CAT2(a,b)
#define TEST(name) \
    static void TT_CAT(tt_fn_,__LINE__)(); \
    static ::tinytest::Reg TT_CAT(tt_reg_,__LINE__)(name, TT_CAT(tt_fn_,__LINE__)); \
    static void TT_CAT(tt_fn_,__LINE__)()

#define CHECK(cond) do{ ::tinytest::checks()++; if(!(cond)){ ::tinytest::fails()++; \
    std::printf("  CHECK failed: %s  (%s:%d)\n", #cond, __FILE__, __LINE__);} }while(0)

#define CHECK_EQ(a,b) do{ ::tinytest::checks()++; auto _va=(a); auto _vb=(b); if(!(_va==_vb)){ \
    ::tinytest::fails()++; std::ostringstream _o; _o<<_va<<" != "<<_vb; \
    std::printf("  CHECK_EQ failed: %s == %s  [%s]  (%s:%d)\n", #a,#b,_o.str().c_str(),__FILE__,__LINE__);} }while(0)

#define REQUIRE(cond) do{ ::tinytest::checks()++; if(!(cond)){ ::tinytest::fails()++; \
    std::printf("  REQUIRE failed: %s  (%s:%d)\n", #cond, __FILE__, __LINE__); throw ::tinytest::Abort{};} }while(0)
