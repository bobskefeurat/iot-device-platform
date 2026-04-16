from datetime import datetime, timedelta, timezone

from backend.models import Component

HEARTBEAT_TIMEOUT = timedelta(seconds=90)


def sync_device_components(device, payload_components):
    current_components = {}
    incoming_local_ids = set()

    for component in device.components:
        current_components[component.component_local_id] = component

    for incoming_component in payload_components:
        existing_component = current_components.get(incoming_component.component_local_id)

        incoming_local_ids.add(incoming_component.component_local_id)

        if existing_component is None:
            device.components.append(
                Component(
                    component_local_id=incoming_component.component_local_id,
                    model_name=incoming_component.model_name,
                    component_type=incoming_component.component_type,
                )
            )
        else:
            if existing_component.model_name != incoming_component.model_name:
                existing_component.model_name = incoming_component.model_name

    for local_id, component in current_components.items():
        if local_id not in incoming_local_ids:
            device.components.remove(component)


def utc_now() -> datetime:
    return datetime.now(timezone.utc)


def normalize_utc_datetime(value: datetime | None) -> datetime | None:
    if value is None:
        return None

    if value.tzinfo is None:
        return value.replace(tzinfo=timezone.utc)

    return value.astimezone(timezone.utc)


def device_status(last_seen: datetime | None) -> str:
    last_seen_dt = normalize_utc_datetime(last_seen)

    if not last_seen_dt:
        return "OFFLINE"

    if utc_now() - last_seen_dt <= HEARTBEAT_TIMEOUT:
        return "ONLINE"

    return "OFFLINE"


def build_device_response(device):
    return {
        "id": device.id,
        "name": device.name,
        "status": device_status(device.last_seen),
        "last_seen": normalize_utc_datetime(device.last_seen),
        "components": [
            {
                "component_local_id": component.component_local_id,
                "model_name": component.model_name,
                "component_type": component.component_type,
            }
            for component in device.components
        ],
    }
