#include <QtTest/QtTest>
#include "test_us_memory.h"
#include "us_memory.h"

void TestUSMemory::testRssNow() {
    long int rss = US_Memory::rss_now();
    QVERIFY(rss > 0); // Check that RSS is greater than 0
}

void TestUSMemory::testRssMax() {
    long int rssMax = 0;
    long int newRssMax = US_Memory::rss_max(rssMax);
    QVERIFY(newRssMax >= rssMax); // Check that the new RSS max is not less than the old one
}

void TestUSMemory::testMemoryProfile() {
    int memA, memT, memU;
    int memAvPc = US_Memory::memory_profile(&memA, &memT, &memU);
    QVERIFY(memAvPc >= 0 && memAvPc <= 100); // Check that memory availability percentage is within 0-100
    QVERIFY(memA >= 0);
    QVERIFY(memT > 0);
    QVERIFY(memU >= 0 && memU <= memT);
}
