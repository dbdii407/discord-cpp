#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../crypto.hpp"
#include "../random.hpp"
#include "./tcp.hpp"
#include "./url.hpp"

namespace p$web::ws {
  using exception = p$err::exception;

  constexpr int OPCODE_UNKNOWN = EOF;
  constexpr int OPCODE_CONTINUATION = 0b00000000;
  constexpr int OPCODE_TEXT = 0b00000001;
  constexpr int OPCODE_BINARY = 0b00000010;
  constexpr int OPCODE_CLOSE = 0b00001000;
  constexpr int OPCODE_PING = 0b00001001;
  constexpr int OPCODE_PONG = 0b00001010;

  constexpr int MASK = 0b10000000;
  constexpr int FIN = 0b10000000;
  constexpr int MASKLEN = 4;

  enum class opcode {
    UNKNOWN = OPCODE_UNKNOWN,

    // Actual OPCODES
    CONTINUATION = OPCODE_CONTINUATION,
    TEXT,
    BINARY,
    CLOSE = OPCODE_CLOSE,
    PING = OPCODE_PING,
    PONG = OPCODE_PONG
  };

  static std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

  std::string createHandshakeKey() {
    auto random = p$random::string(12, std::vector<char>({
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
      'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
      'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
      'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    }));

    auto hash = p$crypto::sha1(random + magic);

    return p$crypto::base64(hash);
  }

  std::vector<uint8_t> createMaskingKey() {
    auto out = std::vector<uint8_t>();

    for (auto i = 0; i < MASKLEN; i++) {
      auto next = p$random::number(0, 255);
      out.push_back(next);
    }

    return out;
  }

  std::vector<uint8_t> UInt64ToUInt8(ulong i) {
    auto vect = std::vector<uint8_t>();

    vect.push_back((i >> 56) & 0xFF);
    vect.push_back((i >> 48) & 0xFF);
    vect.push_back((i >> 40) & 0xFF);
    vect.push_back((i >> 32) & 0xFF);
    vect.push_back((i >> 24) & 0xFF);
    vect.push_back((i >> 16) & 0xFF);
    vect.push_back((i >>  8) & 0xFF);
    vect.push_back((i >>  0) & 0xFF);

    return vect;
  }

  std::vector<uint8_t> UInt16ToUint8(ulong i) {
    auto vect = std::vector<uint8_t>();

    vect.push_back((i >> 8) & 0xFF);
    vect.push_back((i >> 0) & 0xFF);

    return vect;
  }

  std::string encode(bool masked, opcode op, std::string data) {
    if (op == opcode::UNKNOWN)
      throw exception("must be a valid opcode");

    auto vect = std::vector<uint8_t>();
    auto length = data.size();
    auto p = 0;

    auto size = (length >= 65536 ? 6 : 0) + (length >= 126 ? 2 : 0) +
                (masked ? MASKLEN : 0) + 2;

    vect.resize(size);

    vect[p++] = FIN + std::underlying_type_t<opcode>(op);

    if (length <= 125)
      vect[p++] = (char(length) & 0xFF) | (masked ? MASK : 0);

    else if (length >= 126 && length <= 65565) {
      vect[p++] = char(0x7E) | (masked ? MASK : 0);

      for (auto i : UInt16ToUint8(length))
        vect[p++] = i;
    }

    else {
      vect[p++] = char(0x7F) | (masked ? MASK : 0);

      for (auto i : UInt64ToUInt8(length))
        vect[p++] = i;
    }

    auto key = createMaskingKey();

    if (masked) {
      vect[p++] = key[0];
      vect[p++] = key[1];
      vect[p++] = key[2];
      vect[p++] = key[3];
    }

    p$funcs::append(vect, data);

    if (masked) {
      for (auto i = 0; i != length; ++i)
        vect[size + i] ^= key[i & 0x03];
    }

    return p$funcs::to<std::basic_string, char>(vect);
  }

  uint64_t UInt8ToUInt64(std::vector<uint8_t> &vect, int pos = 0) {
    return (((uint64_t) vect[pos + 0]) << 56) |
           (((uint64_t) vect[pos + 1]) << 48) |
           (((uint64_t) vect[pos + 2]) << 40) |
           (((uint64_t) vect[pos + 3]) << 32) |
           (((uint64_t) vect[pos + 4]) << 24) |
           (((uint64_t) vect[pos + 5]) << 16) |
           (((uint64_t) vect[pos + 6]) <<  8) |
           (((uint64_t) vect[pos + 7]) <<  0);
  }

  uint16_t UInt8ToUInt16(std::vector<uint8_t> &vect, int pos = 0) {
    return (((uint16_t) vect[pos + 0]) << 8) |
           (((uint16_t) vect[pos + 1]) << 0);
  }

  std::pair<opcode, std::optional<std::string>> decode(bool masked, std::string recvd, std::string &rxbuf) {
    if (rxbuf.length())
      recvd = rxbuf + recvd;
    
    rxbuf.clear();

    auto vect = p$funcs::to<std::vector, uint8_t>(recvd);

    if (vect.size() < 2) {
      p$funcs::append(rxbuf, recvd);

      return {
        opcode::UNKNOWN,
        {}
      };
    }

    // auto fin = (vect[0] & FIN) == FIN; // if 1, last message in series
    auto opc = vect[0] & 0x0F;
    // auto msk = (vect[1] & MASK) == MASK;
    auto len = vect[1] & 0x7F;

    if (len == 126)
      len = UInt8ToUInt16(vect, 2);

    if (len == 127)
      len = UInt8ToUInt64(vect, 2);

    auto size = (masked ? MASKLEN : 0) + (len >= 65536 ? 6 : 0) +
                (len >= 126 ? 2 : 0) + 2;

    if (size + len > vect.size()) {
      p$funcs::append(rxbuf, recvd);

      return {
        opcode::UNKNOWN,
        {}
      };
    }

    auto key = std::vector<int>();

    if (masked) {
      for (auto i = 0; i < MASKLEN; i++)
        key[i] = ((int) vect[size + i]) << 0;
    }

    // --------

    if (opc == OPCODE_CLOSE) {
      printf("ws close len: %i\r\n", len);

      return {
        opcode::CLOSE,
        {}
      };
    }

    if (opc == OPCODE_TEXT || opc == OPCODE_BINARY || opc == OPCODE_CONTINUATION) {
      auto end = p$funcs::iterator(vect, size + len);
      auto begin = p$funcs::iterator(vect, size);

      auto data = std::string(begin, end);

      p$funcs::chop(vect, size + len);

      if (!vect.empty())
        rxbuf = p$funcs::to<std::basic_string, char>(vect);

      return {
        opcode::TEXT,
        data
      };
    }

    return {
      opcode::UNKNOWN,
      {}
    };
  }
}

namespace p$web::wss {
  class Socket : p$web::tcps::Socket {
    private:
    p$web::url::parsed parsed;
    std::string rxbuf;
    bool initiated;
    bool open;

    public:
      using p$web::tcps::Socket::connected;
      using p$web::tcps::Socket::loop;

      virtual void ws_on_disconnect() { }
      virtual void ws_on_connect() { }
      virtual void ws_on_open() { }
      virtual void ws_on_close() { }
      virtual void ws_on_text(std::string text) {}

     void tcp_on_disconnect() {
       ws_on_disconnect();
     }
     
      void tcp_on_connect() {
        ws_on_connect();

        auto list = std::vector<std::string>();

        auto top = p$string::format("GET /%s HTTP/1.1", parsed.query.data());

        list.push_back(top);
        list.push_back("Host: " + parsed.host);
        list.push_back("Upgrade: websocket");
        list.push_back("Connection: Upgrade");
        list.push_back("Sec-WebSocket-Key: " + p$web::ws::createHandshakeKey());
        list.push_back("Sec-WebSocket-Version: 13");
        list.push_back({});
        list.push_back({});

        auto request = p$string::join(list, "\r\n");

        p$web::tcps::Socket::write(request);
      }

      void tcp_on_recvd(std::string recvd) {
        if (!open) {
          if (!initiated) {
            auto response = p$string::split<std::list>(recvd, "\r\n");

            auto top = p$funcs::pop(response);

            if (!top || top != "HTTP/1.1 101 Switching Protocols")
              return;

            initiated = !0;
            open = !0;

            return ws_on_open();
          }

          else {
            printf("-> %s", recvd.data());
          }
        }



        else {
          auto [opcode, dec] = p$web::ws::decode(!1, recvd, rxbuf);

          if (opcode == p$web::ws::opcode::UNKNOWN)
            return;

          printf("opcode: %i dec: %s\r\n", 
            std::underlying_type_t<p$web::ws::opcode>(opcode),
            (*dec).data()
          );

          if (opcode == p$web::ws::opcode::CLOSE) {
            open = !1;
            return ws_on_close();
          }

          if (opcode == p$web::ws::opcode::TEXT)
            return ws_on_text(*dec);
        }

      }

      Socket(std::string_view addr) : p$web::tcps::Socket() {
        parsed = p$web::url::parse(&addr[0]);
        initiated = !1;
        open = !1;
      }

      void connect() {
        p$web::tcps::Socket::connect(parsed.port, parsed.host);
      }

      void write(std::string text) {
        auto enc = p$web::ws::encode(!0, p$web::ws::opcode::TEXT, text);
        p$web::tcps::Socket::write(enc);
      }
  };
}