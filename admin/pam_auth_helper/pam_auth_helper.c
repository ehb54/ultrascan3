// /usr/local/bin/pam_auth_helper.c
// compile:
// make
// install:
// make install

// pam_auth_helper.c - PAM authentication helper using stdin input
#include <security/pam_appl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int pam_conv_func( int num_msg, const struct pam_message ** msg,
                          struct pam_response ** resp, void * appdata_ptr ) {
   ( void ) num_msg;
   ( void ) msg;

   char ** cred = appdata_ptr;

   struct pam_response * r = calloc( 1, sizeof( struct pam_response ) );
   if ( r == NULL ) {
      return PAM_BUF_ERR;
   }

   r[ 0 ].resp = strdup( cred[ 0 ] );  // password from PAM_AUTHTOK
   r[ 0 ].resp_retcode = 0;

   *resp = r;
   return PAM_SUCCESS;
}

int main( void ) {
   if ( getenv( "RECURSION_GUARD" ) ) {
      fprintf( stderr, "Detected recursive PAM invocation. Exiting.\n" );
      return 1;
   }

   const char * username = getenv( "PAM_USER" );
   const char * password = getenv( "PAM_AUTHTOK" );

   if ( !username || !password ) {
      fprintf( stderr, "Missing PAM_USER or PAM_AUTHTOK\n" );
      return 1;
   }

   // fprintf( stderr, "DEBUG: user=%s pass=%s\n", username, password );
   
   char * cred[ 2 ] = { ( char * ) password, NULL };

   struct pam_conv conv = { pam_conv_func, cred };
   pam_handle_t * pamh = NULL;

   if ( pam_start( "php-helper", username, &conv, &pamh ) != PAM_SUCCESS ) {
      return 1;
   }

   if ( pam_authenticate( pamh, 0 ) != PAM_SUCCESS ) {
      pam_end( pamh, PAM_SUCCESS );
      return 1;
   }

   if ( pam_acct_mgmt( pamh, 0 ) != PAM_SUCCESS ) {
      pam_end( pamh, PAM_SUCCESS );
      return 1;
   }

   pam_end( pamh, PAM_SUCCESS );
   return 0;
}
