from tests.data import (
    TEST_ADDED_COMPONENT,
    TEST_COMPONENTS,
    TEST_DEVICE_ID,
    TEST_DEVICE_NAME,
)
from tests.helpers import assert_components_match, create_device
from tests.support import TestingSessionLocal, client
from backend.models import Component, Device


def test_register_device():
    response = create_device()

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data["name"] == TEST_DEVICE_NAME
    assert_components_match(data["components"], TEST_COMPONENTS)


def test_register_device_requires_components():
    response = client.post(
        "/devices",
        json={
            "id": TEST_DEVICE_ID,
            "name": TEST_DEVICE_NAME,
        },
    )

    assert response.status_code == 422


def test_register_device_allows_empty_components():
    response = client.post(
        "/devices",
        json={
            "id": TEST_DEVICE_ID,
            "name": TEST_DEVICE_NAME,
            "device_components": [],
        },
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
            "component_local_id": TEST_COMPONENTS[0]["component_local_id"],
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
            "component_local_id": TEST_COMPONENTS[0]["component_local_id"],
            "model_name": "pump-model-b",
            "component_type": "sensor",
        },
        TEST_COMPONENTS[1],
    ]

    expected_components = [
        {
            "component_local_id": TEST_COMPONENTS[0]["component_local_id"],
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
            "component_local_id": TEST_COMPONENTS[0]["component_local_id"],
            "model_name": "pump-model-b",
            "component_type": TEST_COMPONENTS[0]["component_type"],
        },
        TEST_ADDED_COMPONENT,
    ]

    response = create_device(components=reconciled_components)

    assert response.status_code == 200
    assert_components_match(response.json()["components"], reconciled_components)


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
            "device_components": [
                {
                    "component_local_id": "pump_1",
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
            "device_components": [
                {
                    "component_local_id": "pump_1",
                    "model_name": "pump-model-a",
                    "component_type": "invalid",
                }
            ],
        },
    )

    assert response.status_code == 422
