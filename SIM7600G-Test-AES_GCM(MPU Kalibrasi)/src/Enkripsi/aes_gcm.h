#ifndef AES_GCM_H
#define AES_GCM_H

#include <cstddef>
#include <cstdint>

/// \file aes_gcm.h
/// Minimal AES‑GCM helper for embedded (ESP32 / mbedTLS).
/// All functions return **true** on success (tag verified) and **false** on any error.
/// The caller must ensure that key, nonce, tag, and buffer pointers are valid and
/// large enough (ciphertext_out/plaintext_out >= `length`).
///
/// Build requirements (ESP32 Arduino / ESP‑IDF):
///   -DMBEDTLS_GCM_C  -DMBEDTLS_AES_C
/// and `#include <mbedtls/gcm.h>` linked automatically by SDK.

namespace crypto {

constexpr std::size_t AES128_KEY_SIZE      = 16;  //!< 128‑bit key size in bytes
constexpr std::size_t AES_GCM_NONCE_SIZE   = 12;  //!< Recommended nonce length (96‑bit)
constexpr std::size_t AES_GCM_TAG_SIZE     = 16;  //!< Authentication tag (128‑bit)

/**
 * Encrypt and generate authentication tag (AES‑GCM).
 *
 * @param key              Pointer to AES key (16 or 32 bytes)
 * @param key_len_bits     Length of key in *bits* (128 or 256)
 * @param plaintext        Data to encrypt
 * @param length           Length of @p plaintext & @p ciphertext_out
 * @param aad              Additional authenticated data (can be nullptr if aad_len==0)
 * @param aad_len          Length of @p aad in bytes
 * @param nonce            Unique nonce/IV (recommend 12 bytes)
 * @param nonce_len        Length of nonce
 * @param ciphertext_out   Buffer for resulting ciphertext (same length as plaintext)
 * @param tag_out          Buffer for resulting tag (16 bytes)
 */
bool aes_gcm_encrypt(const uint8_t* key, std::size_t key_len_bits,
                     const uint8_t* plaintext, std::size_t length,
                     const uint8_t* aad, std::size_t aad_len,
                     const uint8_t* nonce, std::size_t nonce_len,
                     uint8_t* ciphertext_out,
                     uint8_t* tag_out);

/**
 * Decrypt and verify tag (AES‑GCM).
 *
 * @return true if authentication passes and data decrypted into @p plaintext_out.
 */
bool aes_gcm_decrypt(const uint8_t* key, std::size_t key_len_bits,
                     const uint8_t* ciphertext, std::size_t length,
                     const uint8_t* aad, std::size_t aad_len,
                     const uint8_t* nonce, std::size_t nonce_len,
                     const uint8_t* tag,
                     uint8_t* plaintext_out);

} // namespace crypto

#endif // AES_GCM_H

