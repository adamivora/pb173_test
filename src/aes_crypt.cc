#include "aes_crypt.h"
#include <fstream>
#include <stdexcept>
#include "constants.h"
#include "mbedtls/cipher.h"

AES::AES() {
  mbedtls_cipher_init(&context_);
  switch (mbedtls_cipher_setup(
      &context_, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC))) {
    case MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA:
      throw std::runtime_error("MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA");
    case MBEDTLS_ERR_CIPHER_ALLOC_FAILED:
      throw std::runtime_error("MBEDTLS_ERR_CIPHER_ALLOC_FAILED");
  }
}

void AES::init(const std::vector<unsigned char> &aes_key,
               const std::vector<unsigned char> &iv,
               mbedtls_operation_t operation,
               mbedtls_cipher_padding_t padding_mode) {
  if (aes_key.size() != KEY_BYTES) {
    throw std::runtime_error("aes_key has invalid length");
  }

  if (iv.size() != KEY_BYTES) {
    throw std::runtime_error("iv has invalid length");
  }

  if (mbedtls_cipher_setkey(&context_, aes_key.data(), KEY_BITS, operation) <
      0) {
    throw std::runtime_error("mbedtls_cipher_setkey failed");
  }

  if (mbedtls_cipher_set_iv(&context_, iv.data(), KEY_BYTES) < 0) {
    throw std::runtime_error("MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA");
  }

  if (mbedtls_cipher_set_padding_mode(&context_, padding_mode) < 0) {
    throw std::runtime_error("mbedtls_cipher_set_padding_mode failed");
  }
}

void AES::encrypt(
    std::istream &input, std::ostream &output,
    const std::vector<unsigned char> &aes_key,
    const std::vector<unsigned char> &iv,
    mbedtls_cipher_padding_t padding_mode /* = MBEDTLS_PADDING_PKCS7 */) {
  init(aes_key, iv, MBEDTLS_ENCRYPT, padding_mode);
  process(input, output);
}

void AES::decrypt(
    std::istream &input, std::ostream &output,
    const std::vector<unsigned char> &aes_key,
    const std::vector<unsigned char> &iv,
    mbedtls_cipher_padding_t padding_mode /* = MBEDTLS_PADDING_PKCS7 */) {
  init(aes_key, iv, MBEDTLS_DECRYPT, padding_mode);
  process(input, output);
}

void AES::process(std::istream &input, std::ostream &output) {
  if (!input) {
    throw std::runtime_error("input stream invalid");
  }

  if (!output) {
    throw std::runtime_error("output stream invalid");
  }

  std::vector<unsigned char> buffer(KEY_BYTES);
  char *b = reinterpret_cast<char *>(buffer.data());

  std::vector<unsigned char> output_buffer(KEY_BYTES);
  char *o = reinterpret_cast<char *>(output_buffer.data());

  int read_bytes;
  size_t olen;
  while ((read_bytes = input.read(b, KEY_BYTES).gcount()) != 0) {
    if (mbedtls_cipher_update(&context_, buffer.data(), read_bytes,
                              output_buffer.data(), &olen) < 0) {
      throw std::runtime_error("mbedtls_cipher_update failed");
    }
    output.write(o, olen);
  }

  if (input.bad()) {
    throw std::runtime_error("input stream error while reading");
  }

  if (mbedtls_cipher_finish(&context_, output_buffer.data(), &olen) < 0) {
    throw std::runtime_error("mbedtls_cipher_finish failed");
  }
  output.write(o, olen);
}
