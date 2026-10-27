# PC-001 Project Charter

| Field        | Value            |
|--------------|------------------|
| Project Name | PowerStand       |
| Document ID  | PC-001           |
| Version      | 0.1 Draft        |
| Status       | Draft            |
| Author       | Michael Zhang    |
| Date         | 2026-10-26       |

---
# 1. Executive Summary

PowerStand is an open-source digital model railroad control stand designed to deliver a realistic, tactile, and highly configurable locomotive operating experience.

The system combines dedicated physical controls, touchscreen interaction, embedded hardware, and support for multiple train-control protocols into a single operator console.

PowerStand emphasizes physical operation through locomotive-inspired controls including a throttle lever, brake lever, reverser, horn, and lighting controls etc.

The platform is built around an ESP32 hardware platform, a 7-inch touchscreen display, and other control hardwares inlcuding switches, rotary encoders, I/O extenders, mono LCD displays etc., while maintaining support for future hardware expansion and software enhancements.

The primary control protocol shall be DCC-EX, with WiThrottle support provided for interoperability with JMRI and other compatible systems.

---

# 2. Vision Statement

To develop a capable open-source locomotive-inspired model railroad control stand, combining the realism of physical controls with the flexibility of modern digital command systems.

PowerStand shall provide:

- Realistic operator interaction, this is a console with physical controls for major locomotive operations
- Multi-protocol compatibility, this console supports DCCEX protocol as primary system integrator, WiThrottle is the secondary protocol
- Open-source hardware and software, all desing documents, software and hardware are free to use to hobbiest.
- Expandable architecture, this design shall suport future expensibility with other hardware, software and protocols
- Modern graphical user interface, lvgl based UI design that can be easily migrated to other platforms.
- Long-term maintainability

---

# 3. Mission Statement

PowerStand provids a dedicated control stand that places physical controls at the center of the user experience.

The project will prioritize reliability, operator immersion, responsiveness, and extensibility.

---

# 4. Project Objectives

## PO-01 Physical Control First

Train operation shall be primarily performed through dedicated physical controls.

Primary controls should remain usable without continuous visual attention.

---

## PO-02 Multi-Protocol Operation

PowerStand shall support multiple railroad command protocols.

Initial implementation:

- DCC-EX (Primary)
- WiThrottle (Secondary)

Future protocols may be added without major architectural changes.

---

## PO-03 Open Architecture

The project shall provide complete documentation and source code necessary for modification, maintenance, and community contribution.

---

## PO-04 Modern User Interface

The platform shall utilize LVGL to provide a responsive, intuitive, and maintainable user interface.

---

## PO-05 Hardware Expandability

The electronics architecture shall support additional controls, displays, indicators, and expansion modules without major redesign.

---

# 5. Product Goals

PowerStand shall support:

- Locomotive acquisition (short and long addresses)
- Speed control (combined target speed control with throttle and brake inputs)
- Direction control (Forward, Nutual and Backward)
- Function control (CV numbers are mapped to actual customized locomotive function)
- Consist operation (2 major operation modes, physical control and digital control using rotary encoder as single input for speed control)
- Multiple-unit operation (quick switch amoung aquired locomotives)
- Turnout control
- Route control
- Network discovery
- Diagnostics and testing
- Local configuration storage (SPI flash FAT file system support. SD card might also be supported but that relies on the card reader integration from hardware)

---

# 6. User Experience Goals

The operator should be able to:

- Operate a locomotive while primarily interacting with physical controls
- Determine system status at a glance
- Change operating direction quickly
- Activate locomotive functions without navigating multiple screens
- Operate both Chinese and North American locomotives, optional Europe locomotives.
- Transition easily between locomotives and consists

The user interface shall prioritize:

1. Readability
2. Reliability
3. Responsiveness
4. Simplicity
5. Consistency

---

# 7. Functional Scope

## Included in Version 1

### Hardware

- ESP32 controller
- 7-inch 720×480 TFT touch display
- SPI flash storage
- SD card support (optional)
- Rotary encoder with push switch
- Physical throttle control
- Physical brake control
- Physical reverser control
- Physical horn control
- Physical headlight control
- Emergency stop swtich
- Status indicators
- WiFi networking

### Software

- LVGL graphical user interface
- DCC-EX client implementation
- WiThrottle client implementation (optional)
- Roster management
- Configuration storage
- Firmware settings
- Diagnostics
- Event logging

---

## Out of Scope for V1

The following items are considered future enhancements:

- Battery operation
- Bluetooth connectivity
- Radio throttling
- Integrated sound generation
- Cloud synchronization
- Automatic train operation
- Mobile application support
- Multi-user synchronization

---

# 8. System Concept

PowerStand consists of three primary subsystems:

## Human Interface Layer

Responsible for:

- Physical controls
- Operator input
- Touchscreen interaction
- Status indicators

---

## Control Core

Responsible for:

- Locomotive state management
- Protocol abstraction
- Roster management
- Configuration management

---

## Communication Layer

Responsible for:

- DCC-EX communications
- WiThrottle communications
- Network discovery
- Reconnection logic

---

# 9. Design Philosophy

## Physical-First Operation

Physical controls should always provide the most efficient way to operate a locomotive.

Touchscreen interaction supplements physical controls rather than replacing them.

---

## Protocol Independence

PowerStand control logic should not depend on a specific railroad command protocol.

All protocols should interact through a common control abstraction layer.

---

## Community Use and Sharing

PowerStand is intended to be openly accessible to hobbyists, educators, clubs, researchers, and model railroad enthusiasts.

All project artifacts, including firmware, documentation, electrical designs, mechanical designs, CAD models, graphics, and associated assets shall be publicly available through the project repository.

Users are encouraged to:

- Study the design
- Build personal copies
- Modify the project
- Share improvements
- Contribute enhancements
- Create derivative non-commercial works

Commercial use of PowerStand or derivative works is prohibited without written permission from the project owner.

This includes, but is not limited to:

- Selling complete systems
- Selling kits
- Selling PCBs
- Selling printed parts
- Selling derivative products
- Commercial redistribution of project assets

All redistributed or modified versions shall provide attribution and remain licensed under CC BY-NC-SA 4.0.

---

## Modular Design

Mechanical, electrical, and software components shall remain as independent as practical.

---

## Maintainability

Code and documentation shall be structured to support long-term maintenance and future contributors.

---

# 10. Success Criteria

The project shall be considered successful when:

## SC-01

A locomotive can be acquired and controlled through DCC-EX.

## SC-02

A locomotive can be acquired and controlled through WiThrottle.

## SC-03

Throttle response meets operator expectations.

## SC-04

Physical controls provide a superior operating experience compared to touchscreen-only operation.

## SC-05

The system can be reproduced using commercially available hardware.

## SC-06

The project can be built and maintained directly from documentation and source code stored in GitHub.

---

# 11. Initial Hardware Concept

The initial PowerStand control stand is expected to include:

| Control        | Description                             |
|----------------|-----------------------------------------|
| Throttle       | Multi-position locomotive-style lever   |
| Brake          | Multi-position locomotive-style lever   |
| Reverser       | Forward / Neutral / Reverse switch      |
| Horn           | Momentary locomotive horn control       |
| Headlight      | Dedicated lighting control              |
| Encoder        | UI navigation and configuration         |
| Touchscreen    | Primary information display             |
| E-Stop         | Immediate emergency stop                |
| Status Display | Auxiliary always-on information display |

---

# 12. Primary Technology Stack

## Hardware

- ESP32
- TFT Touch Display
- LVGL
- SD Card Storage
- SPI Flash File System
- I²C I/O Expansion Devices

## Software

- ESP-IDF
- LVGL
- FreeRTOS
- TCP/IP Networking
- mDNS Discovery

## Development

- GitHub
- GitHub Issues
- GitHub Projects
- ESP-IDF

---

# 13. Repository Structure

```text
PowerStand/
│
├── docs/
│   ├── charter/
│   ├── requirements/
│   ├── architecture/
│   ├── ui/
│   ├── hardware/
│   ├── protocol/
│   └── testing/
│
├── firmware/
│
├── hardware/
│   ├── enclosure/
│   ├── control-panel/
│   ├── throttle/
│   ├── brake/
│   └── electronics/
│
├── assets/
│
├── tools/
│
└── README.md
```

---

# 14. Planned Documentation

The following documents shall be developed after approval of this charter.

| ID      | Document                            |
|---------|-------------------------------------|
| SRD-001 | System Requirements Specification   |
| HDS-001 | Human Device Specification          |
| SAD-001 | System Architecture Document        |
| HWD-001 | Hardware Design Specification       |
| UIS-001 | LVGL User Interface Specification   |
| PRD-001 | Protocol Requirements Specification |
| TST-001 | Master Test Plan                    |

---

# 15. Project Motto

> Real Controls. Modern Protocols. Open Architecture.

---

**End of Document**