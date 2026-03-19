#include "us_spectrum.h"
#include "qwt_plot_marker.h"
#include "us_csv_loader.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_settings.h"
#include <QtNumeric>
#include <algorithm>
#include <cmath>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    US_Spectrum w;
    w.show();
    return application.exec();
}

US_Spectrum::US_Spectrum() : US_Widgets()
{
    setWindowTitle(tr("Spectrum Decomposition"));
    setPalette(US_GuiSettings::frameColor());

    wavl_min = -1;
    wavl_max = -1;

    QLabel *lb_target     = us_banner(tr("Target Spectrum"));
    QLabel *lb_basis      = us_banner(tr("Basis Spectra"));
    QLabel *lb_fit        = us_banner(tr("Fit & Basis Correlation"));
    QLabel *lb_tgt_fname  = us_label(tr("Target Filename"));
    QLabel *lb_tgt_header = us_label(tr("Target Header"));
    QLabel *lb_tgt_wavl   = us_label(tr("Target %1 (nm)").arg(QChar(955)));
    QLabel *lb_wavl_min   = us_label(tr("Min. %1 (nm) to Fit").arg(QChar(955)));
    QLabel *lb_wavl_max   = us_label(tr("Max. %1 (nm) to Fit").arg(QChar(955)));
    QLabel *lb_basis_list = us_label(tr("Basis List"));
    QLabel *lb_basis_1    = us_label(tr("Basis 1"));
    QLabel *lb_basis_2    = us_label(tr("Basis 2"));
    QLabel *lb_rmsd       = us_label(tr("RMSD"));
    QLabel *lb_angle      = us_label(tr("Correlation Angle"));

    lb_tgt_header->setAlignment(Qt::AlignCenter);
    lb_basis_list->setAlignment(Qt::AlignCenter);
    lb_tgt_fname->setAlignment(Qt::AlignCenter);
    lb_wavl_min->setAlignment(Qt::AlignCenter);
    lb_wavl_max->setAlignment(Qt::AlignCenter);
    lb_tgt_wavl->setAlignment(Qt::AlignCenter);
    lb_basis_1->setAlignment(Qt::AlignCenter);
    lb_basis_2->setAlignment(Qt::AlignCenter);
    lb_target->setAlignment(Qt::AlignCenter);
    lb_basis->setAlignment(Qt::AlignCenter);
    lb_angle->setAlignment(Qt::AlignCenter);
    lb_rmsd->setAlignment(Qt::AlignCenter);
    lb_fit->setAlignment(Qt::AlignCenter);

    le_tgt_fname  = us_lineedit("", 1, true);
    le_tgt_header = us_lineedit("", 1, true);
    le_tgt_wavl   = us_lineedit("", 1, true);
    le_wavl_min   = us_lineedit("", 1, false);
    le_wavl_max   = us_lineedit("", 1, false);
    le_angle      = us_lineedit("", 1, true);
    le_rmsd       = us_lineedit("", 1, true);

    QGridLayout* lyt_db_target = us_checkbox("Load from DB", chkb_db_target);
    QGridLayout* lyt_db_basis  = us_checkbox("Load from DB", chkb_db_basis);

    QPushButton *pb_target = us_pushbutton(tr("Load Target"));
    QPushButton *pb_basis  = us_pushbutton(tr("Add Basis"));
    QPushButton *pb_fit    = us_pushbutton(tr("Fit Data"));
    QPushButton *pb_reset  = us_pushbutton(tr("Reset Basis"));
    QPushButton *pb_help   = us_pushbutton(tr("Help"));
    QPushButton *pb_close  = us_pushbutton(tr("Close"));
    QPushButton *pb_save   = us_pushbutton(tr("Save Fitting Data"));

    QFont font = QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1);
    QString hl = tr("%1 (nm)").arg(QChar(955));
    tw_basis = new QTableWidget();
    tw_basis->setRowCount(0);
    tw_basis->setColumnCount(4);
    tw_basis->setPalette(US_GuiSettings::normalColor());
    tw_basis->setFont(font);
    tw_basis->setHorizontalHeaderLabels(QStringList{"Header", hl, "%", "Delete"});
    tw_basis->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    cb_basis_1 = us_comboBox();
    cb_basis_2 = us_comboBox();

    QGridLayout *left_lyt = new QGridLayout();
    left_lyt->setContentsMargins(0, 0, 0, 0);
    left_lyt->setSpacing(1);
    left_lyt->setColumnStretch(0, 1);
    left_lyt->setColumnStretch(1, 1);

    int row = 0;
    left_lyt->addWidget(lb_target,     row++, 0, 1, 2);
    left_lyt->addLayout(lyt_db_target, row,   0, 1, 1);
    left_lyt->addWidget(pb_target,     row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_fname,  row,   0, 1, 1);
    left_lyt->addWidget(le_tgt_fname,  row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_header, row,   0, 1, 1);
    left_lyt->addWidget(le_tgt_header, row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_wavl,   row,   0, 1, 1);
    left_lyt->addWidget(le_tgt_wavl,   row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis,      row++, 0, 1, 2);
    left_lyt->addLayout(lyt_db_basis,  row,   0, 1, 1);
    left_lyt->addWidget(pb_basis,      row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_list, row++, 0, 1, 2);
    left_lyt->addWidget(tw_basis,      row++, 0, 1, 2);
    left_lyt->addWidget(pb_reset,      row++, 0, 1, 2);
    left_lyt->addWidget(lb_fit,        row++, 0, 1, 2);
    left_lyt->addWidget(lb_wavl_min,   row,   0, 1, 1);
    left_lyt->addWidget(le_wavl_min,   row++, 1, 1, 1);
    left_lyt->addWidget(lb_wavl_max,   row,   0, 1, 1);
    left_lyt->addWidget(le_wavl_max,   row++, 1, 1, 1);
    left_lyt->addWidget(pb_fit,        row++, 0, 1, 2);
    left_lyt->addWidget(lb_rmsd,       row,   0, 1, 1);
    left_lyt->addWidget(le_rmsd,       row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_1,    row,   0, 1, 1);
    left_lyt->addWidget(cb_basis_1,    row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_2,    row,   0, 1, 1);
    left_lyt->addWidget(cb_basis_2,    row++, 1, 1, 1);
    left_lyt->addWidget(lb_angle,      row,   0, 1, 1);
    left_lyt->addWidget(le_angle,      row++, 1, 1, 1);
    left_lyt->addWidget(pb_save,       row++, 0, 1, 2);
    left_lyt->addWidget(pb_help,       row++, 0, 1, 2);
    left_lyt->addWidget(pb_close,      row++, 0, 1, 2);

    data_plot = new QwtPlot();
    US_Plot *plot_layout_1 = new US_Plot(data_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
    data_plot->setCanvasBackground(Qt::black);
    data_plot->setTitle("Wavelength Spectrum Fit");
    data_plot->setMinimumSize(600, 200);

    error_plot = new QwtPlot();
    US_Plot *plot_layout_2 = new US_Plot(error_plot, tr(""), tr("Wavelength(nm)"), tr("Extinction"));
    error_plot->setCanvasBackground(Qt::black);
    error_plot->setTitle("Fitting Residuals");
    error_plot->setMinimumSize(600, 200);

    QwtPlotPicker *picker = new US_PlotPicker(data_plot);
    picker->setRubberBand(QwtPicker::VLineRubberBand);

    QVBoxLayout *right_lyt = new QVBoxLayout();
    right_lyt->addLayout(plot_layout_1);
    right_lyt->addLayout(plot_layout_2);
    right_lyt->setContentsMargins(0, 0, 0, 0);
    right_lyt->setSpacing(1);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(1);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addLayout(left_lyt, 1);
    layout->addLayout(right_lyt, 2);

    setLayout(layout);

    connect(le_wavl_min, &QLineEdit::editingFinished, this, &US_Spectrum::min_wavl_updated);
    connect(le_wavl_max, &QLineEdit::editingFinished, this, &US_Spectrum::max_wavl_updated);
    connect(pb_target, &QPushButton::clicked, this, &US_Spectrum::load_target);
    connect(pb_basis, &QPushButton::clicked, this, &US_Spectrum::load_basis);
    connect(pb_reset, &QPushButton::clicked, this, &US_Spectrum::reset);
    connect(pb_close, &QPushButton::clicked, this, &US_Spectrum::close);
    connect(pb_save, &QPushButton::clicked, this, &US_Spectrum::save);
    connect(pb_fit, &QPushButton::clicked, this, &US_Spectrum::fit);
}

QVector<int> US_Spectrum::argsort(const QVector<double> &vec)
{
    QVector<int> indices(vec.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&vec](int a, int b) { return vec[a] < vec[b]; });
    return indices;
}

void US_Spectrum::sort(const QVector<int> &indices, QVector<double> &vec)
{
    QVector<double> vec_c(vec);
    for (int ii = 0; ii < vec.size(); ii++)
    {
        vec[ii] = vec_c[indices.at(ii)];
    }
}

void US_Spectrum::DataProfile::clear_fit()
{
    xvec.clear();
    yvec.clear();
    nnls_factor = -1;
    nnls_percent = -1;
}

void US_Spectrum::clear_fit()
{
    residual.clear();
    solution.clear();
    target.clear_fit();
    int nrows = tw_basis->rowCount();
    for (int ii = 0; ii < basis_list.size(); ii++)
    {
        basis_list[ii].clear_fit();
        if (ii < nrows)
        {
            tw_basis->item(ii, 2)->text().clear();
        }
    }
    le_angle->clear();
    le_rmsd->clear();
    le_wavl_min->clear();
}

void US_Spectrum::clear_plot()
{
    data_plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    error_plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    basis_curves.clear();
    data_plot->replot();
    error_plot->replot();
}

void US_Spectrum::load_target()
{
    if (chkb_db_target->isChecked())
    {
        return;
    }
    QString path = US_Settings::dataDir();
    current_path = current_path.isEmpty() ? path : current_path;

    QString filter = tr("Text Files (*.txt *.csv *.dat *.wa *.dsp);;All Files (*)");
    QString fpath = QFileDialog::getOpenFileName(this, "Load Target Spectrum", current_path, filter);

    if (fpath.isEmpty())
    {
        return;
    }
    QString note = "1st Column -> WAVELENGTH ; 2nd Column -> OD";
    US_CSV_Loader *csv_loader = new US_CSV_Loader(fpath, note, true, this);
    int state = csv_loader->exec();
    if (state != QDialog::Accepted)
    {
        return;
    }
    US_CSV_Data csv_data = csv_loader->data();
    if (csv_data.columnCount() < 2)
    {
        QMessageBox::warning(this, "Error!", "The data file should have two columns of wavelength and OD values!");
        return;
    }

    clear_plot();
    clear_fit();

    QVector<double> lambda(csv_data.columnAt(0));
    QVector<double> od(csv_data.columnAt(1));
    QVector<int> indices = argsort(lambda);
    sort(indices, lambda);
    sort(indices, od);

    target.finfo = QFileInfo(csv_data.filePath());
    target.lambda = lambda;
    target.od = od;
    target.header = csv_data.header().at(1);

    auto minmax = std::minmax_element(target.lambda.begin(), target.lambda.end());
    le_tgt_fname->setText(target.finfo.fileName());
    le_tgt_header->setText(target.header);
    le_tgt_wavl->setText(tr("%1 - %2").arg(*minmax.first).arg(*minmax.second));

    plot();
}

void US_Spectrum::load_basis()
{
    if (chkb_db_basis->isChecked())
    {
        return;
    }
    QString path = US_Settings::dataDir();
    current_path = current_path.isEmpty() ? path : current_path;

    QString filter = tr("Text Files (*.txt *.csv *.dat *.wa *.dsp);;All Files (*)");
    QStringList files;
    files = QFileDialog::getOpenFileNames(this, "Add Basis Spectra", current_path, filter);

    if (files.isEmpty())
    {
        return;
    }

    QVector<US_CSV_Data> data_list;
    for (int ii = 0; ii < files.size(); ii++)
    {
        QString filepath = files.at(ii);
        QString note = "1st Column -> WAVELENGTH ; Others -> OD";
        bool editing = true;
        US_CSV_Loader *csv_loader = new US_CSV_Loader(filepath, note, editing, this);
        int state = csv_loader->exec();
        if (state == QDialog::Rejected)
        {
            int check = QMessageBox::question(this, "Warning!",
                                              "You canceled parsing a file.\n" + filepath +
                                                  "\nDo you want to continue loading the rest of the file(s)?");
            if (check == QMessageBox::No)
            {
                return;
            }
        }
        else if (state == QDialog::Accepted)
        {
            US_CSV_Data csv_data = csv_loader->data();
            if (csv_data.columnCount() < 2)
            {
                int check = QMessageBox::question(this, "Warning!",
                                                  "This file does not have two data columns:\n" + filepath +
                                                      "\nDo you want to continue loading the rest of the file(s)?");
                if (check == QMessageBox::No)
                {
                    return;
                }
            }
            else
            {
                data_list << csv_data;
            }
        }
        else
        {
            int check = QMessageBox::question(this, "Warning!",
                                              "Unable to load the file!\n" + filepath +
                                                  "\nDo you want to continue loading the rest of the file(s)?");
            if (check == QMessageBox::No)
            {
                return;
            }
        }
    }

    for (int ii = 0; ii < data_list.size(); ii++)
    {
        QFileInfo finfo(data_list[ii].filePath());
        QVector<double> xvals = data_list.at(ii).columnAt(0);
        QVector<int> indices = argsort(xvals);
        sort(indices, xvals);
        for (int jj = 1; jj < data_list.at(ii).columnCount(); jj++)
        {
            QVector<double> yvals(data_list.at(ii).columnAt(jj));
            sort(indices, yvals);
            DataProfile dp;
            dp.lambda << xvals;
            dp.od << yvals;
            dp.header = data_list.at(ii).header().at(jj);
            dp.finfo = finfo;
            basis_list << dp;
        }
    }

    clear_fit();
    fill_table();
    fill_combo();
    plot();
}

void US_Spectrum::fill_table()
{
    tw_basis->disconnect();
    int nrows = basis_list.size();
    tw_basis->setRowCount(nrows);
    tw_basis->setColumnCount(4);
    if (nrows == 0)
    {
        tw_basis->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        return;
    }

    for (int ii = 0; ii < nrows; ii++)
    {
        QTableWidgetItem *item_0 = new QTableWidgetItem();
        item_0->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        if (basis_list.at(ii).highlight)
        {
            item_0->setCheckState(Qt::Checked);
        }
        else
        {
            item_0->setCheckState(Qt::Unchecked);
        }
        item_0->setText(basis_list.at(ii).header);
        item_0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QTableWidgetItem *item_1 = new QTableWidgetItem();
        item_1->setFlags(Qt::NoItemFlags);
        auto minmax = std::minmax_element(basis_list.at(ii).lambda.begin(), basis_list.at(ii).lambda.end());
        item_1->setText(tr("%1 - %2").arg(*minmax.first).arg(*minmax.second));
        item_1->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *item_2 = new QTableWidgetItem();
        item_2->setFlags(Qt::NoItemFlags);
        item_2->setTextAlignment(Qt::AlignCenter);
        item_2->setText("");

        QPushButton *btn = new QPushButton("");
        btn->setFlat(true);
        btn->setIcon(this->style()->standardIcon(QStyle::SP_BrowserStop));

        connect(btn, &QPushButton::clicked, this, [=]() { delete_basis(ii); });

        QWidget *cell_3 = new QWidget();
        QHBoxLayout *lyt = new QHBoxLayout(cell_3);
        lyt->setContentsMargins(0, 0, 0, 0);
        lyt->addWidget(btn, 0, Qt::AlignCenter);

        tw_basis->setItem(ii, 0, item_0);
        tw_basis->setItem(ii, 1, item_1);
        tw_basis->setItem(ii, 2, item_2);
        tw_basis->setCellWidget(ii, 3, cell_3);
    }

    tw_basis->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tw_basis->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tw_basis->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tw_basis->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    connect(tw_basis, &QTableWidget::itemChanged, this, &US_Spectrum::basis_checked);
}

void US_Spectrum::fill_combo()
{
    cb_basis_1->disconnect();
    cb_basis_2->disconnect();
    cb_basis_1->clear();
    cb_basis_2->clear();

    for (int ii = 0; ii < basis_list.size(); ii++)
    {
        cb_basis_1->addItem(basis_list.at(ii).header);
        cb_basis_2->addItem(basis_list.at(ii).header);
    }

    if (cb_basis_1->count() > 0)
    {
        cb_basis_1->setCurrentIndex(0);
        cb_basis_2->setCurrentIndex(0);
    }
    connect(cb_basis_1, qOverload<int>(&QComboBox::currentIndexChanged), this, &US_Spectrum::find_angle);
    connect(cb_basis_2, qOverload<int>(&QComboBox::currentIndexChanged), this, &US_Spectrum::find_angle);
    find_angle();
}

void US_Spectrum::basis_checked(QTableWidgetItem *item)
{
    if (item->column() != 0)
    {
        return;
    }
    int id = item->row();
    bool checked = item->checkState() == Qt::Checked;
    if (checked == basis_list.at(id).highlight)
    {
        basis_list[id].header = item->text();
        fill_combo();
    }
    else
    {
        basis_list[id].highlight = checked;
        highlight();
    }
}

void US_Spectrum::plot()
{
    clear_plot();

    double min_wavl_plot;
    double max_wavl_plot;

    for (int ii = 0; ii < basis_list.size(); ii++)
    {
        double *xx;
        double *yy;
        int np;
        if (basis_list.at(ii).xvec.isEmpty())
        {
            xx = basis_list[ii].lambda.data();
            yy = basis_list[ii].od.data();
            np = basis_list[ii].od.size();
        }
        else
        {
            xx = basis_list[ii].xvec.data();
            yy = basis_list[ii].yvec.data();
            np = basis_list[ii].yvec.size();
        }

        min_wavl_plot = xx[0];
        max_wavl_plot = xx[np - 1];

        QwtSymbol *symb = new QwtSymbol();
        symb->setStyle(QwtSymbol::Ellipse);
        symb->setPen(QPen(Qt::yellow));
        symb->setBrush(QBrush(Qt::yellow));
        symb->setSize(3);

        QwtPlotCurve *curve;
        curve = us_curve(data_plot, basis_list.at(ii).header);
        curve->setSymbol(symb);
        curve->setStyle(QwtPlotCurve::NoCurve);
        curve->setSamples(xx, yy, np);
        basis_curves << curve;
    }

    if (!target.od.isEmpty())
    {
        double *xx;
        double *yy;
        int np;
        if (target.xvec.isEmpty())
        {
            xx = target.lambda.data();
            yy = target.od.data();
            np = target.od.size();
        }
        else
        {
            xx = target.xvec.data();
            yy = target.yvec.data();
            np = target.yvec.size();
        }

        min_wavl_plot = xx[0];
        max_wavl_plot = xx[np - 1];

        QwtPlotCurve *curve;
        curve = us_curve(data_plot, target.header);
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setSamples(xx, yy, np);
        curve->setPen(Qt::green, 3, Qt::SolidLine);

        if (!solution.isEmpty())
        {
            curve = us_curve(data_plot, "Fitted Curve");
            curve->setStyle(QwtPlotCurve::Lines);
            curve->setSamples(xx, solution.data(), solution.size());
            curve->setPen(Qt::magenta, 3, Qt::SolidLine);
        }

        if (!residual.isEmpty())
        {
            curve = us_curve(error_plot, "Residual Curve");
            curve->setStyle(QwtPlotCurve::Lines);
            curve->setSamples(xx, residual.data(), residual.size());
            curve->setPen(Qt::red, 3, Qt::SolidLine);

            QwtPlotMarker *hline = new QwtPlotMarker();
            hline->setLineStyle(QwtPlotMarker::HLine);
            hline->setYValue(0.0);
            hline->setLinePen(QPen(Qt::white, 1, Qt::DashLine));
            hline->attach(error_plot);
        }

        min_wavl_plot -= 5;
        max_wavl_plot += 5;
        data_plot->setAxisScale( QwtPlot::xBottom, min_wavl_plot, max_wavl_plot);
        error_plot->setAxisScale( QwtPlot::xBottom, min_wavl_plot, max_wavl_plot);
    }

    us_grid(data_plot);
    data_plot->replot();
    error_plot->replot();
    highlight();
}

void US_Spectrum::highlight()
{
    if (basis_list.size() != basis_curves.size())
    {
        return;
    }
    for (int ii = 0; ii < basis_list.size(); ii++)
    {
        if (basis_list.at(ii).highlight)
        {
            basis_curves[ii]->setStyle(QwtPlotCurve::Lines);
            basis_curves[ii]->setPen(Qt::yellow, 3, Qt::SolidLine);
            basis_curves[ii]->setSymbol(nullptr);
        }
        else
        {
            basis_curves[ii]->setStyle(QwtPlotCurve::NoCurve);
            QwtSymbol *symb = new QwtSymbol();
            symb->setStyle(QwtSymbol::Ellipse);
            symb->setPen(QPen(Qt::yellow));
            symb->setBrush(QBrush(Qt::yellow));
            symb->setSize(3);
            basis_curves[ii]->setSymbol(symb);
        }
    }
    data_plot->replot();
}

bool US_Spectrum::find_lambda(const DataProfile &data, const double wvl, int &id)
{
    int np = data.lambda.size();
    int ii = id;
    double eps = 0.001;
    while (ii < np)
    {
        double data_wvl = data.lambda.at(ii);
        id = ii;
        if (wvl - data_wvl > eps)
        {
            ii++;
        }
        else if (wvl - data_wvl < -eps)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    id = np;
    return false;
}

void US_Spectrum::overlap()
{
    double min = 0;
    double max = 10000;
    if (wavl_min > 0)
    {
        min = wavl_min;
    }
    if (wavl_max > 0)
    {
        max = wavl_max;
    }
    auto minmax = std::minmax_element(target.lambda.begin(), target.lambda.end());
    min = std::max(min, *minmax.first);
    max = std::min(max, *minmax.second);

    for (int ii = 0; ii < basis_list.size(); ii++)
    {
        auto minmax = std::minmax_element(basis_list.at(ii).lambda.begin(), basis_list.at(ii).lambda.end());
        min = std::max(min, *minmax.first);
        max = std::min(max, *minmax.second);
    }

    int id_tgt = 0;
    QVector<int> id_base(basis_list.size(), 0);
    double wvl = min;
    while (wvl <= max)
    {
        if (find_lambda(target, wvl, id_tgt))
        {
            bool flag = true;
            for (int ii = 0; ii < basis_list.size(); ii++)
            {
                if (!find_lambda(basis_list.at(ii), wvl, id_base[ii]))
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                target.xvec << target.lambda.at(id_tgt);
                target.yvec << target.od.at(id_tgt);
                // qDebug() << "tgt: x:" << target.xvec.last() << " y:" << target.yvec.last();
                for (int ii = 0; ii < basis_list.size(); ii++)
                {
                    int id = id_base.at(ii);
                    basis_list[ii].xvec << basis_list.at(ii).lambda.at(id);
                    basis_list[ii].yvec << basis_list.at(ii).od.at(id);
                    // qDebug() << "base" << ii << ": x:" << all_basis[ii].xvec.last()
                    //  << " y:" << all_basis[ii].yvec.last();
                }
            }
        }
        wvl++;
    }
    data_plot->replot();
}

void US_Spectrum::fit()
{
    clear_fit();
    clear_plot();

    if (target.lambda.isEmpty())
    {
        QMessageBox::warning(this, "Warning!", "Target spectrum is not loaded yet!");
        plot();
        return;
    }

    if (basis_list.size() < 2)
    {
        QMessageBox::warning(this, "Warning!", "At least two basis spectra are needed!");
        plot();
        return;
    }
    overlap();
    if (target.xvec.size() < basis_list.size())
    {
        QString msg = tr("The target and basis spectra do not overlap or are not aligned.\n"
                         "Please ensure that they share common wavelength values.");
        QMessageBox::warning(this, "Warning!", msg);
        clear_fit();
        return;
    }

    wavl_min = target.xvec.first();
    wavl_max = target.xvec.last();
    le_wavl_min->setText(tr("%1").arg(wavl_min));
    le_wavl_max->setText(tr("%1").arg(wavl_max));


    int nwvl = target.xvec.size();
    int order = basis_list.size();
    double nnls_rnorm;
    QVector<double> nnls_a(nwvl * order);
    QVector<double> nnls_b(target.yvec);
    QVector<double> nnls_x(order, 0);

    int nn = 0;
    for (int ii = 0; ii < order; ii++)
    {
        for (int jj = 0; jj < nwvl; jj++)
        {
            nnls_a[nn] = basis_list.at(ii).yvec.at(jj);
            nn++;
        }
    }

    int state = US_Math2::nnls(nnls_a.data(), nwvl, nwvl, order, nnls_b.data(), nnls_x.data(), &nnls_rnorm);

    if (state != 0)
    {
        QMessageBox::critical(this, "Fitting Error!", "NNLS fitting failed!");
        return;
    }

    double totfac = 0;
    for (int ii = 0; ii < order; ii++)
    {
        totfac += nnls_x.at(ii);
    }

    double acc = 0;
    for (int ii = 0; ii < order; ii++)
    {
        basis_list[ii].nnls_factor = nnls_x.at(ii);
        double perc = 100.0 * nnls_x.at(ii) / totfac;
        if (ii < order - 1)
        {
            perc = qRound(perc * 10) / 10.0;
            acc += perc;
        }
        else
        {
            perc = 100 - acc;
        }
        basis_list[ii].nnls_percent = perc;
        tw_basis->item(ii, 2)->setText(QString::number(perc));
    }

    solution.fill(0, nwvl);
    for (int ii = 0; ii < nwvl; ii++)
    {
        for (int jj = 0; jj < order; jj++)
        {
            solution[ii] += basis_list.at(jj).yvec.at(ii) * nnls_x.at(jj);
        }
    }

    double rmsd = 0.0;
    residual.fill(0, nwvl);
    for (int ii = 0; ii < nwvl; ii++)
    {
        double val = solution.at(ii) - target.yvec.at(ii);
        residual[ii] = val;
        rmsd += std::pow(val, 2.0);
    }
    rmsd = std::sqrt(rmsd / nwvl);
    le_rmsd->setText(QString::number(rmsd, 'f', 6));

    plot();
    find_angle();
}

void US_Spectrum::min_wavl_updated()
{
    bool ok;
    double wavl = le_wavl_min->text().toDouble(&ok);
    if (!ok || wavl <= 0)
    {
        le_wavl_min->clear();
        if (wavl_min > 0)
        {
            le_wavl_min->setText(QString::number(wavl_min));
        }
    }
    wavl_min = qRound(wavl);
}

void US_Spectrum::max_wavl_updated()
{
    bool ok;
    double wavl = le_wavl_max->text().toDouble(&ok);
    if (!ok || wavl <= 0)
    {
        le_wavl_max->clear();
        if (wavl_max > 0)
        {
            le_wavl_max->setText(QString::number(wavl_max));
        }
    }
    wavl_max = qRound(wavl);
}

void US_Spectrum::delete_basis(int row)
{
    QString msg = tr("Are you sure you want to delete the curve you double-clicked ?");
    int yes = QMessageBox::question(this, "Delete a Basis Profile", msg, QMessageBox::Yes | QMessageBox::No);
    if (yes == QMessageBox::Yes)
    {
        clear_plot();
        clear_fit();
        basis_list.removeAt(row);
        fill_combo();
        fill_table();
        plot();
    }
}

void US_Spectrum::reset()
{
    clear_plot();
    clear_fit();
    basis_list.clear();
    fill_combo();
    fill_table();
    plot();
}

void US_Spectrum::find_angle()
{
    le_angle->clear();
    int id_1 = cb_basis_1->currentIndex();
    int id_2 = cb_basis_2->currentIndex();
    if (id_1 == -1 || id_2 == -1)
    {
        return;
    }

    QVector<double> vec_1 = basis_list.at(id_1).yvec;
    QVector<double> vec_2 = basis_list.at(id_2).yvec;

    if (vec_1.isEmpty() || vec_2.isEmpty())
    {
        return;
    }

    double dotproduct = 0.0;
    double norm_1 = 0.0;
    double norm_2 = 0.0;

    for (int ii = 0; ii < vec_1.size(); ii++)
    {
        dotproduct += vec_1.at(ii) * vec_2.at(ii);
        norm_1 += std::pow(vec_1.at(ii), 2);
        norm_2 += std::pow(vec_2.at(ii), 2);
    }
    norm_1 = std::sqrt(norm_1);
    norm_2 = std::sqrt(norm_2);
    double angle = dotproduct / (norm_1 * norm_2);
    angle = 180 * std::acos(angle) / M_PI;
    le_angle->setText(QString::number(angle, 'f', 2));
}

void US_Spectrum::save()
{
    if (target.xvec.isEmpty() || solution.isEmpty() || residual.isEmpty())
    {
        QMessageBox::warning(this, "Warning!", "Fitted Data Not Found!");
        return;
    }
    QString basename = QFileDialog::getSaveFileName(this, "CSV Filename", US_Settings::resultDir(), "All Files (*)");
    if (basename.isEmpty())
    {
        return;
    }
    qDebug() << basename;
    if (basename.toLower().endsWith(".csv"))
    {
        basename.chop(4);
    }
    QString csvfile = basename + ".csv";

    QStringList header;
    header << "wavelength (nm)" << target.header << "Fitted Values"
           << "Residuals";
    QStringList titles{"", "(%)", "(factor)"};
    foreach (QString t, titles)
    {
        for (int ii = 0; ii < basis_list.size(); ii++)
        {
            QString h = tr("Base-%1 %2 %3").arg(ii + 1).arg(basis_list.at(ii).header).arg(t);
            header << h.trimmed();
        }
    }

    header << "RMSD";

    QString delimiter = ";";
    for (int ii = 0; ii < header.size(); ii++)
    {
        QString str = header.at(ii);
        header[ii] = str.replace(delimiter, "-");
    }

    QVector<QVector<double>> columns;
    columns << target.xvec;
    columns << target.yvec;
    columns << solution;
    columns << residual;

    for (int ii = 0; ii < 3; ii++)
    {
        for (int jj = 0; jj < basis_list.size(); jj++)
        {
            if (ii == 0)
            {
                columns << basis_list.at(jj).yvec;
            }
            else if (ii == 1)
            {
                QVector<double> v;
                v << basis_list.at(jj).nnls_percent;
                columns << v;
            }
            else if (ii == 2)
            {
                QVector<double> v;
                v << basis_list.at(jj).nnls_factor;
                columns << v;
            }
        }
    }

    QVector<double> rmsd;
    rmsd << le_rmsd->text().toDouble();
    columns << rmsd;

    QFile file(csvfile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream ts(&file);
        QString item;
        int ncols = columns.size();
        int maxrows = columns.first().size();

        for (int ii = -1; ii < maxrows; ii++)
        {
            for (int jj = 0; jj < ncols; jj++)
            {
                if (ii == -1)
                {
                    item = header.at(jj);
                }
                else if (ii < columns.at(jj).size())
                {
                    item = QString::number(columns.at(jj).at(ii));
                }
                else
                {
                    item = "";
                }
                ts << item.trimmed();
                if (jj < ncols - 1)
                    ts << delimiter;
                else
                    ts << "\n";
            }
        }
        file.close();
    }
    else
    {
        QString msg = tr("Cannot open the following file to write!\n%1").arg(csvfile);
    }
}
