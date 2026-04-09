from typing import Literal

from pydantic import BaseModel, field_validator

class ComponentInput(BaseModel):
    local_id : str
    model_name : str
    component_type : Literal["sensor", "actuator"]

class DeviceInput(BaseModel):
    id: str
    name: str
    components : list[ComponentInput]

    @field_validator("components")
    @classmethod
    def validate_unique_component_local_ids(cls, components: list[ComponentInput]):
        seen_local_ids = set()

        for component in components:
            if component.local_id in seen_local_ids:
                raise ValueError("component local_id values must be unique")

            seen_local_ids.add(component.local_id)

        return components
