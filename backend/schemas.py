from typing import Literal

from pydantic import BaseModel

from backend.device_mode import ESPMode

class ComponentInput(BaseModel):
    component_local_id : str
    model_name : str
    component_type : Literal["sensor", "actuator"]

class DeviceInput(BaseModel):
    id: str
    name: str
    device_components : list[ComponentInput]

class MeasurementInput(BaseModel):
    component_local_id :str
    mean_adc : int
    moisture_percent : int

class HeartbeatResponse(BaseModel):
    mode : ESPMode