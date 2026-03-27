from fastapi import FastAPI, Depends
from datetime import datetime
from pydantic import BaseModel

from backend.database import Device, get_db


app = FastAPI()

class DeviceInput(BaseModel):
    id : str
    name : str


@app.get("/")
def root():
    return {"message": "IoT Backend Running"}

@app.get("/health")
def get_health():
    return {"status": "online"}

@app.get("/devices")
def get_devices(db = Depends(get_db)):

    devices = db.query(Device).all()

    device_info = []

    for device in devices:
        device_info.append(
        {
            "id" : device.id,
            "name" : device.name,
            "status" : device.status,
            "last_seen" : device.last_seen
        }
        )

    return device_info

@app.post("/devices")
def post_device(payload : DeviceInput, db = Depends(get_db)):
    
    device = db.query(Device).filter(Device.id == payload.id).first()

    if device is None:
         device = Device(id = payload.id)
         db.add(device)

    device.name = payload.name
    device.status = "ONLINE"
    device.last_seen = datetime.now().isoformat()

    db.commit()

    return {
        "message" : "SUCCESS",
        "device": {
            "id" : device.id,
            "name" :  device.name,
            "status" : device.status,
            "last_seen" : device.last_seen
        }
    }

@app.delete("/devices")
def delete_device(id : str, db = Depends(get_db)):

    device = db.query(Device).filter(Device.id == id).first()

    if device is None:
        
        return {
            "message" : "DEVICE NOT FOUND"
        }
    
    db.delete(device)
    db.commit()

    return {
        "message" : "device deleted",
        "id" : id
    }
    
