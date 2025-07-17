#include "aes_gcm.h"
#include <mbedtls/gcm.h>
#include <cstring>

namespace crypto {

static bool crypt_gcm(int mode,
                      const uint8_t* key, std::size_t key_len_bits,
                      const uint8_t* input, std::size_t length,
                      const uint8_t* aad, std::size_t aad_len,
                      const uint8_t* nonce, std::size_t nonce_len,
                      uint8_t* output,
                      uint8_t* tag /*in decrypt = expected tag*/)
{
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES,
                                 key,
                                 static_cast<int>(key_len_bits));
    if (ret != 0) {
        mbedtls_gcm_free(&ctx);
        return false;
    }

    if (mode == MBEDTLS_GCM_ENCRYPT) {
        ret = mbedtls_gcm_crypt_and_tag(&ctx,
                                        MBEDTLS_GCM_ENCRYPT,
                                        length,
                                        nonce, nonce_len,
                                        aad, aad_len,
                                        input, output,
                                        AES_GCM_TAG_SIZE,
                                        tag);
    } else { // decrypt + verify
        ret = mbedtls_gcm_auth_decrypt(&ctx,
                                       length,
                                       nonce, nonce_len,
                                       aad, aad_len,
                                       tag, AES_GCM_TAG_SIZE,
                                       input, output);
    }

    mbedtls_gcm_free(&ctx);
    return (ret == 0);
}

bool aes_gcm_encrypt(const uint8_t* key, std::size_t key_len_bits,
                     const uint8_t* plaintext, std::size_t length,
                     const uint8_t* aad, std::size_t aad_len,
                     const uint8_t* nonce, std::size_t nonce_len,
                     uint8_t* ciphertext_out,
                     uint8_t* tag_out)
{
    return crypt_gcm(MBEDTLS_GCM_ENCRYPT,
                     key, key_len_bits,
                     plaintext, length,
                     aad, aad_len,
                     nonce, nonce_len,
                     ciphertext_out,
                     tag_out);
}

bool aes_gcm_decrypt(const uint8_t* key, std::size_t key_len_bits,
                     const uint8_t* ciphertext, std::size_t length,
                     const uint8_t* aad, std::size_t aad_len,
                     const uint8_t* nonce, std::size_t nonce_len,
                     const uint8_t* tag_in,
                     uint8_t* plaintext_out)
{
    // crypt_gcm expects writable tag buffer (mbedTLS may scrub it)
    uint8_t tag_buf[AES_GCM_TAG_SIZE];
    std::memcpy(tag_buf, tag_in, AES_GCM_TAG_SIZE);

    return crypt_gcm(MBEDTLS_GCM_DECRYPT,
                     key, key_len_bits,
                     ciphertext, length,
                     aad, aad_len,
                     nonce, nonce_len,
                     plaintext_out,
                     tag_buf);
}

} // namespace crypto
