#include "us_spectrum.h"
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
    tw_basis->setHorizontalHeaderLabels(QStringList{"Header", hl, "%", ""});
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
    connect(pb_save, &QPushButton::clicked, this, &US_Spectrum::save);
    connect(pb_close, &QPushButton::clicked, this, &US_Spectrum::close);
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

void US_Spectrum::DataProfile::clear(bool all)
{
    xvec.clear();
    yvec.clear();
    nnls_factor = -1;
    nnls_percent = -1;
    if (curve != nullptr)
    {
        curve->detach();
        curve = nullptr;
    }
    if (all)
    {
        lambda.clear();
        od.clear();
        header.clear();
        finfo = QFileInfo();
        highlight = false;
    }
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

    QVector<double> lambda(csv_data.columnAt(0));
    QVector<double> od(csv_data.columnAt(1));
    QVector<int> indices = argsort(lambda);
    sort(indices, lambda);
    sort(indices, od);

    target.clear(true);
    target.finfo = QFileInfo(csv_data.filePath());
    target.lambda << lambda;
    target.od << od;
    target.header = csv_data.header().at(1);

    auto minmax = std::minmax_element(target.lambda.begin(), target.lambda.end());
    le_tgt_fname->setText(target.finfo.fileName());
    le_tgt_header->setText(target.header);
    le_tgt_minL->setText(tr("%1 nm").arg(*minmax.first));
    le_tgt_maxL->setText(tr("%1 nm").arg(*minmax.second));

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        all_basis[ii].clear(false);
    }

    plot_target();
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

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        all_basis[ii].clear(false);
    }

    solution.clear(true);
    residual.clear(true);

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

    fill_table();
    fill_combo();
    plot_basis();
}

void US_Spectrum::fill_table()
{
    tw_basis->disconnect();
    int nrows = all_basis.size();
    tw_basis->setRowCount(nrows);
    tw_basis->setColumnCount(4);

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
    tw_basis->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    if (nrows == 0)
    {
        tw_basis->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        tw_basis->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }
    else
    {
        tw_basis->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tw_basis->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    }

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
void US_Spectrum::plot_basis()
{
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
        symb->setPen(QPen(Qt::green));
        symb->setBrush(QBrush(Qt::green));
        symb->setSize(3);

        if (all_basis[ii].curve != nullptr)
        {
            all_basis[ii].curve->detach();
        }
        QwtPlotCurve *curve;
        curve = us_curve(data_plot, all_basis.at(ii).header);
        curve->setSymbol(symb);
        curve->setStyle(QwtPlotCurve::NoCurve);
        curve->setSamples(xx, yy, np);
        all_basis[ii].curve = curve;
    }
    data_plot->replot();
    highlight();
}

void US_Spectrum::highlight()
{
    for (int ii = 0; ii < all_basis.size(); ii++)
    {

        if (all_basis[ii].curve != nullptr)
        {
            if (all_basis.at(ii).highlight)
            {
                all_basis[ii].curve->setStyle(QwtPlotCurve::Lines);
                all_basis[ii].curve->setPen(Qt::green, 3, Qt::SolidLine);
                all_basis[ii].curve->setSymbol(nullptr);
            }
            else
            {
                all_basis[ii].curve->setStyle(QwtPlotCurve::NoCurve);
                QwtSymbol *symb = new QwtSymbol();
                symb->setStyle(QwtSymbol::Ellipse);
                symb->setPen(QPen(Qt::green));
                symb->setBrush(QBrush(Qt::green));
                symb->setSize(3);
                all_basis[ii].curve->setSymbol(symb);
            }
        }
    }
    data_plot->replot();
}

void US_Spectrum::plot_target()
{
    if (target.curve != nullptr)
    {
        target.curve->detach();
    }
    if (target.od.isEmpty() && target.xvec.isEmpty())
    {
        return;
    }
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
    curve->setPen(Qt::yellow, 3, Qt::SolidLine);
    target.curve = curve;
    data_plot->replot();
}

void US_Spectrum::fit()
{
    // unsigned int min_lambda = w_target.lambda_min;
    // unsigned int max_lambda = w_target.lambda_max;
    // unsigned int points, order, i, k, counter=0;
    // double *nnls_a, *nnls_b, *nnls_x, nnls_rnorm, *nnls_wp, *nnls_zzp, *x, *y;
    // float fval = 0.0;
    // QVector <float> solution, b;
    // QPen pen;
    // residuals.clear();
    // solution.clear();

    // b.clear();
    // int *nnls_indexp;
    QString str = "Please note:\n\n"
                  "The target and basic spectra have different limits.\n"
                  "These vectors need to be congruent before you can fit\n"
                  "the data. You can correct the problem by running\n"
                  "\"Find Extinction Profile Overlap\".";

    // for (i=0; i< (unsigned int) v_basis.size(); i++)
    // {
    //   if(v_basis[i].lambda_min != min_lambda || v_basis[i].lambda_max != max_lambda || v_basis[i].wvl.size() !=
    //   w_target.wvl.size())
    //    {
    //        QMessageBox::warning(this, tr("UltraScan Warning"), str );
    //        return;
    //    }
    // }

    // points = w_target.lambda_max - w_target.lambda_min + 1;

    // points = w_target.wvl.size();
    // x = new double [points];
    // y = new double [points];

    // order = v_basis.size(); // no baseline necessary with gaussians
    // nnls_a = new double [points * order]; // contains the model functions, end-to-end
    // nnls_b = new double [points]; // contains the experimental data
    // nnls_zzp = new double [points]; // pre-allocated working space for nnls
    // nnls_x = new double [order]; // the solution vector, pre-allocated for nnls
    // nnls_wp = new double [order]; // pre-allocated working space for nnls, On exit, wp[] will contain the dual
    // solution vector, wp[i]=0.0 for all i in set p and wp[i]<=0.0 for all i in set z.

    // nnls_indexp = new int [order];
    // //extinction
    // for (i=0; i<points; i++)
    // {
    //   x[i] = w_target.wvl[i];

    //   nnls_b[i] = w_target.extinction[i];
    //   b.push_back((float) nnls_b[i]);
    // }

    // counter = 0;
    // //basis
    // for (k=0; k<order; k++)
    // {
    //   for(i = 0; i<points; i++)
    //    {
    // nnls_a[counter] = v_basis[k].extinction[i];
    // counter ++;
    //    }
    // }

    // US_Math2::nnls(nnls_a, points, points, order, nnls_b, nnls_x, &nnls_rnorm, nnls_wp, nnls_zzp, nnls_indexp);

    // QVector <float> results;
    // results.clear();
    // fval = 0.0;
    // for (i=0; i< (unsigned int) v_basis.size(); i++)
    // {
    //    fval += nnls_x[i];
    // }
    // str = tr ("%1 : %2\% (%3)");
    // for (i=0; i< (unsigned int) v_basis.size(); i++)
    // {
    //    results.push_back(100.0 * nnls_x[i]/fval);
    //    // str = QString::asprintf( (v_basis[i].filenameBasis +": %3.2f%% (%6.4e)").toLocal8Bit().data(), results[i],
    //    nnls_x[i] );
    //    // lw_basis->item((int)i)->setText(str);
    //    // tw_basis->item((int)i)->setText(str.arg(v_basis[i].header).
    //    //                                  arg(results.at(i), 0, 'f', 2).arg(nnls_x[i], 0, 'e'));
    //    v_basis[i].nnls_factor = nnls_x[i];
    //    v_basis[i].nnls_percentage = results[i];
    // }

    // for (i=0; i<points; i++)
    // {
    //    solution.push_back(0.0);
    // }

    // // Solution
    // for (k=0; k<order; k++)
    // {
    //   for (i=0; i<points; i++)
    //    {
    //    	solution[i] += v_basis[k].extinction[i] * nnls_x[k];
    //    }
    //  }

    // for (i=0; i<points; i++)
    // {
    //    residuals.push_back(solution[i] - b[i]);
    //    y[i] = solution[i];
    // }

    // dataPlotClear( error_plot );
    // QwtPlotCurve *resid_curve = us_curve(error_plot, "Residuals");
    // QwtPlotCurve *target_curve = us_curve(error_plot,"Mean");
    // if (solution_curve != NULL)
    // {
    //    solution_curve->detach();
    // }
    // solution_curve = us_curve(data_plot, "Solution");

    // resid_curve->setStyle(QwtPlotCurve::Lines);
    // target_curve->setStyle(QwtPlotCurve::Lines);
    // solution_curve->setStyle(QwtPlotCurve::Lines);

    // solution_curve->setSamples(x, y, points);
    // pen.setColor(Qt::magenta);
    // pen.setWidth(3);
    // solution_curve->setPen(pen);
    // //Update w_solution's profile
    // w_solution.matchingCurve = solution_curve;
    // data_plot->replot();

    // for(unsigned int j = 0; j < points; j++)
    // {
    //   w_solution.wvl.push_back(x[j]);
    //   w_solution.extinction.push_back(y[j]);
    // }
    // w_solution.lambda_min = w_target.lambda_min;
    // w_solution.lambda_max = w_target.lambda_max;
    // fval = 0.0;
    // for (i=0; i<points; i++)
    // {
    //    y[i] = residuals[i];
    //    fval += pow(residuals[i], (float) 2.0);
    // }
    // fval /= points;
    // str = tr (" %1");
    // le_rmsd->setText(str.arg(pow(fval, (float) 0.5), 0, 'e'));
    // // le_rmsd->setText(QString::asprintf(" %3.2e", pow(fval, (float) 0.5)));
    // resid_curve->setSamples(x, y, points);
    // pen.setColor(Qt::yellow);
    // pen.setWidth(2);
    // resid_curve->setPen(pen);
    // error_plot->replot();

    // x[1] = x[points - 1];
    // y[0] = 0.0;
    // y[1] = 0.0;
    // target_curve->setSamples(x, y, 2);
    // pen.setColor(Qt::red);
    // pen.setWidth(3);
    // target_curve->setPen(pen);
    // error_plot->replot();
    // // pb_save->setEnabled(true);
    // delete [] x;
    // delete [] y;
}

// Delete upon double click
void US_Spectrum::delete_basis(int row)
{
    qDebug() << row;
    QString msg = tr("Are you sure you want to delete the curve you double-clicked ?");
    int yes = QMessageBox::question(this, "Delete a Basis Profile", msg, QMessageBox::Yes | QMessageBox::No);
    if (yes == QMessageBox::Yes)
    {
        for (int ii = 0; ii < all_basis.size(); ii++)
        {
            if (ii == row)
            {
                all_basis[ii].clear(true);
            }
            else
            {
                all_basis[ii].clear(false);
            }
        }
        all_basis.removeAt(row);
        tw_basis->removeRow(row);
        plot_basis();
    }
}

void US_Spectrum::reset()
{
    target.clear(false);
    solution.clear(true);
    residual.clear(true);

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        all_basis[ii].clear(true);
    }

    all_basis.clear();
    fill_table();
    plot_basis();
}

void US_Spectrum::trim_data_profile(DataProfile &data, double min, double max)
{
    data.clear(false);
    int id = 0;
    int np = data.lambda.size();
    QVector<double> xvec, yvec;
    while (!qFuzzyCompare(data.lambda.at(id), min))
    {
        id++;
        if (id >= np)
        {
            return;
        }
    }
    double wvl = min;
    while (true)
    {
        if (id >= np)
        {
            break;
        }
        if (qFuzzyCompare(wvl, data.lambda.at(id)))
        {
            xvec << data.lambda.at(id);
            yvec << data.od.at(id);
            wvl++;
            if (std::abs(wvl - max) > 0.1)
            {
                break;
            }
        }
        id++;
    }
    data.xvec << xvec;
    data.yvec << yvec;
}

void US_Spectrum::find_overlap()
{
    target.clear(false);
    solution.clear(true);
    residual.clear(true);

    if (all_basis.isEmpty())
    {
        return;
    }

    auto minmax = std::minmax_element(target.lambda.begin(), target.lambda.end());
    double max_of_min = *minmax.first;
    double min_of_max = *minmax.second;

    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        auto minmax = std::minmax_element(all_basis.at(ii).lambda.begin(), all_basis.at(ii).lambda.end());
        max_of_min = std::max(min_of_max, *minmax.first);
        min_of_max = std::min(max_of_min, *minmax.second);
    }

    trim_data_profile(target, max_of_min, min_of_max);
    for (int ii = 0; ii < all_basis.size(); ii++)
    {
        trim_data_profile(all_basis[ii], max_of_min, min_of_max);
    }

    int np_tgt = target.xvec.size();
    
}

void US_Spectrum::find_angle()
{
    le_angle->clear();
    int id_1 = cb_basis_1->currentIndex();
    int id_2 = cb_basis_2->currentIndex();
    if (id_1 == 0 || id_2 == 0)
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
    if (target.xvec.isEmpty() || solution.xvec.isEmpty() || residual.xvec.isEmpty())
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
    columns << solution.xvec;
    columns << solution.yvec;
    columns << residual.yvec;
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
