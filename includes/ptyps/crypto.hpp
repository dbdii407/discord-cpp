#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include <openssl/sha.h>
#include <string>

#include "./funcs.hpp"

namespace ptyps::crypto {
  std::string sha1(std::string in) {
    auto to = ptyps::funcs::to<std::basic_string, u_char>(in);

    std::vector<u_char> vect(SHA_DIGEST_LENGTH);

    SHA1(&to[0], to.length(), &vect[0]);

    // I'm gonna be honest, I'd like to have this not use a
    // straight up char and sprintf but I currently don't know
    // how to do that. Everything I try no workie.
    //
    // https://memset.wordpress.com/2010/10/06/using-sha1-function/

    char buffer[SHA_DIGEST_LENGTH * 2];

    for (auto i = 0; i < SHA_DIGEST_LENGTH; i++)
      sprintf((char *) & (buffer[i * 2]), "%02x", vect[i]);

    return std::string(buffer);
  }

  // ---- base64
  
  static const std::string b64chars
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  // I, can't remember where I found this code. Otherwise, I'd credit it.

  std::string base64(std::string data) {
    auto out = std::string();
    auto valb = -6;
    auto vala = 0;

    for (auto next : data) {
      vala = (vala << 8) + next;
      valb += 8;

      while (valb >= 0) {
        out.push_back(b64chars[(vala >> valb) & 0x3F]);
        valb -= 6;
      }
    }

    if (valb >- 6)
      out.push_back(b64chars[((vala << 8) >> (valb + 8)) & 0x3F]);
    
    while (out.size() % 4)
      out.push_back('=');
      
    return out;
  }
}