# cbuf — MISRA C:2012 Circular Buffer Library

[![CI](https://github.com/YOUR_USERNAME/cbuf-lib/actions/workflows/ci.yml/badge.svg)](https://github.com/YOUR_USERNAME/cbuf-lib/actions/workflows/ci.yml)
[![MISRA C:2012](https://img.shields.io/badge/MISRA-C%3A2012-blue)](https://www.misra.org.uk/)
[![C Standard](https://img.shields.io/badge/C-C99-brightgreen)](https://en.wikipedia.org/wiki/C99)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A **MISRA C:2012 compliant** circular buffer (ring buffer) library for bare-metal and
safety-critical embedded systems. Designed with zero dynamic memory allocation, full
null-pointer protection, and 100% unit test coverage via the Unity framework.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Building](#building)
- [Running Tests](#running-tests)
- [Static Analysis](#static-analysis)
- [CI/CD Pipeline](#cicd-pipeline)
- [API Reference](#api-reference)
- [MISRA C:2012 Compliance](#misra-c2012-compliance)
- [Design Decisions](#design-decisions)

---

## Overview

A circular buffer (ring buffer) is a fundamental data structure in embedded systems.
It decouples a **producer** and a **consumer** that operate at different speeds — for
example, a UART receive ISR writing bytes into the buffer while an application task
drains it at its own pace.

This library is built to the standards expected in professional embedded teams:

- No `malloc` or `free` — all state lives in a caller-owned struct
- No global state — multiple independent instances are fully supported
- MISRA C:2012 compliance enforced by automated static analysis on every commit
- Full unit test suite with boundary and wrap-around coverage

---

## Features

| Feature | Detail |
|---|---|
| Standards compliance | MISRA C:2012, C99 |
| Memory model | Zero dynamic allocation — no heap usage |
| Null safety | Every API function validates pointer arguments |
| Capacity | Configurable via `CBUF_MAX_SIZE` (default: 256 bytes) |
| Test framework | Unity (vendored, no external install required) |
| Build system | CMake 3.20+ with out-of-source builds |
| Static analysis | cppcheck with MISRA addon, clean with `--error-exitcode=1` |
| CI | GitHub Actions — build, test, and analysis on every push |

---

## Project Structure

```
cbuf-lib/
├── .github/
│   └── workflows/
│       └── ci.yml              # GitHub Actions pipeline
├── include/
│   └── cbuf.h                  # Public API — only file callers include
├── src/
│   └── cbuf.c                  # Implementation
├── tests/
│   ├── CMakeLists.txt
│   └── test_cbuf.c             # Unity test suite
├── vendor/
│   └── unity/                  # Unity test framework (vendored)
│       ├── unity.c
│       ├── unity.h
│       └── unity_internals.h
├── CMakeLists.txt              # Root build definition
├── cppcheck-suppressions.txt   # Documented analysis suppressions
├── Makefile                    # Convenience targets
├── .gitignore
└── README.md
```

---

## Requirements

| Tool | Version | Purpose |
|---|---|---|
| `gcc` | 11+ | Host compilation and testing |
| `cmake` | 3.20+ | Build system |
| `make` | any | Convenience targets |
| `cppcheck` | 2.x | MISRA C static analysis |
| `arm-none-eabi-gcc` | 12+ | Cross-compile verification (optional) |

Install on Ubuntu / WSL2:

```bash
sudo apt update && sudo apt install -y gcc cmake make cppcheck
```

---

## Building

This project uses out-of-source builds. All build artifacts go into `build/` which
is excluded from source control.

```bash
# Configure (Debug build — includes test binary)
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel
```

Or using the Makefile shortcut:

```bash
make test
```

---

## Running Tests

```bash
# After building
cd build && ctest --output-on-failure
```

Expected output:

```
Test project /home/user/cbuf-lib/build
    Start 1: cbuf_unit_tests
1/1 Test #1: cbuf_unit_tests ..................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1
```

### Test Coverage

The test suite covers:

| Scenario | Test |
|---|---|
| Initialisation with valid pointer | `test_cbuf_init_withValidPointer_returnsOk` |
| Null pointer rejection | `test_cbuf_init_withNull_returnsErrNull` |
| Push to full buffer | `test_cbuf_push_whenFull_returnsErrFull` |
| Pop from empty buffer | `test_cbuf_pop_emptyBuffer_returnsErrEmpty` |
| FIFO ordering preserved | `test_cbuf_pushPop_multipleBytes_maintainsFifoOrder` |
| **Index wrap-around** | `test_cbuf_wrapAround_indicesWrapCorrectly` |
| Peek does not consume data | `test_cbuf_peek_doesNotConsumeData` |
| Flush resets all state | `test_cbuf_flush_clearsBuffer` |

> **The wrap-around test is the critical one.** It fills the buffer completely, drains
> half, refills the freed space, then drains everything — forcing both `head` and `tail`
> indices to cross the `CBUF_MAX_SIZE` boundary. This is where broken ring buffer
> implementations fail.

---

## Static Analysis

MISRA C:2012 compliance is verified using `cppcheck` with the MISRA addon.

```bash
# Run analysis
make analyze

# Or directly
cppcheck \
  --enable=all \
  --disable=information \
  --std=c99 \
  --addon=misra \
  --suppressions-list=cppcheck-suppressions.txt \
  --error-exitcode=1 \
  -I include/ \
  src/cbuf.c
```

A clean run produces no output and exits with code `0`. Any MISRA violation causes
exit code `1`, which fails the CI pipeline.

### Suppressions

All suppressions are documented in `cppcheck-suppressions.txt` with explicit
architectural justification. No violation is suppressed without a stated reason.

| Suppression | Reason |
|---|---|
| `unusedFunction` | Public API functions are called by external translation units (test suite, application). Not called within the library itself by design. |
| `misra-c2012-8.7` | All functions have external linkage by design — this is a public library. cppcheck analyses `cbuf.c` in isolation and cannot verify cross-TU usage. |
| `missingIncludeSystem` | System headers (`stdint.h`, `stdbool.h`) are available on the target toolchain but not on the analysis host path. |

---

## CI/CD Pipeline

Every push and pull request to `main` triggers three parallel jobs on GitHub Actions:

```
Push / PR
    │
    ├── build-and-test ─────── cmake configure → build → ctest
    │                                                        │
    ├── static-analysis ─────── cppcheck MISRA C:2012       ├── All must pass
    │                                                        │   before merge
    └── warnings-clean ──────── gcc -Wall -Wextra -Werror ──┘
```

**Branch protection is enforced** — pull requests to `main` cannot be merged unless
all three jobs pass. This guarantees the `main` branch is always in a buildable,
tested, analysis-clean state.

To run the pipeline locally without pushing (requires Docker):

```bash
# Install act
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Run full pipeline locally
act push

# Run a specific job
act -j static-analysis
```

---

## API Reference

Include the library in your project:

```c
#include "cbuf.h"
```

### Types

```c
/* Control structure — declare on the stack or as a static local.
 * Never allocate with malloc. Never access fields directly. */
typedef struct { ... } cbuf_t;

/* Return codes */
typedef enum {
    CBUF_OK         = 0,
    CBUF_ERR_FULL   = 1,
    CBUF_ERR_EMPTY  = 2,
    CBUF_ERR_NULL   = 3
} cbuf_status_t;
```

### Functions

```c
/* Initialise — must be called before any other function */
cbuf_status_t cbuf_init(cbuf_t *cb);

/* Write one byte. Returns CBUF_ERR_FULL if no space available. */
cbuf_status_t cbuf_push(cbuf_t *cb, uint8_t byte);

/* Read and remove one byte. Returns CBUF_ERR_EMPTY if buffer is empty. */
cbuf_status_t cbuf_pop(cbuf_t *cb, uint8_t *byte);

/* Read without removing. Does not advance the tail index. */
cbuf_status_t cbuf_peek(const cbuf_t *cb, uint8_t *byte);

/* Number of bytes currently held in the buffer. */
uint16_t cbuf_count(const cbuf_t *cb);

/* True if no bytes available to read. */
bool cbuf_is_empty(const cbuf_t *cb);

/* True if no space available to write. */
bool cbuf_is_full(const cbuf_t *cb);

/* Discard all contents. Equivalent to calling cbuf_init again. */
cbuf_status_t cbuf_flush(cbuf_t *cb);
```

### Usage Example

```c
#include "cbuf.h"

int main(void)
{
    cbuf_t rx_buffer;
    uint8_t byte = 0U;

    /* Always initialise before use */
    (void)cbuf_init(&rx_buffer);

    /* Producer — e.g. called from a UART ISR */
    (void)cbuf_push(&rx_buffer, 0x41U);
    (void)cbuf_push(&rx_buffer, 0x42U);
    (void)cbuf_push(&rx_buffer, 0x43U);

    /* Consumer — e.g. called from application task */
    while (!cbuf_is_empty(&rx_buffer))
    {
        if (cbuf_pop(&rx_buffer, &byte) == CBUF_OK)
        {
            /* process byte */
        }
    }

    return 0;
}
```

---

## MISRA C:2012 Compliance

The following rules are actively enforced by cppcheck on every commit:

| Rule | Description | How enforced in this codebase |
|---|---|---|
| 4.6 | Use `stdint.h` typedefs | All types are `uint8_t`, `uint16_t`, `bool` — no raw `int` |
| 7.2 | Integer constants use `U`/`L` suffix | All literals written as `0U`, `256U`, etc. |
| 8.4 | Prototype visible at definition | All functions declared in `cbuf.h` before defined in `cbuf.c` |
| 14.4 | `if` condition is essentially Boolean | All pointer checks written as `ptr != NULL` |
| 15.5 | Single point of exit per function | Every function has exactly one `return` statement |
| 21.3 | No dynamic memory allocation | No `malloc`, `calloc`, `realloc`, or `free` anywhere |

---

## Design Decisions

**Why a fixed-size buffer?**
Dynamic allocation is banned in safety-critical code (MISRA Rule 21.3, CERT C MEM35).
A fixed-size buffer with `CBUF_MAX_SIZE` means worst-case memory usage is known at
compile time — a hard requirement in systems with no MMU.

**Why a `count` field instead of computing `head == tail`?**
The classic ring buffer uses `head == tail` to mean empty and `(head + 1) % size == tail`
to mean full — which wastes one slot and makes the logic harder to audit. An explicit
`count` field makes every state transition readable and MISRA-auditable without
sacrificing performance.

**Why no interrupt safety (critical sections)?**
This library is intentionally scope-limited to single-producer / single-consumer usage
without concurrent access protection. Adding `__disable_irq()` / `__enable_irq()` calls
would couple the library to a specific HAL, breaking portability and testability on
host. The caller is responsible for wrapping push/pop in critical sections when used
from an ISR context.

---

## Toolchain Versions Used

```
gcc      11.4.0
cmake    3.22.1
cppcheck 2.10
```

Reproducible builds are guaranteed within the same major version of each tool.