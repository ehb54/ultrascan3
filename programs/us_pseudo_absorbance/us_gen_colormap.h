#ifndef US_GEN_COLORMAP_H
#define US_GEN_COLORMAP_H

#include <QObject>
#include <QVector>
#include <QColor>

class HSVcolormap
{
//    Q_OBJECT;
public:
    HSVcolormap();
    HSVcolormap(int n_colors);
    HSVcolormap(int n_colors, int hue_start, int hue_stop);

    void set_n_colors(int num);
    int set_hue_start(int num);
    int set_hue_stop(int num);
    int set_saturation(int num);
    int set_value(int num);
    int set_transparency(int num);
    int get_colorlist(QVector<QColor> &colorList);

private:
    int nc = -1;
    int hstart = -1;
    int hstop = -1;
    int saturation = -1;
    int value = -1;
    int alpha = 255;
    QVector<int> rang_hvalues(void);
};

#endif // US_GEN_COLORMAP_H
