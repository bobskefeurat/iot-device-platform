from backend.models import Component, Device
from tests.data import (
    TEST_COMPONENTS,
    TEST_DEVICE_ID,
    TEST_DEVICE_NAME,
    TEST_SECOND_COMPONENTS,
    TEST_SECOND_DEVICE_ID,
    TEST_SECOND_DEVICE_NAME,
    TEST_UNKNOWN_DEVICE_ID,
)
from tests.helpers import assert_components_match, create_device
from tests.support import TestingSessionLocal, client

def test_get_health():
    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status": "online"}

def test_get_devices():
    create_device()

    response = client.get("/devices")

    assert response.status_code == 200

    data = response.json()

    assert isinstance(data, list)
    assert len(data) == 1

    first_device = data[0]

    assert first_device["id"] == TEST_DEVICE_ID
    assert first_device["name"] == TEST_DEVICE_NAME
    assert_components_match(first_device["components"], TEST_COMPONENTS)

def test_get_device():
    create_device()

    response = client.get("/devices/" + TEST_DEVICE_ID)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data["name"] == TEST_DEVICE_NAME
    assert_components_match(data["components"], TEST_COMPONENTS)

def test_get_unknown_device():
    response = client.get("/devices/" + TEST_UNKNOWN_DEVICE_ID)

    assert response.status_code == 404

def test_delete_device():
    create_device()

    response = client.delete("/devices/" + TEST_DEVICE_ID)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID

def test_delete_unknown_device():
    response = client.delete("/devices/" + TEST_UNKNOWN_DEVICE_ID)

    assert response.status_code == 404
def test_delete_device_removes_components():
    create_device()

    response = client.delete("/devices/" + TEST_DEVICE_ID)

    assert response.status_code == 200

    db = TestingSessionLocal()
    try:
        assert db.query(Device).filter(Device.id == TEST_DEVICE_ID).first() is None
        assert db.query(Component).filter(Component.device_id == TEST_DEVICE_ID).count() == 0
    finally:
        db.close()

def test_get_devices_returns_multiple_devices():
    create_device()

    second_response = create_device(
        device_id=TEST_SECOND_DEVICE_ID,
        name=TEST_SECOND_DEVICE_NAME,
        components=TEST_SECOND_COMPONENTS,
    )

    assert second_response.status_code == 200

    response = client.get("/devices")

    assert response.status_code == 200

    data = response.json()
    devices_by_id = {device["id"]: device for device in data}

    assert len(data) == 2
    assert devices_by_id[TEST_DEVICE_ID]["name"] == TEST_DEVICE_NAME
    assert_components_match(devices_by_id[TEST_DEVICE_ID]["components"], TEST_COMPONENTS)
    assert devices_by_id[TEST_SECOND_DEVICE_ID]["name"] == TEST_SECOND_DEVICE_NAME
    assert_components_match(devices_by_id[TEST_SECOND_DEVICE_ID]["components"], TEST_SECOND_COMPONENTS)
