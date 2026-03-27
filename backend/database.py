from sqlalchemy import create_engine, Column, String
from sqlalchemy.orm import sessionmaker, declarative_base

DATABASE_URL = "sqlite:///./devices.db"

engine = create_engine(
    DATABASE_URL, 
    connect_args={"check_same_thread" : False})

Base = declarative_base()

SessionLocal = sessionmaker(
    autocommit=False,
    autoflush=False,
    bind=engine
)

class Device(Base):
    __tablename__ = "devices"

    id = Column(String, primary_key=True)
    name = Column(String)
    status = Column(String)
    last_seen = Column(String)

Base.metadata.create_all(bind=engine)

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()
