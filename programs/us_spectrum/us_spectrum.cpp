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

    QLabel *lb_target = us_banner(tr("Target Spectrum"));
    QLabel *lb_basis = us_banner(tr("Basis Spectra"));
    QLabel *lb_fit = us_banner(tr("Fit & Basis Correlation"));
    QLabel *lb_tgt_fname = us_label(tr("Target Filename"));
    QLabel *lb_tgt_header = us_label(tr("Target Header"));
    QLabel *lb_tgt_minL = us_label(tr("Min. %1").arg(QChar(955)));
    QLabel *lb_tgt_maxL = us_label(tr("Max. %1").arg(QChar(955)));
    QLabel *lb_fit_minL = us_label(tr("Min. %1").arg(QChar(955)));
    QLabel *lb_fit_maxL = us_label(tr("Max. %1").arg(QChar(955)));
    QLabel *lb_basis_list = us_label(tr("Basis List"));
    QLabel *lb_basis_1 = us_label(tr("Basis 1"));
    QLabel *lb_basis_2 = us_label(tr("Basis 2"));
    QLabel *lb_rmsd = us_label(tr("RMSD"));
    QLabel *lb_angle = us_label(tr("Correlation Angle"));

    lb_tgt_header->setAlignment(Qt::AlignCenter);
    lb_basis_list->setAlignment(Qt::AlignCenter);
    lb_tgt_fname->setAlignment(Qt::AlignCenter);
    lb_tgt_minL->setAlignment(Qt::AlignCenter);
    lb_tgt_maxL->setAlignment(Qt::AlignCenter);
    lb_fit_minL->setAlignment(Qt::AlignCenter);
    lb_fit_maxL->setAlignment(Qt::AlignCenter);
    lb_basis_1->setAlignment(Qt::AlignCenter);
    lb_basis_2->setAlignment(Qt::AlignCenter);
    lb_target->setAlignment(Qt::AlignCenter);
    lb_basis->setAlignment(Qt::AlignCenter);
    lb_angle->setAlignment(Qt::AlignCenter);
    lb_rmsd->setAlignment(Qt::AlignCenter);
    lb_fit->setAlignment(Qt::AlignCenter);

    le_tgt_fname = us_lineedit("", 1, true);
    le_tgt_header = us_lineedit("", 1, true);
    le_tgt_minL = us_lineedit("", 1, true);
    le_tgt_maxL = us_lineedit("", 1, true);
    le_fit_minL = us_lineedit("", 1, true);
    le_fit_maxL = us_lineedit("", 1, true);
    le_angle = us_lineedit("", 1, true);
    le_rmsd = us_lineedit("", 1, true);

    QPushButton *pb_target = us_pushbutton(tr("Load Target"));
    QPushButton *pb_basis = us_pushbutton(tr("Add Basis"));
    QPushButton *pb_fit = us_pushbutton(tr("Fit Data"));
    QPushButton *pb_reset = us_pushbutton(tr("Reset"));
    QPushButton *pb_help = us_pushbutton(tr("Help"));
    QPushButton *pb_close = us_pushbutton(tr("Close"));
    QPushButton *pb_save = us_pushbutton(tr("Save Fitting Data"));

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
    left_lyt->addWidget(lb_target, row++, 0, 1, 2);
    left_lyt->addWidget(pb_target, row++, 0, 1, 2);
    left_lyt->addWidget(lb_tgt_fname, row, 0, 1, 1);
    left_lyt->addWidget(le_tgt_fname, row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_header, row, 0, 1, 1);
    left_lyt->addWidget(le_tgt_header, row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_minL, row, 0, 1, 1);
    left_lyt->addWidget(le_tgt_minL, row++, 1, 1, 1);
    left_lyt->addWidget(lb_tgt_maxL, row, 0, 1, 1);
    left_lyt->addWidget(le_tgt_maxL, row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis, row++, 0, 1, 2);
    left_lyt->addWidget(pb_basis, row, 0, 1, 1);
    left_lyt->addWidget(pb_reset, row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_list, row++, 0, 1, 2);
    left_lyt->addWidget(tw_basis, row++, 0, 1, 2);
    left_lyt->addWidget(lb_fit, row++, 0, 1, 2);
    left_lyt->addWidget(pb_fit, row++, 0, 1, 2);
    left_lyt->addWidget(lb_fit_minL, row, 0, 1, 1);
    left_lyt->addWidget(le_fit_minL, row++, 1, 1, 1);
    left_lyt->addWidget(lb_fit_maxL, row, 0, 1, 1);
    left_lyt->addWidget(le_fit_maxL, row++, 1, 1, 1);
    left_lyt->addWidget(lb_rmsd, row, 0, 1, 1);
    left_lyt->addWidget(le_rmsd, row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_1, row, 0, 1, 1);
    left_lyt->addWidget(cb_basis_1, row++, 1, 1, 1);
    left_lyt->addWidget(lb_basis_2, row, 0, 1, 1);
    left_lyt->addWidget(cb_basis_2, row++, 1, 1, 1);
    left_lyt->addWidget(lb_angle, row, 0, 1, 1);
    left_lyt->addWidget(le_angle, row++, 1, 1, 1);
    left_lyt->addWidget(pb_save, row++, 0, 1, 2);
    left_lyt->addWidget(pb_help, row++, 0, 1, 2);
    left_lyt->addWidget(pb_close, row++, 0, 1, 2);

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
    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        all_basis[ii].clear_fit();
        if (ii < nrows)
        {
            tw_basis->item(ii, 2)->text().clear();
        }
    }
    le_angle->clear();
    le_rmsd->clear();
    le_fit_minL->clear();
    le_fit_maxL->clear();
}

void US_Spectrum::clear_plot()
{
    data_plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    error_plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
    basis_curves.clear();
    data_plot->replot();
    error_plot->replot();
}

// brings in the target spectrum according to user specification
void US_Spectrum::load_target()
{
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
    le_tgt_minL->setText(tr("%1 nm").arg(*minmax.first));
    le_tgt_maxL->setText(tr("%1 nm").arg(*minmax.second));

    plot();
}

// loads basis spectra according to user specification
void US_Spectrum::load_basis()
{
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
            all_basis << dp;
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
    int nrows = all_basis.size();
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
        if (all_basis.at(ii).highlight)
        {
            item_0->setCheckState(Qt::Checked);
        }
        else
        {
            item_0->setCheckState(Qt::Unchecked);
        }
        item_0->setText(all_basis.at(ii).header);
        item_0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QTableWidgetItem *item_1 = new QTableWidgetItem();
        item_1->setFlags(Qt::NoItemFlags);
        auto minmax = std::minmax_element(all_basis.at(ii).lambda.begin(), all_basis.at(ii).lambda.end());
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

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        cb_basis_1->addItem(all_basis.at(ii).header);
        cb_basis_2->addItem(all_basis.at(ii).header);
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
    if (checked == all_basis.at(id).highlight)
    {
        all_basis[id].header = item->text();
        fill_combo();
    }
    else
    {
        all_basis[id].highlight = checked;
        highlight();
    }
}

// Takes the information in the basis vector to plot all of the curves for the basis spectrums
void US_Spectrum::plot()
{
    clear_plot();

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        double *xx;
        double *yy;
        int np;
        if (all_basis.at(ii).xvec.isEmpty())
        {
            xx = all_basis[ii].lambda.data();
            yy = all_basis[ii].od.data();
            np = all_basis[ii].od.size();
        }
        else
        {
            xx = all_basis[ii].xvec.data();
            yy = all_basis[ii].yvec.data();
            np = all_basis[ii].yvec.size();
        }

        QwtSymbol *symb = new QwtSymbol();
        symb->setStyle(QwtSymbol::Ellipse);
        symb->setPen(QPen(Qt::yellow));
        symb->setBrush(QBrush(Qt::yellow));
        symb->setSize(3);

        QwtPlotCurve *curve;
        curve = us_curve(data_plot, all_basis.at(ii).header);
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
    }

    us_grid(data_plot);
    data_plot->replot();
    error_plot->replot();
    highlight();
}

void US_Spectrum::highlight()
{
    if (all_basis.size() != basis_curves.size())
    {
        return;
    }
    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        if (all_basis.at(ii).highlight)
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

void US_Spectrum::overlap()
{
    auto minmax = std::minmax_element(target.lambda.begin(), target.lambda.end());
    double min = *minmax.first;
    double max = *minmax.second;

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        auto minmax = std::minmax_element(all_basis.at(ii).lambda.begin(), all_basis.at(ii).lambda.end());
        min = std::max(min, *minmax.first);
        max = std::min(max, *minmax.second);
    }

    int id_tgt = 0;
    QVector<int> id_base(all_basis.size(), 0);
    double wvl = min;
    while (wvl <= max)
    {
        id_tgt = find_lambda(target, id_tgt, wvl);
        if (id_tgt < target.lambda.size())
        {
            bool flag = true;
            for (int ii = 0; ii < all_basis.size(); ii++)
            {
                id_base[ii] = find_lambda(all_basis[ii], id_base.at(ii), wvl);
                if (id_base.at(ii) == all_basis.at(ii).lambda.size())
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
                for (int ii = 0; ii < all_basis.size(); ii++)
                {
                    int id = id_base.at(ii);
                    all_basis[ii].xvec << all_basis.at(ii).lambda.at(id);
                    all_basis[ii].yvec << all_basis.at(ii).od.at(id);
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

    if (all_basis.isEmpty())
    {
        return;
    }

    overlap();
    if (target.xvec.size() < all_basis.size())
    {
        QString msg = tr("The target and basic spectra do not overlap or are not aligned.\n"
                         "Please ensure that they share common wavelength values.");
        QMessageBox::warning(this, "Warning!", msg);
        clear_fit();
        return;
    }

    le_fit_minL->setText(QString::number(target.xvec.first()));
    le_fit_maxL->setText(QString::number(target.xvec.last()));

    int nwvl = target.xvec.size();
    int order = all_basis.size();
    double nnls_rnorm;
    QVector<double> nnls_a(nwvl * order);
    QVector<double> nnls_b(target.yvec);
    QVector<double> nnls_x(order, 0);

    int nn = 0;
    for (int ii = 0; ii < order; ii++)
    {
        for (int jj = 0; jj < nwvl; jj++)
        {
            nnls_a[nn] = all_basis.at(ii).yvec.at(jj);
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

    for (int ii = 0; ii < order; ii++)
    {
        all_basis[ii].nnls_factor = nnls_x.at(ii);
        all_basis[ii].nnls_percent = 100.0 * nnls_x.at(ii) / totfac;
        tw_basis->item(ii, 2)->setText(QString::number(all_basis.at(ii).nnls_percent));
    }

    solution.fill(0, nwvl);
    for (int ii = 0; ii < nwvl; ii++)
    {
        for (int jj = 0; jj < order; jj++)
        {
            solution[ii] += all_basis.at(jj).yvec.at(ii) * nnls_x.at(jj);
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
    le_rmsd->setText(QString::number(rmsd));

    plot();
    find_angle();
}

// Delete upon double click
void US_Spectrum::delete_basis(int row)
{
    QString msg = tr("Are you sure you want to delete the curve you double-clicked ?");
    int yes = QMessageBox::question(this, "Delete a Basis Profile", msg, QMessageBox::Yes | QMessageBox::No);
    if (yes == QMessageBox::Yes)
    {
        clear_plot();
        clear_fit();
        all_basis.removeAt(row);
        fill_combo();
        fill_table();
        plot();
    }
}

void US_Spectrum::reset()
{
    clear_plot();
    clear_fit();
    all_basis.clear();
    fill_combo();
    fill_table();
    plot();
}

int US_Spectrum::find_lambda(DataProfile &data, int id, double wvl)
{
    int np = data.lambda.size();
    int id_wvl = np;
    while (id < np)
    {
        if (qFuzzyCompare(wvl, data.lambda.at(id)))
        {
            id_wvl = id;
            break;
        }
        id++;
    }
    return id_wvl;
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

    QVector<double> vec_1 = all_basis.at(id_1).yvec;
    QVector<double> vec_2 = all_basis.at(id_2).yvec;

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
    le_angle->setText(QString::number(angle));
}

void US_Spectrum::save()
{
    if (target.xvec.isEmpty() || solution.isEmpty() || residual.isEmpty())
    {
        QMessageBox::warning(this, "Warning!", "Fitted Data Not Found!");
        return;
    }
    QString basename = QFileDialog::getSaveFileName(this, "Set the Base Name for the 'CSV' and 'DAT' Files",
                                                    US_Settings::resultDir(), "All Files (*)");
    if (basename.isEmpty())
    {
        return;
    }

    QString csvfile = basename + ".spectrum_fit.csv";
    QString datfile = basename + ".spectrum_fit.dat";

    QVector<QVector<double>> columns;
    QStringList header;
    columns << target.xvec;
    columns << solution;
    columns << residual;
    header << "wavelength (nm)" << "Fitted Extinction" << "Residuals";
    columns << target.yvec;
    header << "Target: " + target.header;
    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        columns << all_basis.at(ii).yvec;
        header << "Base: " + all_basis.at(ii).header;
    }

    QString seprtr = ";";
    bool flag = false;
    for (int ii = 0; ii < header.size(); ii++)
    {
        QString item = header.at(ii);
        if (item.contains(seprtr))
        {
            header[ii] = item.replace(seprtr, "-");
            flag = true;
        }
    }
    US_CSV_Data csv_data;
    if (!csv_data.setData(csvfile, header, columns))
    {
        QMessageBox::warning(this, "Error!", "Error in making the csv data:\n\n" + csv_data.error());
    }
    if (!csv_data.writeFile(seprtr))
    {
        QMessageBox::warning(this, "Error!", "Error in writing the csv file:\n\n" + csv_data.error());
    }
    if (flag)
    {
        QMessageBox::warning(this, "Warning!",
                             "Some headers contained characters identical to the separator (;). "
                             "All such characters have been replaced with (-).");
    }
}
