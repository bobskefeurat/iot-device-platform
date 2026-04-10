from pathlib import Path
from datetime import datetime, timedelta, timezone
import pytest
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from fastapi.testclient import TestClient

from backend.main import app, HEARTBEAT_TIMEOUT
from backend.database import Base, get_db
from backend.models import Device, Component, Measurement

#-------------------CONFIG-------------------

TESTS_DIR = Path(__file__).resolve().parent
TEST_DATABASE_PATH = TESTS_DIR / "test_devices.db"
TEST_DATABASE_URL = f"sqlite:///{TEST_DATABASE_PATH}"

TEST_DEVICE_ID = "AA:BB:CC:DD:EE:FF"
TEST_DEVICE_NAME = "test-device"
TEST_SECOND_DEVICE_ID = "66:55:44:33:22:11"
TEST_SECOND_DEVICE_NAME = "second-test-device"
TEST_UNKNOWN_DEVICE_ID = "11:22:33:44:55:66"
TEST_COMPONENT_LOCAL_ID = "moisture_sensor_1"
TEST_UNKNOWN_COMPONENT_LOCAL_ID = "unknown_sensor_1"
TEST_COMPONENTS = [
    {
        "local_id": "pump_1",
        "model_name": "pump-model-a",
        "component_type": "actuator",
    },
    {
        "local_id": "moisture_sensor_1",
        "model_name": "sensor-model-a",
        "component_type": "sensor",
    },
]
TEST_SECOND_COMPONENTS = [
    {
        "local_id": "pump_1",
        "model_name": "pump-model-b",
        "component_type": "actuator",
    },
]
TEST_ADDED_COMPONENT = {
    "local_id": "water_level_sensor_1",
    "model_name": "level-sensor-model-a",
    "component_type": "sensor",
}


engine = create_engine(
    TEST_DATABASE_URL,
    connect_args= {"check_same_thread" : False}
)

TestingSessionLocal = sessionmaker(
    autocommit = False,
    autoflush = False,
    bind= engine
)

#-------------------APP-TEST-SETUP-------------------

def override_get_db():
    db = TestingSessionLocal()
    try:
        yield db
    finally:
        db.close()

app.dependency_overrides[get_db] = override_get_db
client = TestClient(app)

#-------------------RESET-DATABASE-PER-TEST-------------------

@pytest.fixture(autouse=True)
def reset_test_database():
    Base.metadata.drop_all(bind=engine)
    Base.metadata.create_all(bind=engine)

#-------------------TESTS-------------------

def test_get_health():

    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status" : "online"}

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


def test_register_device():

    response = create_device()

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data ["name"] == TEST_DEVICE_NAME
    assert_components_match(data["components"], TEST_COMPONENTS)

def test_get_device():

    create_device()

    response = client.get("/devices/"+ TEST_DEVICE_ID)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data["name"] == TEST_DEVICE_NAME
    assert_components_match(data["components"], TEST_COMPONENTS)

def test_get_unknown_device():

    response = client.get("/devices/"+ TEST_UNKNOWN_DEVICE_ID)

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

def test_receive_heartbeat():

    create_device()

    response = client.post("/devices/" + TEST_DEVICE_ID + "/heartbeat")

    assert response.status_code == 204

def test_receive_heartbeat_unknown_device():

    response = client.post("/devices/" + TEST_UNKNOWN_DEVICE_ID + "/heartbeat")

    assert response.status_code == 404

def test_receive_measurement():

    create_device()

    response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
            "moisture_percent": 56,
        },
    )

    assert response.status_code == 204

    db = TestingSessionLocal()
    try:
        measurement = db.query(Measurement).filter(
            Measurement.device_id == TEST_DEVICE_ID,
            Measurement.component_local_id == TEST_COMPONENT_LOCAL_ID,
        ).first()

        assert measurement is not None
        assert measurement.mean_adc == 1234
        assert measurement.moisture_percent == 56
    finally:
        db.close()

def test_receive_measurement_unknown_device():

    response = client.post(
        "/devices/" + TEST_UNKNOWN_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
            "moisture_percent": 56,
        },
    )

    assert response.status_code == 404

def test_receive_measurement_unknown_component():

    create_device()

    response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_UNKNOWN_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
            "moisture_percent": 56,
        },
    )

    assert response.status_code == 404

def test_receive_measurement_rejects_missing_field():

    create_device()

    response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
        },
    )

    assert response.status_code == 422

def test_receive_measurement_rejects_wrong_type():

    create_device()

    response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": "invalid",
            "moisture_percent": 56,
        },
    )

    assert response.status_code == 422

def test_receive_measurement_stores_multiple_measurements_for_same_component():

    create_device()

    first_response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
            "moisture_percent": 56,
        },
    )

    second_response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1300,
            "moisture_percent": 60,
        },
    )

    assert first_response.status_code == 204
    assert second_response.status_code == 204

    db = TestingSessionLocal()
    try:
        measurements = db.query(Measurement).filter(
            Measurement.device_id == TEST_DEVICE_ID,
            Measurement.component_local_id == TEST_COMPONENT_LOCAL_ID,
        ).all()

        assert len(measurements) == 2
    finally:
        db.close()

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

#-------------------COMPONENT-TESTS-------------------

def test_register_device_requires_components():
    
    response = client.post("/devices", 
        json = {
            "id" : TEST_DEVICE_ID,
            "name" : TEST_DEVICE_NAME,
        }
    )

    assert response.status_code == 422

def test_register_device_allows_empty_components():
    
    response = client.post("/devices", 
        json = {
            "id" : TEST_DEVICE_ID,
            "name" : TEST_DEVICE_NAME,
            "components" : []
        }
    )

    assert response.status_code == 200
    assert response.json()["components"] == []


def test_register_existing_device_updates_name():

    create_device()

    updated_name = "updated-device-name"

    response = create_device(name=updated_name)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data["name"] == updated_name
    assert_components_match(data["components"], TEST_COMPONENTS)


def test_register_existing_device_does_not_create_duplicate_device():
    
    create_device()

    response = create_device()

    assert response.status_code == 200

    db = TestingSessionLocal()
    try:
        assert db.query(Device).count() == 1
        assert db.query(Component).filter(Component.device_id == TEST_DEVICE_ID).count() == len(TEST_COMPONENTS)
    finally:
        db.close()

def test_register_device_updates_component_model_name():
    
    create_device()

    updated_components = [
        {
            "local_id": TEST_COMPONENTS[0]["local_id"],
            "model_name": "pump-model-b",
            "component_type": TEST_COMPONENTS[0]["component_type"],
        },
        TEST_COMPONENTS[1],
    ]

    response = create_device(components=updated_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], updated_components)

def test_register_device_keeps_existing_component_type():
    
    create_device()

    changed_type_components = [
        {
            "local_id": TEST_COMPONENTS[0]["local_id"],
            "model_name": "pump-model-b",
            "component_type": "sensor",
        },
        TEST_COMPONENTS[1],
    ]

    expected_components = [
        {
            "local_id": TEST_COMPONENTS[0]["local_id"],
            "model_name": "pump-model-b",
            "component_type": TEST_COMPONENTS[0]["component_type"],
        },
        TEST_COMPONENTS[1],
    ]

    response = create_device(components=changed_type_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], expected_components)

def test_register_device_adds_new_component():
    
    create_device()

    updated_components = TEST_COMPONENTS + [TEST_ADDED_COMPONENT]

    response = create_device(components=updated_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], updated_components)

def test_register_device_removes_missing_component():
    
    create_device()

    remaining_components = [TEST_COMPONENTS[0]]

    response = create_device(components=remaining_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], remaining_components)

def test_register_device_reconciles_components():
    
    create_device()

    reconciled_components = [
        {
            "local_id": TEST_COMPONENTS[0]["local_id"],
            "model_name": "pump-model-b",
            "component_type": TEST_COMPONENTS[0]["component_type"],
        },
        TEST_ADDED_COMPONENT,
    ]

    response = create_device(components=reconciled_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], reconciled_components)

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

def test_register_device_updates_last_seen_on_reregistration():
    
    create_device()

    first_response = client.get("/devices/" + TEST_DEVICE_ID)
    first_last_seen = first_response.json()["last_seen"]

    response = create_device()

    assert response.status_code == 200
    assert response.json()["last_seen"] != first_last_seen

def test_register_device_rejects_missing_component_field():
    
    response = client.post(
        "/devices",
        json={
            "id": TEST_DEVICE_ID,
            "name": TEST_DEVICE_NAME,
            "components": [
                {
                    "local_id": "pump_1",
                    "component_type": "actuator",
                }
            ],
        },
    )

    assert response.status_code == 422

def test_register_device_rejects_invalid_component_type():
    
    response = client.post(
        "/devices",
        json={
            "id": TEST_DEVICE_ID,
            "name": TEST_DEVICE_NAME,
            "components": [
                {
                    "local_id": "pump_1",
                    "model_name": "pump-model-a",
                    "component_type": "invalid",
                }
            ],
        },
    )

    assert response.status_code == 422

#-------------------HELPER-FUNCTIONS-------------------

def assert_components_match(actual_components, expected_components):
    assert sorted(actual_components, key=lambda component: component["local_id"]) == sorted(
        expected_components, key=lambda component: component["local_id"]
    )

def create_device(device_id=TEST_DEVICE_ID, name=TEST_DEVICE_NAME, components=None):

    if components is None:
        components = TEST_COMPONENTS

    return client.post("/devices", 
        json = {
            "id" : device_id,
            "name" : name,
            "components" : components
        }
    )
