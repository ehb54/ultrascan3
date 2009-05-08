#include "../include/us_edscan.h"


edit_single_F::edit_single_F(QWidget *p, const char* name) : QFrame(p, name)
{
   USglobal = new US_Config();

   int ypos=5, xpos=5;
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setFrameStyle(QFrame::Panel|QFrame::Raised);
   setLineWidth(2);
   
   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setAutoDefault(false);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, 100, 26);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += 105;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, 100, 26);
   connect(pb_cancel, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += 105;

   pb_accept = new QPushButton(tr("Accept"), this);
   Q_CHECK_PTR(pb_accept);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_accept->setGeometry(xpos, ypos, 100, 26);
   connect(pb_accept, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += 105;

   pb_zoom = new QPushButton(tr("Turn Zoom Off"), this);
   Q_CHECK_PTR(pb_zoom);
   pb_zoom->setAutoDefault(false);
   pb_zoom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_zoom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_zoom->setGeometry(xpos, ypos, 100, 26);
   connect(pb_zoom, SIGNAL(clicked()), SIGNAL(clicked()));

   ypos += 33;

   lbl_instruct1 = new QLabel(tr(" Step-by-Step\n Instructions:"),this);
   lbl_instruct1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_instruct1->setAlignment(AlignLeft|AlignVCenter);
   lbl_instruct1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_instruct1->setGeometry(7, ypos, 100, 82);
   lbl_instruct1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_instruct2 = new QLabel(tr("Please drag the Mouse over the area that needs"
                                 " to be edited. When you have zoomed the\n"
                                 "scan to the desired level, turn off zooming"
                                 " and mark the point that you want to move by\n"
                                 " clicking on it with the mouse."), this);
   lbl_instruct2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_instruct2->setAlignment(AlignCenter|AlignVCenter);
   lbl_instruct2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_instruct2->setGeometry(107, ypos, 526, 82);
   lbl_instruct2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   QVBoxLayout *bottom_box=new QVBoxLayout(this,5);
   QHBoxLayout *instructs= new QHBoxLayout(bottom_box);
   instructs->addWidget(lbl_instruct1);
   instructs->addWidget(lbl_instruct2);
   QHBoxLayout *buttons= new QHBoxLayout(bottom_box,5);
   buttons->addWidget(pb_help);
   buttons->addWidget(pb_cancel);
   buttons->addWidget(pb_accept);
   buttons->addWidget(pb_zoom);


}

edit_single_F::~edit_single_F()
{
}

void edit_single_F::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_spikes.html");
}


edit_single_Win::edit_single_Win(double **rad, double **abs, int pts,
                                 QWidget *p , const char *name) : QWidget(p, name)
{
   USglobal = new US_Config();

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   points = pts;
   temp_rad = rad;
   temp_abs = abs;
   zoom_enabled = TRUE;
   move_point = FALSE;
   radius = new double [points];
   radius = *rad;
   absorbance = new double [points];
   absorbance = *abs; 
   scan_plt = new QwtPlot(this);
   scan_plt->enableOutline(true);
   scan_plt->setOutlinePen(white);
   scan_plt->setOutlineStyle(Qwt::Rect);
   scan_plt->setTitle(tr("Edit Window for Single Scans:"));
   scan_plt->enableGridXMin();
   scan_plt->enableGridYMin();
   scan_plt->setPalette(QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   scan_plt->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   scan_plt->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   //scan_plt->setPlotBackground(USglobal->global_colors.plot);      //old version
   scan_plt->setCanvasBackground(USglobal->global_colors.plot);      //new version
   scan_plt->setMargin(USglobal->config_list.margin);
   scan_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   scan_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));

   frm = new edit_single_F(this);
    
   scan = scan_plt->insertCurve("Scan");
   scan_plt->setCurvePen(scan, yellow);
   scan_plt->setCurveData(scan, radius, absorbance, points);
   scan_plt->replot();
   //   connect(scan_plt, SIGNAL(plotMouseMoved(const QMouseEvent&)),
   //           SLOT(plotMouseMoved( const QMouseEvent&)));
   connect(scan_plt, SIGNAL(plotMousePressed(const QMouseEvent &)),
           SLOT(plotMousePressed( const QMouseEvent&)));
   connect(scan_plt, SIGNAL(plotMouseReleased(const QMouseEvent &)),
           SLOT(plotMouseReleased( const QMouseEvent&)));
   connect(frm->pb_zoom, SIGNAL(clicked()), SLOT(zoom()));
   connect(frm->pb_cancel, SIGNAL(clicked()), SLOT(cancel()));
   connect(frm->pb_accept, SIGNAL(clicked()), SLOT(accept()));
   connect(frm->pb_accept, SIGNAL(clicked()), SIGNAL(clicked()));

   //setFixedSize(640, 480);
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 640, 480);
   
   QGridLayout * background = new QGridLayout(this,2,1,2);
   background->addWidget(scan_plt,0,0);
   background->addWidget(frm,1,0);
      
}

edit_single_Win::~edit_single_Win()
{
}

void edit_single_Win::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void edit_single_Win::zoom()
{
   if(zoom_enabled)
   {
      frm->pb_zoom->setText(tr("Turn Zoom On"));
      zoom_enabled = FALSE;
      scan_plt->enableOutline(FALSE);
   }
   else
   {
      frm->pb_zoom->setText(tr("Turn Zoom Off"));
      zoom_enabled = TRUE;
      scan_plt->enableOutline(TRUE);
      scan_plt->setOutlinePen(white);
      scan_plt->setOutlineStyle(Qwt::Rect);
   }
}
/*
  void edit_single_Win::resizeEvent(QResizeEvent *e)
  {
  QRect r(0, 0, e->size().width(), e->size().height() - 129);
  scan_plt->setGeometry(r);
  frm->setGeometry(0, r.bottom(), r.width(), r.height());
  }
*/
void edit_single_Win::plotMousePressed(const QMouseEvent &e)
{
   int dist;
   uint vert, horiz;
   double xmin, xmax, ymin, ymax;
   double xv[2], yv[2], xh[2], yh[2], lengthx, lengthy;

   if (zoom_enabled)
   {
      p1 = e.pos();
   }
   else
   {
      if (!move_point)
      {
         scan_plt->closestCurve(e.x(), e.y(), dist, xval, yval, index);
         //old version:const QwtScaleDiv & QwtPlot::axisScale ( int axis ) const
         //xmin = scan_plt->axisScale(QwtPlot::xBottom).lBound();
         //xmax = scan_plt->axisScale(QwtPlot::xBottom).hBound();
         //ymin = scan_plt->axisScale(QwtPlot::yLeft).lBound();
         //ymax = scan_plt->axisScale(QwtPlot::yLeft).hBound();
         //new version:const QwtScaleDiv * QwtPlot::axisScale ( int axis ) const
         xmin = scan_plt->axisScale(QwtPlot::xBottom)->lBound();
         xmax = scan_plt->axisScale(QwtPlot::xBottom)->hBound();
         ymin = scan_plt->axisScale(QwtPlot::yLeft)->lBound();
         ymax = scan_plt->axisScale(QwtPlot::yLeft)->hBound();

         lengthx = (xmax-xmin) / 60;
         lengthy = (ymax-ymin) / 30;
         xh[0] = xval - lengthx;      //horizontal
         xh[1] = xval + lengthx;
         yh[0] = yval;
         yh[1] = yval;
         horiz = scan_plt->insertCurve("horizontal");
         scan_plt->setCurvePen(horiz, QPen(red, 2));
         scan_plt->setCurveData(horiz, xh, yh, 2);
         xv[0] = xval;
         xv[1] = xval;
         yv[0] = yval - lengthy;
         yv[1] = yval + lengthy;
         vert = scan_plt->insertCurve("vertical");
         scan_plt->setCurvePen(vert, QPen(red, 2));
         scan_plt->setCurveData(vert, xv, yv, 2);
         scan_plt->replot();
         move_point = TRUE;
         frm->lbl_instruct2->setText(tr("Please move the mouse now to the position where"
                                        " the point should be moved to\n and click on "
                                        "that point."));
      }
      else
      {
         absorbance2[index] = scan_plt->invTransform(QwtPlot::yLeft, e.y());
         update_plot();
      }
   }
}

void edit_single_Win::update_plot()
{
   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(8);

   scan_plt->clear();
   scan = scan_plt->insertCurve("Scan");
   scan_plt->setCurvePen(scan, yellow);
   scan_plt->setCurveData(scan, radius2, absorbance2, points2);
   scan_plt->enableGridXMin();
   scan_plt->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
   scan_plt->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
   scan_plt->setCurveSymbol(scan, QwtSymbol(sym));
   scan_plt->replot();
}

void edit_single_Win::plotMouseReleased(const QMouseEvent &e)
{
   int x1, y1, x2, y2, i, j, k, count;
   double r1, r2;
   if (zoom_enabled)
   {
      x1 = qwtMin(p1.x(), e.x());
      x2 = qwtMax(p1.x(), e.x());
      y1 = qwtMin(p1.y(), e.y());
      y2 = qwtMax(p1.y(), e.y());
      
      r1 = scan_plt->invTransform(QwtPlot::xBottom, x1);
      r2 = scan_plt->invTransform(QwtPlot::xBottom, x2);
      i = find_indexf(r1, &radius, points); 
      j = find_indexr(r2, &radius, points);
      points2 = j-i+1;
      absorbance2 = new double [points2];
      radius2 = new double [points2];
      count=0;
      for(k=i; k<=j; k++)
      {
         radius2[count]     = radius[k];
         absorbance2[count] = absorbance[k];
         count++;
      }
      update_plot();
   }
}

void edit_single_Win::accept()
{
   int i;
   if (move_point)
   {
      i = find_indexf(xval, temp_rad, points);
      (*temp_abs)[i]=absorbance2[index];
   }
   close();
}

void edit_single_Win::cancel()
{
   close();
}



US_ZoomFrame::US_ZoomFrame(QWidget *p, const char* name) : QFrame(p, name)
{
   
   USglobal = new US_Config();

   int ypos=5, xpos=5;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, 100, 26);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += 105;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_cancel->setGeometry(xpos, ypos, 100, 26);
   connect(pb_cancel, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += 105;

   pb_accept = new QPushButton(tr("Accept"), this);
   Q_CHECK_PTR(pb_accept);
   pb_accept->setAutoDefault(false);
   pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_accept->setGeometry(xpos, ypos, 100, 26);
   connect(pb_accept, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += 105;

   pb_zoom = new QPushButton(tr("Turn Zoom Off"), this);
   Q_CHECK_PTR(pb_zoom);
   pb_zoom->setAutoDefault(false);
   pb_zoom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_zoom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_zoom->setGeometry(xpos, ypos, 100, 26);
   connect(pb_zoom, SIGNAL(clicked()), SIGNAL(clicked()));

   xpos += 105;

   pb_reset_zoom = new QPushButton(tr("Reset Zoom"), this);
   Q_CHECK_PTR(pb_zoom);
   pb_reset_zoom->setAutoDefault(false);
   pb_reset_zoom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset_zoom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_reset_zoom->setGeometry(xpos, ypos, 100, 26);
   connect(pb_reset_zoom, SIGNAL(clicked()), SIGNAL(clicked()));

   ypos += 33;

   lbl_instruct1 = new QLabel(tr(" Step-by-Step\n Instructions:"),this);
   lbl_instruct1->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_instruct1->setAlignment(AlignLeft|AlignVCenter);
   lbl_instruct1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
   lbl_instruct1->setGeometry(7, ypos, 100, 82);
   lbl_instruct1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   lbl_instruct2 = new QLabel(tr("Please drag the Mouse over the area that needs"
                                 " to be edited. When you have zoomed the\n"
                                 "scan to the desired level, turn off zooming"
                                 " and click on the point that marks the meniscus."), this);
   lbl_instruct2->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_instruct2->setAlignment(AlignCenter|AlignVCenter);
   lbl_instruct2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_instruct2->setGeometry(107, ypos, 526, 82);
   lbl_instruct2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

}

US_ZoomFrame::~US_ZoomFrame()
{
}

void US_ZoomFrame::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/edit_spikes.html");
}

US_ZoomWin::US_ZoomWin(double ***rad, double ***abs, int pts, int scns, float *tmp_xval,
                       QWidget *p , const char *name) : QWidget(p, name)
{
   int i;
   USglobal = new US_Config();
   
   points = pts;
   scans = scns;
   radius_val = tmp_xval;
   temp_rad = rad;
   temp_abs = abs;
   zoom_enabled = TRUE;
   move_point = FALSE;
   //   radius = new double *[scans];
   //   absorbance = new double *[scans];
   scan = new uint [scans];
   //   for (i=0; i<points; i++)
   //   {
   //      radius[i] = new double [points];
   //      absorbance[i] = new double [points];
   //   }
   radius = *rad;
   absorbance = *abs; 
   scan_plt = new QwtPlot(this);
   scan_plt->enableOutline(true);
   scan_plt->setOutlinePen(white);
   scan_plt->setOutlineStyle(Qwt::Rect);
   scan_plt->setTitle(tr("Zoom Window:"));
   //scan_plt->setPlotBackground(black);      //old version
   scan_plt->setCanvasBackground(black);      //new version
   scan_plt->setMargin(USglobal->config_list.margin);
   
   frm = new US_ZoomFrame(this);
   frm->setFrameStyle(QFrame::Panel|QFrame::Raised);
   frm->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   frm->setLineWidth(2);
    
   for (i=0; i<scans; i++)
   {
      scan[i] = scan_plt->insertCurve("Scan");
      scan_plt->setCurvePen(scan[i], yellow);
      scan_plt->setCurveData(scan[i], radius[i], absorbance[i], points);
   }
   scan_plt->enableGridXMin();
   scan_plt->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
   scan_plt->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
   scan_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (cm)"));
   scan_plt->setAxisTitle(QwtPlot::yLeft, tr("Absorbance"));
   scan_plt->setAxisTitle(QwtPlot::xBottom, tr("Radius (in cm)"));
   scan_plt->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   scan_plt->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plt->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plt->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plt->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   scan_plt->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   scan_plt->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));   
   scan_plt->replot();
   connect(scan_plt, SIGNAL(plotMouseMoved(const QMouseEvent&)),
           SLOT(plotMouseMoved( const QMouseEvent&)));
   connect(scan_plt, SIGNAL(plotMousePressed(const QMouseEvent &)),
           SLOT(plotMousePressed( const QMouseEvent&)));
   connect(scan_plt, SIGNAL(plotMouseReleased(const QMouseEvent &)),
           SLOT(plotMouseReleased( const QMouseEvent&)));
   connect(frm->pb_zoom, SIGNAL(clicked()), SLOT(zoom()));
   connect(frm->pb_reset_zoom, SIGNAL(clicked()), SLOT(reset_zoom()));
   connect(frm->pb_cancel, SIGNAL(clicked()), SLOT(cancel()));
   connect(frm->pb_accept, SIGNAL(clicked()), SLOT(accept()));
   connect(frm->pb_accept, SIGNAL(clicked()), SIGNAL(clicked()));
   //xlimit_left = scan_plt->axisScale(QwtPlot::xBottom).lBound();
   //xlimit_right = scan_plt->axisScale(QwtPlot::xBottom).hBound();
   xlimit_left = scan_plt->axisScale(QwtPlot::xBottom)->lBound();
   xlimit_right = scan_plt->axisScale(QwtPlot::xBottom)->hBound();
}

US_ZoomWin::~US_ZoomWin()
{
}

void US_ZoomWin::zoom()
{
   if(zoom_enabled)
   {
      frm->pb_zoom->setText(tr("Turn Zoom On"));
      zoom_enabled = FALSE;
      scan_plt->enableOutline(FALSE);
   }
   else
   {
      frm->pb_zoom->setText(tr("Turn Zoom Off"));
      zoom_enabled = TRUE;
      scan_plt->enableOutline(TRUE);
      scan_plt->setOutlinePen(white);
      scan_plt->setOutlineStyle(Qwt::Rect);
   }
}

void US_ZoomWin::resizeEvent(QResizeEvent *e)
{
   QRect r(0, 0, e->size().width(), e->size().height() - 129);
   scan_plt->setGeometry(r);
   frm->setGeometry(0, r.bottom(), r.width(), r.height());
}

void US_ZoomWin::reset_zoom()
{
}

void US_ZoomWin::plotMousePressed(const QMouseEvent &e)
{
   int dist;
   uint vert, horiz;
   double xmin, xmax, ymin, ymax;
   double xv[2], yv[2], xh[2], yh[2], lengthx, lengthy;

   if (zoom_enabled)
   {
      p1 = e.pos();
   }
   else
   {
      if (!move_point)
      {
         scan_plt->closestCurve(e.x(), e.y(), dist, xval, yval, index);
         //old version:const QwtScaleDiv & QwtPlot::axisScale ( int axis ) const
         //xmin = scan_plt->axisScale(QwtPlot::xBottom).lBound();
         //xmax = scan_plt->axisScale(QwtPlot::xBottom).hBound();
         //ymin = scan_plt->axisScale(QwtPlot::yLeft).lBound();
         //ymax = scan_plt->axisScale(QwtPlot::yLeft).hBound();
         //new version:const QwtScaleDiv * QwtPlot::axisScale ( int axis ) const
         xmin = scan_plt->axisScale(QwtPlot::xBottom)->lBound();
         xmax = scan_plt->axisScale(QwtPlot::xBottom)->hBound();
         ymin = scan_plt->axisScale(QwtPlot::yLeft)->lBound();
         ymax = scan_plt->axisScale(QwtPlot::yLeft)->hBound();

         lengthx = (xmax-xmin) / 60;
         lengthy = (ymax-ymin) / 30;
         xh[0] = xval - lengthx;      //horizontal
         xh[1] = xval + lengthx;
         yh[0] = yval;
         yh[1] = yval;
         horiz = scan_plt->insertCurve("horizontal");
         scan_plt->setCurvePen(horiz, QPen(red, 2));
         scan_plt->setCurveData(horiz, xh, yh, 2);
         xv[0] = xval;
         xv[1] = xval;
         yv[0] = yval - lengthy;
         yv[1] = yval + lengthy;
         vert = scan_plt->insertCurve("vertical");
         scan_plt->setCurvePen(vert, QPen(red, 2));
         scan_plt->setCurveData(vert, xv, yv, 2);
         scan_plt->replot();
         move_point = TRUE;
         frm->lbl_instruct2->setText(tr("Please move the mouse now to the position where"
                                        " the point should be moved to\n and click on "
                                        "that point."));
         *radius_val = xval;
      }
      else
      {
         update_plot();
      }
   }
}

void US_ZoomWin::update_plot()
{
   int i;
   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Ellipse);
   sym.setPen(blue);
   sym.setBrush(white);
   sym.setSize(5);

   scan_plt->clear();
   for (i=0; i<scans; i++)
   {
      scan[i] = scan_plt->insertCurve("Scan");
      scan_plt->setCurvePen(scan[i], yellow);
      scan_plt->setCurveData(scan[i], radius2[i], absorbance2[i], points2);
      scan_plt->enableGridXMin();
      scan_plt->setGridMajPen(QPen(colorGroup().light(), 0, DotLine));
      scan_plt->setGridMinPen(QPen(colorGroup().mid(), 0 , DotLine));
      scan_plt->setCurveSymbol(scan[i], QwtSymbol(sym));
   }
   scan_plt->replot();
}

void US_ZoomWin::plotMouseReleased(const QMouseEvent &e)
{
   int x1, y1, x2, y2, i, j, k, m, count;
   double r1, r2;
   if (zoom_enabled)
   {

      //Find the bounding rectangle from mouse pressed event (p1), -dragged- and mouse released event (e)

      x1 = qwtMin(p1.x(), e.x());
      x2 = qwtMax(p1.x(), e.x());
      y1 = qwtMin(p1.y(), e.y());
      y2 = qwtMax(p1.y(), e.y());
      
      r1 = scan_plt->invTransform(QwtPlot::xBottom, x1);
      r2 = scan_plt->invTransform(QwtPlot::xBottom, x2);
      i = find_indexf(r1, &radius[0], points); 
      j = find_indexr(r2, &radius[0], points);
      points2 = j-i+1;
      absorbance2 = new double * [scans];
      radius2 = new double * [scans];
      for (m=0; m<scans; m++)
      {
         absorbance2[m] = new double [points2];
         radius2[m]       = new double [points2];
      }
      for (m=0; m<scans; m++)
      {
         count=0;
         for(k=i; k<=j; k++)
         {
            radius2[m][count]     = radius[m][k];
            absorbance2[m][count] = absorbance[m][k];
            count++;
         }
      }
      update_plot();
   }
}

void US_ZoomWin::accept()
{
   close();
}

void US_ZoomWin::cancel()
{
   close();
}



