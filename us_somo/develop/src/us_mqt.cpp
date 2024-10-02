#include "../include/us_mqt.h"
//Added by qt3to4:
#include <QResizeEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QFocusEvent>

static QTextStream& qStdOut()
{
   static QTextStream ts( stdout );
   return ts;
}

mQTextEdit::mQTextEdit( QWidget *parent ) : QTextEdit( parent ) {
   cli_progress = (bool *)0;
   cli_prefix   = "unknown";
   // qStdOut()    << "tso mQTextEdit constructor\n" << Qt::flush;
}

mQTextEdit::~mQTextEdit() {
}

void mQTextEdit::append( const QString & text ) {
   // qDebug() << QString("mQTextEdit append %1").arg( text );
   if ( cli_progress && *cli_progress ) {
      qStdOut() << QString( "~texts %1 :\n%2\n~texte\n" ).arg( cli_prefix ).arg( text ) << Qt::flush;
   }
   emit QTextEdit::append( text );
}

void mQTextEdit::set_cli_progress( bool & cli_progress ) {
   this->cli_progress = & cli_progress;
}

void mQTextEdit::set_cli_prefix( QString cli_prefix ) {
   this->cli_prefix = cli_prefix;
}

mQProgressBar::mQProgressBar( QWidget *parent ) : QProgressBar( parent ) {
   cli_progress = (bool *)0;
   cli_prefix   = "unknown";
   // qStdOut()    << "tso mQProgressBar constructor\n" << Qt::flush;
}

mQProgressBar::~mQProgressBar() {
}

void mQProgressBar::setValue( int val ) {
   // qDebug() << QString("mQProgressBar setValue %1").arg(val);
   if ( cli_progress && *cli_progress ) {
      static QString lastprogress;
      double progress = (double) val / (double) maximum();
      if ( progress > 1 ) {
         progress = 1;
      }
      QString newprogress = QString::number( progress, 'f', 2 );
      if ( lastprogress != newprogress ) {
         lastprogress = newprogress;
         qStdOut() << QString( "~pgrs %1 : %2\n" ).arg( cli_prefix ).arg( newprogress ) << Qt::flush;
      }
   }
   emit QProgressBar::setValue( val );
}

void mQProgressBar::set_cli_progress( bool & cli_progress ) {
   this->cli_progress = & cli_progress;
}

void mQProgressBar::set_cli_prefix( QString cli_prefix ) {
   this->cli_prefix = cli_prefix;
}

mQwtPlot::mQwtPlot( QWidget *parent ) : QwtPlot( parent ) {}
mQwtPlot::~mQwtPlot() {}

void mQwtPlot::resizeEvent ( QResizeEvent *e )
{
   QwtPlot::resizeEvent( e );
   emit( resized() );
}

mQLineEdit::mQLineEdit( QWidget *parent ) : QLineEdit( parent ) {}

mQLineEdit::~mQLineEdit() {}

void mQLineEdit::focusInEvent ( QFocusEvent *e )
{
   QLineEdit::focusInEvent( e );
   emit( focussed( true ) );
}

void mQLineEdit::focusOutEvent ( QFocusEvent *e )
{
   QLineEdit::focusOutEvent( e );
   emit( focussed( false ) );
}

void mQLineEdit::mousePressEvent ( QMouseEvent *e )
{
   QLineEdit::mousePressEvent( e );
   emit( pressed() );
}

mQLabel::mQLabel( QWidget *parent ) : QLabel( parent ) {}
mQLabel::mQLabel( const QString & text, QWidget *parent ) : QLabel( text, parent ) {}

mQLabel::~mQLabel() {}

void mQLabel::mousePressEvent ( QMouseEvent *e )
{
   QLabel::mousePressEvent( e );
   emit( pressed() );
}

void mQLabel::resizeEvent ( QResizeEvent *e )
{
   QLabel::resizeEvent( e );
   emit( resized() );
}

mQPushButton::mQPushButton( QWidget *parent ) : QPushButton( parent ) {}
mQPushButton::mQPushButton( const QString & text, QWidget *parent ) : QPushButton( text, parent ) {}

mQPushButton::~mQPushButton() {}

void mQPushButton::mouseDoubleClickEvent ( QMouseEvent *e )
{
   QPushButton::mouseDoubleClickEvent( e );
   emit( doubleClicked() );
}

void mQThread::sleep( unsigned long secs )
{
   QThread::sleep( secs );
}

void mQThread::msleep( unsigned long msecs )
{
   QThread::msleep( msecs );
}

void mQThread::usleep( unsigned long usecs )
{
   QThread::usleep( usecs );
}

void ShowHide::hide_widgets( const std::set < QWidget *> & widgets, bool hide, QWidget * do_resize ) {
   std::set < QWidget * > always_hide_widgets;
   return hide_widgets( widgets, always_hide_widgets, hide, do_resize );
}

void ShowHide::hide_widgets( const std::set < QWidget *> & widgets,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide,
                             QWidget * do_resize ) {
   for ( auto const & widget : widgets ) {
      hide || always_hide_widgets.count( widget ) ? widget->hide() : widget->show();
   }
   if ( do_resize ) {
       do_resize->resize( 0, 0 );
   }
}

void ShowHide::hide_widgets( const std::vector < QWidget *> & widgets, bool hide, QWidget * do_resize ) {
   std::set < QWidget * > always_hide_widgets;
   return hide_widgets( widgets, always_hide_widgets, hide, do_resize );
}

void ShowHide::hide_widgets( const std::vector < QWidget *> & widgets,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide, QWidget * do_resize ) {
   for ( unsigned int i = 0; i < ( unsigned int )widgets.size(); ++i ) {
      hide || always_hide_widgets.count( widgets[ i ] ) ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
   if ( do_resize ) {
       do_resize->resize( 0, 0 );
   }
}

void ShowHide::hide_widgets( const std::vector < std::vector < QWidget *> > & widgets, int row, bool hide, QWidget * do_resize ) {
   std::set < QWidget * > always_hide_widgets;
   return hide_widgets( widgets, row, always_hide_widgets, hide, do_resize );
}   

void ShowHide::hide_widgets( const std::vector < std::vector < QWidget *> > & widgets,
                             int row,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide,
                             QWidget * do_resize ) {
   if ( ( int )widgets.size() >= row ) {
      return;
   }
   for ( unsigned int i = 0; i < ( unsigned int )widgets[ row ].size(); ++i ) {
      hide || always_hide_widgets.count( widgets[ row ][ i ] ) ? widgets[ row ][ i ]->hide() : widgets[ row ][ i ]->show();
   }
   if ( do_resize ) {
       do_resize->resize( 0, 0 );
   }
}

void ShowHide::only_widgets( const std::vector < std::vector < QWidget *> > & widgets, int row, bool hide, QWidget * do_resize ) {
   std::set < QWidget * > always_hide_widgets;
   return only_widgets( widgets, row, always_hide_widgets, hide, do_resize );
}

void ShowHide::only_widgets( const std::vector < std::vector < QWidget *> > & widgets,
                             int row,
                             const std::set < QWidget *> & always_hide_widgets,
                             bool hide,
                             QWidget * do_resize ) {
   for ( int j = 0; j < (int) widgets.size(); ++j ) {
      if ( j != row ) {
         for ( int i = 0; i < (int) widgets[ j ].size(); ++i ) {
            hide || always_hide_widgets.count( widgets[ row ][ i ] ) ? widgets[ j ][ i ]->hide() : widgets[ j ][ i ]->show();
         }
      }
   }
   if ( do_resize ) {
       do_resize->resize( 0, 0 );
   }
}

QStringList MQT::get_lb_qsl( QListWidget * lb, bool only_selected )
{
   QStringList qsl;
   for ( int i = 0; i < lb->count(); ++i )
   {
      if ( !only_selected || lb->item( i )->isSelected() )
      {
         qsl << lb->item( i )->text();
      }
   }
   return qsl;
}

PC::PC( QColor bgc )
{
   push_back_color_if_ok( bgc, Qt::yellow );
   push_back_color_if_ok( bgc, Qt::green );
   push_back_color_if_ok( bgc, Qt::cyan );
   push_back_color_if_ok( bgc, Qt::blue );
   push_back_color_if_ok( bgc, Qt::red );
   push_back_color_if_ok( bgc, Qt::magenta );
   push_back_color_if_ok( bgc, Qt::darkYellow );
   push_back_color_if_ok( bgc, Qt::darkGreen );
   push_back_color_if_ok( bgc, Qt::darkCyan );
   push_back_color_if_ok( bgc, Qt::darkBlue );
   push_back_color_if_ok( bgc, Qt::darkRed );
   push_back_color_if_ok( bgc, Qt::darkMagenta );
   push_back_color_if_ok( bgc, Qt::white );
   push_back_color_if_ok( bgc, QColor( 240, 248, 255 ) ); /* Alice Blue */
   push_back_color_if_ok( bgc, QColor( 250, 235, 215 ) ); /* Antique White */
   push_back_color_if_ok( bgc, QColor( 0, 255, 255 ) ); /* Aqua* */
   push_back_color_if_ok( bgc, QColor( 127, 255, 212 ) ); /* Aquamarine */
   push_back_color_if_ok( bgc, QColor( 240, 255, 255 ) ); /* Azure */
   push_back_color_if_ok( bgc, QColor( 245, 245, 220 ) ); /* Beige */
   push_back_color_if_ok( bgc, QColor( 255, 228, 196 ) ); /* Bisque */
   push_back_color_if_ok( bgc, QColor( 0, 0, 0 ) ); /* Black* */
   push_back_color_if_ok( bgc, QColor( 255, 235, 205 ) ); /* Blanched Almond */
   push_back_color_if_ok( bgc, QColor( 0, 0, 255 ) ); /* Blue* */
   push_back_color_if_ok( bgc, QColor( 138, 43, 226 ) ); /* Blue-Violet */
   push_back_color_if_ok( bgc, QColor( 165, 42, 42 ) ); /* Brown */
   push_back_color_if_ok( bgc, QColor( 222, 184, 135 ) ); /* Burlywood */
   push_back_color_if_ok( bgc, QColor( 95, 158, 160 ) ); /* Cadet Blue */
   push_back_color_if_ok( bgc, QColor( 127, 255, 0 ) ); /* Chartreuse */
   push_back_color_if_ok( bgc, QColor( 210, 105, 30 ) ); /* Chocolate */
   push_back_color_if_ok( bgc, QColor( 255, 127, 80 ) ); /* Coral */
   push_back_color_if_ok( bgc, QColor( 100, 149, 237 ) ); /* Cornflower Blue */
   push_back_color_if_ok( bgc, QColor( 255, 248, 220 ) ); /* Cornsilk */
   push_back_color_if_ok( bgc, QColor( 0, 255, 255 ) ); /* Cyan */
   push_back_color_if_ok( bgc, QColor( 0, 0, 139 ) ); /* Dark Blue */
   push_back_color_if_ok( bgc, QColor( 0, 139, 139 ) ); /* Dark Cyan */
   push_back_color_if_ok( bgc, QColor( 184, 134, 11 ) ); /* Dark Goldenrod */
   push_back_color_if_ok( bgc, QColor( 169, 169, 169 ) ); /* Dark Gray */
   push_back_color_if_ok( bgc, QColor( 0, 100, 0 ) ); /* Dark Green */
   push_back_color_if_ok( bgc, QColor( 189, 183, 107 ) ); /* Dark Khaki */
   push_back_color_if_ok( bgc, QColor( 139, 0, 139 ) ); /* Dark Magenta */
   push_back_color_if_ok( bgc, QColor( 85, 107, 47 ) ); /* Dark Olive Green */
   push_back_color_if_ok( bgc, QColor( 255, 140, 0 ) ); /* Dark Orange */
   push_back_color_if_ok( bgc, QColor( 153, 50, 204 ) ); /* Dark Orchid */
   push_back_color_if_ok( bgc, QColor( 139, 0, 0 ) ); /* Dark Red */
   push_back_color_if_ok( bgc, QColor( 233, 150, 122 ) ); /* Dark Salmon */
   push_back_color_if_ok( bgc, QColor( 143, 188, 143 ) ); /* Dark Sea Green */
   push_back_color_if_ok( bgc, QColor( 72, 61, 139 ) ); /* Dark Slate Blue */
   push_back_color_if_ok( bgc, QColor( 47, 79, 79 ) ); /* Dark Slate Gray */
   push_back_color_if_ok( bgc, QColor( 0, 206, 209 ) ); /* Dark Turquoise */
   push_back_color_if_ok( bgc, QColor( 148, 0, 211 ) ); /* Dark Violet */
   push_back_color_if_ok( bgc, QColor( 255, 20, 147 ) ); /* Deep Pink */
   push_back_color_if_ok( bgc, QColor( 0, 191, 255 ) ); /* Deep Sky Blue */
   push_back_color_if_ok( bgc, QColor( 105, 105, 105 ) ); /* Dim Gray */
   push_back_color_if_ok( bgc, QColor( 30, 144, 255 ) ); /* Dodger Blue */
   push_back_color_if_ok( bgc, QColor( 178, 34, 34 ) ); /* Firebrick */
   push_back_color_if_ok( bgc, QColor( 255, 250, 240 ) ); /* Floral White */
   push_back_color_if_ok( bgc, QColor( 34, 139, 34 ) ); /* Forest Green */
   push_back_color_if_ok( bgc, QColor( 255, 0, 255 ) ); /* Fuschia* */
   push_back_color_if_ok( bgc, QColor( 220, 220, 220 ) ); /* Gainsboro */
   push_back_color_if_ok( bgc, QColor( 255, 250, 250 ) ); /* Ghost White */
   push_back_color_if_ok( bgc, QColor( 255, 215, 0 ) ); /* Gold */
   push_back_color_if_ok( bgc, QColor( 218, 165, 32 ) ); /* Goldenrod */
   push_back_color_if_ok( bgc, QColor( 128, 128, 128 ) ); /* Gray* */
   push_back_color_if_ok( bgc, QColor( 0, 128, 0 ) ); /* Green* */
   push_back_color_if_ok( bgc, QColor( 173, 255, 47 ) ); /* Green-Yellow */
   push_back_color_if_ok( bgc, QColor( 240, 255, 240 ) ); /* Honeydew */
   push_back_color_if_ok( bgc, QColor( 255, 105, 180 ) ); /* Hot Pink */
   push_back_color_if_ok( bgc, QColor( 205, 92, 92 ) ); /* Indian Red */
   push_back_color_if_ok( bgc, QColor( 255, 255, 240 ) ); /* Ivory */
   push_back_color_if_ok( bgc, QColor( 240, 230, 140 ) ); /* Khaki */
   push_back_color_if_ok( bgc, QColor( 230, 230, 250 ) ); /* Lavender */
   push_back_color_if_ok( bgc, QColor( 255, 240, 245 ) ); /* Lavender Blush */
   push_back_color_if_ok( bgc, QColor( 124, 252, 0 ) ); /* Lawn Green */
   push_back_color_if_ok( bgc, QColor( 255, 250, 205 ) ); /* Lemon Chiffon */
   push_back_color_if_ok( bgc, QColor( 173, 216, 230 ) ); /* Light Blue */
   push_back_color_if_ok( bgc, QColor( 240, 128, 128 ) ); /* Light Coral */
   push_back_color_if_ok( bgc, QColor( 224, 255, 255 ) ); /* Light Cyan */
   push_back_color_if_ok( bgc, QColor( 238, 221, 130 ) ); /* Light Goldenrod */
   push_back_color_if_ok( bgc, QColor( 250, 250, 210 ) ); /* Light Goldenrod Yellow */
   push_back_color_if_ok( bgc, QColor( 211, 211, 211 ) ); /* Light Gray */
   push_back_color_if_ok( bgc, QColor( 144, 238, 144 ) ); /* Light Green */
   push_back_color_if_ok( bgc, QColor( 255, 182, 193 ) ); /* Light Pink */
   push_back_color_if_ok( bgc, QColor( 255, 160, 122 ) ); /* Light Salmon */
   push_back_color_if_ok( bgc, QColor( 32, 178, 170 ) ); /* Light Sea Green */
   push_back_color_if_ok( bgc, QColor( 135, 206, 250 ) ); /* Light Sky Blue */
   push_back_color_if_ok( bgc, QColor( 132, 112, 255 ) ); /* Light Slate Blue */
   push_back_color_if_ok( bgc, QColor( 119, 136, 153 ) ); /* Light Slate Gray */
   push_back_color_if_ok( bgc, QColor( 176, 196, 222 ) ); /* Light Steel Blue */
   push_back_color_if_ok( bgc, QColor( 255, 255, 224 ) ); /* Light Yellow */
   push_back_color_if_ok( bgc, QColor( 0, 255, 0 ) ); /* Lime* */
   push_back_color_if_ok( bgc, QColor( 50, 205, 50 ) ); /* Lime Green */
   push_back_color_if_ok( bgc, QColor( 250, 240, 230 ) ); /* Linen */
   push_back_color_if_ok( bgc, QColor( 255, 0, 255 ) ); /* Magenta */
   push_back_color_if_ok( bgc, QColor( 128, 0, 0 ) ); /* Maroon* */
   push_back_color_if_ok( bgc, QColor( 102, 205, 170 ) ); /* Medium Aquamarine */
   push_back_color_if_ok( bgc, QColor( 0, 0, 205 ) ); /* Medium Blue */
   push_back_color_if_ok( bgc, QColor( 186, 85, 211 ) ); /* Medium Orchid */
   push_back_color_if_ok( bgc, QColor( 147, 112, 219 ) ); /* Medium Purple */
   push_back_color_if_ok( bgc, QColor( 60, 179, 113 ) ); /* Medium Sea Green */
   push_back_color_if_ok( bgc, QColor( 123, 104, 238 ) ); /* Medium Slate Blue */
   push_back_color_if_ok( bgc, QColor( 0, 250, 154 ) ); /* Medium Spring Green */
   push_back_color_if_ok( bgc, QColor( 72, 209, 204 ) ); /* Medium Turquoise */
   push_back_color_if_ok( bgc, QColor( 199, 21, 133 ) ); /* Medium Violet-Red */
   push_back_color_if_ok( bgc, QColor( 25, 25, 112 ) ); /* Midnight Blue */
   push_back_color_if_ok( bgc, QColor( 245, 255, 250 ) ); /* Mint Cream */
   push_back_color_if_ok( bgc, QColor( 255, 228, 225 ) ); /* Misty Rose */
   push_back_color_if_ok( bgc, QColor( 255, 228, 181 ) ); /* Moccasin */
   push_back_color_if_ok( bgc, QColor( 255, 222, 173 ) ); /* Navajo White */
   push_back_color_if_ok( bgc, QColor( 0, 0, 128 ) ); /* Navy* */
   push_back_color_if_ok( bgc, QColor( 253, 245, 230 ) ); /* Old Lace */
   push_back_color_if_ok( bgc, QColor( 128, 128, 0 ) ); /* Olive* */
   push_back_color_if_ok( bgc, QColor( 107, 142, 35 ) ); /* Olive Drab */
   push_back_color_if_ok( bgc, QColor( 255, 165, 0 ) ); /* Orange */
   push_back_color_if_ok( bgc, QColor( 255, 69, 0 ) ); /* Orange-Red */
   push_back_color_if_ok( bgc, QColor( 218, 112, 214 ) ); /* Orchid */
   push_back_color_if_ok( bgc, QColor( 238, 232, 170 ) ); /* Pale Goldenrod */
   push_back_color_if_ok( bgc, QColor( 152, 251, 152 ) ); /* Pale Green */
   push_back_color_if_ok( bgc, QColor( 175, 238, 238 ) ); /* Pale Turquoise */
   push_back_color_if_ok( bgc, QColor( 219, 112, 147 ) ); /* Pale Violet-Red */
   push_back_color_if_ok( bgc, QColor( 255, 239, 213 ) ); /* Papaya Whip */
   push_back_color_if_ok( bgc, QColor( 255, 218, 185 ) ); /* Peach Puff */
   push_back_color_if_ok( bgc, QColor( 205, 133, 63 ) ); /* Peru */
   push_back_color_if_ok( bgc, QColor( 255, 192, 203 ) ); /* Pink */
   push_back_color_if_ok( bgc, QColor( 221, 160, 221 ) ); /* Plum */
   push_back_color_if_ok( bgc, QColor( 176, 224, 230 ) ); /* Powder Blue */
   push_back_color_if_ok( bgc, QColor( 128, 0, 128 ) ); /* Purple* */
   push_back_color_if_ok( bgc, QColor( 255, 0, 0 ) ); /* Red* */
   push_back_color_if_ok( bgc, QColor( 188, 143, 143 ) ); /* Rosy Brown */
   push_back_color_if_ok( bgc, QColor( 65, 105, 225 ) ); /* Royal Blue */
   push_back_color_if_ok( bgc, QColor( 139, 69, 19 ) ); /* Saddle Brown */
   push_back_color_if_ok( bgc, QColor( 250, 128, 114 ) ); /* Salmon */
   push_back_color_if_ok( bgc, QColor( 244, 164, 96 ) ); /* Sandy Brown */
   push_back_color_if_ok( bgc, QColor( 46, 139, 87 ) ); /* Sea Green */
   push_back_color_if_ok( bgc, QColor( 255, 245, 238 ) ); /* Seashell */
   push_back_color_if_ok( bgc, QColor( 160, 82, 45 ) ); /* Sienna */
   push_back_color_if_ok( bgc, QColor( 192, 192, 192 ) ); /* Silver* */
   push_back_color_if_ok( bgc, QColor( 135, 206, 235 ) ); /* Sky Blue */
   push_back_color_if_ok( bgc, QColor( 106, 90, 205 ) ); /* Slate Blue */
   push_back_color_if_ok( bgc, QColor( 112, 128, 144 ) ); /* Slate Gray */
   push_back_color_if_ok( bgc, QColor( 255, 250, 250 ) ); /* Snow */
   push_back_color_if_ok( bgc, QColor( 0, 255, 127 ) ); /* Spring Green */
   push_back_color_if_ok( bgc, QColor( 70, 130, 180 ) ); /* Steel Blue */
   push_back_color_if_ok( bgc, QColor( 210, 180, 140 ) ); /* Tan */
   push_back_color_if_ok( bgc, QColor( 0, 128, 128 ) ); /* Teal* */
   push_back_color_if_ok( bgc, QColor( 216, 191, 216 ) ); /* Thistle */
   push_back_color_if_ok( bgc, QColor( 255, 99, 71 ) ); /* Tomato */
   push_back_color_if_ok( bgc, QColor( 64, 224, 208 ) ); /* Turquoise */
   push_back_color_if_ok( bgc, QColor( 238, 130, 238 ) ); /* Violet */
   push_back_color_if_ok( bgc, QColor( 208, 32, 144 ) ); /* Violet-Red */
   push_back_color_if_ok( bgc, QColor( 245, 222, 179 ) ); /* Wheat */
   push_back_color_if_ok( bgc, QColor( 255, 255, 255 ) ); /* White* */
   push_back_color_if_ok( bgc, QColor( 245, 245, 245 ) ); /* White Smoke */
   push_back_color_if_ok( bgc, QColor( 255, 255, 0 ) ); /* Yellow* */
   push_back_color_if_ok( bgc, QColor( 154, 205, 50 ) ); /* Yellow-Green */
}

void PC::color_rotate()
{
   std::vector < QColor >  new_plot_colors;

   for ( int i = 1; i < (int) plot_colors.size(); i++ )
   {
      new_plot_colors.push_back( plot_colors[ i ] );
   }
   new_plot_colors.push_back( plot_colors[ 0 ] );
   plot_colors = new_plot_colors;
}

void PC::push_back_color_if_ok( QColor bg, QColor set )
{
   double sum = 
      fabs( (float) bg.red  () - (float) set.red  () ) +
      fabs( (float) bg.green() - (float) set.green() ) +
      fabs( (float) bg.blue () - (float) set.blue () );
   if ( sum > 150 )
   {
      if ( plot_colors.size() )
      {
         bg = plot_colors.back();
         double sum = 
            fabs( (float) bg.red  () - (float) set.red  () ) +
            fabs( (float) bg.green() - (float) set.green() ) +
            fabs( (float) bg.blue () - (float) set.blue () );
         if ( sum > 100 )
         {
            plot_colors.push_back( set );
         }
      } else {
         plot_colors.push_back( set );
      }
   }
}

QColor PC::color( int i )
{
   return plot_colors[ i % plot_colors.size() ];
}
