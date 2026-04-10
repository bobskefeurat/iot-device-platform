from typing import Literal

from pydantic import BaseModel

class ComponentInput(BaseModel):
    local_id : str
    model_name : str
    component_type : Literal["sensor", "actuator"]

class DeviceInput(BaseModel):
    id: str
    name: str
    components : list[ComponentInput]

class MeasurementInput(BaseModel):
    component_local_id :str
    mean_adc : int
    moisture_percent : int