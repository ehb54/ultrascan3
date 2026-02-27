//! \file us_crypto.h
#ifndef US_CRYPTO_H
#define US_CRYPTO_H

#include "us_extern.h"

/*! \brief A class to encrypt and decrypt the user's database passwords.
    
    The class uses the master password and the AES-128 encryption
    algorithm.  The password is automatically retrieved from 
    global memory vi \ref US_Global.  If the master password is not 
    yet in global memory, US_Global prompts the user for it.
*/
class US_UTIL_EXTERN US_Crypto
{
  public:
    /*! A static function to encrypt a password string.
        \param  plain_text     The string to be encrypted.
        \param  pw             The password to use for encryption
        \retval encryptionData A list of two strings containing the ciphertext
                               of the password and the initialization vector
                               used during encryption.
    */
    static QStringList encrypt( const QString&, const QString& );

    /*! A static function to decrypt a string.
        \param  ciphertext The string to be encrypted.
        \param  pw         Password used to encrypt the string
        \param  initVector An initialization string used when the string was encrypted.
        \retval plaintext  The plaintext password.
    */
    static QString     decrypt( const QString& ciphertext, const QString& pw, const QString& initVector );

    //! \brief Legacy version of the decrypt function.
    //! \deprecated
    //! \param  ciphertext The string to be encrypted.
    //! \param  pw         Password used to encrypt the string
    //! \param  initVector An initialization string used when the string was encrypted.
    //! \retval plaintext  The plaintext password.
    static QString     decryptLegacy( const QString& ciphertext, const QString& pw, const QString& initVector );

    static void fillRandomBytes( QByteArray& ba );

    //! \brief A static function to derive a 32 byte key from a password and salt using PBkdf2.
    //! \param pwUtf8 ByteArray of password in utf8
    //! \param salt ByteArray of salt
    //! \param key32 Key to be derived
    static bool deriveKeyPBKDF2_32( const QByteArray& pwUtf8, const QByteArray& salt, unsigned char key32[32] );

    //! \brief A static function to derive a 16 byte key from a password.
    //! \param pw ByteArray of password
    //! \param key16 Key to be derived
    static bool deriveLegacyKey_16( const QString& pw, unsigned char key16[16] );
};

#endif
