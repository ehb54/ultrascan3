//! \file us_pabs_common.h
#ifndef US_PABS_COMMON_H
#define US_PABS_COMMON_H

#include <QObject>
#include <QVector>
#include <QColor>

/**
 * @class HSVcolormap
 * @brief The HSVcolormap class provides methods to generate a colormap based on the HSV color model.
 */
class HSVcolormap
{
    public:
        /**
         * @brief Default constructor for HSVcolormap.
         */
        HSVcolormap();

        /**
         * @brief Constructor for HSVcolormap with a specified number of colors.
         * @param n_colors Number of colors
         */
        HSVcolormap(int n_colors);

        /**
         * @brief Constructor for HSVcolormap with specified number of colors, start hue, and stop hue.
         * @param n_colors Number of colors
         * @param hue_start Starting hue
         * @param hue_stop Stopping hue
         */
        HSVcolormap(int n_colors, int hue_start, int hue_stop);

        /**
         * @brief Set the number of colors in the colormap.
         * @param num Number of colors
         */
        void set_n_colors(int num);

        /**
         * @brief Set the starting hue for the colormap.
         * @param num Starting hue
         * @return The starting hue
         */
        int set_hue_start(int num);

        /**
         * @brief Set the stopping hue for the colormap.
         * @param num Stopping hue
         * @return The stopping hue
         */
        int set_hue_stop(int num);

        /**
         * @brief Set the saturation value for the colormap.
         * @param num Saturation value
         * @return The saturation value
         */
        int set_saturation(int num);

        /**
         * @brief Set the value (brightness) for the colormap.
         * @param num Value (brightness)
         * @return The value (brightness)
         */
        int set_value(int num);

        /**
         * @brief Set the transparency (alpha) for the colormap.
         * @param num Transparency value
         * @return The transparency value
         */
        int set_transparency(int num);

        /**
         * @brief Get the list of colors in the colormap.
         * @param colorList Reference to a QVector of QColor to store the color list
         * @return The number of colors in the color list
         */
        int get_colorlist(QVector<QColor> &colorList);

    private:
        int nc = -1; ///< Number of colors
        int hstart = -1; ///< Starting hue
        int hstop = -1; ///< Stopping hue
        int saturation = -1; ///< Saturation value
        int value = -1; ///< Brightness value
        int alpha = 255; ///< Transparency value

        /**
         * @brief Generate a range of hue values for the colormap.
         * @return QVector of hue values
         */
        QVector<int> rang_hvalues(void);
};

/**
 * @class CCW_ITEM
 * @brief The CCW_ITEM class represents items in a cross-cell wavelength (CCW) list.
 */
class CCW_ITEM {
    public:
        QList<int> runClass; ///< List of run classes
        QList<int> cell; ///< List of cells
        QList<char> channel; ///< List of channels
        QList<QVector<double>> wavelength; ///< List of wavelength vectors
        QList<QVector<int>> index; ///< List of index vectors

        /**
         * @brief Clear the contents of the CCW_ITEM.
         */
        void clear(void);

        /**
         * @brief Get the size of the CCW_ITEM.
         * @return The size of the CCW_ITEM
         */
        int size(void);
};

/**
 * @class CCW
 * @brief The CCW class represents a cross-cell wavelength (CCW) list.
 */
class CCW {
    public:
        QVector<int> index; ///< List of indices
        QVector<int> cell; ///< List of cells
        QVector<char> channel; ///< List of channels
        QVector<double> wavelength; ///< List of wavelengths
        QStringList runId; ///< List of run IDs

        /**
         * @brief Clear the contents of the CCW.
         */
        void clear(void);

        /**
         * @brief Get the size of the CCW.
         * @return The size of the CCW
         */
        int size(void);
};

#endif // US_PABS_COMMON_H
