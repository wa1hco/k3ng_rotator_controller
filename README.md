# K3NG Rotator Controller

## Introduction

This is an Arduino-based rotator interface that interfaces a computer to a rotator or rotator controller, emulating the Yaesu GS-232A/B and Easycom protocols which are supported by a myriad of logging, contest, and control programs.  It can be easily interfaced with commercial rotator control units.  With the addition of a proper capacity power supply and several interface components such as relays, this unit could also serve as a total replacement for a rotator control unit or serve as the basis for a 100% homebrew rotation system.  Several azimuth and elevation position sensors including potentiometers, rotary encoders, and I2C devices are supported.  The code is very flexible, modular, and easy to read allowing intermediate and advanced experimenters and builders to customize it.

## Documentation

Full documentation is located [here](https://github.com/k3ng/k3ng_rotator_controller/wiki).  Please read it!  Volunteers for maintaining documentation are needed.

## Unit Testing (Host)

A lightweight native C++ test harness is included to support refactoring and regression testing of hardware-independent logic.

Run tests:

```bash
cd tests
make test
```

See `tests/README.md` for details and conventions.

## PC Simulator (Dual Axis)

A dual-axis software simulator is available for high-speed, low-noise development on a host PC.
It includes:

- Simulated controller display (sensor/readout and commanded targets)
- Simulated hardware position display (actual mechanism state)
- Yaesu/Easycom-style command shim (`proto` command)
- Mechanical effects (`backlash`, `stiction`)
- Fault injection (`stall`, `dropout`, `stuck_cw`, `stuck_ccw`)

Build and run:

```bash
cd simulator
make build
./pc_rotator_sim
```

Example session:

```text
status
proto W 270 60
run 4000 20
fault az stall on
status
```

### GUI Simulator (Mouse Control, Fast Real Time)

A browser-based GUI is served by the C++ simulator backend.

Features:

- Virtual controller panel and hardware panel shown side-by-side
- Mouse-click controls for `FWD / CW`, `REV / CCW`, and `STOP`
- Real-time simulation loop
- Speed control in seconds per 360 degrees (default: 10 seconds)

Run:

```bash
cd simulator
make gui
./gui_server
```

Then open:

`http://localhost:8080/`

## Features

* Azimuth only and azimuth / elevation rotator support
* Serial interface using the standard Arduino USB port
* Control Port Protocol Support:
 * Yaesu GS-232A & GS-232B
 * Easycom
* Support for position sensors:
 * Potentiometers / Analog Voltage
 * Rotary Encoders
 * Incremental Encoders
 * Pulse Output
 * HMC5883L digital compass
 * ADXL345 accelerometer
 * LSM303 digital compass and accelerometer
 * HH-12 / AS5045
 * A2 Absolute Encoder (under development)
* LCD display (2 or 4 rows, at least 16 columns)
* Can be interfaced with non-Yaesu rotators, including homebrew systems
* Directional indication on LCD display (North, South, North Northwest, etc.) along with degrees
* Intelligent automatic rotation (utilizes overlap on 450 degree rotators)
* Support for both 360 degree and 450 degree azimuth rotators or any rotation capability up to 719 degrees
* North Center and South Center support
* Support for any starting point (fully clockwise)
* Optional automatic azimuthal rotation slowdown feature when reaching target azimuth
* Optional rotation smooth ramp up
* Optional brake engage/disengage lines for azimuth and elevation
* Buttons for manual rotation
* Command timeout
* Timed interval rotation
* Overlap LED Indicator
* Help screen
* Speed Control, both single PWM output (compatible with Yaesu controllers) and dual PWM rotate CW and rotate CCW outputs and dual elevate up and elevate down outputs
* Variable frequency outputs
* Preset Control using either potentiometers or rotary encoders with optional preset start button
* Speed Potentiometer
* Manual Rotation Limits
* Classic 4 bit, Adafruit I2C LCD, and Yourduino.com Display Support
* Optional tenth of a degree support with Easycom protocol (i.e. 123.4 degrees)
* Park button
* Azimuth and elevation calibration tables
* Host unit and Remote unit operation for remotely located sensors using two Arduinos or ATMega chips
* Works with hamlib rotctl/rotcltd, HRD, N1MM, PST Rotator, and many more programs
* Moon and Sun Tracking
* GPS Interfacing
* Realtime Clock Interfacing

## Acknowledgements

John, W3SA, has tested on a Yaesu Az/El unit, contributed several updates to the elevation code, and tweaked the code for a 16 column LCD display.

Anthony, M0UPU, [wrote about](http://ava.upuaut.net/?p=372) his rotator controller construction and is offering PC boards.

Bent, OZ1CT, has contributed several ideas and feature requests, and performed testing.

G4HSK has a [nice page documenting](http://radio.g4hsk.co.uk/2m-eme/rotator-controller/) his project using this code, the PstRotator control software, and a Yaesu G-5500 rotator.

All trademarks mentioned on this page and in the code are property of their respective owners.

## DXpeditions

I will donate parts, units, or specially customized software for DXpeditions.  Email me at anthony dot good at gmail dot com.  DX IS!

## Support and Feature Requests

Please consult [this page](https://blog.radioartisan.com/support-for-k3ng-projects/) for support information.  Feature requests and bugs are documented and tracked on [GitHub](https://github.com/k3ng/k3ng_rotator_controller/issues).

Please note that I do this work in my spare time as I can and I am not a professional developer, however I play one on TV.  I do my best to answer support requests,  however I don’t like having to answer questions for items that are explained in the [documentation](https://github.com/k3ng/k3ng_rotator_controller/wiki).  I do maintain a list of [feature requests](https://github.com/k3ng/k3ng_rotator_controller/issues).  Development items are prioritized by me based on the level of difficulty and what I’m interested in.  I welcome code contributions, code testing, bug reports, and any help you can provide.  This can even be helping with [documentation](https://github.com/k3ng/k3ng_rotator_controller/wiki) or providing support to others on the [Radio Artisan discussion group](https://groups.yahoo.com/neo/groups/radioartisan/info).
