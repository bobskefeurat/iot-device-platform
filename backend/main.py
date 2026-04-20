from fastapi import FastAPI

from backend.api.health import router as health_router
from backend.api.devices import router as devices_router
from backend.api.signals import router as signals_router
from backend.api.config import router as config_router
from backend.database import Base, engine

app = FastAPI()
Base.metadata.create_all(bind=engine)

app.include_router(health_router)
app.include_router(devices_router)
app.include_router(signals_router)
app.include_router(config_router)
