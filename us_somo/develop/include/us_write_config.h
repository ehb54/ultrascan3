#ifndef US_WRITE_CONFIG_H
#define US_WRITE_CONFIG_H

#include "us_util.h"

class US_EXTERN US_Write_Config : public QObject {
  Q_OBJECT

 public:
  US_Write_Config(QObject* parent = 0, const char* name = 0);
  ~US_Write_Config();

 public:
  bool write_config(struct Config);
  void errorMessage(QString&, QString&);
};

#endif
