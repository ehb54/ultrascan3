//! \file us_gmp_progress.h
//! \brief Shared, single, non-closable progress dialog for the GMP Reporter and the autoflow Analysis.
#ifndef US_GMP_PROGRESS_H
#define US_GMP_PROGRESS_H

#include <QtWidgets>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QScreen>

#include "us_gui_settings.h"

/**
 * @class US_GmpProgress
 * @brief Single, persistent, non-closable progress dialog for the GMP Reporter.
 *
 * One instance is created (lazily, by US_ReporterGMP::gmp_progress()) as a child
 * of the reporter's top-level window and is re-used for every stage of a run
 * (stand-alone and autoflow): it is only ever hidden (finish()), never closed
 * or re-created between stages.  The user cannot close it (title-bar close,
 * Esc and Alt+F4 are all swallowed).  It is window-modal to its parent and is
 * re-centered over the parent window every time it is (re)shown.
 *
 * Light default-theme look.  Layout: stage title ("Step 2 of 4: ..."), a detail line (what is happening
 * now), an optional "overall" bar (e.g. triple/model k of N) and a "step" bar.
 *
 * For source compatibility with the former QProgressDialog usage it also offers
 * setValue()/value()/maximum()/setRange()/setLabelText(), which drive the step
 * bar / detail line.
 */
class US_GmpProgress : public QDialog
{
  public:
    explicit US_GmpProgress( QWidget* parent )
      : QDialog( parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint ),
        stage_key()
    {
      setWindowTitle( tr( "GMP Report Generator" ) );
      setWindowModality( Qt::WindowModal );

      // Plain, light look (default application palette/theme, like the other
      // "Please Wait" dialogs) -- deliberately NOT the teal UltraScan frame palette.
      setPalette( QApplication::palette() );

      const QString fam = US_GuiSettings::fontFamily();
      const int     fsz = US_GuiSettings::fontSize();

      lb_stage   = new QLabel( this );
      lb_stage->setFont( QFont( fam, fsz + 1, QFont::Bold ) );
      lb_stage->setAlignment( Qt::AlignCenter );
      lb_stage->setWordWrap( true );     // long stage titles wrap instead of being cut off
      lb_stage->setMinimumHeight( lb_stage->fontMetrics().lineSpacing() + 4 );

      QFrame* line = new QFrame( this );
      line->setFrameShape ( QFrame::HLine );
      line->setFrameShadow( QFrame::Sunken );

      lb_detail  = new QLabel( this );
      lb_detail->setFont( QFont( fam, fsz, QFont::Normal ) );
      lb_detail->setWordWrap( true );
      lb_detail->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
      lb_detail->setMinimumHeight( 2 * lb_detail->fontMetrics().lineSpacing() + 6 );

      lb_overall = new QLabel( this );
      lb_overall->setFont( QFont( fam, fsz, QFont::Bold ) );
      lb_overall->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

      pb_overall = new QProgressBar( this );
      pb_step    = new QProgressBar( this );
      for ( QProgressBar* pb : { pb_overall, pb_step } )
      {
        pb->setRange( 0, 1 );
        pb->setValue( 0 );
        pb->setAlignment( Qt::AlignCenter );
        pb->setFont( QFont( fam, fsz, QFont::Bold ) );
        pb->setMinimumHeight( 22 );
      }
      pb_overall->setFormat( "%v / %m" );

      QVBoxLayout* lo = new QVBoxLayout( this );
      lo->setContentsMargins( 16, 12, 16, 16 );
      lo->setSpacing( 8 );
      lo->addWidget( lb_stage   );
      lo->addWidget( line       );
      lo->addWidget( lb_detail  );
      lo->addWidget( lb_overall );
      lo->addWidget( pb_overall );
      lo->addWidget( pb_step    );
      lo->setSizeConstraint( QLayout::SetFixedSize );   // user cannot resize it

      setMinimumWidth( 540 );
      lb_overall->hide();
      pb_overall->hide();
      clock.start();
    }

    //! Get the one dialog owned by `owner` (via `dlg`), creating it on first use as a child
    //! of owner->window() -- the main window when `owner` is embedded in one.  Re-parents
    //! (re-creates) it only if the owner has since moved to another top-level window.
    static US_GmpProgress* acquire( QWidget* owner, US_GmpProgress*& dlg )
    {
      QWidget* top = owner->window();
      if ( dlg  &&  dlg->parentWidget() != top  &&  ! dlg->isVisible() )
      {
        delete dlg;
        dlg = nullptr;
      }
      if ( ! dlg )
        dlg = new US_GmpProgress( top );
      return dlg;
    }

    //! Enter (or continue) stage n of total (total <= 0: show the title only).  A different stage resets/hides the
    //! overall bar; the step bar is reset to 0..step_max.  Shows the dialog if hidden.
    void setStage( int n, int total, const QString& title, const QString& detail, int step_max )
    {
      const QString key = QString( "%1/%2/%3" ).arg( n ).arg( total ).arg( title );
      bool newstage = ( key != stage_key );
      stage_key = key;
      lb_stage->setText( ( total > 0 ) ? tr( "Step %1 of %2:  %3" ).arg( n ).arg( total ).arg( title )
                                       : title );     // total <= 0: title only, no "Step n of m"
      lb_detail->setText( detail );
      pb_step->setRange( 0, step_max );
      pb_step->setValue( 0 );
      if ( newstage )
        showOverall( false );
      showDialog();
    }

    //! Overall bar (e.g. "Triple/model k of N").
    void setOverall( int value, int maxval, const QString& caption )
    {
      lb_overall->setText( caption );
      pb_overall->setRange( 0, maxval );
      pb_overall->setValue( value );
      showOverall( true );
      pump( true );
    }

    void hideOverall( void ) { showOverall( false ); }

    //! Detail line only.
    void setDetail( const QString& text ) { lb_detail->setText( text ); pump( true ); }

    //! Detail line + indeterminate step bar (work with no measurable steps).
    void setBusy( const QString& text )
    {
      lb_detail->setText( text );
      pb_step->setRange( 0, 0 );
      showDialog();
    }

    //! The only way the dialog goes away.
    void finish( void ) { hide(); }

    // --- QProgressDialog-compatible subset (drives the step bar / detail line) ---
    void setValue    ( int v )              { pb_step->setValue( v ); pump(); }
    int  value       ( void ) const         { return pb_step->value(); }
    int  maximum     ( void ) const         { return pb_step->maximum(); }
    void setRange    ( int lo, int hi )     { pb_step->setRange( lo, hi ); pb_step->setValue( lo ); pump(); }
    void setLabelText( const QString& t )   { setDetail( t ); }

  protected:
    // Not closable by the user: close button, Alt+F4 and Esc are all ignored.
    void closeEvent   ( QCloseEvent* e ) override { e->ignore(); }
    void reject       ( void )           override { }
    void keyPressEvent( QKeyEvent* e )   override
    {
      if ( e->key() == Qt::Key_Escape ) { e->accept(); return; }
      QDialog::keyPressEvent( e );
    }
    void showEvent    ( QShowEvent* e )  override
    {
      QDialog::showEvent( e );
      place();
    }

  private:
    QLabel*       lb_stage;
    QLabel*       lb_detail;
    QLabel*       lb_overall;
    QProgressBar* pb_overall;
    QProgressBar* pb_step;
    QString       stage_key;
    QElapsedTimer clock;

    void showOverall( bool on )
    {
      if ( lb_overall->isVisible() == on  &&  pb_overall->isVisible() == on  &&  isVisible() )
        return;
      lb_overall->setVisible( on );
      pb_overall->setVisible( on );
      if ( isVisible() )
        place();
    }

    void showDialog( void )
    {
      if ( ! isVisible() )
        show();            // showEvent() centers it
      else
        place();
      raise();
      pump( true );
    }

    // Center over the parent's top-level window (or the screen), kept on-screen.
    void place( void )
    {
      adjustSize();
      QWidget* top = parentWidget() ? parentWidget()->window() : nullptr;
      QRect    area;
      if ( top  &&  top->isVisible()  &&  ! top->isMinimized() )
        area = top->frameGeometry();
      else
        area = QGuiApplication::primaryScreen()->availableGeometry();

      QPoint   p   = area.center() - QPoint( width() / 2, height() / 2 );
      QScreen* scr = QGuiApplication::screenAt( area.center() );
      if ( scr == nullptr )
        scr = QGuiApplication::primaryScreen();
      QRect    sg  = scr->availableGeometry();
      p.setX( qBound( sg.left(), p.x(), qMax( sg.left(), sg.right()  - width()  ) ) );
      p.setY( qBound( sg.top(),  p.y(), qMax( sg.top(),  sg.bottom() - height() ) ) );
      move( p );
    }

    // Repaint (the old modal QProgressDialog::setValue() did this implicitly);
    // throttled so fast per-component updates stay cheap.
    void pump( bool force = false )
    {
      if ( ! isVisible() )
        return;
      if ( force  ||  clock.elapsed() > 40 )
      {
        clock.restart();
        QCoreApplication::processEvents();
      }
    }
};

#endif
