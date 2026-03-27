from fastapi import FastAPI
from datetime import datetime
from pydantic import BaseModel


app = FastAPI()

devices = []

class DeviceInput(BaseModel):
    id : str
    name : str

class Device:

    def __init__(self, id):
        self.id = id
        self.name = None
        self.status = None
        self.last_seen = None

    def get_device_info(self):
        return {
            "id" : self.id,
            "name" : self.name,
            "status"  : self.status,
            "last_seen" : self.last_seen
        }


@app.get("/")
def root():
    return {"message": "IoT Backend Running"}

@app.get("/health")
def get_health():
    return{"Backend Status" : "Online"}

@app.get("/devices")
def get_devices():

    device_info = []

    for device in devices:
        device_info.append(device.get_device_info())

    return device_info

@app.post("/devices")
def post_devices(payload : DeviceInput):
    id = payload.id
    name = payload.name

    current_device = None

    for device in devices:
        if device.id == id:
            current_device = device
            break
    
    if current_device is None:
        current_device = Device(id)
        devices.append(current_device)

    current_device.name = name
    current_device.status = "online"
    current_device.last_seen = datetime.now().isoformat()

    return {
        "message" : "success",
        "device" : current_device.get_device_info()
    }
