# I2C LCD HD44780 Driver Design Decisions

**Project:** PowerStand  
**Component:** i2c_lcd_hd44780  
**Architecture Version:** 1.0  
**Status:** Approved

---

# Overview

This document captures the architectural and API design decisions for the `i2c_lcd_hd44780` component used within the PowerStand project.

The driver provides a low-level interface to an HD44780-compatible LCD controller connected through a PCF8574 I/O expander backpack over I2C.

The component is intentionally designed as a controller driver rather than a display module abstraction or user interface library.

---

# Design Goals

## Primary Goals

- Reusable HD44780 controller driver.
- ESP-IDF native implementation.
- C language implementation.
- Consistent architecture with existing PowerStand components.
- Simple and deterministic behavior.
- No busy-flag reads.
- No dynamic display geometry assumptions.
- Fixed hardware design targeting the PowerStand LCD module and backpack.

## Non-Goals

The driver shall not:

- Provide text formatting.
- Provide `printf()` style interfaces.
- Manage menus.
- Manage display layouts.
- Perform row/column coordinate translation.
- Understand application-specific custom characters.
- Support arbitrary backpack wiring configurations.
- Support arbitrary LCD module geometries.

---

# Dependencies

## Direct Dependencies

### i2c_io_expander_pcf8574

The HD44780 driver depends on the PCF8574 I/O expander driver for all LCD bus transactions.

Responsibilities:

- RS signal control
- EN signal control
- Backlight control
- LCD data bus control (D4-D7)

### i2c_master_device

The PCF8574 driver depends on the `i2c_master_device` component.

Responsibilities:

- Device registration
- Device configuration
- I2C read/write transactions
- Error handling
- Bus management

### ESP-IDF I2C Driver

The `i2c_master_device` component is built upon the ESP-IDF I2C driver.

Responsibilities:

- Hardware abstraction
- I2C peripheral control
- Transaction execution

## Dependency Chain

```text
Application / UI Layer
    ↓
LCD Module (16x2 or 20x4 etc.)
    ↓
i2c_lcd_hd44780
    ↓
i2c_io_expander_pcf8574
    ↓
i2c_master_device
    ↓
ESP-IDF I2C Driver
```

---

## Driver Ownership Model

### Decision

The LCD driver shall create and own the PCF8574 driver instance internally.

The PCF8574 handle shall not be exposed through the public API.

### Rationale

The target hardware is a physically integrated LCD module and backpack assembly.

Users interact with a single LCD module rather than separately managing an HD44780 controller and a PCF8574 I/O expander.

The underlying PCF8574 implementation shall remain an internal implementation detail.

---

# Hardware Assumptions

## Decision

This driver is intentionally implemented for a specific LCD module and PCF8574 backpack combination used by the PowerStand project.

The driver is not intended to support arbitrary backpack hardware.

Hardware example:

https://www.amazon.com/dp/B086VVT4NH, 2004A LCD module with i2c interface adapter

https://www.amazon.com/dp/B01MXGXPKU, i2c interface module


## Rationale

The PowerStand project uses a known and fixed hardware design.

---

# Backpack Pin Mapping

## Decision

Backpack pin assignments are fixed.

The mapping shall be implemented as private constants within the driver's source file.

No public configuration API shall be provided.

## Pin Assignment

```text
PCF8574      LCD

P0 ---------- RS
P1 ---------- RW
P2 ---------- EN
P3 ---------- Backlight

P4 ---------- D4
P5 ---------- D5
P6 ---------- D6
P7 ---------- D7
```

## Notes

- RW will not be used for read operations.
- RW shall remain driven low during normal driver operation.
- Busy-flag reads are not supported.

---

# Software Stack

```text
Application / UI Layer
    ↓
LCD Module
    ↓
i2c_lcd_hd44780
    ↓
i2c_io_expander_pcf8574
    ↓
i2c_master_device
```

Responsibilities are intentionally separated between layers.

---

# Driver Scope

## The Driver Controls

- HD44780 command transmission.
- HD44780 data transmission.
- HD44780 initialization.
- DDRAM addressing.
- CGRAM addressing.
- Display state management.
- Cursor state management.
- Blink state management.
- Backpack backlight control.
- Entry mode configuration.

## The Driver Does Not Control

- Screen layouts.
- Menu systems.
- Text wrapping.
- String formatting.
- Row/column positioning.
- User interface behavior.
- Application-specific custom characters.

---

# Display Geometry

## Decision

The driver shall not expose row/column APIs.

The HD44780 controller fundamentally understands DDRAM addresses, not display row and column coordinates.

Geometry handling belongs to a higher layer.

## Rationale

The HD44780 supports:

- 1-line mode
- 2-line mode

There is no native 4-line mode.

A 20x4 display module is implemented through LCD module-specific wiring and address mapping.

For example:

```text
Visible Row 0 -> DDRAM 0x00
Visible Row 1 -> DDRAM 0x40
Visible Row 2 -> DDRAM 0x14
Visible Row 3 -> DDRAM 0x54
```

These mappings are module-specific and do not belong in the HD44780 controller driver.

---

# DDRAM Addressing

## Decision

Expose DDRAM addressing directly.

```c
esp_err_t i2c_lcd_hd44780_set_ddram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address);
```

## Rationale

This API directly represents an HD44780 hardware capability.

It avoids introducing geometry assumptions into the driver.

---

# Text Output

## Decision

The driver shall provide single-byte data output only.

```c
esp_err_t i2c_lcd_hd44780_write_data(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t data);
```

## Not Included

```c
write_string()
puts()
printf()
snprintf()
```

## Rationale

These functions belong to a higher abstraction layer.

The HD44780 driver is responsible only for transmitting display data bytes.

---

## Entry Mode Support

### Decision

Entry Mode configuration shall be exposed publicly.

```c
typedef enum
{
    I2C_LCD_HD44780_ENTRY_INCREMENT = 0,
    I2C_LCD_HD44780_ENTRY_DECREMENT
} i2c_lcd_hd44780_entry_direction_t;

esp_err_t i2c_lcd_hd44780_set_entry_mode(
    i2c_lcd_hd44780_handle_t handle,
    i2c_lcd_hd44780_entry_direction_t direction,
    bool display_shift_enabled);
```

### Rationale

The Entry Mode Set instruction is a native HD44780 controller capability.

Controller capabilities belong in the controller layer.

Policy decisions remain the responsibility of higher layers.

---

## CGRAM Support

### Decision

Expose CGRAM addressing directly.

```c
esp_err_t i2c_lcd_hd44780_set_cgram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address);
```

### Rationale

CGRAM is a native HD44780 controller feature.

Character creation is implemented by:

1. Setting a CGRAM address.
2. Writing bitmap data using `write_data()`.

The driver shall expose the controller mechanism rather than a higher-level convenience abstraction.

Examples include:

- Battery icons
- Degree symbols
- WiFi indicators
- Arrows
- Progress bar segments

The semantic meaning of a CGRAM character remains an application responsibility.

---

# Busy Flag Support

## Decision

Busy-flag reads are not supported.

RW-based read operations shall not be implemented.

## Rationale

The LCD is connected through a PCF8574 backpack.

Reading the busy flag:

- Reading the flag through a 100KHz bus takes longer time than most of the command durations
- Adds complexity
- Requires bidirectional bus handling
- Provides minimal practical benefit

Fixed delays are sufficient and simpler.

---

# Timing Strategy

## Long Delay Commands

The following commands require long delays:

```text
0x01 Clear Display
0x02 Return Home
```

## Short Delay Commands

All other commands use the normal execution delay.

## Implementation

Timing management shall remain entirely internal to the driver.

No delay-related APIs shall be exposed.

---

# Display State Management

## Decision

The driver shall maintain internal display state.

Example internal state:

```c
typedef struct
{
    i2c_io_expander_pcf8574_handle_t io_expander_handle;

    bool display_enabled;
    bool cursor_enabled;
    bool blink_enabled;
    bool backlight_enabled;

} i2c_lcd_hd44780_t;
```

## Rationale

The HD44780 Display Control command combines:

```text
Display
Cursor
Blink
```

into a single register.

Maintaining state internally simplifies implementation and improves readability.

---

## Thread Safety

### Decision

The driver is not thread-safe.

### Requirement

Access to a driver instance shall be serialized by the owning task.

Example documentation:

```c
/**************************************************************
 * @note
 *
 * This driver is not thread-safe.
 *
 * Access to a driver instance shall be serialized
 * by the owning task.
 **************************************************************/
```

### Rationale

The underlying `i2c_io_expander_pcf8574` driver is also not thread-safe.

Maintaining a consistent threading model across the driver stack avoids unneceesary complexity.

---

# Backlight Control

## Decision

Backlight control shall be included in the driver.

```c
esp_err_t i2c_lcd_hd44780_set_backlight_enabed(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);
```

## Rationale

The backlight is part of the specific backpack hardware design.

Users of the component are expected to control it through the driver.

---

# Public API

## Lifecycle

```c
esp_err_t i2c_lcd_hd44780_create(
    const i2c_lcd_hd44780_config_t *config,
    i2c_lcd_hd44780_handle_t *handle);

esp_err_t i2c_lcd_hd44780_destroy(
    i2c_lcd_hd44780_handle_t handle);
```

## Screen Control

```c
esp_err_t i2c_lcd_hd44780_clear(
    i2c_lcd_hd44780_handle_t handle);

esp_err_t i2c_lcd_hd44780_home(
    i2c_lcd_hd44780_handle_t handle);
```

## Data Operations
```c
esp_err_t i2c_lcd_hd44780_write_data(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t data);

esp_err_t i2c_lcd_hd44780_set_ddram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address);

esp_err_t i2c_lcd_hd44780_set_entry_mode(
    i2c_lcd_hd44780_handle_t handle,
    i2c_lcd_hd44780_entry_direction_t direction,
    bool display_shift_enabled);
```

## Display Control
```c
esp_err_t i2c_lcd_hd44780_set_display_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

esp_err_t i2c_lcd_hd44780_set_cursor_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

esp_err_t i2c_lcd_hd44780_set_blink_enabled(
   i2c_lcd_hd44780_handle_t handle,
    bool enabled);
```

## Backlight Control

```c
esp_err_t i2c_lcd_hd44780_set_backlight_enabed(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);
```
## CGRAM Control
```c
esp_err_t i2c_lcd_hd44780_create_char(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t location,
    const uint8_t address);
```

---

# Internal APIs

The following helpers shall remain private:

```c
lcd_write_nibble()

lcd_write_byte()

lcd_write_command()

lcd_write_expander()
lcd_update_display_control()

lcd_pulse_enable()
```

No raw command interface shall be exposed publicly.

---

# Explicitly Rejected APIs
The following APIs shall not be included:

```c
write_string()

puts()

printf()

snprintf()

set_cursor(row, column)

send_command()

read_busy_flag()

read_data()

read_ddram()

read_cgram()

create_char()
```

## Rationale

These APIs:

- Add unnecessary abstraction leakage.
- Increase complexity.
- Belong to higher-level software layers.
- Are not required by the target hardware architecture

---

# Future LCD Module Layer

## future module/UI layer may be created above the driver.

Example APIs:

```c
lcd_module_set_cursor(row, column);

lcd_module_write_string(...);

lcd_module_printf(...);

lcd_*_module_draw_battery_icon(...);
```
Responsibilities may include:

- Display geometry management
- Row/column translation
- Text formatting - Custom character usage
- Application UI behavior

This layer shall build upon the lower-level `i2c_lcd_hd44780` driver without modifying the driver's responsibilities.

---
# Summary

The `i2c_lcd_hd44780` component is a low-level HD44780 controller driver designed specifically for the PowerStand hardware platform.

Key principles:

- Controller-focused API.
- Fixed hardware assumptions.
- No geometry knowledge.
- No string handling.
- No busy-flag reads.
- Internal state tracking.
- CGRAM support included.
- Backlight control included.
- Clean separation between controller, module, and application layers.