#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace bma253 {

class BMA253Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;

  float get_setup_priority() const override;

  void set_acceleration_x_sensor(sensor::Sensor *accel_x_sensor) { accel_x_sensor_ = accel_x_sensor; }
  void set_acceleration_y_sensor(sensor::Sensor *accel_y_sensor) { accel_y_sensor_ = accel_y_sensor; }
  void set_acceleration_z_sensor(sensor::Sensor *accel_z_sensor) { accel_z_sensor_ = accel_z_sensor; }

  void set_orientation_sensor(sensor::Sensor *orientation_sensor) { orientation_sensor_ = orientation_sensor; }

  void dump_config();

 protected:
  sensor::Sensor *accel_x_sensor_{nullptr};
  sensor::Sensor *accel_y_sensor_{nullptr};
  sensor::Sensor *accel_z_sensor_{nullptr};

  sensor::Sensor *orientation_sensor_{nullptr};

  bool read_int16_le(uint8_t start, int16_t *values, size_t len);
};

}  // namespace bma253
}  // namespace esphome