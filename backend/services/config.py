from backend.models import DeviceConfig
import uuid

def write_config_to_db(
        device_config : DeviceConfig, 
        new_heartbeat_interval : int
        ):

    device_config.heartbeat_interval = new_heartbeat_interval
    device_config.desired_config_id = str(uuid.uuid4())

    return

def build_device_config_response(device_config : DeviceConfig):
    return {
        "heartbeat_interval" : device_config.heartbeat_interval,
        "desired_config_id" : device_config.desired_config_id
    }