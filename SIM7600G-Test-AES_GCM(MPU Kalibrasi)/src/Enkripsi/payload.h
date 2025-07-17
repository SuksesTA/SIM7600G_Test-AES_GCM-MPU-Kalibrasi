#ifndef PAYLOAD_ENCRYPT_H
#define PAYLOAD_ENCRYPT_H

#include <Arduino.h>
#include <cstring>
#include "Enkripsi/aes_gcm.h"
#include <base64.h>

namespace payload {

/**
 * Encrypt plain text with AES-GCM-128 and output as Base64 string.
 * Result format: Base64(NONCE | CIPHER | TAG)
 *
 * @param plain  Text to encrypt (CSV string)
 * @param key    16-byte AES key
 * @param outB64 Output string (Base64 encoded)
 * @return true on success, false if failed
 */
inline bool encryptB64(const String &plain,
                       const uint8_t *key,
                       String &outB64)
{
    const size_t len = plain.length();
    if (len == 0 || len > 256) return false;  // batasi panjang plaintext

    // Buffer statis
    uint8_t nonce[crypto::AES_GCM_NONCE_SIZE];      // 12 byte
    uint8_t cipher[256];                            // maks 256 byte plaintext
    uint8_t tag[crypto::AES_GCM_TAG_SIZE];          // 16 byte
    uint8_t blob[12 + 256 + 16];                    // NONCE + CIPHER + TAG

    // 1. Isi nonce dengan random
    esp_fill_random(nonce, sizeof(nonce));

    // 2. Enkripsi
    bool ok = crypto::aes_gcm_encrypt(
        key, 128,
        reinterpret_cast<const uint8_t*>(plain.c_str()), len,
        nullptr, 0,  // no AAD
        nonce, sizeof(nonce),
        cipher,
        tag
    );

    if (!ok) return false;

    // 3. Gabungkan NONCE + CIPHER + TAG ke blob
    std::memcpy(blob, nonce, sizeof(nonce));
    std::memcpy(blob + sizeof(nonce), cipher, len);
    std::memcpy(blob + sizeof(nonce) + len, tag, sizeof(tag));

    // 4. Base64 encode
    outB64 = base64::encode(blob, sizeof(nonce) + len + sizeof(tag));
    return true;
}

} // namespace payload

#endif // PAYLOAD_ENCRYPT_H