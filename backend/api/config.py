from fastapi import APIRouter, Depends, HTTPException

from backend.database import get_db
from backend.schemas import DeviceConfigInput
from backend.models import DeviceConfig
from backend.services.config import build_device_config_response, write_config_to_db

router = APIRouter()

@router.post("/devices/{id}/config/update")
def update_desired_config(id: str, new_heartbeat_interval : int, db=Depends(get_db)):

    device_config = db.query(DeviceConfig).filter(
        DeviceConfig.device_id == id
    ).first()

    if device_config is None:
        raise HTTPException(status_code=404, detail= "DEVICE CONFIG NOT FOUND")
    
    write_config_to_db(device_config, new_heartbeat_interval)

    db.commit()

    return {
        "message" : f"Config updated for device: {id}",
        "desired_config_id" : device_config.desired_config_id,
        "heartbeat_interval" : device_config.heartbeat_interval 
    }

@router.post("/devices/{id}/config/sync")
def sync_config(id: str, payload: DeviceConfigInput, db=Depends(get_db)):
    
    device_config = db.query(DeviceConfig).filter(
        DeviceConfig.device_id == id
    ).first()

    if device_config is None:
        raise HTTPException(status_code=404, detail= "DEVICE CONFIG NOT FOUND")
    
    if payload.applied_device_config_id != device_config.desired_config_id:
        return build_device_config_response(device_config)
    
    device_config.applied_config_id = device_config.desired_config_id

    db.commit()
    
    return {
        "message" : "config synced"
    }