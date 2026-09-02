//! \file us_style.h
#ifndef US_STYLE_H
#define US_STYLE_H

#include <QtWidgets>

#include "us_extern.h"

//! \brief The UltraScan widget style: modern shapes, per-widget palettes.
class US_GUI_EXTERN US_Style : public QProxyStyle
{
   Q_OBJECT

   public:
      //! \brief Wrap a base style
      //! \param base The style to fall back to.  US_Style takes ownership.
      explicit US_Style( QStyle* base );

      void drawPrimitive     ( PrimitiveElement, const QStyleOption*,
                               QPainter*, const QWidget* = nullptr )
                               const override;

      void drawControl       ( ControlElement, const QStyleOption*,
                               QPainter*, const QWidget* = nullptr )
                               const override;

      void drawComplexControl( ComplexControl, const QStyleOptionComplex*,
                               QPainter*, const QWidget* = nullptr )
                               const override;

      int  pixelMetric       ( PixelMetric, const QStyleOption* = nullptr,
                               const QWidget* = nullptr ) const override;

      int  styleHint         ( StyleHint, const QStyleOption* = nullptr,
                               const QWidget* = nullptr,
                               QStyleHintReturn* = nullptr ) const override;
};

#endif
