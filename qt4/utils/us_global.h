#ifndef US_GLOBAL_H
#define US_GLOBAL_H

#include <QSharedMemory>
#include <QPoint>

class US_Global
{
  public:
    US_Global();
    ~US_Global();

    bool   isValid() { return valid; }
    QPoint global_position( void );
    void   set_global_position( const QPoint& );

  private:
    struct global
    {
      QPoint current_position;
      // Add other global values as necessary
    } global;

    bool          valid;
    QSharedMemory sharedMemory;

    void read_global ( void );
    void write_global( void );
};

#endif
