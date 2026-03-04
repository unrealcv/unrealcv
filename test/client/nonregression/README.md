# Client non-regression tests

This folder isolates migration-hardening tests for the Python client stack.

## Modules

- `test_decoder_nonregression.py`: `MsgDecoder` behavior locks (shape, mode routing, invalid bytes).
- `test_api_nonregression.py`: `UnrealCv_API` command construction and decode/cache contracts.
- `test_transport_nonregression.py`: `Client`/`SocketMessage` framing, reconnect, queue, and lifecycle invariants.

These tests are intentionally explicit and fixture-driven to preserve current behavior during refactors (for example, transport stack migration).
