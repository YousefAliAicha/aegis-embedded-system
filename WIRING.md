# AEGIS — Wiring Reference

Complete pin-level connection map for the Deneyap Kart 1A build.

---

## 1602A LCD (16-pin, HD44780, 4-bit parallel mode)

| LCD Pin | Label | Connected to |
|---|---|---|
| 1  | VSS | GND (unified ground rail) |
| 2  | VDD | MB102 5 V output rail |
| 3  | VO  | 10 kΩ potentiometer wiper (divider between 5 V and GND) |
| 4  | RS  | D10 |
| 5  | R/W | GND (write-only mode — R/W is never needed in this project) |
| 6  | E   | D4 |
| 7  | D0  | No connection |
| 8  | D1  | No connection |
| 9  | D2  | No connection |
| 10 | D3  | No connection |
| 11 | D4  | D5 |
| 12 | D5  | D6 |
| 13 | D6  | D7 |
| 14 | D7  | D12 |
| 15 | A (BLA) | 5 V via 220 Ω resistor (backlight anode) |
| 16 | K (BLK) | GND (backlight cathode) |

**Note on VO (contrast):** Wire a 10 kΩ potentiometer between the 5 V rail and GND. Connect the wiper to LCD pin 3. Adjust until characters are clearly visible. Do not use the direct-GND shortcut — it only works on some LCD batch variants. See [issue #4](./issue-04-lcd-batch-variance-friction-fit.md).

---

## HC-SR04 Ultrasonic Sensor

| HC-SR04 Pin | Connected to |
|---|---|
| VCC  | MB102 5 V output rail |
| TRIG | D1 |
| ECHO | D11 |
| GND  | GND (unified ground rail) |

**Note on ECHO voltage:** The HC-SR04 outputs a 5 V ECHO pulse. `D11` on the Deneyap Kart 1A handles this safely. Do not remap ECHO to an arbitrary GPIO without checking 5 V tolerance first.

---

## SG90 Servo Motor

| SG90 Wire | Colour (typical) | Connected to |
|---|---|---|
| GND | Brown / Black | GND (unified ground rail) |
| VCC | Red | MB102 5 V output rail |
| PWM | Orange / Yellow | D0 |

---

## MB102 Breadboard Power Module

| MB102 Terminal | Connected to |
|---|---|
| Input barrel jack | 9 V DC wall adapter, centre-positive, ≥ 1 A |
| 5 V output rail | Breadboard positive rail (feeds servo, HC-SR04, LCD) |
| GND output rail | Breadboard negative rail **and** Deneyap Kart GND pin |

**Critical:** The MB102 GND must be wired directly to the Deneyap Kart's GND pin. Without a common ground reference, all signal levels between the two supply domains are undefined.

**Do not use a 9 V PP3 battery** as the MB102 input. See [issue #5](./issue-05-9v-battery-unstable-mb102.md).

---

## Deneyap Kart 1A — Pin Summary

| Deneyap Pin | Function | Connected to |
|---|---|---|
| D0  | PWM output | SG90 signal wire |
| D1  | Digital output | HC-SR04 TRIG |
| D4  | Digital output | LCD E (Enable) |
| D5  | Digital output | LCD D4 |
| D6  | Digital output | LCD D5 |
| D7  | Digital output | LCD D6 |
| D10 | Digital output | LCD RS |
| D11 | Digital input  | HC-SR04 ECHO |
| D12 | Digital output | LCD D7 |
| GND | Ground reference | MB102 GND rail |

Pins D9, D14, and other flash-bus-adjacent lines are intentionally left unconnected. See [issue #1](./issue-01-flash-bus-pin-conflict.md).
