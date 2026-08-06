// Qt backend for la::Parallel -- used when GRPY runs inside SOMO (or any Qt app).
// Shares SOMO's QThreadPool (throttled by SOMO's numThreads setting) so there is a
// single thread pool, not OpenMP's runtime competing with Qt's. This is also the
// backend that makes the eventual in-process (function-call) GRPY integration clean:
// the same pool, and progress delivered as a Qt signal instead of scraped stdout.
//
// Include this (and link QtCore + QtConcurrent) only in the SOMO/Qt build; the
// numeric core and the standalone CLI use la::Serial / a std::thread pool and never
// depend on Qt.
#pragma once
#include <QThreadPool>
#include <QtConcurrent>
#include <QVector>
#include "linalg.hpp"

namespace la {

struct QtParallel : Parallel {
    // maxThreads<=0 uses the global pool as-is (SOMO sets it from config numThreads).
    explicit QtParallel(int maxThreads = 0) {
        if (maxThreads > 0)
            QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
    }
    void for_range(int n, const std::function<void(int)>& f) override {
        if (n <= 0) return;
        if (n == 1) { f(0); return; }
        QVector<int> idx(n);
        for (int i = 0; i < n; ++i) idx[i] = i;
        // blockingMap runs f over the global QThreadPool and returns when all done.
        QtConcurrent::blockingMap(idx, [&f](int i) { f(i); });
    }
};

}  // namespace la
