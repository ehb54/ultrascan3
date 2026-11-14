#pragma once

#include <QWidget>
#include "us_resids_bitmap.h"
#include "us_noise.h"

// This executable never links the application classes. These concrete doubles
// supply only the legacy input accessors; the production dialog is compiled intact.
struct ResidualInputs
{
    US_DataIO::EditedData edited;
    US_DataIO::RawData simulated;
    QList<int> excluded;
    US_Noise ti, ri;
    QPointer<US_ResidsBitmap> bitmap;
    QString triple = "1 / A / 280";
};

#define RESIDUAL_OWNER(Class, prefix) \
class Class : public QWidget \
{ \
public: \
    explicit Class(ResidualInputs& inputs) : input(inputs) {} \
    US_DataIO::EditedData* prefix##_editdata() { return &input.edited; } \
    US_DataIO::RawData* prefix##_simdata() { return &input.simulated; } \
    QList<int>* prefix##_excllist() { return &input.excluded; } \
    US_Noise* prefix##_ti_noise() { return &input.ti; } \
    US_Noise* prefix##_ri_noise() { return &input.ri; } \
    QPointer<US_ResidsBitmap> prefix##_resbmap() { return input.bitmap; } \
    QString prefix##_tripleInfo() { return input.triple; } \
private: \
    ResidualInputs& input; \
};

RESIDUAL_OWNER(US_FeMatch, fem)
RESIDUAL_OWNER(US_Analysis_auto, aa)
RESIDUAL_OWNER(US_ReporterGMP, rg)
#undef RESIDUAL_OWNER
