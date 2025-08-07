//! \file us_vector.cpp

#include "us_vector.h"

#include "us_math2.h"

US_Vector::US_Vector(int size, double value) { v.fill(value, size); };

void US_Vector::add(const US_Vector& other) {
  Q_ASSERT(other.size() == v.size());

  for (int i = 0; i < v.size(); i++) v[i] += other[i];
}

void US_Vector::add(double value) {
  for (int i = 0; i < v.size(); i++) v[i] += value;
}

double US_Vector::dot(const US_Vector& other) {
  Q_ASSERT(other.size() == v.size());

  double value = 0.0;

  for (int i = 0; i < v.size(); i++) value += v[i] * other[i];

  return value;
}

void US_Vector::mult(const US_Vector& other) {
  Q_ASSERT(other.size() == v.size());

  for (int i = 0; i < v.size(); i++) v[i] *= other[i];
}

void US_Vector::scale(double value) {
  for (int i = 0; i < v.size(); i++) v[i] *= value;
}

double US_Vector::distance(const US_Vector& other) {
  Q_ASSERT(other.size() == v.size());
  double value = 0.0;

  for (int i = 0; i < v.size(); i++) value += sq(v[i] - other[i]);

  return sqrt(value);
}

double US_Vector::L2norm(void) {
  double value = 0.0;

  for (int i = 0; i < v.size(); i++) value += sq(v[i]);

  return sqrt(value);
}
