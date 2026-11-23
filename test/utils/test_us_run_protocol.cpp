//! \file test_us_run_protocol.cpp
#include "us_run_protocol.h"
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <new>

TEST( US_RunProtocolDefaults, ResetCopiesInitializedControlFlags )
{
   alignas( US_RunProtocol ) unsigned char storage[sizeof( US_RunProtocol )];
   std::memset( storage, 0xAA, sizeof( storage ) );
   auto                                                 destroy = []( US_RunProtocol* value ) { value->~US_RunProtocol(); };
   std::unique_ptr<US_RunProtocol, decltype( destroy )> protocol( new ( storage ) US_RunProtocol, destroy );

   // Check object representations before reading bools: this fails deterministically
   // when a constructor omits an assignment, without making the test itself read an invalid bool.
   auto                                                 flagByte = []( const bool& flag )
   {
      unsigned char value;
      std::memcpy( &value, &flag, sizeof( value ) );
      return value;
   };
   ASSERT_EQ( flagByte( protocol->rpRotor.vel_mwl_prot ), 0 );
   ASSERT_EQ( flagByte( protocol->rpRotor.importData ), 0 );
   ASSERT_EQ( flagByte( protocol->rpRotor.importData_absorbance_t ), 0 );
   ASSERT_EQ( flagByte( protocol->rpRotor.importData_absorbance_pa ), 0 );
   ASSERT_EQ( flagByte( protocol->rpSpeed.spin_down ), 0 );
   ASSERT_EQ( flagByte( protocol->rpSpeed.radial_calib ), 0 );
   EXPECT_EQ( protocol->projectID, -1 );
   EXPECT_EQ( protocol->protoID, -1 );
   EXPECT_EQ( protocol->rpRotor.operID, -1 );
   EXPECT_EQ( protocol->rpRotor.instID, -1 );

   US_RunProtocol reset = *protocol;
   reset                = US_RunProtocol(); // The reporter's reset_report_panel path.
   EXPECT_FALSE( reset.rpRotor.vel_mwl_prot );
   EXPECT_FALSE( reset.rpRotor.importData );
   EXPECT_FALSE( reset.rpRotor.importData_absorbance_t );
   EXPECT_FALSE( reset.rpRotor.importData_absorbance_pa );
   EXPECT_FALSE( reset.rpSpeed.spin_down );
   EXPECT_FALSE( reset.rpSpeed.radial_calib );
}
