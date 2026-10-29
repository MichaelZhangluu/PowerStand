# STD-001
# PowerStand C Coding Standard

## Document Information

| Field | Value |
|---------|---------|
| Document ID | STD-001 |
| Title | PowerStand C Coding Standard |
| Project | PowerStand |
| Language | C17 |
| Platform | ESP-IDF 6.1 |
| Status | Draft |

---

# 1. Purpose

This standard defines mandatory coding practices for all PowerStand firmware components.

The objectives are:

- Improve readability
- Improve maintainability
- Improve security
- Reduce defect density
- Simplify code reviews
- Simplify debugging
- Promote deterministic behavior

This standard applies to:

- Application code
- Drivers
- Middleware
- Hardware abstraction layers
- Unit tests

---

# 2. General Philosophy

PowerStand prioritizes:

1. Correctness over brevity.
2. Readability over cleverness.
3. Deterministic behavior over optimization.
4. Explicit behavior over implicit behavior.
5. Maintainability over convenience.

Code shall be understandable by a reviewer without requiring external context.

---

# 3. Language Standard

Firmware shall be written in:

- C17

Compiler extensions shall be avoided unless required by ESP-IDF.

---

# 4. File Organization

Each module shall contain:

```text
module/
├── CMakeLists.txt
├── include/
│   └── module.h
└── module.c
```

Public interfaces belong in:

```text
include/
```

Private definitions belong in:

```text
module.c
```

---

# 5. Naming Conventions

## Functions

```c
module_action_object()
```

## Structures

```c
module_type_t
```

## Handles

```c
module_handle_t
```

## Enumerations

```c
module_enum_t
```

## Macros

```c
UPPER_CASE
```

---

# 6. Function Structure

## Rule

All local variables shall be declared at the beginning of the function.

```c
esp_err_t status;
uint8_t value;
uint32_t index;

status = ESP_OK;
```

Declarations inside execution blocks are prohibited.

---

# 7. Error Handling

## Status Driven Design

Every function shall use status-driven error handling.

```c
esp_err_t status;

status = ESP_OK;

if (handle == NULL)
{
    status = ESP_ERR_INVALID_ARG;
}

if (status == ESP_OK)
{
    status = device_read();
}

return status;
```

## Single Return Rule

Functions shall have a single return statement whenever practical.

## Prohibited Macros In Reusable Drivers

```c
ESP_ERROR_CHECK()
ESP_RETURN_ON_ERROR()
ESP_RETURN_ON_FALSE()
```

Drivers shall return errors to callers.

---

# 8. Memory Management

- Heap allocation shall be minimized.
- Static allocation is preferred.
- calloc() is preferred over malloc().
- All resources shall be released in failure paths.

---

# 9. Thread Safety

- Thread safety shall be evaluated per module.
- Mutexes shall not be added preemptively.
- Ownership models are preferred.
- Shared resources may use SemaphoreHandle_t when justified.

---

# 10. Type Usage

Use explicit-width integer types:

```c
uint8_t
uint16_t
uint32_t
uint64_t
int8_t
int16_t
int32_t
int64_t
```

Avoid:

```c
short
long
unsigned
```

---

# 11. Pointer Validation

All public APIs shall validate pointer arguments for NULL.

---

# 12. Magic Numbers

Magic numbers are prohibited.

Use:

```c
#define
```

or

```c
enum
```

instead.

---

# 13. Logging

Use ESP_LOGE(), ESP_LOGW(), ESP_LOGI(), and ESP_LOGD() appropriately.

Logs shall contain actionable information.

---

# 14. Comments

Comments shall explain:

- Why
- Assumptions
- Constraints

Comments shall not merely restate the code.

---

# 15. Security Requirements

All external inputs shall be treated as untrusted.

Required:

- Bounds checking
- Range validation
- NULL validation
- Integer overflow awareness

No driver shall assume valid caller inputs.

---

# 16. Ownership Rules

Ownership transfer shall always be explicitly documented.

If a function allocates memory, creates handles, acquires locks, or acquires hardware resources, ownership and release responsibility must be documented in the API comments.

---

# 17. Static Analysis Compatibility

The codebase should remain compatible with:

- MISRA C
- CERT C
- Cppcheck
- Clang-Tidy

without major refactoring.

---

# 18. Exceptions

Exceptions to this standard require documented justification within the source file.
