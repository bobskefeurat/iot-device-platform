import pytest
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from fastapi.testclient import TestClient

from backend.main import app
from backend.database import Base, get_db

#-------------------CONFIG-------------------

TEST_DATABASE_URL = "sqlite:///./test_devices.db"

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

def test_add_device():

    device_id = "1111"
    device_name = "test-add-device"

    response = client.post(
        "/devices", 
        json= {
            "id" : device_id, 
            "name" : device_name
        }
    )

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == device_id
    assert data ["name"] == device_name

def test_get_device():

    device_id = "2222"
    device_name = "test-get-device"

    client.post(
        "/devices", 
        json={
            "id" : device_id,
            "name" : device_name
        }
    )

    response = client.get("/devices/"+ device_id)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == device_id
    assert data["name"] == device_name

def test_get_device_not_found():

    response = client.get("/devices/0000")

    assert response.status_code == 404

def test_delete_device():

    device_id = "3333"
    device_name = "test-delete-device"

    client.post(
        "/devices", 
        json={
            "id" : device_id, 
            "name" : device_name
        }
    )

    response = client.delete("/devices/" + device_id)

    assert response.status_code == 200

    data = response.json()

    assert data["id"] == device_id

def test_delete_device_not_found():
    
    response = client.delete("/devices/0000")

    assert response.status_code == 404



