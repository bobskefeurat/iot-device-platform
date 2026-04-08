# IoT Device Platform

Minimal system project for an IoT device platform with:

- a FastAPI backend
- a SQLite database for device state
- an ESP32 device client

The backend is the central hub. Devices register themselves, send heartbeats, and can be queried through HTTP/JSON.

## Current Status

This repository currently contains:

- a working backend API in `backend/`
- SQLite-based persistence in `backend/data/devices.db`
- automated backend tests in `tests/`
- Docker support for the backend
- ESP32 firmware source in `device/esp32/`

## Repository Layout

```text
backend/        FastAPI app, database setup, schemas, models
device/esp32/   ESP32 firmware source
tests/          Backend API tests
scripts/        Helper scripts for local development
```

## Run The Backend Locally

1. Create and activate a virtual environment.
2. Install dependencies.
3. Start the API server.

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn backend.main:app --reload
```

The API will be available at `http://127.0.0.1:8000`.

## Run With Docker

```bash
docker build -t iot-device-platform .
docker run --rm -p 8000:8000 iot-device-platform
```

## API Endpoints

- `GET /` returns a basic startup message
- `GET /health` returns backend health status
- `GET /devices` lists registered devices
- `GET /devices/{id}` returns one device
- `POST /devices` registers or updates a device
- `POST /devices/{id}/heartbeat` updates `last_seen`
- `DELETE /devices/{id}` deletes a device

Example device registration:

```bash
curl -X POST http://127.0.0.1:8000/devices \
  -H "Content-Type: application/json" \
  -d '{"id":"AA:BB:CC:DD:EE:FF","name":"demo-device"}'
```

## Run Tests

```bash
pytest -q
```

## ESP32 Configuration

The ESP32 project expects a local config file that is not committed to git.

Use `device/esp32/include/local_config.example.h` as the template for your own `device/esp32/include/local_config.h`, then set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `BACKEND_URL`

## Notes

- The backend stores device data in a local SQLite file.
- Device status is derived from the latest heartbeat timestamp.
- This project is still in an early incremental stage.
