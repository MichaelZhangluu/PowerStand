# PowerStand

> Real Controls. Modern Protocols. Open Architecture.

PowerStand is an open-source digital locomotive control stand for model railroads.

Inspired by modern locomotive control desks, PowerStand combines physical controls, touchscreen operation, and support for multiple railroad control protocols into a single operator console.

PowerStand prioritizes tactile operation through dedicated physical controls including throttle, brake, reverser, horn, and lighting controls.

The project is designed around the ESP32 platform and a 7-inch touchscreen display and is intended to be fully documented, reproducible, and expandable.

---

# Project Goals

PowerStand aims to provide:

- Realistic locomotive-inspired operation
- Support for DCC-EX
- Support for WiThrottle
- Physical-first user interaction
- Modern LVGL-based user interface
- Expandable hardware architecture
- Complete mechanical and electrical documentation
- Reproducible DIY construction

---

# Planned Features

## Physical Controls

- Throttle Lever
- Brake Lever
- Reverser Switch
- Headlight Control
- Horn Control
- Emergency Stop
- Rotary Encoder

## Display

- 7-inch TFT Touchscreen
- LVGL User Interface
- Optional Auxiliary Status Display

## Protocol Support

### Initial Release

- DCC-EX
- WiThrottle

### Future Consideration

- Additional command station protocols
- Wireless accessories
- Expansion modules

---

# Repository Structure

```text
PowerStand/
│
├── docs/
├── firmware/
├── hardware/
├── assets/
└── tools/
```

---

The ESP-IDF project can be opened and built from the repository root. Its CMake
entry point discovers the application and components in `firmware/`.

# Development Status

Current Phase:

✅ Project Definition

Next Milestones:

- Human Device Specification
- Hardware Design
- Firmware Architecture
- Prototype Electronics
- Prototype Mechanical Design

---

# Contributing

Community contributions are welcome.

Examples include:

- Documentation improvements
- Bug fixes
- Firmware enhancements
- UI improvements
- CAD improvements
- Hardware testing

All contributions must remain under the project license.

---

# Commercial Use

PowerStand may be used freely for:

- Personal projects
- Home layouts
- Educational purposes
- Research
- Clubs
- Non-commercial community activities

Commercial use is prohibited without written permission from the project owner.

Examples of commercial use include:

- Selling assembled PowerStand devices
- Selling kits
- Selling PCBs
- Selling 3D printed parts
- Selling derivative products
- Commercial consulting using PowerStand designs as a product

For commercial licensing inquiries, contact the project owner.

---

# License

PowerStand is licensed under:

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)

You are free to:

- Share
- Copy
- Distribute
- Modify
- Adapt
- Build

provided that:

- Appropriate attribution is given
- The use is non-commercial
- Derivative works are released under the same license

License details:

https://creativecommons.org/licenses/by-nc-sa/4.0/

---

# Disclaimer

PowerStand is provided "as-is" without warranty of any kind.

Users are responsible for ensuring compliance with local laws, electrical safety standards, and operating practices.

---

PowerStand™

Real Controls. Modern Protocols. Open Architecture.