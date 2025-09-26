#include "qt_test_base.h"

// Static member definitions (put in qt_test_base.cpp)
std::unique_ptr<QCoreApplication> QtTestBase::app_ = nullptr;
std::unique_ptr<QApplication> GuiTestBase::app_ = nullptr;
