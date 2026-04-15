from tests.data import TEST_COMPONENTS, TEST_DEVICE_ID, TEST_DEVICE_NAME
from tests.support import client


def assert_components_match(actual_components, expected_components):
    assert sorted(actual_components, key=lambda component: component["component_local_id"]) == sorted(
        expected_components, key=lambda component: component["component_local_id"]
    )


def create_device(device_id=TEST_DEVICE_ID, name=TEST_DEVICE_NAME, components=None):
    if components is None:
        components = TEST_COMPONENTS

    return client.post(
        "/devices",
        json={
            "id": device_id,
            "name": name,
            "device_components": components,
        },
    )

