#include "us_pabs_common.h"

HSVcolormap::HSVcolormap(){}

HSVcolormap::HSVcolormap(int n_colors)
{
    hstart = 300;
    hstop = 0;
    nc = n_colors;
    saturation = 255;
    value = 255;
}

HSVcolormap::HSVcolormap(int n_colors, int hue_start, int hue_stop)
{
    hstart = hue_start;
    hstop = hue_stop;
    nc = n_colors;
    saturation = 255;
    value = 255;
}

void HSVcolormap::set_n_colors(int num) { nc = num; }

int HSVcolormap::set_hue_start(int num) {
    if (num < 0 || num > 359)
        return 1;
    hstart = num;
    return 0;
}

int HSVcolormap::set_hue_stop(int num) {
    if (num < 0 || num > 359)
        return 1;
    hstop = num;
    return 0;
}

int HSVcolormap::set_saturation(int num) {
    if (num < 0 || num > 255)
        return 1;
    saturation = num;
    return 0;
}

int HSVcolormap::set_value(int num) {
    if (num < 0 || num > 255)
        return 1;
    value = num;
    return 0;
}

int HSVcolormap::set_transparency(int num){
    if (num < 0 || num > 255)
        return 1;
    alpha = num;
    return 0;
}

int HSVcolormap::get_colorlist(QVector<QColor> &colorList){
    bool state = true;
    state = state && (nc > 0) && (hstart >= 0) && (hstop >= 0);
    state = state && (saturation >= 0) && (value >= 0);
    colorList.clear();
    if (! state)
        return 1;
    QVector<int> hvals = rang_hvalues();
    for (int i = 0; i < nc; ++i){
        QColor color(QColor::fromHsv(hvals.at(i), saturation, value, alpha));
        colorList << color;
    }
    return 0;
}

QVector<int> HSVcolormap::rang_hvalues(void){
    QVector<int> hrange;

    if (nc == 1){
        hrange << hstop;
        return hrange;
    }

    double delta = (hstop - hstart) / (nc - 1);
    double val;
    for(int i = 0; i < nc - 1; ++i){
        val = hstart + delta * i;
        hrange << qRound(val);
    }
    hrange << hstop;
    return hrange;
}

void CCW::clear(){
    cell.clear();
    channel.clear();
    wavelength.clear();
    index.clear();
    runId.clear();
}

int CCW::size(){
    return cell.size();
}

void CCW_ITEM::clear(void){
    runClass.clear();
    cell.clear();
    channel.clear();
    wavelength.clear();
    index.clear();
}

int CCW_ITEM::size(void){
    return cell.size();
}
