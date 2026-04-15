from backend.models import Measurement
from tests.data import (
    TEST_COMPONENT_LOCAL_ID,
    TEST_COMPONENTS,
    TEST_DEVICE_ID,
    TEST_SECOND_DEVICE_ID,
    TEST_SECOND_DEVICE_NAME,
    TEST_UNKNOWN_COMPONENT_LOCAL_ID,
    TEST_UNKNOWN_DEVICE_ID,
)
from tests.helpers import create_device
from tests.support import TestingSessionLocal, client


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


def test_receive_measurement_creates_exactly_one_row_with_expected_fields():
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
        measurements = db.query(Measurement).all()

        assert len(measurements) == 1

        measurement = measurements[0]

        assert measurement.device_id == TEST_DEVICE_ID
        assert measurement.component_local_id == TEST_COMPONENT_LOCAL_ID
        assert measurement.mean_adc == 1234
        assert measurement.moisture_percent == 56
        assert measurement.created_at is not None
    finally:
        db.close()


def test_receive_measurement_associates_with_correct_device_when_multiple_devices_exist():
    create_device()
    create_device(
        device_id=TEST_SECOND_DEVICE_ID,
        name=TEST_SECOND_DEVICE_NAME,
        components=TEST_COMPONENTS,
    )

    response = client.post(
        "/devices/" + TEST_SECOND_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1500,
            "moisture_percent": 61,
        },
    )

    assert response.status_code == 204

    db = TestingSessionLocal()
    try:
        second_device_measurement = db.query(Measurement).filter(
            Measurement.device_id == TEST_SECOND_DEVICE_ID,
            Measurement.component_local_id == TEST_COMPONENT_LOCAL_ID,
        ).first()

        first_device_measurement = db.query(Measurement).filter(
            Measurement.device_id == TEST_DEVICE_ID,
            Measurement.component_local_id == TEST_COMPONENT_LOCAL_ID,
        ).first()

        assert second_device_measurement is not None
        assert second_device_measurement.mean_adc == 1500
        assert second_device_measurement.moisture_percent == 61
        assert first_device_measurement is None
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


def test_receive_measurement_unknown_device_does_not_store_measurement():
    response = client.post(
        "/devices/" + TEST_UNKNOWN_DEVICE_ID + "/measurements",
        json={
            "component_local_id": TEST_COMPONENT_LOCAL_ID,
            "mean_adc": 1234,
            "moisture_percent": 56,
        },
    )

    assert response.status_code == 404

    db = TestingSessionLocal()
    try:
        assert db.query(Measurement).count() == 0
    finally:
        db.close()


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


def test_receive_measurement_unknown_component_does_not_store_measurement():
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

    db = TestingSessionLocal()
    try:
        assert db.query(Measurement).count() == 0
    finally:
        db.close()


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


def test_receive_measurement_rejects_missing_component_local_id():
    create_device()

    response = client.post(
        "/devices/" + TEST_DEVICE_ID + "/measurements",
        json={
            "mean_adc": 1234,
            "moisture_percent": 56,
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
