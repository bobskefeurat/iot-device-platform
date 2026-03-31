from fastapi import FastAPI, Depends, HTTPException
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

@app.get("/devices/{id}")
def get_device(id : str, db = Depends(get_db)):

    device = db.query(Device).filter(Device.id == id).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")

    return {
        "id" : device.id,
        "name" : device.name,
        "status" : device.status,
        "last_seen" : device.last_seen
    }


@app.post("/devices")
def add_device(payload : DeviceInput, db = Depends(get_db)):
    
    device = db.query(Device).filter(Device.id == payload.id).first()

    if device is None:
         device = Device(id = payload.id)
         db.add(device)

    device.name = payload.name
    device.status = "ONLINE"
    device.last_seen = datetime.now().isoformat()

    db.commit()

    return {
            "id" : device.id,
            "name" :  device.name,
            "status" : device.status,
            "last_seen" : device.last_seen
     }
    

@app.delete("/devices/{id}")
def delete_device(id : str, db = Depends(get_db)):

    device = db.query(Device).filter(Device.id == id).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")
    
    db.delete(device)
    db.commit()

    return {
        "message" : "DEVICE DELETED",
        "id" : id
    }
    
