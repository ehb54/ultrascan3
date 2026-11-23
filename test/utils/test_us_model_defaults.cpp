//! \file test_us_model_defaults.cpp
#include "us_model.h"
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <new>

TEST( US_ModelDefaults, OrdinaryModelCanBeCopiedWithoutCustomGridMetadata )
{
   alignas( US_Model ) unsigned char storage[sizeof( US_Model )];
   std::memset( storage, 0xAA, sizeof( storage ) );
   auto                                           destroy = []( US_Model* value ) { value->~US_Model(); };
   std::unique_ptr<US_Model, decltype( destroy )> model( new ( storage ) US_Model, destroy );
   auto                                           flagByte = []( const bool& flag )
   {
      unsigned char value;
      std::memcpy( &value, &flag, sizeof( value ) );
      return value;
   };
   ASSERT_EQ( flagByte( model->customGridData.xLogarithmic ), 0 );
   ASSERT_EQ( flagByte( model->customGridData.midpointBins ), 0 );
   US_Model saved;
   saved = *model; // The reporter saves the loaded ordinary model this way.
   EXPECT_FALSE( saved.customGridData.xLogarithmic );
   EXPECT_FALSE( saved.customGridData.midpointBins );
   EXPECT_TRUE( saved.customGridData.grids.isEmpty() );
}
