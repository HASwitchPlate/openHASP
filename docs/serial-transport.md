# Generic UART transport

This extension is optional firmware code, with an ESP32 hardware
implementation, not a dynamically installed plugin. `hasp_uart::UartTransport`
owns its configuration, lifecycle, framing state, dispatch integration and
diagnostics.

## Build and configuration

Add these flags to your existing board environment or `override.build_flags`
in `platformio_override.ini`:

```ini
-D HASP_USE_UART_TRANSPORT=1
```

Leave the flag unset for stock behavior. UART transport and custom extensions are
independent features and may be enabled together. A separate LVGL task requires
openHASP's ESP MQTT/GUI mutex support; unsupported combinations fail compilation.

Upload `/serial.json` using the normal panel file editor, then restart. This
separate file is not part of openHASP's main `config.json` and is not rewritten
by normal configuration saves. There is no new web settings form.

Safe initial contents:

```json
{"enabled":false,"uart":1,"rx":-1,"tx":-1,"baud":115200}
```

| Key | Meaning |
| --- | --- |
| `enabled` | Boolean; absent file or false disables the transport |
| `uart` | Available hardware UART number, 1 or 2; UART0 is excluded |
| `rx` | Panel receive GPIO, connected to the host's TX |
| `tx` | Panel transmit GPIO, connected to the host's RX |
| `baud` | Integer 1200–921600; the examples use 115200 |

All enabled settings must be valid; invalid configuration fails closed and logs
the reason on the normal debug output. Settings apply only at boot. Framing is
fixed at 8N1, non-inverted, no flow control. Logs and console echoes stay on the
normal console, never the transport UART. There is no transport auto-detection.

The startup checks reject unavailable/input-only TX pins, known system/display/
touch/backlight pins, configured GPIO functions, default console pins and active
S3 USB CDC pins. A configured serial dimmer reserves UART1; a compiled Tasmota
client reserves UART2. Do not add another UART consumer after boot. Custom
hardware, microphone, SD and expansion circuitry still require a board-level
pin review; software checks cannot establish electrical availability.

## Porting to other hardware

The implementation follows openHASP's existing `src/hal` and per-platform `src/dev`
patterns and its `ESP32` compile guards. It adds a small UART transport abstraction
without changing BaseDevice.

| Layer | Files | Responsibilities |
| --- | --- | --- |
| Framing | `src/sys/svc/hasp_uart_framing.h` | Bounded lines, output queue, partial writes, readiness marker; standard C++ only |
| Message content | `src/sys/svc/hasp_uart_messages.h` | Stock state JSON to event/page tokens; existing ArduinoJson dependency only |
| openHASP integration | `src/sys/svc/hasp_uart_transport.{h,cpp}` | UartTransport configuration, lifecycle, dispatch, pin ownership and diagnostics |
| Hardware contract | `src/hal/hasp_uart.h` | UART configuration, byte I/O, pin ownership, locking |
| ESP32 implementation | `src/hal/esp32/hasp_uart_esp32.cpp` | HardwareSerial, pin/UART checks and FreeRTOS mutex |

To port it, implement the functions declared in `hasp_uart.h` and define
`HASP_UART_EXTERNAL_HAL` to replace the built-in implementation. The interface
uses only standard integer/size types, not ESP32, Arduino, LVGL or JSON types.
`read()` returns -1 when no byte is available. Writes must be nonblocking and
may return short counts; the common queue retains unsent bytes. Supply a mutex
when hooks can run concurrently, or no-op locking on a single-threaded target.
Callbacks are never called from an ISR. Keep GUI locking in the platform's
normal openHASP loop, not in the UART HAL.

Port numbers and pin validity are hardware decisions: the common code does not
assume ESP32 ranges. The ESP32 implementation excludes UART0, but a future port
can choose its own safe port policy. The test HAL deliberately uses UART4 and
pins 101/102. Targets lacking an implementation compile a disabled fallback;
they do not accidentally enable unvalidated pins.

The current configuration loader uses openHASP's `HASP_FS` when SPIFFS/LittleFS
is enabled. Platforms using another storage model also need to supply configuration
loading in UartTransport; message encoding and parsing need no changes.

## Wire format

Host → panel uses ordinary openHASP commands, one per LF-terminated line:

```text
jsonl {"page":1,"id":21,"text":"0.125"}
page 2
```

Up to 1022 bytes of command text are accepted, plus optional CR and final LF.
NUL/control bytes, embedded CR and oversized lines are discarded through LF.
Empty lines are ignored. The receiver never executes a partial/overflow suffix.
It reads at most 256 bytes per main-loop iteration and dispatches complete
commands with `dispatch_text_line` in the existing GUI execution context.

Panel → host uses simple tokens separated by one ASCII space:

```text
event p1b48 down
event p1b48 release
page 2
ready 1
```

Object event names `down`, `up`, `release`, `lost`, `long`, `hold` and `changed`
are extracted from the stock state payload using openHASP's existing ArduinoJson
library. Only the top-level `event` string is used. Tags, values and metadata
are omitted.
Numeric page states are forwarded. Other states remain MQTT-only. No broker,
MQTT topic prefix or MQTT packet framing is carried on the UART.

The fixed output queue holds eight frames of up to 512 bytes before LF. The
main loop drains at most 128 bytes at a time, limited by UART write capacity;
callbacks never wait for transmission at wire speed. Queue operations use a
mutex, so callbacks cannot interleave partial frames. A full queue is cleared
and replaced with a readiness marker, cancelling a peer's held touch and
requesting a refresh. The event that caused overflow is discarded. This avoids
silently retaining a held key after losing its release, but does not guarantee
event delivery. Repeated overload can still prevent operation.

`serial.dropped` is added to the ordinary sensor JSON when enabled. It counts
rejected output frames/overflow incidents, not every frame cleared by recovery.
It is diagnostic data, not an acknowledgement or link-health measure.

## Startup, reload and recovery

State forwarding is suppressed until a layout file has been read and the first
main-loop iteration after startup runs. The first output is `\nready 1\n`,
including a leading LF to terminate a stale partial input at the host. The normal
file-layout reload path also replaces pending output with that marker before
forwarding later events. This says the loader has returned; it does not validate
application-specific object IDs or certify every JSONL record. Upstream's
parser still reports layout errors through normal logging.

Uploading a layout file alone does not load it. Use the normal panel reload
operation or restart. For layouts streamed as individual JSONL commands, send
this compatibility command after the complete layout is installed:

```text
custom/serialready 1
```

It can also be used to resend readiness when the host starts after the panel.
There is no periodic ready spam, heartbeat, acknowledgement, or link-loss
watchdog. A wire/power failure can still lose a held touch's release. Hardware
validation and a separate decision on link-loss handling remain necessary.
