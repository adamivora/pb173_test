#ifndef PB173_SHA_H_
#define PB173_SHA_H_
#include <iostream>
#include <vector>
#include "mbedtls/sha512.h"

class SHA {
 private:
  mbedtls_sha512_context context_;

 public:
  SHA() { mbedtls_sha512_init(&context_); }

  std::vector<unsigned char> hash(std::istream &input);

  ~SHA() { mbedtls_sha512_free(&context_); }
};

#endif  // PB173_SHA_H_
