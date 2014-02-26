#include "../include/us_colorgradient.h"

US_GradientWidget::US_GradientWidget(vector <QColor> *color_array,
                                     QWidget *p, const char* name) : QLabel(p, name)
{
   this->color_array = color_array;
}

US_GradientWidget::~US_GradientWidget()
{
}

void US_GradientWidget::paintEvent(QPaintEvent *)
{
   if((*color_array).size() < 1)
   {
      return;
   }
   unsigned int h, w, i;
   float count, ratio;
   w = this->width();
   h = this->height();
   QColor col;
   QPainter paint(this);
   ratio = (float) (*color_array).size()/(float) w;
   count = 0.0;
   for (i=0; i<w; i++)
   {
      if (count > (*color_array).size() - 1)
      {
         count = (*color_array).size() - 1;
      }
      col = (*color_array)[(unsigned int) count];
      count += ratio;
      paint.setPen(col);
      paint.drawLine (i, 0, i, h);
   }
   paint.end();
}

US_ColorGradient::US_ColorGradient(QWidget *p, const char* name) : QWidget(p, name)
{
   gradient.step.clear();
   color_array.clear();
   gradient.startcolor = QColor(Qt::black);
   current_step = 0;
   USglobal = new US_Config();

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_banner1 = new QLabel(tr("Please select the number of Colors\nand Color Steps for your gradient:"),this);
   lbl_banner1->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   pb_startcolor = new QPushButton(tr("Select Starting Color"), this);
   Q_CHECK_PTR(pb_startcolor);
   pb_startcolor->setAutoDefault(false);
   pb_startcolor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_startcolor->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_startcolor, SIGNAL(clicked()), SLOT(select_startcolor()));

   lbl_startcolor = new QLabel("", this);
   lbl_startcolor->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_startcolor->setPaletteBackgroundColor(Qt::black);

   lbl_colors = new QLabel(tr(" Number of Colors: "),this);
   Q_CHECK_PTR(lbl_colors);
   lbl_colors->setAlignment(AlignHCenter|AlignVCenter);
   lbl_colors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_colors->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   cnt_colors = new QwtCounter(this);
   cnt_colors->setNumButtons(3);
   cnt_colors->setRange(0, 100, 1);
   cnt_colors->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_colors->setValue(0);
   cnt_colors->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect(cnt_colors, SIGNAL(valueChanged(double)), SLOT(update_colors(double)));

   lbl_step = new QLabel(tr(" Select a Color Step: "),this);
   Q_CHECK_PTR(lbl_step);
   lbl_step->setAlignment(AlignHCenter|AlignVCenter);
   lbl_step->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_step->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   cnt_step = new QwtCounter(this);
   cnt_step->setEnabled(false);
   cnt_step->setNumButtons(3);
   cnt_step->setRange(1, gradient.step.size(), 1);
   cnt_step->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_step->setValue(1);
   cnt_step->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect(cnt_step, SIGNAL(valueChanged(double)), SLOT(update_step(double)));

   lbl_points = new QLabel(tr(" Number of Points in this Step: "),this);
   Q_CHECK_PTR(lbl_points);
   lbl_points->setAlignment(AlignHCenter|AlignVCenter);
   lbl_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_points->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

   cnt_points = new QwtCounter(this);
   cnt_points->setEnabled(false);
   cnt_points->setNumButtons(3);
   cnt_points->setRange(0, 255, 1);
   cnt_points->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_points->setValue(0);
   cnt_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect(cnt_points, SIGNAL(valueChanged(double)), SLOT(update_points(double)));

   pb_stepcolor = new QPushButton(tr("Select Step Color"), this);
   Q_CHECK_PTR(pb_stepcolor);
   pb_stepcolor->setAutoDefault(false);
   pb_stepcolor->setEnabled(false);
   pb_stepcolor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stepcolor->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_stepcolor, SIGNAL(clicked()), SLOT(update_stepcolor()));

   lbl_stepcolor = new QLabel("", this);
   lbl_stepcolor->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_stepcolor->setPaletteBackgroundColor (Qt::black);

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_reset = new QPushButton(tr("Reset"), this);
   Q_CHECK_PTR(pb_reset);
   pb_reset->setAutoDefault(false);
   pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

   pb_save = new QPushButton(tr("Save Gradient"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setEnabled(false);
   pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));

   lbl_gradient = new US_GradientWidget(&color_array, this, "");
   lbl_gradient->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
   lbl_gradient->setPaletteBackgroundColor (Qt::black);

   unsigned int i, j, rows=8, columns=2, margins=2, spacing=2;

   QGridLayout * Grid = new QGridLayout(this, rows, columns, margins, spacing);
   j=0;
   for (i=0; i<rows; i++)
   {
      Grid->setRowSpacing(i, 26);
   }
   Grid->addMultiCellWidget(lbl_banner1, j, j, 0, 1);
   j++;
   Grid->addWidget(pb_startcolor, j, 0);
   Grid->addWidget(lbl_startcolor, j, 1);
   j++;
   Grid->addWidget(lbl_colors, j, 0);
   Grid->addWidget(cnt_colors, j, 1);
   j++;
   Grid->addWidget(lbl_step, j, 0);
   Grid->addWidget(cnt_step, j, 1);
   j++;
   Grid->addWidget(lbl_points, j, 0);
   Grid->addWidget(cnt_points, j, 1);
   j++;
   Grid->addWidget(pb_stepcolor, j, 0);
   Grid->addWidget(lbl_stepcolor, j, 1);
   j++;
   Grid->addWidget(pb_help, j, 0);
   Grid->addWidget(pb_reset, j, 1);
   j++;
   Grid->addWidget(pb_save, j, 0);
   Grid->addWidget(pb_close, j, 1);
   j++;
   Grid->addMultiCellWidget(lbl_gradient, j, j, 0, 1);

   qApp->processEvents();
   QRect r = Grid->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;
   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

US_ColorGradient::~US_ColorGradient()
{
}

void US_ColorGradient::select_startcolor()
{
   QColor temp_color = gradient.startcolor;
   gradient.startcolor = QColorDialog::getColor(gradient.startcolor, this);
   if (!gradient.startcolor.isValid())
   {
      gradient.startcolor = temp_color;
   }
   else
   {
      lbl_startcolor->setPaletteBackgroundColor(gradient.startcolor);
   }
}

void US_ColorGradient::update_colors(double val)
{
   unsigned int k=gradient.step.size();
   gradient.step.resize((unsigned int) val);
   if (val > 0)
   {
      for (unsigned int i=k; i<(unsigned int) val; i++)
      {
         gradient.step[i].color = QColor(Qt::black);
         gradient.step[i].points = 0;
      }
      pb_stepcolor->setEnabled(true);
      pb_save->setEnabled(true);
      cnt_step->setEnabled(true);
      cnt_step->setRange(1, gradient.step.size(), 1);
      cnt_points->setEnabled(true);
   }
   else
   {
      pb_stepcolor->setEnabled(false);
      pb_save->setEnabled(false);
      cnt_step->setEnabled(false);
      cnt_step->setRange(1, gradient.step.size(), 1);
      cnt_step->setValue(1);
      cnt_colors->setValue(0);
      cnt_points->setEnabled(false);
      cnt_points->setValue(0);
   }
}

void US_ColorGradient::update_step(double val)
{
   current_step = (unsigned int) val;
   current_step --;
   lbl_stepcolor->setPaletteBackgroundColor(gradient.step[current_step].color);
   cnt_points->setValue(gradient.step[current_step].points);
}

void US_ColorGradient::update_points(double val)
{
   gradient.step[current_step].points = (unsigned int) val;
   update_array();
}

void US_ColorGradient::update_stepcolor()
{
   QColor temp_color = gradient.step[current_step].color;
   gradient.step[current_step].color = QColorDialog::getColor(gradient.step[current_step].color, this);
   if (!gradient.step[current_step].color.isValid())
   {
      gradient.step[current_step].color = temp_color;
   }
   else
   {
      lbl_stepcolor->setPaletteBackgroundColor(gradient.step[current_step].color);
   }
}

void US_ColorGradient::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/colorgradient.html");
}

void US_ColorGradient::reset()
{
   gradient.step.clear();
   lbl_stepcolor->setPaletteBackgroundColor(Qt::black);
   lbl_startcolor->setPaletteBackgroundColor(Qt::black);
   current_step = 0;
   gradient.startcolor = QColor(Qt::black);
   update_colors(0);
}

void US_ColorGradient::save()
{
   QString str1, str2, filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.map", 0);
   if (!filename.isEmpty())
   {
      if (filename.right(4) != ".map")
      {
         filename.append(".map");
      }
      switch(QMessageBox::information(this, tr("UltraScan - Color Gradient:"),
                                      tr("Click 'OK' to save Color Gradient information to :\n" + filename),
                                      tr("OK"), tr("CANCEL"),   0, 1))
      {
      case 0:
         {
            QFile f(filename);
            if (f.open(IO_WriteOnly | IO_Translate))
            {
               QTextStream ts (&f);
               for (unsigned int i=0; i<color_array.size(); i++)
               {
                  ts << color_array[i].red() << " " << color_array[i].green() << " " << color_array[i].blue() << endl;
               }
               f.close();
            }
            else
            {
               QMessageBox::message(tr("Attention:"), tr("Unable to write the Color Gradient to a file!\n"
                                                         "Please check diskspace and make sure\nthe disk is not write-protected."));
            }
            break;
         }
      case 1:
         {
            break;
         }
      }
   }
}

void US_ColorGradient::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_ColorGradient::update_array()
{
   color_array.clear();
   QColor temp_color;
   unsigned int i, j;
   double distance_red, distance_green, distance_blue, red, green, blue;
   int red1, red2, green1, green2, blue1, blue2;
   for (i=0; i<gradient.step.size(); i++)
   {
      if (i == 0) // use start color
      {
         gradient.startcolor.getRgb(&red1, &green1, &blue1);
      }
      else
      {
         gradient.step[i-1].color.getRgb(&red1, &green1, &blue1);
      }
      gradient.step[i].color.getRgb(&red2, &green2, &blue2);
      distance_red = red2 - red1;
      distance_green = green2 - green1;
      distance_blue = blue2 - blue1;
      distance_red = distance_red/gradient.step[i].points;
      distance_green = distance_green/gradient.step[i].points;
      distance_blue = distance_blue/gradient.step[i].points;
      temp_color.setRgb(red1, green1, blue1);
      red = red1;
      green = green1;
      blue = blue1;
      for (j=0; j<gradient.step[i].points; j++)
      {
         color_array.push_back(temp_color); // add the step's beginning color (which is the same as the ending color from the previous step)
         red += distance_red;
         green += distance_green;
         blue += distance_blue;
         temp_color.setRgb((int) red, (int) green, (int) blue);
      }
   }
   color_array.push_back(gradient.step[gradient.step.size() - 1].color); // add the last color step color
   lbl_gradient->repaint();
}
