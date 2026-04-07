from pydantic import BaseModel

class ComponentInput(BaseModel):
    key : str
    name : str
    component_type : str

class DeviceInput(BaseModel):
    id: str
    name: str
    components : list[ComponentInput] | None = None
