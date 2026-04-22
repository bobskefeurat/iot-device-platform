# ESP32 Structure

This directory is organized by responsibility instead of keeping all modules flat in `src/` and `include/`.

## Layout

- `src/app` and `include/app`
  - Entry point and runtime orchestration.
  - `main.c` handles bootstrapping and top-level startup flow.
  - `device_runtime.c` coordinates tasks for menu, calibration, and live readings.
- `src/device` and `include/device`
  - Device identity and component metadata.
- `src/drivers` and `include/drivers`
  - Low-level hardware and console access.
  - Keeps raw device interaction separate from higher-level flows.
- `src/features/moisture` and `include/features/moisture`
  - Moisture-specific feature logic such as calibration.
- `src/network` and `include/network`
  - Network-related modules.
  - `wifi_manager` handles Wi-Fi connectivity.
  - `http_client` provides generic HTTP transport helpers.
  - `backend/` contains backend-specific API client and payload code.
- `src/ui` and `include/ui`
  - Serial menu presentation and command handling.
- `include/config`
  - Local configuration headers such as Wi-Fi credentials and backend URL.
