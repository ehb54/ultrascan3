// Copyright (c) 2022. Lorem ipsum dolor sit amet, consectetur adipiscing elit.
// Morbi non lorem porttitor neque feugiat blandit. Ut vitae ipsum eget quam lacinia accumsan.
// Etiam sed turpis ac ipsum condimentum fringilla. Maecenas magna.
// Proin dapibus sapien vel ante. Aliquam erat volutpat. Pellentesque sagittis ligula eget metus.
// Vestibulum commodo. Ut rhoncus gravida arcu.

//
// Created by doluk on 8/23/22.
//

#include "us_math_bf.h"
int main(void) {
   qDebug() << US_Math_BF::bessel_J0(0.1) << 0.99750156206604003230;
   qDebug() << US_Math_BF::bessel_J0(2.0) << 0.22389077914123566805;
   qDebug() << US_Math_BF::bessel_J0(100.0) << 0.019985850304223122424;
   qDebug() << US_Math_BF::bessel_J0(1.0e+10) << 2.1755917502468917269e-06;
   qDebug() << US_Math_BF::bessel_J1(2.0) << 0.57672480775687338720;
   qDebug() << US_Math_BF::bessel_J1(100.0) << -0.07714535201411215803;
   qDebug() << US_Math_BF::bessel_J1(1.0e+10) << -7.676508175684157103e-06;
   qDebug() << US_Math_BF::bessel_Y0(2) << 0.5103756726497451196;
   qDebug() << US_Math_BF::bessel_Y0(256.0) << -0.03381290171792454909;
   qDebug() << US_Math_BF::bessel_Y0(4294967296.0) << 3.657903190017678681e-06;
   qDebug() << US_Math_BF::bessel_Y1(2) << -0.10703243154093754689;
   qDebug() << US_Math_BF::bessel_Y1(100.0) << -0.020372312002759793305;
   qDebug() << US_Math_BF::bessel_Y1(4294967296.0) << 0.000011612249378370766284;
}