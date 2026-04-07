from sqlalchemy import Column, ForeignKey, String
from backend.database import Base

class Component(Base):
    __tablename__ = "components"

    device_id = Column(String, ForeignKey("devices.id"), primary_key=True)
    key = Column(String, primary_key=True)
    name = Column(String)
    component_type = Column(String)

class Device(Base):
    __tablename__ = "devices"

    id = Column(String, primary_key=True)
    name = Column(String)
    last_seen = Column(String)

