from fastapi import APIRouter

router = APIRouter()


@router.get("/")
def root():
    return {"message": "IoT Backend Running"}


@router.get("/health")
def get_health():
    return {"status": "online"}
