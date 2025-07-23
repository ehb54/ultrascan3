/**
 * @file us_timer.h
 * @brief Defines the US_Timer class for managing multiple timers.
 */

#ifndef US_TIMER_H
#define US_TIMER_H

#include <qstring.h>
#include <qdatetime.h>
#include <map>
#include "us_extern.h"

using namespace std;

/**
 * @class US_Timer
 * @brief A class to manage multiple timers.
 *
 * This class provides functionality to initialize, start, end, and list timers,
 * allowing for time tracking of various events.
 */
class US_UTIL_EXTERN US_Timer
{
public:
    /**
     * @brief Constructs a US_Timer object.
     */
    US_Timer();

    /**
     * @brief Destructs a US_Timer object.
     */
    ~US_Timer();

    /**
     * @brief Initializes a timer with a given name.
     *
     * @param name The name of the timer to initialize.
     */
    void init_timer(QString name);

    /**
     * @brief Starts a timer with a given name.
     *
     * @param name The name of the timer to start.
     * @return True if the timer was successfully started, false otherwise.
     */
    bool start_timer(QString name);

    /**
     * @brief Ends a timer with a given name.
     *
     * @param name The name of the timer to end.
     * @return True if the timer was successfully ended, false otherwise.
     */
    bool end_timer(QString name);

    /**
     * @brief Lists the time recorded by a specific timer.
     *
     * @param name The name of the timer to list the time for.
     * @return The recorded time as a QString.
     */
    QString list_time(QString name);

    /**
     * @brief Lists the times recorded by all timers.
     *
     * @return A QString containing the recorded times for all timers.
     */
    QString list_times();

private:
    map<QString, QTime> timers;            ///< Map of timer names to QTime objects.
    map<QString, unsigned int> counts;     ///< Map of timer names to counts.
    map<QString, unsigned long> times;     ///< Map of timer names to recorded times.
    map<QString, unsigned long> times2;    ///< Map of timer names to secondary recorded times.
};

#endif // US_TIMER_H
