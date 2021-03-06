#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../crypto.hpp"
#include "../random.hpp"
#include "./tcp.hpp"
#include "./url.hpp"

namespace ptyps::web::ws {
  using exception = ptyps::err::exception;

  enum class state {
    CONNECTING,
    OPEN,
    CLOSING,
    CLOSE
  };

  // -----

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
    CONTINUATION = OPCODE_CONTINUATION,
    TEXT,
    BINARY,
    CLOSE = OPCODE_CLOSE,
    PING = OPCODE_PING,
    PONG = OPCODE_PONG
  };

  // -----

  constexpr int STATUS_NORMAL = 1000;
  constexpr int STATUS_GOING_AWAY = 1001;
  constexpr int STATUS_PROTO_ERROR = 1002;
  constexpr int STATUS_UNSUPPORTED_DATA = 1003;
  constexpr int STATUS_NO_STATUS = 1005;
  constexpr int STATUS_RESERVED = 1006; // not to be used
  constexpr int STATUS_INVALID_PAYLOAD = 1007;
  constexpr int STATUS_POLICY_VIOLATION = 1008;
  constexpr int STATUS_MSG_TOO_BIG = 1009;
  constexpr int STATUS_EXT_REQUIRED = 1010;
  constexpr int STATUS_INTERNAL_EP_ERROR = 1011;
  constexpr int STATUS_SERVICE_RESTART = 1012;
  constexpr int STATUS_TRY_AGAIN_LATER = 1013;
  constexpr int STATUS_BAD_GATEWAY = 1014;
  constexpr int STATUS_TLS_HANDSHAKE = 1015;

  constexpr int STATUS_SUBPROTO_ERROR = 3000;
  constexpr int STATUS_INVALID_SUBPROTO_DATA = 3001;

  enum class status {
    NORMAL = 1000,
    GOING_AWAY,
    PROTO_ERROR,
    UNSUPPORTED_DATA,

    NO_STATUS = 1005,
    RESERVED,
    INVALID_PAYLOAD,
    POLICY_VIOLATION,
    MSG_TOO_BIG,
    EXT_REQUIRED,
    INTERNAL_EP_ERROR,
    SERVICE_RESTART,
    TRY_AGAIN_LATER,
    BAD_GATEWAY,
    TLS_HANDSHAKE,

    SUBPROTO_ERROR = 3000,
    INVALID_SUBPROTO_DATA
  };


  static std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

  std::string createHandshakeKey() {
    auto random = ptyps::random::string(12, std::vector<char>({
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
      'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
      'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
      'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    }));

    auto hash = ptyps::crypto::sha1(random + magic);

    return ptyps::crypto::base64(hash);
  }

  std::vector<uint8_t> createMaskingKey() {
    auto out = std::vector<uint8_t>();

    for (auto i = 0; i < MASKLEN; i++) {
      auto next = ptyps::random::number(0, 255);
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

  std::vector<uint8_t> UInt16ToUInt8(ulong i) {
    auto vect = std::vector<uint8_t>();

    vect.push_back((i >> 8) & 0xFF);
    vect.push_back((i >> 0) & 0xFF);

    return vect;
  }

  std::string encode(bool masked, opcode op, std::string data) {
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

      for (auto i : UInt16ToUInt8(length))
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

    ptyps::funcs::append(vect, data);

    if (masked) {
      for (auto i = 0; i != length; ++i)
        vect[size + i] ^= key[i & 0x03];
    }

    return ptyps::funcs::to<std::basic_string, char>(vect);
  }

  uint16_t UInt16FromUInt8(std::vector<uint8_t> &vect, int &pos) {
    return ((uint16_t) vect[pos++] << 8) |
           ((uint16_t) vect[pos++] << 0);
  }

  uint64_t UInt64FromUInt8(std::vector<uint8_t> &vect, int &pos) {
    return ((uint64_t) vect[pos++] << 56) |
           ((uint64_t) vect[pos++] << 48) |
           ((uint64_t) vect[pos++] << 40) |
           ((uint64_t) vect[pos++] << 32) |
           ((uint64_t) vect[pos++] << 24) |
           ((uint64_t) vect[pos++] << 16) |
           ((uint64_t) vect[pos++] <<  8) |
           ((uint64_t) vect[pos++] <<  0);
  }

  using decode_variant = std::variant<std::monostate, std::string, status>;
  using decode_callback = std::function<void(opcode, decode_variant)>;

  void decode(std::string recvd, std::string &rxbuf, decode_callback func) {
    if (rxbuf.length())
      recvd = rxbuf + recvd;
    
    rxbuf.clear();

    auto vect = ptyps::funcs::to<std::vector, uint8_t>(recvd);

    while (!0) {
      if (vect.size() < 2) {
        ptyps::funcs::append(rxbuf, vect);
        break;
      }

      // auto fin = (vect[0] & FIN) == FIN; // if 1, last message in series
      auto msk = (vect[1] & MASK) == MASK; // if masked

      auto opc = vect[0] & 0x0F; // opcode
      auto len = vect[1] & 0x7F; // content length

      auto pos = 2;

      if (len == 126)
        len = UInt16FromUInt8(vect, pos);

      if (len == 127)
        len = UInt64FromUInt8(vect, pos);

      auto size = (msk ? MASKLEN : 0) + (len >= 65536 ? 6 : 0) + // frame header size
                  (len >= 126 ? 2 : 0) + 2;

      auto total = size + len; // total length of the frame

      if (vect.size() < total) {
        ptyps::funcs::append(rxbuf, vect);
        break;
      }

      auto key = std::vector<int>();

      if (msk) {
        for (auto i = 0; i < MASKLEN; i++)
          key[i] = ((int) vect[size + i]) << 0;
      }

      // -----

      if (opc == OPCODE_PING) {
        printf("ping len: %i\r\n", len);
      }

      if (opc == OPCODE_CLOSE) {
        auto code = UInt16FromUInt8(vect, pos);
        auto status = static_cast<enum status>(code);

        func(opcode::CLOSE, status);
      }

      if (opc == OPCODE_TEXT || opc == OPCODE_BINARY || opc == OPCODE_CONTINUATION) {
        auto begin = ptyps::iter::begin(vect, size);
        auto end = ptyps::iter::begin(vect, total);

        auto data = std::string(begin, end);

        func(opcode::TEXT, data);
      }

      ptyps::funcs::chop(vect, total);
    }
  }
}

namespace ptyps::web::wss {
  using opcode = ptyps::web::ws::opcode;
  using status = ptyps::web::ws::status;
  using state = ptyps::web::ws::state;

  class Socket : ptyps::web::tcps::Socket {
    private:
    ptyps::web::url::parsed parsed;
    std::string rxbuf;
    state cond;

    public:
      using ptyps::web::tcps::Socket::connected;
      using ptyps::web::tcps::Socket::loop;

      virtual void ws_on_disconnect() { }
      virtual void ws_on_connect() { }
      virtual void ws_on_open() { }
      virtual void ws_on_close(ptyps::web::ws::status) { }
      virtual void ws_on_text(std::string text) {}

      void tcp_on_disconnect() {
        cond = state::CLOSE;

        ws_on_disconnect();
     }
     
      void tcp_on_connect() {
        cond = state::CONNECTING;

        ws_on_connect();

        auto list = std::vector<std::string>();

        auto top = ptyps::string::format("GET /%s HTTP/1.1", parsed.query.data());

        list.push_back(top);
        list.push_back("Host: " + parsed.host);
        list.push_back("Upgrade: websocket");
        list.push_back("Connection: Upgrade");
        list.push_back("Sec-WebSocket-Key: " + ptyps::web::ws::createHandshakeKey());
        list.push_back("Sec-WebSocket-Version: 13");
        list.push_back({});
        list.push_back({});

        auto request = ptyps::string::join(list, "\r\n");

        ptyps::web::tcps::Socket::write(request);
      }

      void tcp_on_recvd(std::string recvd) {
        if (cond == state::CONNECTING) {
          auto response = ptyps::string::split<std::list>(recvd, "\r\n");

          auto top = ptyps::funcs::pop(response);

          if (!top || top != "HTTP/1.1 101 Switching Protocols")
            return;

          cond = state::OPEN;

          return ws_on_open();
        }

        if (cond == state::OPEN) {
          ptyps::web::ws::decode(recvd, rxbuf, [&](opcode opcode, ptyps::web::ws::decode_variant vari) {
            if (opcode == opcode::CLOSE) {
              cond = state::CLOSING;

              auto code = std::get<status>(vari);
              return ws_on_close(code);
            }

            if (opcode == opcode::TEXT) {
              auto text = std::get<std::string>(vari);
              return ws_on_text(text);
            }
          });
        }

        if (cond == state::CLOSING) {
          printf(recvd.data());
        }
      }

      Socket(std::string_view addr) : ptyps::web::tcps::Socket() {
        parsed = ptyps::web::url::parse(&addr[0]);
      }

      void connect() {
        ptyps::web::tcps::Socket::connect(parsed.port, parsed.host);
      }

      void write(std::string text) {
        auto enc = ptyps::web::ws::encode(!0, ptyps::web::ws::opcode::TEXT, text);
        ptyps::web::tcps::Socket::write(enc);
      }
  };
}
