from fastapi import APIRouter, Depends, HTTPException

from backend.database import get_db
from backend.models import Device, DeviceConfig
from backend.schemas import DeviceInput
from backend.services.devices import build_device_response, sync_device_components, utc_now
from backend.services.config import write_config_to_db

router = APIRouter()

@router.get("/devices")
def get_devices(db=Depends(get_db)):
    devices = db.query(Device).all()

    device_info = []

    for device in devices:
        device_info.append(build_device_response(device))

    return device_info


@router.get("/devices/{id}")
def get_device(id: str, db=Depends(get_db)):
    device = db.query(Device).filter(
        Device.id == id
    ).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")

    return build_device_response(device)


@router.post("/devices")
def register_device(payload: DeviceInput, db=Depends(get_db)):
    device = db.query(Device).filter(
        Device.id == payload.id
    ).first()

    if device is None:
        device = Device(id=payload.id)
        device_config = DeviceConfig(
            device_id = device.id,
            heartbeat_interval = 30,
            desired_config_id = "default"
        )
        db.add(device)
        db.add(device_config)
        

    device.name = payload.name
    device.last_seen = utc_now()

    sync_device_components(device, payload.device_components)

    db.commit()

    return build_device_response(device)


@router.delete("/devices/{id}")
def delete_device(id: str, db=Depends(get_db)):
    device = db.query(Device).filter(
        Device.id == id
    ).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")

    db.delete(device)
    db.commit()

    return {
        "message": "DEVICE DELETED",
        "id": id,
    }
