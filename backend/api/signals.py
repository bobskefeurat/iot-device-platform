from fastapi import APIRouter, Depends, HTTPException

from backend.database import get_db
from backend.models import Component, Device, Measurement
from backend.schemas import MeasurementInput
from backend.services.devices import utc_now

router = APIRouter()


@router.post("/devices/{id}/heartbeat")
def receive_device_heartbeat(id: str, db=Depends(get_db)):
    device = db.query(Device).filter(
        Device.id == id
    ).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")

    device.last_seen = utc_now()
    db.commit()

    config_id = None

    if device.config is not None:
        config_id = device.config.desired_config_id

    return {
       "desired_config_id" : config_id
       }

@router.post("/devices/{id}/measurements", status_code=204)
def receive_measurement(id: str, payload: MeasurementInput, db=Depends(get_db)):
    device = db.query(Device).filter(
        Device.id == id
    ).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")

    measurement_component = db.query(Component).filter(
        Component.device_id == id,
        Component.component_local_id == payload.component_local_id,
    ).first()

    if measurement_component is None:
        raise HTTPException(status_code=404, detail="COMPONENT NOT FOUND")

    measurement = Measurement(
        device_id=id,
        component_local_id=payload.component_local_id,
        created_at=utc_now(),
        mean_adc=payload.mean_adc,
        moisture_percent=payload.moisture_percent,
    )

    db.add(measurement)
    db.commit()

    return
