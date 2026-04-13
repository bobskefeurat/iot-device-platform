# IoT Device Platform

Incremental system project for an IoT device platform with:

- an ESP32 device firmware
- a FastAPI backend
- SQLite persistence through SQLAlchemy
- backend tests and CI
- project documentation and architecture diagrams

The backend is the central hub. Devices register through HTTP/JSON, send heartbeats, and the backend exposes a measurement ingest API.

## Current Implemented Scope

The repository currently contains:

- a FastAPI backend in `backend/`
- SQLite-backed persistence in `backend/data/devices.db`
- device registration with component reconciliation
- heartbeat ingest and derived `ONLINE` / `OFFLINE` device status
- backend measurement ingest with persisted measurement history
- automated backend tests in `tests/`
- a GitHub Actions workflow for backend tests
- Docker support for the backend
- ESP32 firmware source in `device/esp32/`
- PlantUML-based system, backend, and device diagrams in `development_documents/diagrams/`

Current firmware capabilities include:

- Wi-Fi connection and reconnection
- device ID generation from Wi-Fi MAC address
- device registration client logic
- heartbeat task
- serial command menu
- local live readings from the moisture sensor
- moisture calibration with values stored in NVS

Current limitation:

- the backend measurement endpoint exists, but the ESP32 firmware does not yet post measurements to it

## Repository Layout

```text
backend/                         FastAPI app, database setup, schemas, models
device/esp32/                    ESP32 firmware source (PlatformIO / ESP-IDF)
tests/                           Backend API tests
development_documents/diagrams/  PlantUML diagrams for system, backend, and device
scripts/                         Local development helpers and tmux/session scripts
.github/workflows/               Backend CI workflow
```

## Run The Backend Locally

1. Create and activate a virtual environment.
2. Install runtime dependencies.
3. Start the API server.

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn backend.main:app --reload
```

The API will be available at `http://127.0.0.1:8000`.

Local SQLite data is stored in:

- `backend/data/devices.db`

Note:

- the project currently creates the SQLite schema with `Base.metadata.create_all(...)`
- there is no migration setup yet, so local schema changes may require recreating `backend/data/devices.db`

## Run With Docker

```bash
docker build -t iot-device-platform .
docker run --rm -p 8000:8000 iot-device-platform
```

## API Summary

Current backend endpoints:

- `GET /` returns a basic startup message
- `GET /health` returns backend health status
- `GET /devices` lists registered devices
- `GET /devices/{id}` returns one device with status and components
- `POST /devices` registers or updates a device and reconciles its components
- `POST /devices/{id}/heartbeat` updates `last_seen`
- `POST /devices/{id}/measurements` stores a measurement for an existing component
- `DELETE /devices/{id}` deletes a device and its components

Example device registration:

```bash
curl -X POST http://127.0.0.1:8000/devices \
  -H "Content-Type: application/json" \
  -d '{
    "id": "AA:BB:CC:DD:EE:FF",
    "name": "demo-device",
    "components": [
      {
        "local_id": "moisture_sensor_1",
        "model_name": "Capacitive Soil Moisture Sensor V2.0.0",
        "component_type": "sensor"
      }
    ]
  }'
```

Example measurement submission:

```bash
curl -X POST http://127.0.0.1:8000/devices/AA:BB:CC:DD:EE:FF/measurements \
  -H "Content-Type: application/json" \
  -d '{
    "component_local_id": "moisture_sensor_1",
    "mean_adc": 1234,
    "moisture_percent": 56
  }'
```

## Run Tests

Install test dependencies and run the backend test suite:

```bash
pip install -r requirements.txt
pip install pytest httpx
pytest -q
```

The tests use an isolated SQLite database under `tests/`.

## ESP32 Firmware

The ESP32 project expects a local config file that is not committed to git.

Use `device/esp32/include/local_config.example.h` as the template for your local config and set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `BACKEND_URL`

`BACKEND_URL` is expected to point to the device registration base path, for example:

- `http://192.168.1.100:8000/devices`

Useful PlatformIO commands:

```bash
cd device/esp32
pio run
pio run -t upload
pio device monitor
```

## Project Documentation

PlantUML diagrams are organized by area:

- `development_documents/diagrams/system/`
- `development_documents/diagrams/backend/`
- `development_documents/diagrams/device/`

These diagrams currently cover:

- system overview
- device registration and heartbeat flows
- backend domain model and ERD
- backend API map
- backend measurement flow
- backend offline detection flow
- ESP32 startup, calibration, and measurement flows

## Notes

- Device status is derived from the latest heartbeat timestamp.
- Measurements are stored in the backend database as historical records.
- Firmware and backend integration is still incremental; backend API examples in this README reflect the current backend contract.
