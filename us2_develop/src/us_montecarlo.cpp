#include "../include/us_montecarlo.h"

US_MonteCarlo::US_MonteCarlo(struct MonteCarlo *temp_mc, bool *temp_mc_widget, QWidget *parent, const char *name) 
   : QFrame(parent, name, true)
{
   plot_status = "parameter";
   mc = temp_mc;
   mc_widget = temp_mc_widget;
   *mc_widget = true;   
   USglobal = new US_Config();
   pm = new US_Pixmap();
   stats.points = 0;
   QString str;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   border = 4;
   int xpos = border;
   int ypos = border;
   spacing = 2;
   int buttonh = 26;
   column1 = 125;
   column2 = 125;
   int column3 = 125;
   int column4 = 125;
   int buttonw = 125;
   int column5 = 40;
   int column6 = 150;
   int column7 = 40;
   int span = column1 + column2 + column3 + column4 + column5 + column6 + column7 + 6 * spacing;
   bins = 50;
   print_plot = false;      // show colors in plots, only b/w for printing
   stats_widget = false;   //   is the statistics  widget visible or active?
   beowulf_widget = false;   //   is the beowulf  widget visible or active?
   file_read = false;      // flag decides if update_parameter has to be called or not 
   file_ok = false;         // if we are above 20 iterations and the file exists, the file is OK, also decides if file_read is set
   from_public = false;      // don't send error messages when the iterations are below 20 and updated from an external routine
   dont_show = false;      // don't show the gui stuff when writing the file

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Monte Carlo Control Window"));

   lbl_banner1 = new QLabel(tr("Monte Carlo Control Window"), this);
   lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setGeometry(xpos, ypos, span, buttonh);
   lbl_banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   ypos += buttonh + 3 * spacing;

   pb_filename = new QPushButton(tr("Output File:"), this);
   Q_CHECK_PTR(pb_filename);
   pb_filename->setAutoDefault(false);
   pb_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_filename->setGeometry(xpos, ypos, column1, buttonh);
   connect(pb_filename, SIGNAL(clicked()), SLOT(select_file()));

   xpos += column1 + spacing;

   le_filename = new QLineEdit(this, "File Name dialog");
   le_filename->setGeometry(xpos, ypos, column2 + column3 + spacing, buttonh);
   le_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_filename->setText(" " + (*mc).filename);
   connect(le_filename, SIGNAL(textChanged(const QString &)), SLOT(update_file(const QString &)));   

   xpos += column2 + column3 + 2 * spacing;

   lbl_new = new QLabel(tr(" Overwrite:"), this);
   Q_CHECK_PTR(lbl_new);
   lbl_new->setAlignment(AlignLeft|AlignVCenter);
   lbl_new->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_new->setGeometry(xpos, ypos, column4, buttonh);
   lbl_new->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column4 + spacing;

   cb_new = new QCheckBox(this);
   Q_CHECK_PTR(cb_new);
   cb_new->setGeometry(xpos + (unsigned int) (column5 / 2) - 7, ypos+5, 14, 14);
   cb_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_new->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).append)
   {
      cb_new->setChecked(false);
   }
   else
   {
      cb_new->setChecked(true);
   }
   connect(cb_new, SIGNAL(clicked()), SLOT(set_new()));

   xpos += column5 + spacing;

   lbl_append = new QLabel(tr(" Append:"), this);
   Q_CHECK_PTR(lbl_append);
   lbl_append->setAlignment(AlignLeft|AlignVCenter);
   lbl_append->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_append->setGeometry(xpos, ypos, column6, buttonh);
   lbl_append->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column6 + spacing;

   cb_append = new QCheckBox(this);
   Q_CHECK_PTR(cb_append);
   cb_append->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_append->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_append->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).append)
   {
      cb_append->setChecked(true);
   }
   else
   {
      cb_append->setChecked(false);
   }
   connect(cb_append, SIGNAL(clicked()), SLOT(set_append()));

   xpos = border;
   ypos += buttonh + 3 * spacing;

   lbl_banner2 = new QLabel(tr("Monte Carlo Data Type:"), this);
   lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner2->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner2->setGeometry(xpos, ypos, span, buttonh);
   lbl_banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + 3 * spacing;

   lbl_bootstrap = new QLabel(tr(" Bootstrap:"), this);
   Q_CHECK_PTR(lbl_bootstrap);
   lbl_bootstrap->setAlignment(AlignLeft|AlignVCenter);
   lbl_bootstrap->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bootstrap->setGeometry(xpos, ypos, column1, buttonh);
   lbl_bootstrap->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   cb_bootstrap = new QCheckBox(this);
   Q_CHECK_PTR(cb_bootstrap);
   cb_bootstrap->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_bootstrap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_bootstrap->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).data_type == 1)
   {
      cb_bootstrap->setChecked(true);
   }
   else
   {
      cb_bootstrap->setChecked(false);
   }
   connect(cb_bootstrap, SIGNAL(clicked()), SLOT(set_bootstrap()));

   xpos += column7 + spacing;

   cnt_percent_bootstrap= new QwtCounter(this);
   Q_CHECK_PTR(cnt_percent_bootstrap);
   cnt_percent_bootstrap->setRange(1, 100, 1);
   cnt_percent_bootstrap->setNumButtons(2);
   cnt_percent_bootstrap->setValue((*mc).percent_bootstrap);
   cnt_percent_bootstrap->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_percent_bootstrap->setGeometry(xpos, ypos-1, column2 + column3 - column7, buttonh - 2);
   // connect(cnt_percent_bootstrap, SIGNAL(buttonReleased(double)), SLOT(update_percent_bootstrap(double)));
   connect(cnt_percent_bootstrap, SIGNAL(valueChanged(double)), SLOT(update_percent_bootstrap(double)));

   xpos = border + column1 + column2 + column3 + 3 * spacing;

   lbl_percent_bootstrap = new QLabel(tr("% of Input Data"), this);
   lbl_percent_bootstrap->setAlignment(AlignLeft|AlignVCenter);
   lbl_percent_bootstrap->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_percent_bootstrap->setGeometry(xpos, ypos, column4, buttonh);
   lbl_percent_bootstrap->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   xpos += column4 + column5 + 2 * spacing;

   lb_rules = new QListBox(this, "Rules");
   lb_rules->setGeometry(xpos, ypos, column6 + column7 + spacing, 3 * buttonh + 2 * spacing);
   lb_rules->setSelected((*mc).rule, true);
   lb_rules->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_rules->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_rules->insertItem(tr("SD of Point Rule"));            // rule 0
   lb_rules->insertItem(tr("SD of Run or Point Rule"));   // rule 1
   lb_rules->insertItem(tr("SD of Run Rule"));             // rule 2
   lb_rules->insertItem(tr("SD of 5 Point Average"));      // rule 3
   lb_rules->insertItem(tr("SD of 10 Point Average"));      // etc...
   lb_rules->insertItem(tr("SD of 15 Point Average"));
   lb_rules->insertItem(tr("SD of 20 Point Average"));
   lb_rules->insertItem(tr("SD of 25 Point Average"));
   lb_rules->insertItem(tr("SD of 30 Point Average"));
   lb_rules->setCurrentItem((*mc).rule);
   connect(lb_rules, SIGNAL(selected(int)), SLOT(update_rule(int)));
   connect(lb_rules, SIGNAL(highlighted(int)), SLOT(update_rule(int)));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_gaussian = new QLabel(tr(" Gaussian:"), this);
   Q_CHECK_PTR(lbl_gaussian);
   lbl_gaussian->setAlignment(AlignLeft|AlignVCenter);
   lbl_gaussian->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_gaussian->setGeometry(xpos, ypos, column1, buttonh);
   lbl_gaussian->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   cb_gaussian = new QCheckBox(this);
   Q_CHECK_PTR(cb_gaussian);
   cb_gaussian->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_gaussian->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).data_type == 0)
   {
      cb_gaussian->setChecked(true);
   }
   else
   {
      cb_gaussian->setChecked(false);
   }
   connect(cb_gaussian, SIGNAL(clicked()), SLOT(set_gaussian()));

   xpos += column7 + spacing;

   cnt_percent_gaussian= new QwtCounter(this);
   Q_CHECK_PTR(cnt_percent_gaussian);
   cnt_percent_gaussian->setRange(1, 99, 1);
   cnt_percent_gaussian->setNumButtons(2);
   cnt_percent_gaussian->setValue((*mc).percent_gaussian);
   cnt_percent_gaussian->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_percent_gaussian->setGeometry(xpos, ypos+1, column2 + column3 - column7, buttonh-2);
   // connect(cnt_percent_gaussian, SIGNAL(buttonReleased(double)), SLOT(update_percent_gaussian(double)));
   connect(cnt_percent_gaussian, SIGNAL(valueChanged(double)), SLOT(update_percent_gaussian(double)));


   xpos = border + column1 + column2 + column3 + 3 * spacing;

   lbl_percent_gaussian = new QLabel(tr("% of Normal Gaussian"), this);
   lbl_percent_gaussian->setAlignment(AlignLeft|AlignVCenter);
   lbl_percent_gaussian->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_percent_gaussian->setGeometry(xpos, ypos+1, column1+30, buttonh-2);
   lbl_percent_gaussian->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   xpos = border;
   ypos += buttonh + spacing;
   plot_start = ypos + spacing;

   lbl_mixed = new QLabel(tr(" Mixed:"), this);
   Q_CHECK_PTR(lbl_mixed);
   lbl_mixed->setAlignment(AlignLeft|AlignVCenter);
   lbl_mixed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mixed->setGeometry(xpos, ypos, column1, buttonh);
   lbl_mixed->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   cb_mixed = new QCheckBox(this);
   Q_CHECK_PTR(cb_mixed);
   cb_mixed->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_mixed->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_mixed->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).data_type == 2)
   {
      cb_mixed->setChecked(true);
   }
   else
   {
      cb_mixed->setChecked(false);
   }
   connect(cb_mixed, SIGNAL(clicked()), SLOT(set_mixed()));

   xpos += column7 + spacing; 

   lbl_ignoreVariance = new QLabel(tr(" Ignore Runs with Variance above: "), this);
   Q_CHECK_PTR(lbl_ignoreVariance);
   lbl_ignoreVariance->setAlignment(AlignLeft|AlignVCenter);
   lbl_ignoreVariance->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ignoreVariance->setGeometry(xpos, ypos, column2 + column3 - column7, buttonh);
   lbl_ignoreVariance->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos = border + column1 + column2 + column3 + 3 * spacing;

   str.sprintf("%2.4e", (*mc).varianceThreshold);
   le_ignoreVariance = new QLineEdit(this, "Variance Threshold");
   le_ignoreVariance->setGeometry(xpos, ypos, column4, buttonh);
   le_ignoreVariance->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_ignoreVariance->setText(str);
   connect(le_ignoreVariance, SIGNAL(textChanged(const QString &)), SLOT(update_ignoreVariance(const QString &)));   

   xpos = border;
   ypos += buttonh + 2 * spacing;

   lbl_guesses = new QLabel(tr("Settings for Initial Parameter Guesses:"), this);
   Q_CHECK_PTR(lbl_guesses);
   lbl_guesses->setAlignment(AlignCenter|AlignVCenter);
   lbl_guesses->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_guesses->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_guesses->setGeometry(xpos, ypos, span, buttonh);
   lbl_guesses->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + 2 * spacing;

   lbl_originalFit = new QLabel(tr(" From Original Fit:"), this);
   Q_CHECK_PTR(lbl_originalFit);
   lbl_originalFit->setAlignment(AlignLeft|AlignVCenter);
   lbl_originalFit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_originalFit->setGeometry(xpos, ypos, column1, buttonh);
   lbl_originalFit->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   cb_originalFit = new QCheckBox(this);
   Q_CHECK_PTR(cb_originalFit);
   cb_originalFit->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_originalFit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_originalFit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).addNoise == 2)
   {
      cb_originalFit->setChecked(true);
   }
   else
   {
      cb_originalFit->setChecked(false);
   }
   connect(cb_originalFit, SIGNAL(clicked()), SLOT(set_originalFit()));

   xpos += column7 + spacing;

   lbl_lastFit = new QLabel(tr(" From Previous Fit:"), this);
   Q_CHECK_PTR(lbl_lastFit);
   lbl_lastFit->setAlignment(AlignLeft|AlignVCenter);
   lbl_lastFit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_lastFit->setGeometry(xpos, ypos, column4, buttonh);
   lbl_lastFit->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column4 + spacing;

   cb_lastFit = new QCheckBox(this);
   Q_CHECK_PTR(cb_lastFit);
   cb_lastFit->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_lastFit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_lastFit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).addNoise == 0)
   {
      cb_lastFit->setChecked(true);
   }
   else
   {
      cb_lastFit->setChecked(false);
   }
   connect(cb_lastFit, SIGNAL(clicked()), SLOT(set_lastFit()));

   xpos += column7 + spacing;

   lbl_random = new QLabel(tr(" With Added Noise:"), this);
   Q_CHECK_PTR(lbl_random);
   lbl_random->setAlignment(AlignLeft|AlignVCenter);
   lbl_random->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_random->setGeometry(xpos, ypos, column4, buttonh);
   lbl_random->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column4 + spacing;

   cb_random = new QCheckBox(this);
   Q_CHECK_PTR(cb_random);
   cb_random->setGeometry(xpos + (unsigned int) (column7 / 2) - 7, ypos+5, 14, 14);
   cb_random->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   cb_random->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   if ((*mc).addNoise == 1)
   {
      cb_random->setChecked(true);
   }
   else
   {
      cb_random->setChecked(false);
   }
   connect(cb_random, SIGNAL(clicked()), SLOT(set_random()));

   xpos += column7 + spacing;

   str.sprintf("%3.2f", (*mc).parameterNoise);
   le_noiseLevel = new QLineEdit(this, "Random Noise");
   le_noiseLevel->setGeometry(xpos, ypos, 60, buttonh);
   le_noiseLevel->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_noiseLevel->setText(str);
   connect(le_noiseLevel, SIGNAL(textChanged(const QString &)), SLOT(update_noiseLevel(const QString &)));   

   xpos += 60 + spacing;

   lbl_percent = new QLabel("%", this);
   Q_CHECK_PTR(lbl_percent);
   lbl_percent->setAlignment(AlignCenter|AlignVCenter);
   lbl_percent->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_percent->setGeometry(xpos, ypos, 20, buttonh);
   lbl_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos += 26 + spacing;

   pb_beowulf = new QPushButton(tr("Beowulf Control"), this);
   Q_CHECK_PTR(pb_beowulf);
   pb_beowulf->setAutoDefault(false);
   pb_beowulf->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_beowulf->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_beowulf->setGeometry(xpos, ypos, buttonw+20, buttonh);
#ifdef WIN32
   pb_beowulf->setEnabled(false);
#endif
#ifdef UNIX
   connect(pb_beowulf, SIGNAL(clicked()), SLOT(beowulf()));
#endif
   xpos = border;
   ypos += buttonh + spacing;
   plot_start = ypos + spacing;

   lbl_iterations = new QLabel(tr(" Total Iterations:"), this);
   Q_CHECK_PTR(lbl_iterations);
   lbl_iterations->setAlignment(AlignLeft|AlignVCenter);
   lbl_iterations->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_iterations->setGeometry(xpos, ypos, column1, buttonh);
   lbl_iterations->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   str.sprintf("%d", (*mc).iterations);
   le_iterations = new QLineEdit(this, "File Name dialog");
   le_iterations->setGeometry(xpos, ypos, column2, buttonh);
   le_iterations->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_iterations->setText(str);
   connect(le_iterations, SIGNAL(textChanged(const QString &)), SLOT(update_total_iterations(const QString &)));   

   ypos += buttonh + spacing;
   xpos = border;

   lbl_current_iteration1 = new QLabel(tr(" Current Iteration:"), this);
   Q_CHECK_PTR(lbl_current_iteration1);
   lbl_current_iteration1->setAlignment(AlignLeft|AlignVCenter);
   lbl_current_iteration1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_current_iteration1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_current_iteration1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   lbl_current_iteration2 = new QLabel(" 0", this);
   Q_CHECK_PTR(lbl_current_iteration2);
   lbl_current_iteration2->setAlignment(AlignLeft|AlignVCenter);
   lbl_current_iteration2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_current_iteration2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_current_iteration2->setGeometry(xpos, ypos, column2, buttonh);

   ypos += buttonh + spacing;
   xpos = border;

   lbl_seed = new QLabel(tr(" Random Seed:"), this);
   Q_CHECK_PTR(lbl_seed);
   lbl_seed->setAlignment(AlignLeft|AlignVCenter);
   lbl_seed->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_seed->setGeometry(xpos, ypos, column1, buttonh);
   lbl_seed->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   str.sprintf("%d", (*mc).random_seed);
   le_seed = new QLineEdit(this, "Random Seed");
   le_seed->setGeometry(xpos, ypos, column2, buttonh);
   le_seed->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_seed->setText(str);
   connect(le_seed, SIGNAL(textChanged(const QString &)), SLOT(select_seed(const QString &)));   

   xpos = border;
   ypos += buttonh + 3 * spacing;

   lbl_banner3 = new QLabel(tr("Parameter Information:\n(double - click to select)"), this);
   lbl_banner3->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner3->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner3->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner3->setGeometry(xpos, ypos, column1 + column2 + spacing, 2*buttonh + spacing);
   lbl_banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += 2 * buttonh + 4 * spacing;

   lb_parameters = new QListBox(this, "Datasets");
   lb_parameters->setGeometry(xpos, ypos, column1 + column2 + spacing, 3 * buttonh);
   lb_parameters->setSelected(0, true);
   lb_parameters->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_parameters->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   for (unsigned int i=0; i<(*mc).parameter.size(); i++)
   {
      lb_parameters->insertItem((*mc).parameter[i]);
   }
   lb_parameters->setSelected(0, true);
   lb_parameters->setCurrentItem(0);
   connect(lb_parameters, SIGNAL(selected(int)), SLOT(show_parameter(int)));

   ypos += 3 * buttonh + 3 * spacing;
   xpos = border;

   lbl_mean1 = new QLabel(tr(" Mean Value:"), this);
   Q_CHECK_PTR(lbl_mean1);
   lbl_mean1->setAlignment(AlignLeft|AlignVCenter);
   lbl_mean1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mean1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_mean1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   lbl_mean2 = new QLabel(" 0", this);
   Q_CHECK_PTR(lbl_mean2);
   lbl_mean2->setAlignment(AlignLeft|AlignVCenter);
   lbl_mean2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_mean2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_mean2->setGeometry(xpos, ypos, column2, buttonh);

   ypos += buttonh + spacing;
   xpos = border;

   lbl_bins = new QLabel(tr(" Number of Bins:"), this);
   Q_CHECK_PTR(lbl_bins);
   lbl_bins->setAlignment(AlignLeft|AlignVCenter);
   lbl_bins->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bins->setGeometry(xpos, ypos, column1, buttonh);
   lbl_bins->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   str.sprintf("%d", 50);
   le_bins = new QLineEdit(this, "Bins");
   le_bins->setGeometry(xpos, ypos, column2, buttonh);
   le_bins->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_bins->setText(str);
   connect(le_bins, SIGNAL(textChanged(const QString &)), SLOT(update_bins(const QString &)));   
   connect(le_bins, SIGNAL(returnPressed()), SLOT(show_parameter()));   

   ypos += buttonh + spacing;
   xpos = border;

   lbl_entries1 = new QLabel(tr(" # of Data Points:"), this);
   Q_CHECK_PTR(lbl_entries1);
   lbl_entries1->setAlignment(AlignLeft|AlignVCenter);
   lbl_entries1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_entries1->setGeometry(xpos, ypos, column1, buttonh);
   lbl_entries1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   xpos += column1 + spacing;

   lbl_entries2 = new QLabel(" 0", this);
   Q_CHECK_PTR(lbl_entries2);
   lbl_entries2->setAlignment(AlignLeft|AlignVCenter);
   lbl_entries2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_entries2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_entries2->setGeometry(xpos, ypos, column2, buttonh);

   ypos += buttonh + spacing;
   xpos = border;

   pb_update = new QPushButton(tr("Update Parameter"), this);
   Q_CHECK_PTR(pb_update);
   pb_update->setAutoDefault(false);
   pb_update->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_update->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_update->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_update, SIGNAL(clicked()), SLOT(update_parameter()));

   xpos += buttonw + spacing;

   pb_statistics = new QPushButton(tr("Statistics"), this);
   Q_CHECK_PTR(pb_statistics);
   pb_statistics->setAutoDefault(false);
   pb_statistics->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_statistics->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_statistics->setEnabled(false);
   pb_statistics->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_statistics, SIGNAL(clicked()), SLOT(statistics()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_print = new QPushButton(tr("Print Histogram"), this);
   Q_CHECK_PTR(pb_print);
   pb_print->setAutoDefault(false);
   pb_print->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_print->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_print->setGeometry(xpos, ypos, buttonw, buttonh);
   pb_print->setEnabled(false);
   connect(pb_print, SIGNAL(clicked()), SLOT(print()));

   xpos += buttonw + spacing;

   pb_save = new QPushButton(tr("Save Data"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_save->setEnabled(false);
   pb_save->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_start = new QPushButton("Start", this);
   Q_CHECK_PTR(pb_start);
   pb_start->setAutoDefault(false);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_start->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   xpos += buttonw + spacing;

   pb_stop = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setAutoDefault(false);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_stop->setEnabled(false);
   pb_stop->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   ypos += buttonh + spacing;
   xpos = border;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += buttonw + spacing;

   pb_quit = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_quit);
   pb_quit->setAutoDefault(false);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_quit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_quit->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   ypos += buttonh + spacing;
   xpos = border;

   lbl_status = new QLabel(tr("  Status:"),this);
   lbl_status->setAlignment(AlignLeft|AlignVCenter);
   lbl_status->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_status->setGeometry(xpos, ypos, 65, buttonh);
   lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   progress = new QProgressBar(this, "Loading Progress");
   progress->setGeometry(xpos+65, ypos, 2*buttonw+spacing-65, buttonh);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   progress->setTotalSteps((*mc).iterations);
   progress->reset();
   progress->setProgress(0);

   ypos += buttonh + spacing + border;
   xpos = border;

   histogram_plot = new QwtPlot(this);
   Q_CHECK_PTR(histogram_plot);
   histogram_plot->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   histogram_plot->enableGridXMin();
   histogram_plot->enableGridYMin();
   histogram_plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   histogram_plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   histogram_plot->setTitle(tr("Histogram"));
   //histogram_plot->setPlotBackground(USglobal->global_colors.plot);      //old version
   histogram_plot->setCanvasBackground(USglobal->global_colors.plot);      //new version
   histogram_plot->setMargin(USglobal->config_list.margin);
   histogram_plot->enableOutline(true);
   histogram_plot->setAxisTitle(QwtPlot::xBottom, tr("Bins"));
   histogram_plot->setAxisTitle(QwtPlot::yLeft, tr("Frequency"));
   histogram_plot->setOutlinePen(white);
   histogram_plot->setOutlineStyle(Qwt::Rect);
   histogram_plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   histogram_plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   histogram_plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   histogram_plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   histogram_plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));         
   
   connect(histogram_plot, SIGNAL(plotMousePressed(const QMouseEvent &)),
           SLOT(plotMousePressed( const QMouseEvent&)));
   connect(histogram_plot, SIGNAL(plotMouseReleased(const QMouseEvent &)),
           SLOT(plotMouseReleased( const QMouseEvent&)));

   global_Xpos += 30;
   global_Ypos += 30;

   setMinimumSize(span + 2 * border, ypos);
   setGeometry(global_Xpos, global_Ypos, span + 2 * border, ypos);
}

US_MonteCarlo::~US_MonteCarlo()
{
}

void US_MonteCarlo::resizeEvent(QResizeEvent *e)
{
   int dialogw = 250 + spacing + 2 * border;
   histogram_plot->setGeometry(dialogw, plot_start, e->size().width()-dialogw - border, e->size().height() - border - plot_start);
}

void US_MonteCarlo::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   *mc_widget = false;
   if (stats_widget)
   {
      stats_widget = false;
      stats_window->close();
   }
#ifdef UNIX
   if (beowulf_widget)
   {
      beowulf_widget = false;
      beowulf_W->close();
   }
#endif
   e->accept();
}

#ifdef WIN32
void US_MonteCarlo::beowulf()
{
}
#endif

#ifdef UNIX
void US_MonteCarlo::beowulf()
{
   QString str=USglobal->config_list.root_dir + "/beowulf.start";
   QFile f(str);
   if(f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f);
      ts << (*mc).fitName << endl;
      ts << (*mc).data_type << endl; // 0=gaussian, 1=bootstrap, 2=mixed 
      ts << (*mc).iterations << endl;
      ts << (*mc).filename << endl;
      ts << (*mc).append << endl;
      ts << (*mc).status << endl;      // true = run, false = stop
      ts << (*mc).addNoise << endl;
      ts << (*mc).percent_bootstrap << endl;
      ts << (*mc).percent_gaussian << endl;
      ts << (*mc).rule << endl;
      ts << (*mc).varianceThreshold << endl;
      ts << (*mc).parameterNoise << endl;
      ts << (*mc).run_id << endl;
      ts << (*mc).parameters << endl;
      for (unsigned int i=0; i<(*mc).parameter.size(); i++)
      {
         ts << (*mc).parameter[i] << endl;
      }
      f.close();
   }
   else
   {
      QMessageBox::message(tr("Attention:"), tr("Unable to write the Beowulf Start File:\n\n")
                           + str + tr("\n\nPlease make sure you have write permission."));
   }
   
   beowulf_W = new US_Beowulf((*mc).filename, (*mc).experiment, &beowulf_widget);
   beowulf_W->show();
}
#endif

void US_MonteCarlo::set_new()
{
   (*mc).append = false;
   cb_new->setChecked(true);
   cb_append->setChecked(false);
}

void US_MonteCarlo::set_append()
{
   (*mc).append = true;
   cb_new->setChecked(false);
   cb_append->setChecked(true);
}

void US_MonteCarlo::select_file()
{
   QString fn = QFileDialog::getSaveFileName( USglobal->config_list.result_dir, "*.mc", 0);
   int k;
   if ( !fn.isEmpty() ) 
   {
      k = fn.find(".", 0, false);
      if (k != -1) //if an extension was given, strip it
      {
         fn.truncate(k);
      }
      fn += ".mc";
      fn.stripWhiteSpace();
      (*mc).filename = fn.stripWhiteSpace();
      le_filename->setText((*mc).filename);
   }
}

void US_MonteCarlo::update_file(const QString &str)
{
   (*mc).filename = str.stripWhiteSpace();
}

void US_MonteCarlo::set_gaussian()
{
   (*mc).data_type = 0;
   cb_gaussian->setChecked(true);
   cb_bootstrap->setChecked(false);
   cb_mixed->setChecked(false);
}

void US_MonteCarlo::set_bootstrap()
{
   (*mc).data_type = 1;
   cb_gaussian->setChecked(false);
   cb_bootstrap->setChecked(true);
   cb_mixed->setChecked(false);
}

void US_MonteCarlo::set_mixed()
{
   (*mc).data_type = 2;
   cb_gaussian->setChecked(false);
   cb_bootstrap->setChecked(false);
   cb_mixed->setChecked(true);
}

void US_MonteCarlo::set_random()
{
   (*mc).addNoise = 1;
   cb_random->setChecked(true);
   cb_lastFit->setChecked(false);
   cb_originalFit->setChecked(false);
}

void US_MonteCarlo::set_lastFit()
{
   (*mc).addNoise = 0;
   cb_random->setChecked(false);
   cb_lastFit->setChecked(true);
   cb_originalFit->setChecked(false);
}

void US_MonteCarlo::set_originalFit()
{
   (*mc).addNoise = 2;
   cb_random->setChecked(false);
   cb_lastFit->setChecked(false);
   cb_originalFit->setChecked(true);
}

void US_MonteCarlo::update_percent_bootstrap(double val)
{
   (*mc).percent_bootstrap = (int) val;
}

void US_MonteCarlo::update_percent_gaussian(double val)
{
   (*mc).percent_gaussian = (int) val;
}

void US_MonteCarlo::select_seed(const QString &val)
{
   (*mc).random_seed = (int) val.toInt();
}

/* // Alternative method for filtering out duplicates
   void US_MonteCarlo::update_parameter()
   {
   QString trash;
   float val;
   vector <float> test_v;
   unsigned int count = 0, totalcount=0;
   parameter_value.clear();
   QFile f((*mc).filename);
   if (!f.exists())
   {
   file_error();
   return;
   }
   f.open(IO_ReadOnly);
   QFile test_f("/tmp/new.mc");
   test_f.open(IO_WriteOnly | IO_Translate);
   QTextStream ts2(&test_f);
   QTextStream ts(&f);
   while (!ts.eof())
   {
   struct lines temp_row;
   ts >> trash;   // word "Iteration"
   if (trash != "")
   {
   ts >> trash;   // iteration number
   ts >> trash;   // seed
   for (unsigned int i=0; i<(*mc).parameters; i++)
   {
   ts >> val;
   temp_row.value.push_back(val);
   }
   if (count == 0)
   {
   parameter_value.push_back(temp_row);
   test_v = temp_row.value;
   count ++;
   }
   else if (count>0)
   {
   if (test_v[5] != temp_row.value[5] && test_v[15] != temp_row.value[15] && test_v[25] != temp_row.value[25])
   {
   parameter_value.push_back(temp_row);
   test_v = temp_row.value;
   ts2 << "Iteration " << count << " (00000): ";
   for (unsigned int i=0; i<(*mc).parameters; i++)
   {
   ts2 << temp_row.value[i] << " ";
   }
   ts2 << "\n";
   count ++;
   cout << "Count: " << count << endl;
   }
   else
   {
   cout << "they are the same, totalcount: " << totalcount << "\n";
   }
   }
   totalcount++;
   }
   }
   f.close();
   test_f.close();
   if (count > 20)
   {
   file_read = true;
   file_ok = true;
   pb_save->setEnabled(true);
   pb_print->setEnabled(true);
   pb_statistics->setEnabled(true);
   }
   else
   {
   file_ok = false;
   if (!from_public)      //if we couldn't find 20 entries or more and the stuff comes from the local process signal an error
   {
   file_error();
   }
   return;
   }
   stats.points = count-1;
   trash.sprintf(" %d", stats.points);
   lbl_entries2->setText(trash);
   progress->setProgress(count-1);
   show_parameter(lb_parameters->currentItem());
   }
*/

void US_MonteCarlo::update_parameter()
{
   plot_status = "parameter";
   lb_parameters->setEnabled(false);
   QString trash, str;
   float val;
   unsigned int count = 0;
   parameter_value.clear();
   QFile f((*mc).filename);
   if (!f.exists())
   {
      file_error();
      return;
   }
   f.open(IO_ReadOnly);
   QTextStream ts(&f);
   lbl_current_iteration1->setText(tr(" Loading Iteration..."));
   progress->setTotalSteps((*mc).iterations);
   progress->reset();
   while (!ts.eof())
   {
      struct lines temp_row;
      ts >> trash;   // word "Iteration"
      if (trash != "")
      {
         ts >> trash;   // iteration number
         ts >> trash;   // seed
         for (unsigned int i=0; i<(*mc).parameters; i++)
         {
            ts >> val;
            temp_row.value.push_back(val);
         }
         if(temp_row.value[0] <= (*mc).varianceThreshold)
         {
            parameter_value.push_back(temp_row);
            count ++;
            lbl_current_iteration2->setText(str.sprintf(" %d", count));
            qApp->processEvents();
            progress->setProgress(count);
            if (count == (*mc).iterations)
            {
               lbl_current_iteration1->setText(tr(" Current Iteration:"));
               break;
            }
         }
      }
   }
   f.close();
   lbl_current_iteration1->setText(tr(" Current Iteration:"));
   if (count > 20)
   {
      file_read = true;
      file_ok = true;
      pb_save->setEnabled(true);
      pb_print->setEnabled(true);
      pb_statistics->setEnabled(true);
   }
   else
   {
      file_ok = false;
      if (!from_public)      //if we couldn't find 20 entries or more and the stuff comes from the local process signal an error
      {
         file_error();
      }
      return;
   }
   stats.points = count-1;
   trash.sprintf(" %d", stats.points);
   lbl_entries2->setText(trash);
   progress->setProgress(count-1);
   show_parameter(lb_parameters->currentItem());
   lb_parameters->setEnabled(true);
}

void US_MonteCarlo::file_error()
{
   QMessageBox::message(tr("Attention:"), tr("The file ") + (*mc).filename + tr("\n"
                                                                                "does not exist or does not have enough entries.\n\n"
                                                                                "Please add more Monte Carlo iterations before\n"
                                                                                "trying again, at least 20 iterations are needed.\n"));
}

void US_MonteCarlo::show_parameter()
{
   if (stats.points < 20)
   {
      return;
   }
   show_parameter(lb_parameters->currentItem());
}

void US_MonteCarlo::statistics()
{
   if (stats_widget)
   {
      if (stats_window->isVisible())
      {
         stats_window->raise();
      }
      else
      {
         stats_window->show();
      }
      return;
   }

   stats_window = new US_MonteCarloStats_W(&stats, &stats_widget);
   stats_window->show();
}

void US_MonteCarlo::plotMousePressed(const QMouseEvent &e)
{
   p1 = e.pos();
}

void US_MonteCarlo::plotMouseReleased(const QMouseEvent &e)
{
   QString str;
   unsigned int count=0;
   struct lines temp_row;
   vector <struct lines> temp_parval;
   temp_parval.clear();
   double x1, x2;
   p2 = e.pos();
   x1 = min(histogram_plot->invTransform(QwtPlot::xBottom, p1.x()),
            histogram_plot->invTransform(QwtPlot::xBottom, p2.x()));
   x2 = max(histogram_plot->invTransform(QwtPlot::xBottom, p1.x()),
            histogram_plot->invTransform(QwtPlot::xBottom, p2.x()));
   if ((*mc).run_id.right(1) == "3" && plot_status=="mw_distro") // fixed MW distro model
   {
      QString filename2;
      QPixmap p;
      vector <float> tempx, tempy;
      tempx.clear(); 
      tempy.clear(); 
      for (unsigned int i=0; i<amplitude.size(); i++)
      {
         if ((mw[i] < x2) && (mw[i] > x1))
         {
            tempx.push_back(mw[i]);
            tempy.push_back(amplitude[i]);
         }
      }
      double *x, *y;
      x = new double [tempx.size()];
      y = new double [tempx.size()];
      for (unsigned int i=0; i<tempx.size(); i++)
      {
         x[i] = tempx[i];
         y[i] = tempy[i];
      }
      QwtSymbol symbol;
      QPen p_histo, mwline;
      p_histo.setWidth(3);
      mwline.setWidth(2);
      symbol.setSize(7);
      symbol.setStyle(QwtSymbol::Ellipse);
      if (print_plot)
      {
         p_histo.setColor(Qt::white);
         mwline.setColor(Qt::gray);
         symbol.setPen(Qt::black);
         symbol.setBrush(Qt::white);
      }
      else
      {
         p_histo.setColor(Qt::red);
         mwline.setColor(Qt::cyan);
         symbol.setPen(Qt::blue);
         symbol.setBrush(Qt::yellow);
      }
      histogram_plot->clear();
      unsigned int curve1 = histogram_plot->insertCurve("Histogram");
      unsigned int curve2 = histogram_plot->insertCurve("Plot");
      histogram_plot->setCurveData(curve1, x, y, tempx.size());
      histogram_plot->setCurveData(curve2, x, y, tempx.size());
      histogram_plot->setAxisTitle(QwtPlot::xBottom, "Molecular Weight");
      histogram_plot->setTitle("Molecular Weight Distribution");
      histogram_plot->setCurveStyle(curve1, QwtCurve::Sticks);
      histogram_plot->setCurveStyle(curve2, QwtCurve::Lines);
      histogram_plot->setCurvePen(curve1, p_histo);
      histogram_plot->setCurvePen(curve2, mwline);
      histogram_plot->setCurveSymbol(curve1, symbol);
      histogram_plot->replot();
      qApp->processEvents();
      filename2 = htmlDir + "/mw-distro.";
      p = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
      pm->save_file(filename2, p);
      delete [] x;
      delete [] y;
   }
   else
   {
      for (unsigned int i=0; i<parameter_value.size(); i++)
      {
         temp_row = parameter_value[i];
         if ((temp_row.value[current_item] < x2) && (temp_row.value[current_item] > x1))
         {
            temp_parval.push_back(temp_row);
            count ++;
         }
      }
      parameter_value.clear();
      for (unsigned int i=0; i<count; i++)
      {
         parameter_value.push_back(temp_parval[i]);
      }
      stats.points = count;
      show_parameter(current_item);
      str.sprintf(" %d", stats.points);
      lbl_entries2->setText(str);
      progress->setProgress(count);
      lbl_current_iteration2->setText(str);
   }
}

void US_MonteCarlo::show_parameter(int item)
{
   current_item = item;
   QString str;
   createHtmlDir();
   if (!file_read)
   {
      update_parameter();
      if (!file_ok)
      {
         return;
      }
   }
   if (!file_ok)
   {
      QMessageBox::message(tr("Attention:"), tr("The file ") + (*mc).filename + tr("\n"
                                                                                   "didn't have have enough entries last time it\n"
                                                                                   "was read. Please click on \"Update\" before\n"
                                                                                   "proceeding.\n"));
      return;
   }
   QFile f;
   f.setName(htmlDir + "/parameter-" +  str.sprintf("%d.dat", item));
   if(!f.open(IO_WriteOnly | IO_Translate))
   {
      QMessageBox::message(tr("Attention:"), tr("Unable to open data file for Monte Carlo Analysis!\n"
                                                "Please make sure that the disk is not full\n"
                                                "or write protected."));
      return;
   }
   QTextStream ts(&f);
   unsigned int curve1, curve2;
   double *xplot, *yplot, *xnormal, *ynormal;
   unsigned int points=0;
   parameter_bin.clear();
   stats.low  =  (float)  9.9e30;
   stats.high =  (float) -9.9e30;
   double sum = 0.0, m2 = 0.0, m3 = 0.0, m4 = 0.0;
   xplot = new double [stats.points];
   yplot = new double [stats.points];
   xnormal = new double [400];
   ynormal = new double [400];
   for (unsigned int i=0; i<stats.points; i++)
   {
      stats.mean = parameter_value[i].value[item];
      sum += stats.mean;
      stats.high = max(stats.high, stats.mean);
      stats.low = min(stats.low, stats.mean);
      xplot[i] = (double) i;
      yplot[i] = stats.mean;
   }
   stats.mean = sum/stats.points;
   for (unsigned int i=0; i<stats.points; i++)
   {
      m2 += pow((double) (parameter_value[i].value[item] - stats.mean), (double) 2.0);
      m3 += pow((double) (parameter_value[i].value[item] - stats.mean), (double) 3.0);
      m4 += pow((double) (parameter_value[i].value[item] - stats.mean), (double) 4.0);
   }
   m2 = m2/stats.points;
   m3 = m3/stats.points;
   m4 = m4/stats.points;
   stats.skew =  m3 / pow((double) m2, (double) (3.0/2.0));
   stats.kurtosis =  m4 / pow((double) m2, (double) 2.0) - 3.0;
   stats.median = stats.high - (stats.high - stats.low) / 2.0;
   double intercept, slope, sigma, corr;
   linefit(&xplot, &yplot, &slope, &intercept, &sigma, &corr, stats.points);
   stats.correlation = (float) corr;
   stats.std_deviation = pow((double) m2, (double) 0.5);
   stats.std_error = stats.std_deviation / pow((double) stats.points, (double) 0.5);
   stats.variance = m2;
   stats.area = 0.0;
   stats.parameter_name = (*mc).parameter[item];
   delete [] xplot;
   delete [] yplot;
   xplot = new double [bins];
   yplot = new double [bins];
   float binsize = (stats.high - stats.low)/bins;
   points = 0;
   xplot[0] = (double) (stats.low + binsize);
   yplot[0] = 0.0;
   while (parameter_value[points].value[item] < xplot[0] && points < stats.points)
   {
      yplot[0] += 1.0;
      points ++;
   }
   parameter_bin.push_back((unsigned int) yplot[0]);

   for (unsigned int i=1; i<bins; i++)
   {
      points = 0;
      xplot[i] = (double) (stats.low + (binsize * (i+1)));
      yplot[i] = 0.0;
      for (unsigned int j = 0; j< stats.points; j++)
      {
         if (parameter_value[j].value[item] > xplot[i-1] && parameter_value[j].value[item] < xplot[i])
         {
            yplot[i] += 1.0;
         }
      }
      parameter_bin.push_back((unsigned int) yplot[i]);
      stats.area += yplot[i] * binsize;
   }
   double test = 0.0;
   for (unsigned int i=1; i<bins; i++)
   {
      test = max(yplot[i], test);
   }
   int this_bin = 0;
   for (unsigned int i=1; i<bins; i++)
   {
      if (test == yplot[i])
      {
         this_bin = i;
      }
   }
   for (unsigned int i=0; i<400; i++)
   {
      xnormal[i] = stats.low + i * ((stats.high - stats.low)/400);
      ynormal[i] = (stats.area/(stats.std_deviation * pow((double) (2 * M_PI), (double) 0.5))) 
         * exp(((-1.0) * pow((double) (xnormal[i] - stats.mean), (double) 2)) / (2 * pow((double) stats.std_deviation, (double) 2)));
   }
   stats.mode1 = xplot[this_bin - 1];
   stats.mode2 = xplot[this_bin];
   if (!dont_show)
   {
      QwtSymbol symbol;
      QPen p_gaussian, p_histo;
      p_gaussian.setWidth(2); 
      p_histo.setWidth(3);
      symbol.setSize(7);
      symbol.setStyle(QwtSymbol::Ellipse);
      if (print_plot)
      {
         p_gaussian.setColor(Qt::white);
         p_histo.setColor(Qt::white);
         symbol.setPen(Qt::black);
         symbol.setBrush(Qt::white);
      }
      else
      {
         p_gaussian.setColor(Qt::blue);
         p_histo.setColor(Qt::red);
         symbol.setPen(Qt::blue);
         symbol.setBrush(Qt::yellow);
      }
      str.sprintf(" %e", stats.mean);
      lbl_mean2->setText(str);
      histogram_plot->clear();
      curve1 = histogram_plot->insertCurve("Histogram");
      curve2 = histogram_plot->insertCurve("Normal");
      str = (*mc).parameter[item];
      getToken(&str, " "); // strip the first token ("parameter number:")
      histogram_plot->setAxisTitle(QwtPlot::xBottom, str);
      histogram_plot->setCurvePen(curve2, p_gaussian);
      histogram_plot->setCurveData(curve2, xnormal, ynormal, 400);
      histogram_plot->setCurveStyle(curve2, QwtCurve::Lines);
      if (print_plot)
      {
         histogram_plot->setTitle("");
      }
      else
      {
         histogram_plot->setTitle(tr("Monte Carlo Histogram"));
      }
      histogram_plot->setCurveStyle(curve1, QwtCurve::Sticks);
      histogram_plot->setCurvePen(curve1, p_histo);
      histogram_plot->setCurveData(curve1, xplot, yplot, bins);
      histogram_plot->setCurveSymbol(curve1, symbol);
      histogram_plot->replot();
      if (stats_widget)
      {
         stats_window->update_labels();
      }
   }
   ts << tr("\"Value\"") << "\t" <<  tr("\"Frequency\"" )<< "\t" << tr("\"Normal Distribution\"\n");
   float x1, y1;
   for (unsigned int i=0; i<bins; i++)
   {
      x1 = stats.low + i * ((stats.high - stats.low)/bins);
      y1 = (stats.area/(stats.std_deviation * pow((double) (2 * M_PI), (double) 0.5))) 
         * exp(((-1.0) * pow((double) (x1 - stats.mean), (double) 2)) / (2 * pow((double) stats.std_deviation, (double) 2)));
      ts << xplot[i] << "\t" << yplot[i] << "\t" << y1 << "\n";
   }

   delete [] xplot;
   delete [] yplot;
   delete [] xnormal;
   delete [] ynormal;
}

void US_MonteCarlo::update_bins(const QString &val)
{
   bins = val.toUInt();
}

void US_MonteCarlo::update_noiseLevel(const QString &val)
{
   (*mc).parameterNoise = val.toFloat();
}

void US_MonteCarlo::update_ignoreVariance(const QString &val)
{
   (*mc).varianceThreshold = val.toFloat();
}

void US_MonteCarlo::start()
{
   (*mc).status = true;
   pb_stop->setEnabled(true);
   pb_start->setEnabled(false);
   emit iterate();
}

void US_MonteCarlo::update_rule(int rule)
{
   (*mc).rule = rule;
}

void US_MonteCarlo::stop()
{
   pb_stop->setEnabled(false);
   (*mc).status = false;
}

void US_MonteCarlo::print()
{
   QPrinter printer;
   bool print_bw =  false;
   bool print_inv =  false;
   US_SelectPlot *sp;
   sp = new US_SelectPlot(&print_bw, &print_inv);
   sp->exec();
   if   (printer.setup(0))
   {
      if (print_bw)
      {
         PrintFilter pf;
         pf.setOptions(QwtPlotPrintFilter::PrintTitle
                       |QwtPlotPrintFilter::PrintMargin
                       |QwtPlotPrintFilter::PrintLegend
                       |QwtPlotPrintFilter::PrintGrid);
         histogram_plot->print(printer, pf);
      }
      else if (print_inv)
      {
         PrintFilterDark pf;
         pf.setOptions(QwtPlotPrintFilter::PrintTitle
                       |QwtPlotPrintFilter::PrintMargin
                       |QwtPlotPrintFilter::PrintLegend
                       |QwtPlotPrintFilter::PrintGrid);
         histogram_plot->print(printer, pf);
      }
      else
      {
         histogram_plot->print(printer);
      }
   }
}

void US_MonteCarlo::save()
{
   createHtmlDir();
   QString filename, filename2, str;
   QPixmap p;
   QFile f;
   dont_show = false;
   filename = USglobal->config_list.result_dir + "/" + (*mc).run_id + ".Monte-Carlo";
   f.setName(filename);
   f.open(IO_WriteOnly | IO_Translate);
   QTextStream ts(&f);
   ts << (*mc).run_id << endl;   
   ts << (*mc).parameter.size() << endl;   
   for (unsigned int i=0; i<(*mc).parameter.size(); i++)
   {
      ts << (*mc).parameter[i] << endl;   
   }
   f.close();
   filename = htmlDir + "/" + (*mc).run_id + ".res";
   f.setName(filename);
   f.open(IO_WriteOnly | IO_Translate);
   ts << "***************************************************\n";
   ts << tr("* Monte Carlo Results for ") << (*mc).run_id << "\n";
   ts << "***************************************************\n\n\n";
   switch ((*mc).data_type)
   {
   case 0:
      {
         ts << tr("Monte Carlo Random Data Generation Method: Normal Gaussian\n");
         break;
      }
   case 1:
      {
         ts << tr("Monte Carlo Random Data Generation Method: Bootstrap\n\n");
         break;
      }
   case 2:
      {
         ts << tr("Monte Carlo Random Data Generation Method: Mixed Bootstrap and Normal Gaussian\n");
         ts << tr("with ") << (*mc).percent_bootstrap << tr("% bootstrap and the remainder of points with ") 
            << (*mc).percent_gaussian << tr("% gaussian noise\n\n");
         break;
      }
      if ((*mc).data_type != 1)
      {
         switch ((*mc).rule)
         {
         case 0:
            {
               ts << tr("Monte Carlo data points were generated using the standard deviation of the\n");
               ts << tr("original data points.\n");
               break;
            }
         case 1:
            {
               ts << tr("Monte Carlo data points were generated using the standard deviation of the\n");
               ts << tr("original data points, or using the standard deviation of the initial fit, if\n");
               ts << tr("it is larger.\n\n");
               break;
            }
         case 2:
            {
               ts << tr("Monte Carlo data points were generated using the standard deviation of the\n");
               ts << tr("initial fit.\n\n");
               break;
            }
         case 3:
            {
               ts << tr("Monte Carlo data points were generated using a 5 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         case 4:
            {
               ts << tr("Monte Carlo data points were generated using a 10 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         case 5:
            {
               ts << tr("Monte Carlo data points were generated using a 15 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         case 6:
            {
               ts << tr("Monte Carlo data points were generated using a 20 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         case 7:
            {
               ts << tr("Monte Carlo data points were generated using a 25 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         case 8:
            {
               ts << tr("Monte Carlo data points were generated using a 30 point Gaussian smoothing kernel\n");
               ts << tr("on the standard deviation of the initial fit.\n\n");
               break;
            }
         }
      }
   }
   ts << tr("Total Monte Carlo Iterations for this Analysis: ") << stats.points << "\n";
   ts << str.sprintf(tr("Initial guesses for each iteration had %3.2f"), (*mc).parameterNoise) << tr(" % noise added\n");
   ts << str.sprintf(tr("Monte Carlo iterations with a variance larger than %2.4e were ignored\n"), (*mc).varianceThreshold);
   ts << str.sprintf(tr("There were %d fitted parameters analyzed in this Monte Carlo run.\n"), (*mc).parameter.size());
   f.close();

   ts.width(14);
   ts.flags(0x1000);
   progress->setTotalSteps(parameter_value.size());
   progress->reset();
   if ((*mc).run_id.right(1) == "3") // fixed MW distro model
   {
      // first calculate the offsets to grab the correct amplitudes for summing from each iteration
      // the offsets are the variance (first parameter), then the number of components and the baseline for each scan.
      // we need to sum the corresponding amplitudes from each scan and from each iteration
      unsigned int components = 0, i = 1, j, k, l;
      // first parameter is variance, we'll skip it and count until we get to the first baseline
      // go through the first scan to get the number of components:
      while (!(*mc).parameter[i].contains("Baseline")) 
      {
         components ++;
         i++;
      }
      //      cout << "Components: " << components << endl;
      // the number of scans is the total parameters minus the variance
      // divided by the number of components + one for the baseline
      //      scans = ((*mc).parameter.size()-1)/(components + 1);
      //      cout << "Scans: " << scans << endl;
      //      cout << "Iterations: " << parameter_value.size() << endl;
      
      amplitude.clear();
      amplitude.resize(components); // this vector will sum all corresponding component's amplitudes
      mw.clear();
      mw.resize(components);
      // first iterate over all Monte Carlo iterations:
      for (i=0; i<parameter_value.size(); i++) // these are the total iterations
      {
         // now iterate over all parameters in this iteration
         l = 0;
         
         for (j=1; j<(*mc).parameter.size(); j++) // we don't want the variance and the baseline from each iteration
         {
            k = 0;
            while ((*mc).parameter[j].contains("Amplitude"))
            {
               //               cout << "Parameter value: " << parameter_value[i].value[j] << ", alternate: " << (*mc).parameter[j] << endl;
               amplitude[k] += parameter_value[i].value[j];
               j ++;
               k ++;
            }
            l++;
            //            cout << "Scan " << l << ", j: " << j << ", Monte Carlo iteration: " << i << endl;
            // now go on to the next scan
         }
         progress->setProgress(i);
      }
      filename = htmlDir + "/" + "mw-distro.dat";
      f.setName(filename);
      mw[0] = (*mc).mw_lowerLimit;
      float tmp_mw, mw_step = ((*mc).mw_upperLimit - (*mc).mw_lowerLimit)/(components-1);
      tmp_mw = mw[0];
      double *x, *y;
      x = new double [amplitude.size()];
      y = new double [amplitude.size()];
      if (f.open(IO_WriteOnly | IO_Translate))
      {
         for (l=0; l<amplitude.size(); l++)
         {
            mw[l] = tmp_mw;
            y[l] = amplitude[l];
            ts << mw[l] << ", " << amplitude[l] << endl;
            x[l] = mw[l];
            tmp_mw += mw_step;
         }
         f.close();
      }
      double sum1[2], sum2[2], sum3[2], sumz[2], sum4[2], maxval[2];
      unsigned int highest[2];
      sum1[0] = 0.0;
      sum2[0] = 0.0;
      sum3[0] = 0.0;
      sum4[0] = 0.0;
      sumz[0] = 0.0;
      maxval[0] = 0.0;
      sum1[1] = 0.0;
      sum2[1] = 0.0;
      sum3[1] = 0.0;
      sum4[1] = 0.0;
      sumz[1] = 0.0;
      maxval[1] = 0.0;
      highest[0] = 0;
      highest[1] = 0;
      for (unsigned int i=0; i<amplitude.size(); i++)
      {
         sum1[0] += amplitude[i] * mw[i];
         if (amplitude[i] > maxval[0])
         {
            maxval[0] = amplitude[i];
            highest[0] = i;
         }
         sum2[0] += amplitude[i];
         sum3[0] += amplitude[i]/mw[i];
         sum4[0] += amplitude[i]/mw[i];
         sumz[0] += mw[i]*mw[i]*amplitude[i];
      }
      for (unsigned int i=1; i<amplitude.size(); i++)
      {
         sum1[1] += amplitude[i] * mw[i];
         if (amplitude[i] > maxval[1])
         {
            maxval[1] = amplitude[i];
            highest[1] = i;
         }
         sum2[1] += amplitude[i];
         sum3[1] += amplitude[i]/mw[i];
         sum4[1] += amplitude[i]/mw[i];
         sumz[1] += mw[i]*mw[i]*amplitude[i];
      }
      filename = htmlDir + "/" + "mw-distro.res";
      f.setName(filename);
      if (f.open(IO_WriteOnly | IO_Translate))
      {
         ts << "Average Molecular Weights including the first molecular weight point:\n\n";
         ts << "   Weight-Average Molecular Weight:   " << sum1[0]/sum2[0] << endl;
         ts << "   Number-Average Molecular Weight:   " << sum2[0]/sum4[0] << endl;
         ts << "   Z-Average Molecular Weight:        " << sumz[0]/sum1[0] << endl;
         ts << "   Peak Molecular Weight:             " << mw[highest[0]] << "\n\n\n";
         ts << "Average Molecular Weights without the first molecular weight point:\n\n";
         ts << "   Weight-Average Molecular Weight:   " << sum1[1]/sum2[1] << endl;
         ts << "   Number-Average Molecular Weight:   " << sum2[1]/sum4[1] << endl;
         ts << "   Z-Average Molecular Weight:        " << sumz[1]/sum1[1] << endl;
         ts << "   Peak Molecular Weight:             " << mw[highest[1]] << "\n\n\n";
         f.close();
      }      
      if (!dont_show)
      {
         f.setName(htmlDir + "/variance.res");
         show_parameter(0);
         qApp->processEvents();
         filename2 = htmlDir + "/variance.";
         p = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
         pm->save_file(filename2, p);
         if (f.open(IO_WriteOnly | IO_Translate))
         {
            float center, conf99low, conf99high, conf95low, conf95high;
            center = (stats.mode1 + stats.mode2)/2;
            conf99low = stats.mean - 2.576 * stats.std_deviation;
            conf99high = stats.mean + 2.576 * stats.std_deviation;
            conf95low = stats.mean - 1.96 * stats.std_deviation;
            conf95high = stats.mean + 1.96 * stats.std_deviation;
            ts << tr("Parameter Info:\n\n");
            ts << tr("Results for the Variance of this Monte Carlo Analysis\n\n");
            ts << tr("Maximum Value:             ") << stats.high << "\n";
            ts << tr("Minimum Value:             ") << stats.low << "\n";
            ts << tr("Mean Value:                ") << stats.mean << "\n";
            ts << tr("Median Value:              ") << stats.median << "\n";
            ts << tr("Skew Value:                ") << stats.skew << "\n";
            ts << tr("Kurtosis Value:            ") << stats.kurtosis << "\n";
            ts << tr("Lower Mode Limit:          ") << stats.mode1 << "\n";
            ts << tr("Upper Mode Limit:          ") << stats.mode2 << "\n";
            ts << tr("Mode Center:               ") << center << "\n";
            ts << tr("95% Confidence Limits:     +") << (conf95high - center) << ", -" << (center - conf95low) << "\n";
            ts << tr("99% Confidence Limits:     +") << (conf99high - center) << ", -" << (center - conf99low) << "\n";
            ts << tr("Standard Deviation:        ") << stats.std_deviation << "\n";
            ts << tr("Standard Error:            ") << stats.std_error << "\n";
            ts << tr("Variance:                  ") << stats.variance << "\n";
            ts << tr("Correlation Coefficient:   ") << stats.correlation << "\n";
            ts << tr("Gaussian Area:             ") << stats.area << "\n";
            str.sprintf(tr(" %e (low),  %e (high)\n"), conf95low, conf95high);   // the standard error of the distribution
            ts << tr("95 % Confidence Interval: ") << str;
            str.sprintf(tr(" %e (low),  %e (high)\n"), conf99low, conf99high);   // the standard error of the distribution
            ts << tr("99 % Confidence Interval: ") << str;
            f.close();
         }
         QwtSymbol symbol;
         QPen p_histo, mwline;
         p_histo.setWidth(3);
         mwline.setWidth(2);
         symbol.setSize(7);
         symbol.setStyle(QwtSymbol::Ellipse);
         if (print_plot)
         {
            p_histo.setColor(Qt::white);
            mwline.setColor(Qt::gray);
            symbol.setPen(Qt::black);
            symbol.setBrush(Qt::white);
         }
         else
         {
            p_histo.setColor(Qt::red);
            mwline.setColor(Qt::cyan);
            symbol.setPen(Qt::blue);
            symbol.setBrush(Qt::yellow);
         }
         histogram_plot->clear();
         unsigned int curve1 = histogram_plot->insertCurve("Histogram");
         unsigned int curve2 = histogram_plot->insertCurve("Plot");
         histogram_plot->setCurveData(curve1, x, y, amplitude.size());
         histogram_plot->setCurveData(curve2, x, y, amplitude.size());
         histogram_plot->setAxisTitle(QwtPlot::xBottom, "Molecular Weight");
         histogram_plot->setTitle("Molecular Weight Distribution");
         histogram_plot->setCurveStyle(curve1, QwtCurve::Sticks);
         histogram_plot->setCurveStyle(curve2, QwtCurve::Lines);
         histogram_plot->setCurvePen(curve1, p_histo);
         histogram_plot->setCurvePen(curve2, mwline);
         histogram_plot->setCurveSymbol(curve1, symbol);
         histogram_plot->replot();
         plot_status = "mw_distro";
         
         qApp->processEvents();
         filename2 = htmlDir + "/mw-distro.";
         p = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
         pm->save_file(filename2, p);
         delete [] x;
         delete [] y;
      }
   }
   else
   {
      for (unsigned int i=0; i<(*mc).parameter.size(); i++)
      {
         f.setName(htmlDir + str.sprintf("/parameter-%d.res", i));
         show_parameter((int)i);
         qApp->processEvents();
         filename2 = htmlDir + str.sprintf("/parameter-%d.", i);
         p = QPixmap::grabWidget(histogram_plot, 2, 2, histogram_plot->width() - 4, histogram_plot->height() - 4);
         pm->save_file(filename2, p);
         if (f.open(IO_WriteOnly | IO_Translate))
         {
            float center, conf99low, conf99high, conf95low, conf95high;
            center = (stats.mode1 + stats.mode2)/2;
            conf99low = stats.mean - 2.576 * stats.std_deviation;
            conf99high = stats.mean + 2.576 * stats.std_deviation;
            conf95low = stats.mean - 1.96 * stats.std_deviation;
            conf95high = stats.mean + 1.96 * stats.std_deviation;
            ts << tr("Parameter Info:\n\n");
            ts << tr("Results for Parameter ") << (*mc).parameter[i] << "\n\n";
            ts << tr("Maximum Value:             ") << stats.high << "\n";
            ts << tr("Minimum Value:             ") << stats.low << "\n";
            ts << tr("Mean Value:                ") << stats.mean << "\n";
            ts << tr("Median Value:              ") << stats.median << "\n";
            ts << tr("Skew Value:                ") << stats.skew << "\n";
            ts << tr("Kurtosis Value:            ") << stats.kurtosis << "\n";
            ts << tr("Lower Mode Limit:          ") << stats.mode1 << "\n";
            ts << tr("Upper Mode Limit:          ") << stats.mode2 << "\n";
            ts << tr("Mode Center:               ") << center << "\n";
            ts << tr("95% Confidence Limits:     +") << (conf95high - center) << ", -" << (center - conf95low) << "\n";
            ts << tr("99% Confidence Limits:     +") << (conf99high - center) << ", -" << (center - conf99low) << "\n";
            ts << tr("Standard Deviation:        ") << stats.std_deviation << "\n";
            ts << tr("Standard Error:            ") << stats.std_error << "\n";
            ts << tr("Variance:                  ") << stats.variance << "\n";
            ts << tr("Correlation Coefficient:   ") << stats.correlation << "\n";
            ts << tr("Gaussian Area:             ") << stats.area << "\n";
            str.sprintf(tr(" %e (low),  %e (high)\n"), conf95low, conf95high);   // the standard error of the distribution
            ts << tr("95 % Confidence Interval: ") << str;
            str.sprintf(tr(" %e (low),  %e (high)\n"), conf99low, conf99high);   // the standard error of the distribution
            ts << tr("99 % Confidence Interval: ") << str;
            if((*mc).parameter[i].contains("Ln(Association Constant", true) == 1)
            {
               float kd_val = 1.0/exp(stats.mean);
               float kd_min = 1.0/exp(conf95high);
               float kd_max = 1.0/exp(conf95low);
               ts << tr("\n\nKd:                        ") << kd_val << "\n";
               ts << tr("95% Confidence Limits:     +") << (kd_max - kd_val) << ", -" << (kd_val - kd_min) << "\n";
            }
            f.close();
         }
         progress->setProgress(i);
      }
   }
   progress->setTotalSteps((*mc).iterations);
   progress->reset();
   progress->setProgress(stats.points);
   dont_show = false;
}

void US_MonteCarlo::quit()
{
   close();
}

void US_MonteCarlo::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/monte_carlo.html");
}

void US_MonteCarlo::update_iteration()
{
   from_public = true;
   update_parameter();
   from_public = false;   
}

void US_MonteCarlo::update_total_iterations(const QString &val)
{
   (*mc).iterations = (unsigned int) val.toUInt();
   progress->setTotalSteps((*mc).iterations);
   progress->reset();
   progress->setProgress(stats.points);
}

bool US_MonteCarlo::createHtmlDir()
{
   htmlDir = USglobal->config_list.html_dir + "/" + (*mc).run_id + ".mc";
   QDir d(htmlDir);
   if (d.exists())
   {
      return true;
   }
   else
   {
      if (d.mkdir(htmlDir, true))
      {
         return true;
      }
      else
      {
         return false;
      }
   }
}
