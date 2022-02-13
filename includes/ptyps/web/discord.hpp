#pragma once

// Copyright (C) 2022 Dave Perry (dbdii407)

#include "../json.hpp"
#include "./ws.hpp"

namespace ptyps::web::discord {
  using exception = ptyps::err::exception;

  constexpr int OP_DISPATCH = 0;
  constexpr int OP_HEARTBEAT = 1;
  constexpr int OP_IDENTIFY = 2;
  constexpr int OP_PRESENCE_UPDATE = 3;
  constexpr int OP_VOICE_STATE_UPDATE = 4;
  constexpr int OP_RESUME = 6;
  constexpr int OP_RECONNECT = 7;
  constexpr int OP_REQUEST_GUILD_MEMBERS = 8;
  constexpr int OP_INVALID_SESSION = 9;
  constexpr int OP_HELLO = 10;
  constexpr int OP_HEARTBEAT_ACK = 11;

  enum class opcode {
    DISPATCH = OP_DISPATCH,
    HEARTBEAT = OP_HEARTBEAT,
    IDENTIFY = OP_IDENTIFY,
    PRESENCE_UPDATE = OP_PRESENCE_UPDATE,
    VOICE_STATE_UPDATE = OP_VOICE_STATE_UPDATE,
    RESUME = OP_RESUME,
    RECONNECT = OP_RECONNECT,
    REQUEST_GUILD_MEMBERS = OP_REQUEST_GUILD_MEMBERS,
    INVALID_SESSION = OP_INVALID_SESSION,
    HELLO = OP_HELLO,
    HEARTBEAT_ACK = OP_HEARTBEAT_ACK
  };

  std::string createPacket(opcode op, ptyps::json::obj data) {
    return ptyps::json::stringify({
      {"op", std::underlying_type_t<opcode>(op)},
      {"d", data}
    });
  }

  class Gateway : public ptyps::web::wss::Socket {
    private:
      ptyps::json::obj opts;
      int last;

      virtual void gateway_on_disconnect() { }
      virtual void gateway_on_connect() { }
      virtual void gateway_on_open() { }
      virtual void gateway_on_close() { }

      virtual void gateway_on_ready(ptyps::json::obj data) { }
      virtual void gateway_on_guild_create(ptyps::json::obj data) { }

      void ws_on_disconnect() {
        gateway_on_disconnect();
      }

      void ws_on_connect() {
        gateway_on_connect();
      }

      void ws_on_open() {
        gateway_on_open();
      }

      void ws_on_close() {
        gateway_on_close();
      }

      void ws_on_text(std::string text) {
        auto packet = ptyps::json::parse(text);

        auto sequence = ptyps::json::value<int>(packet, "s");

        if (sequence)
          last = *sequence;

        auto opc = ptyps::json::value<int>(packet, "op");

        if (opc == OP_HELLO) {
          auto payload = createPacket(opcode::IDENTIFY, {
            {"token", *ptyps::json::value<std::string>(opts, "token")},
            {"intents", 513},
            {"properties", {
              {"$browser", "chrome"},
              {"$device", "chrome"},
              {"$os", "linux"}
            }}
          });

          write(payload);

          // -----

          // This part confuses me. According to Discord, we have to send
          // a heartbeat payload based on the heartbeat_interval provided
          // to us. However, whenever I try this, it just... disconnects
          // after sending it. This means, according to what I've got from
          // reading the WebSocket RFIC, that it's been formatted incorrectly.
          // However, I can't seem to find an example or an explanation
          // of how to send this properly. So, it won't stay connected
          // for long.
          //
          // https://discord.com/developers/docs/topics/gateway#heartbeat

          auto beat = ptyps::json::value<int>(packet, "d.heartbeat_interval");

          auto time = std::chrono::milliseconds(*beat);

          return ptyps::thread::interval_run(time, [&]() -> bool {
            if (!connected())
              return !0;

            auto payload = std::string();

            if (!last)
              payload = createPacket(opcode::HEARTBEAT, nullptr);

            else
              payload = createPacket(opcode::HEARTBEAT, last);

            write(payload);

            return !1;
          });
        }
      
        if (opc == OP_DISPATCH) {
          auto event = ptyps::json::value<std::string>(packet, "t");

          if (event == "READY") {
            auto data = ptyps::json::get(packet, "d");
            return gateway_on_ready(*data);
          }

          if (event == "GUILD_CREATE") {
            auto data = ptyps::json::get(packet, "d");
            return gateway_on_guild_create(*data);
          }
        }
      }

    public:
      Gateway(std::string_view filepath) : ptyps::web::wss::Socket("wss://gateway.discord.gg/?v=9&encoding=json") {
        opts = ptyps::json::open(&filepath[0]);
      }
  };
}