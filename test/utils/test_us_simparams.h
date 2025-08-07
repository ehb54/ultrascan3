/**
 * @file test_us_simparams.h
 * @brief Header file for the TestUS_SimulationParameters class.
 *
 * This file contains the declaration of the TestUS_SimulationParameters class, which includes
 * unit tests for various functions defined in the US_SimulationParameters class.
 */

#ifndef TEST_US_SimulationParameters_H
#define TEST_US_SimulationParameters_H

#include <QtTest>
#include "us_simparms.h"

/**
 * @class TestUS_SimulationParameters
 * @brief Unit test class for testing the US_SimulationParameters functions.
 *
 * This class contains unit tests for various functions defined in the US_SimulationParameters class.
 * The tests are implemented using the QtTest framework.
 */
class TestUS_SimulationParameters : public QObject {
      Q_OBJECT

   private slots:
      /**
      * @brief Test case for speedstepToXml without set speed.
      */
      void test_speedstepToXml_without_set_speed();

      /**
      * @brief Test case for speedstepToXml with set speed.
      */
      void test_speedstepToXml_with_set_speed();

      /**
      * @brief Test case for save_simparams with all expected properties.
      */
      void test_save_simparms_default();

      /**
      * @brief Test case for save_simparams for optional user mesh.
      */
      void test_save_simparms_user_mesh();

      /**
      * @brief Test case for save_simparams for optional properties.
      */
      void test_save_simparms_optionalAttributes();

      /**
      * @brief Test case for save_simparams for file opening fails
      */
      void test_save_simparms_file_open_fail();

      /**
      * @brief Test case for speedstepFromXml with all attributes.
      */
      void test_speedstepFromXml_AllAttributes();

      /**
      * @brief Test case for speedstepFromXml without all attributes.
      */
      void test_speedstepFromXml_MissingAttributes();

      /**
      * @brief Test case for speedstepFromXml for acceleration flag true value.
      */
      void test_speedstepFromXml_AccelerationFlagTrue();

      /**
      * @brief Test case for speedstepFromXml for acceleration flag false value.
      */
      void test_speedstepFromXml_AccelerationFlagFalse();

      /**
      * @brief Test case for load_simparams for missing file.
      */
      void test_load_simparams_missing_file();

      /**
      * @brief Test case for load_simparams for empty file.
      */
      void test_load_simparams_empty_file();

      /**
      * @brief Test case for load_simparams for minimum file to not fail loading.
      */
      void test_load_simparams_minimum_file();

      /**
      * @brief Test case for load_simparams for all properties except speedstep.
      */
      void test_load_simparams_all();

      /**
      * @brief Test case for load_simparams for file with all properties except speedstep.
      */
      void test_load_simparams_missing_speedstep();
};

#endif // TEST_US_SimulationParameters_H
