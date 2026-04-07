from fastapi import FastAPI, Depends, HTTPException
from datetime import datetime, timedelta

from backend.database import Base, engine, get_db
from backend.models import Device
from backend.schemas import DeviceInput

HEARTBEAT_TIMEOUT = timedelta(seconds=90)

app = FastAPI()
Base.metadata.create_all(bind=engine)

#----------------GENERAL----------------

@app.get("/")
def root():
    return {"message": "IoT Backend Running"}

@app.get("/health")
def get_health():
    return {"status": "online"}

#----------------DEVICE----------------

@app.get("/devices")
def get_devices(db = Depends(get_db)):

    devices = db.query(Device).all()

    device_info = []

    for device in devices:
        device_info.append(
        {
            "id" : device.id,
            "name" : device.name,
            "status": device_status(device.last_seen),
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
        "status": device_status(device.last_seen),
        "last_seen" : device.last_seen
    }


@app.post("/devices")
def register_device(payload : DeviceInput, db = Depends(get_db)):
    
    device = db.query(Device).filter(Device.id == payload.id).first()

    if device is None:
         device = Device(id = payload.id)
         db.add(device)

    device.name = payload.name
    device.last_seen = datetime.now().isoformat()

    db.commit()

    return {
            "id" : device.id,
            "name" :  device.name,
            "status": device_status(device.last_seen),
            "last_seen" : device.last_seen
     }
    
@app.post("/devices/{id}/heartbeat", status_code = 204)
def receive_device_heartbeat(id : str, db = Depends(get_db)):

    device = db.query(Device).filter(Device.id == id).first()

    if device is None:
        raise HTTPException(status_code=404, detail="DEVICE NOT FOUND")
    
    device.last_seen = datetime.now().isoformat()

    db.commit()

    return 

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


#----------------HELPERS----------------

def device_status(last_seen: str | None) -> str:
    if not last_seen:
        return "OFFLINE"

    try:
        last_seen_dt = datetime.fromisoformat(last_seen)
    except ValueError:
        return "OFFLINE"

    if datetime.now() - last_seen_dt <= HEARTBEAT_TIMEOUT:
        return "ONLINE"

    return "OFFLINE"
