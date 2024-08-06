import esphome.codegen as cg
import esphome.config_validation as cv 
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_ACCELERATION_X,
    CONF_ACCELERATION_Y,
    CONF_ACCELERATION_Z,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER_PER_SECOND_SQUARED,
    UNIT_EMPTY,
    ICON_ACCELERATION_X,
    ICON_ACCELERATION_Y,
    ICON_ACCELERATION_Z,
)

DEPENDENCIES = ["i2c"]

bma253_ns = cg.esphome_ns.namespace("bma253")
BMA253Component = bma253_ns.class_(
    "BMA253Component", cg.PollingComponent, i2c.I2CDevice
)

CONF_ORIENTATION = "orientation"

accel_schema = {
    "unit_of_measurement": UNIT_METER_PER_SECOND_SQUARED,
    "accuracy_decimals": 2,
    "state_class": STATE_CLASS_MEASUREMENT,
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BMA253Component),
            cv.Optional(CONF_ACCELERATION_X): sensor.sensor_schema(
                icon=ICON_ACCELERATION_X,
                **accel_schema,
            ),
            cv.Optional(CONF_ACCELERATION_Y): sensor.sensor_schema(
                icon=ICON_ACCELERATION_Y,
                **accel_schema,
            ),
            cv.Optional(CONF_ACCELERATION_Z): sensor.sensor_schema(
                icon=ICON_ACCELERATION_Z,
                **accel_schema,
            ),
            cv.Optional(CONF_ORIENTATION): sensor.sensor_schema(
                icon="mdi:phone-rotate-portrait",
                unit_of_measurement=UNIT_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x18))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    for dir in [CONF_ACCELERATION_X, CONF_ACCELERATION_Y, CONF_ACCELERATION_Z]:
        if dir in config:
            sens = await sensor.new_sensor(config[dir])
            cg.add(getattr(var, f"set_{dir}_sensor")(sens))

    if CONF_ORIENTATION in config:
        sens = await sensor.new_sensor(config[CONF_ORIENTATION])
        cg.add(getattr(var, f"set_orientation_sensor")(sens))