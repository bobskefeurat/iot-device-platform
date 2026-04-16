from datetime import datetime, timedelta, timezone

from backend.models import Device
from backend.services.devices import HEARTBEAT_TIMEOUT
from tests.data import TEST_DEVICE_ID, TEST_UNKNOWN_DEVICE_ID
from tests.helpers import create_device
from tests.support import TestingSessionLocal, client


def test_receive_heartbeat():
    create_device()

    response = client.post("/devices/" + TEST_DEVICE_ID + "/heartbeat")

    assert response.status_code == 204


def test_receive_heartbeat_unknown_device():
    response = client.post("/devices/" + TEST_UNKNOWN_DEVICE_ID + "/heartbeat")

    assert response.status_code == 404


def test_registered_device_is_online():
    create_device()

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    assert data["status"] == "ONLINE"


def test_heartbeat_keeps_device_online():
    create_device()

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    last_seen_first = data["last_seen"]

    client.post("/devices/" + TEST_DEVICE_ID + "/heartbeat")

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    last_seen_second = data["last_seen"]

    assert last_seen_first != last_seen_second
    assert data["status"] == "ONLINE"


def test_timed_out_device_is_offline():
    create_device()

    db = TestingSessionLocal()
    try:
        device = db.query(Device).filter(Device.id == TEST_DEVICE_ID).first()
        device.last_seen = datetime.now(timezone.utc) - HEARTBEAT_TIMEOUT - timedelta(seconds=1)
        db.commit()
    finally:
        db.close()

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    assert data["status"] == "OFFLINE"
