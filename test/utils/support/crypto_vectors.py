"""Independent synthetic US_Crypto wire-format vectors; does not call UltraScan.
Requires Python cryptography; generating these vectors is not part of unit execution.
"""
import hashlib
import json
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.padding import PKCS7

password = 'synthetic-p\u00e4ssword'
plaintext = 'Synthetic caf\u00e9 \u03bc credential'
salt = bytes(range(16))
iv = bytes(range(16, 28))
key = hashlib.pbkdf2_hmac('sha256', password.encode('utf-8'), salt, 200000, 32)
sealed = AESGCM(key).encrypt(iv, plaintext.encode('utf-8'), None)
legacy_password = b'legacy-pass'
legacy_plaintext = 'Legacy caf\u00e9'.encode('latin-1')
legacy_key = legacy_password[:16].ljust(16, b'\0')
legacy_iv = bytes(range(16))
padder = PKCS7(128).padder()
padded = padder.update(legacy_plaintext) + padder.finalize()
encryptor = Cipher(algorithms.AES(legacy_key), modes.CBC(legacy_iv)).encryptor()
legacy_cipher = encryptor.update(padded) + encryptor.finalize()
nul_padder = PKCS7(128).padder()
nul_padded = nul_padder.update(b'left\0right') + nul_padder.finalize()
nul_encryptor = Cipher(algorithms.AES(legacy_key), modes.CBC(legacy_iv)).encryptor()
legacy_nul_cipher = nul_encryptor.update(nul_padded) + nul_encryptor.finalize()
print(json.dumps({
    'pbkdf2_ascii': hashlib.pbkdf2_hmac('sha256', b'password', b'salt', 200000, 32).hex(),
    'pbkdf2_nul': hashlib.pbkdf2_hmac('sha256', b'p\0w', b'salt\0value', 200000, 32).hex(),
    'gcm': {'password': password, 'plaintext': plaintext, 'key': key.hex(),
            'ciphertext': sealed[:-16].hex(), 'metadata': (salt + iv + sealed[-16:]).hex()},
    'legacy': {'password': legacy_password.decode('ascii'), 'plaintext': legacy_plaintext.decode('latin-1'),
               'key': legacy_key.hex(), 'iv': legacy_iv.hex(), 'ciphertext': legacy_cipher.hex(),
               'nul_ciphertext': legacy_nul_cipher.hex()},
}, indent=2, ensure_ascii=False))
