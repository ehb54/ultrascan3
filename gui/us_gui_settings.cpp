#include "us_gui_settings.h"
#include "us_theme.h"
#include "us_defines.h"

namespace
{
   //! One entry of a palette color table
   struct RoleColor
   {
      QPalette::ColorRole role;
      QColor              color;
   };

   //! Apply a role/color table to the Active and Inactive groups of a palette
   void set_enabled( QPalette& p, const QList< RoleColor >& colors )
   {
      for ( int ii = 0; ii < colors.size(); ii++ )
      {
         p.setColor( QPalette::Active  , colors[ ii ].role, colors[ ii ].color );
         p.setColor( QPalette::Inactive, colors[ ii ].role, colors[ ii ].color );
      }
   }

   //! Apply a role/color table to the Disabled group of a palette
   void set_disabled( QPalette& p, const QList< RoleColor >& colors )
   {
      for ( int ii = 0; ii < colors.size(); ii++ )
         p.setColor( QPalette::Disabled, colors[ ii ].role, colors[ ii ].color );
   }
}

// Fonts
QString US_GuiSettings::defaultFontFamily( void ) {
    // The font the desktop uses for its own user interface.  Anything else
    // makes UltraScan stand out as a foreign application.
    const QString family = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();

    return family.isEmpty() ? QString("Helvetica") : family;
}

QString US_GuiSettings::fontFamily(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("fontFamily", defaultFontFamily()).toString();
}

void US_GuiSettings::set_fontFamily(const QString &fontFamily) {
    QSettings settings(US3, "UltraScan");
    if (fontFamily == defaultFontFamily())
        settings.remove("fontFamily");
    else
        settings.setValue("fontFamily", fontFamily);
}

int US_GuiSettings::fontSize(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("fontSize", 10).toInt();
}

void US_GuiSettings::set_fontSize(int fontSize) {
    QSettings settings(US3, "UltraScan");
    if (fontSize == 10)
        settings.remove("fontSize");
    else
        settings.setValue("fontSize", fontSize);
}

QString US_GuiSettings::guiStyle(void) {
    QSettings settings(US3, "UltraScan");

    return settings.value("guiStyle", US_Theme::defaultStyle()).toString();
}

void US_GuiSettings::set_guiStyle(const QString &style) {
    QSettings settings(US3, "UltraScan");

    if (style == US_Theme::defaultStyle())
        settings.remove("guiStyle");
    else
        settings.setValue("guiStyle", style);
}

// Misc
int US_GuiSettings::plotMargin(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("plotMargin", 10).toInt();
}

void US_GuiSettings::set_plotMargin(int fontSize) {
    QSettings settings(US3, "UltraScan");
    if (fontSize == 10)
        settings.remove("plotMargin");
    else
        settings.setValue("plotMargin", fontSize);
}

// Palettes
//
// Every default palette below is derived from the US_Theme token table, so
// the light and the dark variant of the UltraScan look are defined in exactly
// one place.  A palette that the user has redefined in the color
// configuration panel is stored in QSettings and takes precedence; storing a
// palette that equals the default removes the entry again so that such a
// setup keeps following the desktop's light/dark switch.

// Label - the caption of a form field.  Flat text on the window background.
QPalette US_GuiSettings::labelColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/labelColor"))
        return settings.value("palettes/labelColor").value<QPalette>();
    return labelColorDefault();
}

QPalette US_GuiSettings::labelColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText, t.windowText },   // foreground
        { QPalette::Window,     t.window     },   // background
        { QPalette::Light,      t.window     },   // border
        { QPalette::Dark,       t.window     },   // border
        { QPalette::Mid,        t.border     },
        { QPalette::Text,       t.text       },
        { QPalette::Base,       t.base       }
    });

    set_disabled(p, {
        { QPalette::WindowText, t.disabledText },
        { QPalette::Window,     t.window       },
        { QPalette::Light,      t.window       },
        { QPalette::Dark,       t.window       },
        { QPalette::Mid,        t.border       },
        { QPalette::Text,       t.disabledText },
        { QPalette::Base,       t.base         }
    });

    return p;
}

void US_GuiSettings::set_labelColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == labelColorDefault())
        settings.remove("palettes/labelColor");
    else
        settings.setValue("palettes/labelColor", palette);
}

// Edit - line edits, text edits and list widgets
QPalette US_GuiSettings::editColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/editColor"))
        return settings.value("palettes/editColor").value<QPalette>();
    return editColorDefault();
}

QPalette US_GuiSettings::editColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText,      t.windowText   },
        { QPalette::Window,          t.window       },
        { QPalette::Light,           t.base         },   // border
        { QPalette::Dark,            t.borderStrong },   // border
        { QPalette::Mid,             t.border       },   // between light and dark
        { QPalette::Text,            t.text         },   // text w/ Base
        { QPalette::Base,            t.base         },   // bg for text entry
        { QPalette::HighlightedText, t.accentText   },   // selected text
        { QPalette::Highlight,       t.accent       },   // bg for selected text
        { QPalette::Button,          t.button       },
        { QPalette::Midlight,        t.baseAlt      },
        { QPalette::BrightText,      t.attention    },
        { QPalette::ButtonText,      t.buttonText   },
        { QPalette::Shadow,          t.shadow       }
    });

    set_disabled(p, {
        { QPalette::WindowText,      t.disabledText },
        { QPalette::Window,          t.window       },
        { QPalette::Light,           t.baseReadOnly },
        { QPalette::Dark,            t.border       },
        { QPalette::Mid,             t.border       },
        { QPalette::Text,            t.disabledText },
        { QPalette::Base,            t.baseReadOnly },
        { QPalette::HighlightedText, t.accentText   },
        { QPalette::Highlight,       t.accent       },
        { QPalette::Button,          t.window       },
        { QPalette::Midlight,        t.baseReadOnly },
        { QPalette::BrightText,      t.attention    },
        { QPalette::ButtonText,      t.disabledText },
        { QPalette::Shadow,          t.shadow       }
    });

    return p;
}

void US_GuiSettings::set_editColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == editColorDefault())
        settings.remove("palettes/editColor");
    else
        settings.setValue("palettes/editColor", palette);
}

// Read-only variant of the edit palette
QPalette US_GuiSettings::readonlyColor(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p = editColor();

    p.setColor(QPalette::Active,   QPalette::Base, t.baseReadOnly);
    p.setColor(QPalette::Inactive, QPalette::Base, t.baseReadOnly);
    p.setColor(QPalette::Disabled, QPalette::Base, t.baseReadOnly);

    return p;
}

// Frame - the background of an UltraScan window or dialog
QPalette US_GuiSettings::frameColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/frameColor"))
        return settings.value("palettes/frameColor").value<QPalette>();
    return frameColorDefault();
}

QPalette US_GuiSettings::frameColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText,      t.windowText   },   // foreground
        { QPalette::Window,          t.window       },   // background
        { QPalette::Light,           t.windowAlt    },   // border
        { QPalette::Dark,            t.borderStrong },   // border
        { QPalette::Mid,             t.border       },   // between light and dark
        { QPalette::Midlight,        t.windowAlt    },
        { QPalette::Base,            t.base         },   // bg for text entry
        { QPalette::Text,            t.text         },
        { QPalette::Button,          t.button       },
        { QPalette::ButtonText,      t.buttonText   },
        { QPalette::Highlight,       t.accent       },
        { QPalette::HighlightedText, t.accentText   },
        { QPalette::BrightText,      t.attention    },
        { QPalette::Shadow,          t.shadow       }
    });

    set_disabled(p, {
        { QPalette::WindowText,      t.disabledText },
        { QPalette::Window,          t.window       },
        { QPalette::Light,           t.windowAlt    },
        { QPalette::Dark,            t.border       },
        { QPalette::Mid,             t.border       },
        { QPalette::Midlight,        t.windowAlt    },
        { QPalette::Base,            t.baseReadOnly },
        { QPalette::Text,            t.disabledText },
        { QPalette::Button,          t.window       },
        { QPalette::ButtonText,      t.disabledText },
        { QPalette::Highlight,       t.accent       },
        { QPalette::HighlightedText, t.accentText   },
        { QPalette::BrightText,      t.attention    },
        { QPalette::Shadow,          t.shadow       }
    });

    return p;
}

void US_GuiSettings::set_frameColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == frameColorDefault())
        settings.remove("palettes/frameColor");
    else
        settings.setValue("palettes/frameColor", palette);
}

// Banner - the accent colored header of a section
QPalette US_GuiSettings::bannerColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/bannerColor"))
        return settings.value("palettes/bannerColor").value<QPalette>();
    return bannerColorDefault();
}

QPalette US_GuiSettings::bannerColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText, t.bannerText },   // foreground
        { QPalette::Window,     t.bannerBg   },   // background
        { QPalette::Light,      t.bannerBg   },   // border
        { QPalette::Dark,       t.bannerBg   },   // border
        { QPalette::Mid,        t.bannerBg   },
        { QPalette::Midlight,   t.bannerBg   },
        { QPalette::Text,       t.bannerText },
        { QPalette::Base,       t.bannerBg   }
    });

    set_disabled(p, {
        { QPalette::WindowText, t.bannerText },
        { QPalette::Window,     t.bannerBg   },
        { QPalette::Light,      t.bannerBg   },
        { QPalette::Dark,       t.bannerBg   },
        { QPalette::Mid,        t.bannerBg   },
        { QPalette::Midlight,   t.bannerBg   },
        { QPalette::Text,       t.bannerText },
        { QPalette::Base,       t.bannerBg   }
    });

    return p;
}

void US_GuiSettings::set_bannerColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == bannerColorDefault())
        settings.remove("palettes/bannerColor");
    else
        settings.setValue("palettes/bannerColor", palette);
}

// Pushbutton
QPalette US_GuiSettings::pushbColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/pushbColor"))
        return settings.value("palettes/pushbColor").value<QPalette>();
    return pushbColorDefault();
}

QPalette US_GuiSettings::pushbColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    // Midlight is the mouse-over background, Dark the pressed background and
    // Mid the resting outline: see the QPushButton rules in US_Theme.
    set_enabled(p, {
        { QPalette::ButtonText,      t.buttonText    },   // foreground
        { QPalette::Button,          t.button        },   // background
        { QPalette::Light,           t.button        },   // border
        { QPalette::Midlight,        t.buttonHover   },   // hover background
        { QPalette::Mid,             t.border        },   // outline
        { QPalette::Dark,            t.buttonPressed },   // pressed background
        { QPalette::Highlight,       t.accent        },   // focus outline
        { QPalette::HighlightedText, t.accentText    },
        { QPalette::Shadow,          t.shadow        }
    });

    set_disabled(p, {
        { QPalette::ButtonText,      t.disabledText },
        { QPalette::Button,          t.window       },
        { QPalette::Light,           t.window       },
        { QPalette::Midlight,        t.window       },
        { QPalette::Mid,             t.border       },
        { QPalette::Dark,            t.window       },
        { QPalette::Highlight,       t.accent       },
        { QPalette::HighlightedText, t.accentText   },
        { QPalette::Shadow,          t.shadow       }
    });

    return p;
}

void US_GuiSettings::set_pushbColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == pushbColorDefault())
        settings.remove("palettes/pushbColor");
    else
        settings.setValue("palettes/pushbColor", palette);
}

// Normal - every other widget, and the application wide palette
QPalette US_GuiSettings::normalColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/normalColor"))
        return settings.value("palettes/normalColor").value<QPalette>();
    return normalColorDefault();
}

QPalette US_GuiSettings::normalColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText,      t.windowText    },
        { QPalette::Window,          t.window        },
        { QPalette::Light,           t.windowAlt     },
        { QPalette::Midlight,        t.buttonHover   },
        { QPalette::Mid,             t.border        },
        { QPalette::Dark,            t.buttonPressed },
        { QPalette::Text,            t.text          },
        { QPalette::Base,            t.base          },
        { QPalette::HighlightedText, t.accentText    },
        { QPalette::Highlight,       t.accent        },
        { QPalette::Button,          t.button        },
        { QPalette::BrightText,      t.attention     },
        { QPalette::ButtonText,      t.buttonText    },
        { QPalette::Shadow,          t.shadow        }
    });

    set_disabled(p, {
        { QPalette::WindowText,      t.disabledText },
        { QPalette::Window,          t.window       },
        { QPalette::Light,           t.windowAlt    },
        { QPalette::Midlight,        t.window       },
        { QPalette::Mid,             t.border       },
        { QPalette::Dark,            t.border       },
        { QPalette::Text,            t.disabledText },
        { QPalette::Base,            t.baseReadOnly },
        { QPalette::HighlightedText, t.accentText   },
        { QPalette::Highlight,       t.accent       },
        { QPalette::Button,          t.window       },
        { QPalette::BrightText,      t.attention    },
        { QPalette::ButtonText,      t.disabledText },
        { QPalette::Shadow,          t.shadow       }
    });

    return p;
}

void US_GuiSettings::set_normalColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == normalColorDefault())
        settings.remove("palettes/normalColor");
    else
        settings.setValue("palettes/normalColor", palette);
}

// LCD
QPalette US_GuiSettings::lcdColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/lcdColor"))
        return settings.value("palettes/lcdColor").value<QPalette>();
    return lcdColorDefault();
}

QPalette US_GuiSettings::lcdColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText, t.lcdText },   // foreground
        { QPalette::Window,     t.lcdBg   },   // background
        { QPalette::Light,      t.lcdHi1  },   // highlight1
        { QPalette::Dark,       t.lcdHi2  },   // highlight2
        { QPalette::Shadow,     t.lcdHi2  }
    });

    set_disabled(p, {
        { QPalette::WindowText, t.lcdText },
        { QPalette::Window,     t.lcdBg   },
        { QPalette::Light,      t.lcdHi1  },
        { QPalette::Dark,       t.lcdHi2  },
        { QPalette::Shadow,     t.lcdHi2  }
    });

    return p;
}

void US_GuiSettings::set_lcdColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == lcdColorDefault())
        settings.remove("palettes/lcdColor");
    else
        settings.setValue("palettes/lcdColor", palette);
}

// Plot frame
QPalette US_GuiSettings::plotColor(void) {
    QSettings settings(US3, "UltraScan");
    if (settings.contains("palettes/plotColor"))
        return settings.value("palettes/plotColor").value<QPalette>();
    return plotColorDefault();
}

QPalette US_GuiSettings::plotColorDefault(void) {
    const US_ThemeTokens t = US_Theme::tokens();
    QPalette p;

    set_enabled(p, {
        { QPalette::WindowText, t.windowText },   // tick marks
        { QPalette::Window,     t.window     },   // background
        { QPalette::Text,       t.windowText },   // axis text
        { QPalette::Shadow,     t.windowText }    // titles
    });

    set_disabled(p, {
        { QPalette::WindowText, t.mutedText },
        { QPalette::Window,     t.window    },
        { QPalette::Text,       t.mutedText },
        { QPalette::Shadow,     t.mutedText }
    });

    return p;
}

void US_GuiSettings::set_plotColor(const QPalette &palette) {
    QSettings settings(US3, "UltraScan");
    if (palette == plotColorDefault())
        settings.remove("palettes/plotColor");
    else
        settings.setValue("palettes/plotColor", palette);
}


// Plot curve color
QColor US_GuiSettings::plotCurveDefault(void) {
    return US_Theme::tokens().plotCurve;
}

QColor US_GuiSettings::plotCurve(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("palettes/plotCurve", plotCurveDefault()).value<QColor>();
}

void US_GuiSettings::set_plotCurve(const QColor &color) {
    QSettings settings(US3, "UltraScan");
    if (color == plotCurveDefault())
        settings.remove("palettes/plotCurve");
    else
        settings.setValue("palettes/plotCurve", color);
}

// Canvas background
QColor US_GuiSettings::plotCanvasBGDefault(void) {
    return US_Theme::tokens().plotBg;
}

QColor US_GuiSettings::plotCanvasBG(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("palettes/plotCanvasBG", plotCanvasBGDefault()).value<QColor>();
}

void US_GuiSettings::set_plotCanvasBG(const QColor &color) {
    QSettings settings(US3, "UltraScan");
    if (color == plotCanvasBGDefault())
        settings.remove("palettes/plotCanvasBG");
    else
        settings.setValue("palettes/plotCanvasBG", color);
}

// Canvas major gridlines
QColor US_GuiSettings::plotMajGridDefault(void) {
    return US_Theme::tokens().plotMajGrid;
}

QColor US_GuiSettings::plotMajGrid(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("palettes/plotMajGrid", plotMajGridDefault()).value<QColor>();
}

void US_GuiSettings::set_plotMajGrid(const QColor &color) {
    QSettings settings(US3, "UltraScan");
    if (color == plotMajGridDefault())
        settings.remove("palettes/plotMajGrid");
    else
        settings.setValue("palettes/plotMajGrid", color);
}

// Canvas minor gridlines
QColor US_GuiSettings::plotMinGridDefault(void) {
    return US_Theme::tokens().plotMinGrid;
}

QColor US_GuiSettings::plotMinGrid(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("palettes/plotMinGrid", plotMinGridDefault()).value<QColor>();
}

void US_GuiSettings::set_plotMinGrid(const QColor &color) {
    QSettings settings(US3, "UltraScan");
    if (color == plotMinGridDefault())
        settings.remove("palettes/plotMinGrid");
    else
        settings.setValue("palettes/plotMinGrid", color);
}

// Plot Picker rubber band pen and Tracker pen color
QColor US_GuiSettings::plotPickerDefault(void) {
    return US_Theme::tokens().plotPicker;
}

QColor US_GuiSettings::plotPicker(void) {
    QSettings settings(US3, "UltraScan");
    return settings.value("palettes/plotPicker", plotPickerDefault()).value<QColor>();
}

void US_GuiSettings::set_plotPicker(const QColor &color) {
    QSettings settings(US3, "UltraScan");
    if (color == plotPickerDefault())
        settings.remove("palettes/plotPicker");
    else
        settings.setValue("palettes/plotPicker", color);
}
