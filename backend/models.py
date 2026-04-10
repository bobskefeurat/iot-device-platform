from sqlalchemy import Column, DateTime, ForeignKey, String
from sqlalchemy.orm import relationship
from backend.database import Base

class Component(Base):
    __tablename__ = "components"

    device_id = Column(String, ForeignKey("devices.id"), primary_key=True)
    local_id = Column(String, primary_key=True)
    model_name = Column(String)
    component_type = Column(String)

    device = relationship("Device", back_populates="components")


class Device(Base):
    __tablename__ = "devices"

    id = Column(String, primary_key=True)
    name = Column(String)
    last_seen = Column(DateTime(timezone=True))

    components = relationship("Component", back_populates="device", cascade="all, delete-orphan")

