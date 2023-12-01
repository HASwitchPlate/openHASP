# WireGuard Implementation for ESP32 Arduino

This is an implementation of the [WireGuard&reg;](https://www.wireguard.com/) for ESP32 Arduino.

Almost all of this code is based on the [WireGuard Implementation for lwIP](https://github.com/smartalock/wireguard-lwip), but some potion of the code is adjusted to build with ESP32 Arduino.

## How to use 

1. Include `WireGuard-ESP32.h` at the early part of the sketch.

```c++
#include <WireGuard-ESP32.h>
```

2. Define the instance of the `WireGuard` class at module level.

```c++
static WireGuard wg;
```

3. Connect to WiFi AP by using `WiFi` class.

```c++
WiFi.begin(ssid, password);
while( !WiFi.isConnected() ) {
    delay(1000);
}
```

4. Sync the system time via NTP.

```c++
configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");
```

5. Start the WireGuard interface.

```c++
wg.begin(
    local_ip,           // IP address of the local interface
    private_key,        // Private key of the local interface
    endpoint_address,   // Address of the endpoint peer.
    public_key,         // Public key of the endpoint peer.
    endpoint_port);     // Port pf the endpoint peer.
```

You can see an example sketch `uptime_post.ino`, which connects SORACOM Arc WireGuard endpoint and post uptime to SORACOM Harvest via WireGuard connection.

## License

The original WireGuard implementation for lwIP is licensed under BSD 3 clause license so the code in this repository also licensed under the same license.

Original license is below:

The code is copyrighted under BSD 3 clause Copyright (c) 2021 Daniel Hope (www.floorsense.nz)

See LICENSE for details
