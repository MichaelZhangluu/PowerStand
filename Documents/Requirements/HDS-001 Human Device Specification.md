# HDS-001 Human Device Specification

| Field       | Value         |
|-------------|---------------|
| Project     | PowerStand    |
| Document ID | HDS-001       |
| Version     | 0.1 Draft     |
| Status      | Draft         |
| Author      | Michael Zhang |
| Date        | 2026-10-26    |

---

# 1. Purpose

This document defines the operator-facing controls, displays, indicators, and interaction methods for the PowerStand digital locomotive control stand.

The goal of PowerStand is to provide a realistic, tactile, and intuitive model railroad operating experience while maintaining compatibility with digital command systems such as DCC-EX and WiThrottle.

This specification focuses exclusively on human interaction and does not define implementation details.

---

# 2. Design Philosophy

PowerStand shall prioritize physical controls over touchscreen interaction.

The operator should be able to perform common locomotive operations without continuously observing the display.

The touchscreen shall act as an information display and advanced configuration interface rather than the primary driving control.

Design priorities:

1. Physical operation
2. Situational awareness
3. Readability
4. Responsiveness
5. Reliability
6. Expandability

---

# 3. Human Interface Overview

PowerStand shall provide the following operator interfaces:

| Interface                  | Purpose                            |
|----------------------------|------------------------------------|
| Throttle Lever             | Power control                      |
| Brake Lever                | Speed reduction and train handling |
| Reverser                   | Driving direction selection        |
| Headlight Switch           | Headlight control                  |
| Horn Control               | Audible signaling                  |
| Rotary Encoder             | Menu navigation, value input etc.  |
| Encoder Push Button        | Selection and confirmation         |
| Touchscreen                | Information and configuration      |
| Emergency Stop             | Immediate train stop               |
| Optional Auxiliary Display | Persistent status information      |

---

# 4. Operating Modes

The system shall support:

## Drive Mode

Primary operating mode.

Provides:

- Speed control
- Direction control
- Function control
- Locomotive status

---

## Consist Mode

Provides:

- Consist management
- Multiple-unit operation
- Lead locomotive assignment

---

## Yard Mode

Optimized for switching operations.

May provide:

- Fine speed control
- Speed limiting
- Simplified display layout

---

## Configuration Mode

Provides:

- System settings
- Network configuration
- Calibration
- Diagnostics

---

# 5. Throttle Lever

## Purpose

Primary speed command input.

---

## Physical Design

The throttle shall use a locomotive-inspired lever mechanism.

The preferred implementation is:

- 3D printed lever
- Mechanical detents
- Hall-effect position sensing

Alternative implementations may be supported.

---

## Operating Positions

Preferred positions: IDLE, N1-N8

Total positions: 9

---

## Behavior

Moving the throttle forward shall request increasing locomotive speed.

Returning to IDLE shall command the minimum speed state.

---

## Future Capability

Future firmware may support alternative notch mapping for:

- EMD locomotives
- GE locomotives
- Chinese electric locomotives
- Steam locomotives

without requiring hardware changes.

---

# 6. Brake Lever

## Purpose

Secondary train control input.

Provides speed moderation and enhanced operating realism.

---

## Physical Design

Locomotive-inspired lever.

Preferred implementation:

- Hall sensor
- Mechanical detents

---

## Operating Positions

Brake positions includes: RELEASE, MINIMUM, LIGHT, MEDIUM, FULL, EMERGENCY

---

## Expected Behavior

Brake application shall reduce effective locomotive speed.

Exact control algorithms shall be defined in firmware requirements.

---

# 7. Reverser

## Purpose

Direction selection.

---

## Physical Design

Three-position maintained switch: FORWARD, NEUTRAL, REVERSE

---

## Behavior

### Forward

Locomotive moves forward.

### Reverse

Locomotive moves reverse.

### Neutral

Locomotive movement disabled.

Hardware design shall consider to link Neutral position with throttle IDLE position that when reverser is at Neutral, throttle shall not be placed into non-IDEL, when throttle is non-IDEL, reverser shall not be able to change direction.

---

# 8. Horn Control

## Purpose

Locomotive audible signaling.

---

## Physical Design

Momentary operator control.

Examples:

- Paddle switch
- Lever switch
- Industrial momentary switch

---

## Control Functions

PowerStand shall support multiple horn mappings.

### North American Mode

Single horn output (F2)

---

### Chinese Locomotive Mode

Multiple horn outputs mapped to configurable function outputs.

---

# 9. Headlight Control

## Purpose

Headlight operation.

---

## Physical Design

Dedicated maintained switch.

Preferred: OFF, DIM, BRIGHT

---

## Display Feedback

Current headlight state shall be visible on the primary display.

---

# 10. Emergency Stop

## Purpose

Immediate speed command cancellation.

---

## Physical Design

Dedicated emergency stop button.

Requirements:

- Red
- Easily accessible
- Larger than surrounding controls

---

## Behavior

Activating Emergency Stop shall override all other controls.

Protocol-specific emergency stop commands shall be transmitted immediately.

---

# 11. Rotary Encoder

## Purpose

User interface navigation.

---

## Functions

### Rotate

- Scroll lists
- Modify values
- Adjust settings

### Press

- Select item
- Confirm operation

### Press and Hold

Reserved for future functionality.

---

## Non-Driving Requirement

The encoder shall not be required for routine train operation.

---

# 12. Touchscreen

## Purpose

Primary visual interface.

---

## Display

720 × 480 pixels

Landscape orientation

---

## Functions

- Locomotive selection
- Function controls
- Consist management
- Turnout control
- Route control
- Diagnostics
- Configuration

---

## Touch Requirement

All touchscreen operations shall remain operable without a keyboard.

---

# 13. Auxiliary Status Display

## Purpose

Persistent presentation of critical information independent of the primary UI.

---

## Candidate Hardware

Examples:

- 20x4 character LCD
- Small OLED display
- Industrial monochrome display

---

## Information Displayed

Example:

```text
CAB: QJ1485

SPD: 042

DIR: FWD

MU : LEAD
```

---

## Goal

Maintain operator awareness while other screens are active.

---

# 14. Indicators

## Software Indicators

Preferred implementation.

Provide status for:

- Connection state
- Direction
- Headlight state
- Current locomotive
- Consist status
- Command protocol

---

## Physical Indicators

May be provided for:

- Power
- Network
- Emergency stop status

---

# 15. Keyboard Strategy

Version 1 shall not require a physical keyboard.

The touchscreen keyboard shall provide all required text entry functions.

Future versions may support:

- USB keyboards
- Bluetooth keyboards
- Matrix keypads

---

# 16. Primary Operating Workflow

Typical operating sequence:

1. Power on
2. Connect to command system
3. Select locomotive
4. Set direction
5. Advance throttle
6. Operate horn and lights
7. Apply brake as needed
8. Stop locomotive
9. Release locomotive

Routine operation should require no more than occasional touchscreen interaction.

---

# 17. Acceptance Criteria

The specification shall be considered satisfied when:

- All primary train functions are available through physical controls.
- The user can operate a locomotive without continuous touch interaction.
- The user can determine locomotive status at a glance.
- The UI remains readable while standing at a typical layout operating position.
- All controls are fully documented and reproducible.

---

End of Document