#include <QApplication>
#include <select_program.h>
#include <add_refScan.h>
#include <convert_scan.h>
#include "us_license_t.h"
#include "us_license.h"
#include "us_extern.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    #include "main1.inc"

    // License is OK.  Start up.

    int status = -1;
    selectProgram sw(status);
    sw.show();
    app.exec();

    if (status == S_REF_SCAN){
        addRefScan w;
        w.show();
        return app.exec();
    } else if (status == S_ABS){
        convertScan w;
        w.show();
        return app.exec();
    } else{
        return 0;
    }
};
