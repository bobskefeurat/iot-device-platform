import pytest
import time
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from fastapi.testclient import TestClient

from backend.main import app
from backend.database import Base, get_db

#-------------------CONFIG-------------------

TEST_DATABASE_URL = "sqlite:///./test_devices.db"

TEST_DEVICE_ID = "AA:BB:CC:DD:EE:FF"
TEST_DEVICE_NAME = "test-device"
TEST_UNKNOWN_DEVICE_ID = "11:22:33:44:55:66"

engine = create_engine(
    TEST_DATABASE_URL,
    connect_args= {"check_same_thread" : False}
)

TestingSessionLocal = sessionmaker(
    autocommit = False,
    autoflush = False,
    bind= engine
)

#-------------------APP-TEST-SETUP-------------------

def override_get_db():
    db = TestingSessionLocal()
    try:
        yield db
    finally:
        db.close()

app.dependency_overrides[get_db] = override_get_db
client = TestClient(app)

#-------------------RESET-DATABASE-PER-TEST-------------------

@pytest.fixture(autouse=True)
def reset_test_database():
    Base.metadata.drop_all(bind=engine)
    Base.metadata.create_all(bind=engine)

#-------------------TESTS-------------------

def test_get_health():

    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {"status" : "online"}

def test_get_devices():

    response = client.get("/devices")

    assert response.status_code == 200
    assert isinstance(response.json(), list)

def test_register_device():

    response = create_device()

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data ["name"] == TEST_DEVICE_NAME

def test_get_device():

    create_device()

    response = client.get("/devices/"+ TEST_DEVICE_ID)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID
    assert data["name"] == TEST_DEVICE_NAME

def test_get_unknown_device():

    response = client.get("/devices/"+ TEST_UNKNOWN_DEVICE_ID)

    assert response.status_code == 404

def test_delete_device():

    create_device()

    response = client.delete("/devices/" + TEST_DEVICE_ID)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == TEST_DEVICE_ID

def test_delete_unknown_device():
    
    response = client.delete("/devices/" + TEST_UNKNOWN_DEVICE_ID)

    assert response.status_code == 404

def test_receive_heartbeat():

    create_device()

    response = client.post("/devices/" + TEST_DEVICE_ID + "/heartbeat")

    assert response.status_code == 204

def test_receive_heartbeat_unknown_device():

    response = client.post("/devices/" + TEST_UNKNOWN_DEVICE_ID + "/heartbeat")

    assert response.status_code == 404

def test_last_seen():
    
    create_device()

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    last_seen_first = data["last_seen"]

    time.sleep(1)

    client.post("/devices/" + TEST_DEVICE_ID + "/heartbeat")

    device = client.get("/devices/" + TEST_DEVICE_ID)

    data = device.json()

    last_seen_second = data["last_seen"]

    assert last_seen_first != last_seen_second

    
#-------------------HELPER-FUNCTIONS-------------------

def create_device():

    return client.post("/devices", 
        json = {
            "id" : TEST_DEVICE_ID,
            "name" : TEST_DEVICE_NAME
        }
    )


