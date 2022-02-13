discord-cpp
===

FYI: It should be noted that this library doesn't work. It will connect but it won't stay alive.

### Install required libraries

  - boost, this is required for the JSON configuration file.
  - openssl
  - curl

### Build

```console
clear && make
```
### Create configuration file
  - Create config.json in the main directory

  ```js
  {
    "token": "",
    "id": ""
  }
  ```

### Setup Discord
  - From Discord
    - Create server from your Discord application
  
  - From Discord Developers Page
    - Create a new Discord Application (this can be done from [here](https://discord.com/developers/applications))
    - Retrieve your application's ID for your configuration file. This can be found by clicking OAuth2 from the side menu and looking for the ID under "Client ID".
    - Retrieve your application's token for your configutation file. This can be done by clicking Bot from the side menu and selecting "Click to Reveal Token"
    - Generate a URL from the URL Generator found under OAuth. Choose the following scopes:
      - identify
      - guilds
      - bot
      - messages.read
    - Copy the URL from below scopes and open it in your browser.
    - Tell Discord to authorize your bot to join your server.

### Run

```console
./dist/app
```
