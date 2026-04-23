from sqlalchemy import Column, DateTime, ForeignKey, ForeignKeyConstraint, Integer, String, Boolean
from sqlalchemy.orm import relationship
from backend.database import Base

class Measurement(Base):
    __tablename__ = "measurements"
    
    device_id = Column(String, primary_key = True)
    component_local_id = Column(String, primary_key = True)
    created_at = Column(DateTime(timezone = True), primary_key = True)
    mean_adc = Column(Integer)
    moisture_percent = Column(Integer)

    __table_args__ = (
        ForeignKeyConstraint(
            ["device_id", "component_local_id"],
            ["components.device_id", "components.component_local_id"],
        ),
    )

class SensorConfig(Base):
    __tablename__ = "sensor_config"
    device_id = Column(String, primary_key = True)
    component_local_id = Column(String, primary_key = True)
    is_enabled = Column(Boolean)
    sample_interval = Column(Integer)
    desired_config_id = Column(String)
    applied_config_id = Column(String)

    __table_args__ = (
        ForeignKeyConstraint(
            ["device_id", "component_local_id"],
            ["components.device_id", "components.component_local_id"],
        ),
    )

class Component(Base):
    __tablename__ = "components"

    device_id = Column(String, ForeignKey("devices.id"), primary_key = True)
    component_local_id = Column(String, primary_key = True)
    model_name = Column(String)
    component_type = Column(String)

    device = relationship("Device", back_populates="components")

class DeviceConfig(Base):
    __tablename__ = "device_config"

    device_id = Column(String, ForeignKey("devices.id"), primary_key = True)
    heartbeat_interval = Column(Integer)
    desired_config_id = Column(String)
    applied_config_id = Column(String)

    device = relationship("Device", back_populates="config")


class Device(Base):
    __tablename__ = "devices"

    id = Column(String, primary_key = True)
    name = Column(String)
    last_seen = Column(DateTime(timezone = True))

    components = relationship("Component", back_populates="device", cascade="all, delete-orphan")
    config = relationship("DeviceConfig", back_populates="device", uselist=False)
