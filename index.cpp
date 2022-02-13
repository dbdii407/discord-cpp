#include "includes/ptyps/web/discord.hpp"

// Copyright (C) 2022 Dave Perry (dbdii407)

class Bot : public ptyps::web::discord::Gateway {
  private:
    void gateway_on_disconnect() {
      printf("Gateway was disconnected\r\n");
    }

    void gateway_on_connect() {
      printf("Gateway has connected\r\n");
    }

    void gateway_on_open() {
      printf("Gateway was opened\r\n");
    }

    void gateway_on_close() {
      printf("Gateway is closing\r\n");
    }

    void gateway_on_ready(ptyps::json::obj data) {
      auto text = ptyps::json::stringify(data);

      printf("READY: %s\r\n", text.data());
    }

    void gateway_on_guild_create(ptyps::json::obj data) {
      auto text = ptyps::json::stringify(data);

      printf("GUILD: %s\r\n", text.data());
    }

  public:
    Bot() : ptyps::web::discord::Gateway("./config.json") {

    }
};

int main() {
  auto bot = Bot();

  bot.connect();

  bot.loop();
}