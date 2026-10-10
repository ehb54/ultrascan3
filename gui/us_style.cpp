//! \file us_style.cpp
#include "us_style.h"
#include "us_theme.h"

namespace
{
   //! The palette group that matches the state a widget is painted in.
   /*!  Doing this explicitly rather than relying on the palette's current
        group is what makes the "disabled" and "inactive" colors of the color
        configuration panel take effect.
   */
   QPalette::ColorGroup group_of( const QStyleOption* option )
   {
      if ( ! ( option->state & QStyle::State_Enabled ) )
         return QPalette::Disabled;

      if ( ! ( option->state & QStyle::State_Active ) )
         return QPalette::Inactive;

      return QPalette::Active;
   }

   //! A rounded rectangle that sits exactly on the pixel grid
   void round_rect( QPainter* p, const QRect& rect, const qreal rad,
                    const QBrush& fill, const QColor& line )
   {
      const bool  aa = p->testRenderHint( QPainter::Antialiasing );
      const QRectF r = line.isValid()
                     ? QRectF( rect ).adjusted( 0.5, 0.5, -0.5, -0.5 )
                     : QRectF( rect );

      p->setRenderHint( QPainter::Antialiasing, true );
      p->setPen  ( line.isValid() ? QPen( line, 1.0 ) : QPen( Qt::NoPen ) );
      p->setBrush( fill );
      p->drawRoundedRect( r, rad, rad );
      p->setRenderHint( QPainter::Antialiasing, aa );
   }

   //! The outline of an interactive widget: accent while focused or hovered
   QColor outline_of( const QStyleOption* option, const QPalette::ColorGroup cg )
   {
      const QPalette& pal = option->palette;

      if ( option->state & QStyle::State_Enabled )
      {
         if ( option->state & QStyle::State_HasFocus )
            return pal.color( cg, QPalette::Highlight );

         if ( option->state & QStyle::State_MouseOver )
            return pal.color( cg, QPalette::Highlight );
      }

      return pal.color( cg, QPalette::Mid );
   }

   //! The check mark of a checked check box
   void draw_check( QPainter* p, const QRect& rect, const QColor& color )
   {
      const qreal   w = rect.width();
      const QPointF o = rect.topLeft();

      QPainterPath path;
      path.moveTo( o + QPointF( w * 0.22, w * 0.52 ) );
      path.lineTo( o + QPointF( w * 0.42, w * 0.72 ) );
      path.lineTo( o + QPointF( w * 0.78, w * 0.28 ) );

      const bool aa = p->testRenderHint( QPainter::Antialiasing );
      p->setRenderHint( QPainter::Antialiasing, true );
      p->setBrush( Qt::NoBrush );
      p->setPen  ( QPen( color, qMax( 1.6, w * 0.14 ),
                         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
      p->drawPath( path );
      p->setRenderHint( QPainter::Antialiasing, aa );
   }

   //! Shrink a sub-control rectangle down to an arrow sized square
   QRect arrow_rect( const QRect& rect, const qreal fraction = 0.66 )
   {
      const int side = qMax( 7, qRound( qMin( rect.width(), rect.height() )
                                        * fraction ) );
      QRect     r( 0, 0, side, side );
      r.moveCenter( rect.center() );
      return r;
   }
}

US_Style::US_Style( QStyle* base ) : QProxyStyle( base )
{
}

void US_Style::drawPrimitive( PrimitiveElement element,
                              const QStyleOption* option, QPainter* painter,
                              const QWidget* widget ) const
{
   const QPalette&           pal = option->palette;
   const QPalette::ColorGroup cg = group_of( option );
   const qreal               rad = US_Theme::radius();
   const qreal              srad = US_Theme::smallRadius();

   switch ( element )
   {
      // ---- Push buttons and tool buttons --------------------------------
      case PE_PanelButtonCommand:
      case PE_PanelButtonBevel:
      case PE_PanelButtonTool:
      {
         QColor fill = pal.color( cg, QPalette::Button );

         if ( option->state & State_Enabled )
         {
            if ( option->state & ( State_Sunken | State_On ) )
               fill = pal.color( cg, QPalette::Dark );

            else if ( option->state & State_MouseOver )
               fill = pal.color( cg, QPalette::Midlight );
         }

         round_rect( painter, option->rect, rad, fill,
                     outline_of( option, cg ) );
         return;
      }

      // Buttons and entry fields show keyboard focus through their accent
      // outline, so a second rectangle inside them is only noise.  Everything
      // else still needs a visible focus mark.
      case PE_FrameFocusRect:
      {
         if ( qobject_cast< const QAbstractButton*  >( widget ) != nullptr &&
              qobject_cast< const QCheckBox*        >( widget ) == nullptr &&
              qobject_cast< const QRadioButton*     >( widget ) == nullptr )
            return;

         if ( qobject_cast< const QLineEdit*        >( widget ) != nullptr ||
              qobject_cast< const QComboBox*        >( widget ) != nullptr ||
              qobject_cast< const QAbstractSpinBox* >( widget ) != nullptr )
            return;

         const bool aa = painter->testRenderHint( QPainter::Antialiasing );
         painter->setRenderHint( QPainter::Antialiasing, true );
         painter->setBrush( Qt::NoBrush );
         painter->setPen  ( QPen( pal.color( cg, QPalette::Highlight ), 1.0,
                                  Qt::DotLine ) );
         painter->drawRoundedRect(
            QRectF( option->rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ),
            srad, srad );
         painter->setRenderHint( QPainter::Antialiasing, aa );
         return;
      }

      // A default button gets its emphasis from the outline as well
      case PE_FrameDefaultButton:
         return;

      // ---- Entry fields --------------------------------------------------
      case PE_PanelLineEdit:
      {
         const auto* f =
            qstyleoption_cast< const QStyleOptionFrame* >( option );

         // A line edit inside a spin box or combo box has no frame of its
         // own - the complex control draws it.
         if ( f != nullptr  &&  f->lineWidth == 0 )
            break;

         round_rect( painter, option->rect, rad,
                     pal.brush( cg, QPalette::Base ),
                     outline_of( option, cg ) );
         return;
      }

      // Drawn as part of PE_PanelLineEdit above
      case PE_FrameLineEdit:
         return;

      // ---- Frames --------------------------------------------------------
      case PE_Frame:
      case PE_FrameGroupBox:
      case PE_FrameTabWidget:
      {
         // The content of a scroll area is filled square, so the four corners
         // outside the rounded outline have to be given back to the window
         // before the outline is drawn.  In a popup there is no window behind
         // the frame to give them back to.
         const bool popup = ( widget != nullptr &&
                              widget->window() != nullptr &&
                              ( widget->window()->windowFlags() & Qt::Popup )
                                 == Qt::Popup );

         if ( ! popup )
         {
            QPainterPath square;
            QPainterPath rounded;
            square .addRect( option->rect );
            rounded.addRoundedRect( QRectF( option->rect ), rad, rad );

            const bool aa = painter->testRenderHint( QPainter::Antialiasing );
            painter->setRenderHint( QPainter::Antialiasing, true );
            painter->fillPath( square.subtracted( rounded ),
                               pal.brush( cg, QPalette::Window ) );
            painter->setRenderHint( QPainter::Antialiasing, aa );
         }

         round_rect( painter, option->rect, rad, Qt::NoBrush,
                     pal.color( cg, QPalette::Mid ) );
         return;
      }

      // ---- Item view rows -------------------------------------------------
      case PE_PanelItemViewItem:
      {
         if ( option->state & State_Selected )
         {
            round_rect( painter, option->rect, srad,
                        pal.brush( cg, QPalette::Highlight ), QColor() );
            return;
         }

         if ( ( option->state & State_MouseOver ) &&
              ( option->state & State_Enabled ) )
         {
            round_rect( painter, option->rect, srad,
                        pal.brush( cg, QPalette::Midlight ), QColor() );
            return;
         }

         break;   // Let the base style paint alternating row colors
      }

      // ---- Check boxes and radio buttons ----------------------------------
      case PE_IndicatorCheckBox:
      {
         const bool on  = option->state & State_On;
         const bool tri = option->state & State_NoChange;

         const QColor fill = ( on || tri )
                           ? pal.color( cg, QPalette::Highlight )
                           : pal.color( cg, QPalette::Base );
         const QColor line = ( on || tri )
                           ? pal.color( cg, QPalette::Highlight )
                           : outline_of( option, cg );

         round_rect( painter, option->rect, srad, fill, line );

         if ( on )
            draw_check( painter, option->rect,
                        pal.color( cg, QPalette::HighlightedText ) );

         else if ( tri )
         {
            QRect bar = option->rect.adjusted( option->rect.width()  / 4,
                                               option->rect.height() / 2 - 1,
                                              -option->rect.width()  / 4,
                                              -option->rect.height() / 2 + 1 );
            painter->fillRect( bar,
                               pal.color( cg, QPalette::HighlightedText ) );
         }

         return;
      }

      case PE_IndicatorRadioButton:
      {
         const bool   on   = option->state & State_On;
         const QColor fill = on ? pal.color( cg, QPalette::Highlight )
                                : pal.color( cg, QPalette::Base );
         const QColor line = on ? pal.color( cg, QPalette::Highlight )
                                : outline_of( option, cg );

         const bool aa = painter->testRenderHint( QPainter::Antialiasing );
         painter->setRenderHint( QPainter::Antialiasing, true );
         painter->setPen  ( QPen( line, 1.0 ) );
         painter->setBrush( fill );
         painter->drawEllipse(
            QRectF( option->rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );

         if ( on )
         {
            QRectF dot = QRectF( option->rect ).adjusted(
                            option->rect.width() * 0.3,
                            option->rect.width() * 0.3,
                           -option->rect.width() * 0.3,
                           -option->rect.width() * 0.3 );
            painter->setPen  ( Qt::NoPen );
            painter->setBrush( pal.color( cg, QPalette::HighlightedText ) );
            painter->drawEllipse( dot );
         }

         painter->setRenderHint( QPainter::Antialiasing, aa );
         return;
      }

      default:
         break;
   }

   QProxyStyle::drawPrimitive( element, option, painter, widget );
}

void US_Style::drawControl(const ControlElement element, const QStyleOption* option,
                            QPainter* painter, const QWidget* widget ) const
{
   const QPalette&           pal = option->palette;
   const QPalette::ColorGroup cg = group_of( option );
   const qreal               rad = US_Theme::radius();

   switch ( element )
   {
      // ---- Progress bars ---------------------------------------------------
      case CE_ProgressBarGroove:
      {
         round_rect( painter, option->rect, rad,
                     pal.brush( cg, QPalette::Base ),
                     pal.color( cg, QPalette::Mid ) );
         return;
      }

      case CE_ProgressBarContents:
      {
         const auto* pb =
            qstyleoption_cast< const QStyleOptionProgressBar* >( option );

         if ( pb == nullptr )
            break;

         const QRect groove = option->rect.adjusted( 1, 1, -1, -1 );

         if ( groove.width() < 1  ||  groove.height() < 1 )
            return;

         if ( pb->minimum == pb->maximum )     // Busy indicator
            break;

         const double span = static_cast<double>(pb->maximum) - static_cast<double>(pb->minimum);
         const double done = qBound( 0.0,
                                     ( static_cast<double>(pb->progress) -
                                       static_cast<double>(pb->minimum) ) / span, 1.0 );
         const int    wid  = qRound( groove.width() * done );

         if ( wid < 1 )
            return;

         painter->save();
         painter->setClipRect( QRect( groove.x(), groove.y(),
                                      wid, groove.height() ) );
         round_rect( painter, groove, rad - 1,
                     pal.brush( cg, QPalette::Highlight ), QColor() );
         painter->restore();
         return;
      }

      // ---- Item view headers ------------------------------------------------
      case CE_HeaderSection:
      {
         painter->fillRect( option->rect, pal.brush( cg, QPalette::Button ) );
         painter->setPen  ( pal.color( cg, QPalette::Mid ) );
         painter->drawLine( option->rect.bottomLeft(),
                            option->rect.bottomRight() );
         painter->drawLine( option->rect.topRight(),
                            option->rect.bottomRight() );
         return;
      }

      // ---- Tabs --------------------------------------------------------------
      case CE_TabBarTabShape:
      {
         const auto* tab =
            qstyleoption_cast< const QStyleOptionTab* >( option );

         if ( tab == nullptr )
            break;

         // A flat tab with an accent underline instead of a raised bevel
         const bool selected = option->state & State_Selected;
         const bool hovered  = ( option->state & State_MouseOver ) &&
                               ( option->state & State_Enabled );

         if ( hovered && ! selected )
            round_rect( painter, option->rect.adjusted( 1, 1, -1, -1 ),
                        US_Theme::smallRadius(),
                        pal.brush( cg, QPalette::Midlight ), QColor() );

         if ( selected )
         {
            constexpr int    thick = 2;
            const QColor bar   = pal.color( cg, QPalette::Highlight );
            QRect        r     = option->rect;

            switch ( tab->shape )
            {
               case QTabBar::RoundedSouth:
               case QTabBar::TriangularSouth:
                  r.setHeight( thick );
                  break;
               case QTabBar::RoundedWest:
               case QTabBar::TriangularWest:
                  r.setX( r.right() - thick );
                  break;
               case QTabBar::RoundedEast:
               case QTabBar::TriangularEast:
                  r.setWidth( thick );
                  break;
               default:                       // North
                  r.setY( r.bottom() - thick );
                  break;
            }

            painter->fillRect( r, bar );
         }

         return;
      }

      default:
         break;
   }

   QProxyStyle::drawControl( element, option, painter, widget );
}

void US_Style::drawComplexControl( ComplexControl control,
                                   const QStyleOptionComplex* option,
                                   QPainter* painter,
                                   const QWidget* widget ) const
{
   const QPalette&           pal = option->palette;
   const QPalette::ColorGroup cg = group_of( option );
   const qreal               rad = US_Theme::radius();

   switch ( control )
   {
      // ---- Scroll bars ------------------------------------------------------
      case CC_ScrollBar:
      {
         const auto* sb =
            qstyleoption_cast< const QStyleOptionSlider* >( option );

         if ( sb == nullptr )
            break;

         const QRect handle = proxy()->subControlRect( CC_ScrollBar, sb,
                                                       SC_ScrollBarSlider,
                                                       widget );

         if ( handle.isEmpty() )
            return;                            // Nothing to scroll

         QColor fill = pal.color( cg, QPalette::Mid );

         if ( ( option->state & State_Enabled ) &&
              ( ( option->activeSubControls & SC_ScrollBarSlider ) ||
                ( option->state & State_Sunken ) ) )
            fill = pal.color( cg, QPalette::Dark );

         round_rect( painter, handle.adjusted( 2, 2, -2, -2 ),
                     US_Theme::smallRadius(), fill, QColor() );
         return;                               // No groove, no arrow buttons
      }

      // ---- Combo boxes --------------------------------------------------------
      case CC_ComboBox:
      {
         const auto* cb =
            qstyleoption_cast< const QStyleOptionComboBox* >( option );

         if ( cb == nullptr )
            break;

         QColor fill = cb->editable ? pal.color( cg, QPalette::Base )
                                    : pal.color( cg, QPalette::Button );

         if ( ! cb->editable  &&  ( option->state & State_Enabled ) )
         {
            if ( option->state & State_Sunken )
               fill = pal.color( cg, QPalette::Dark );
            else if ( option->state & State_MouseOver )
               fill = pal.color( cg, QPalette::Midlight );
         }

         round_rect( painter, option->rect, rad, fill,
                     outline_of( option, cg ) );

         QStyleOptionComplex arrow = *option;
         arrow.rect = arrow_rect( proxy()->subControlRect( CC_ComboBox, cb,
                                                           SC_ComboBoxArrow,
                                                           widget ) );
         proxy()->drawPrimitive( PE_IndicatorArrowDown, &arrow, painter,
                                 widget );
         return;
      }

      // ---- Spin boxes ----------------------------------------------------------
      case CC_SpinBox:
      {
         const auto* sp =
            qstyleoption_cast< const QStyleOptionSpinBox* >( option );

         if ( sp == nullptr )
            break;

         // Plus/minus symbols are rare and style specific - leave those to
         // the base style rather than half-drawing them.
         if ( sp->buttonSymbols == QAbstractSpinBox::PlusMinus )
            break;

         if ( sp->frame )
            round_rect( painter, option->rect, rad,
                        pal.brush( cg, QPalette::Base ),
                        outline_of( option, cg ) );

         if ( sp->buttonSymbols == QAbstractSpinBox::NoButtons )
            return;

         const bool up_on = sp->stepEnabled & QAbstractSpinBox::StepUpEnabled;
         const bool dn_on = sp->stepEnabled & QAbstractSpinBox::StepDownEnabled;

         QStyleOptionComplex arrow = *option;

         arrow.rect  = arrow_rect( proxy()->subControlRect( CC_SpinBox, sp,
                                                            SC_SpinBoxUp,
                                                            widget ), 1.0 );
         arrow.state = option->state;
         if ( ! up_on ) arrow.state &= ~State_Enabled;
         proxy()->drawPrimitive( PE_IndicatorArrowUp, &arrow, painter,
                                 widget );

         arrow.rect  = arrow_rect( proxy()->subControlRect( CC_SpinBox, sp,
                                                            SC_SpinBoxDown,
                                                            widget ), 1.0 );
         arrow.state = option->state;
         if ( ! dn_on ) arrow.state &= ~State_Enabled;
         proxy()->drawPrimitive( PE_IndicatorArrowDown, &arrow, painter,
                                 widget );

         return;
      }

      default:
         break;
   }

   QProxyStyle::drawComplexControl( control, option, painter, widget );
}

int US_Style::pixelMetric(const PixelMetric metric, const QStyleOption* option,
                           const QWidget* widget ) const
{
   switch ( metric )
   {
      case PM_ScrollBarExtent:      return 13;   // Thin, modern scroll bars
      case PM_ScrollBarSliderMin:   return 28;
      case PM_ButtonDefaultIndicator:
      case PM_DefaultFrameWidth:    break;
      default:                      break;
   }

   return QProxyStyle::pixelMetric( metric, option, widget );
}

int US_Style::styleHint(const StyleHint hint, const QStyleOption* option,
                         const QWidget* widget, QStyleHintReturn* ret ) const
{
   switch ( hint )
   {
      // Embossed or dithered disabled text is a hallmark of the old look
      case SH_EtchDisabledText:      return 0;
      case SH_DitherDisabledText:    return 0;
      default:                       break;
   }

   return QProxyStyle::styleHint( hint, option, widget, ret );
}
