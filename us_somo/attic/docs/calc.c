#include <iostream.h>
#include <math.h>

main() {
  float f;

  f = (277.1 * 8.314e7 * 2.169184) / (.271181 * pow((41000 * M_PI / 30), 2));
  cout << f << endl;
  f = (16379 * 0.271181 * pow((41000 * M_PI / 30), 2)) / (277.1 * 8.314e7);
  cout << f << endl;
}
