#include "bma253.h"

#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace bma253 {

static const char *const TAG = "bma253";

// clang-format off
const float GRAVITY_EARTH        = 9.80665f;

const uint8_t REG_CHIPID         = 0x00;
const uint8_t CHIP_ID            = 0b11111010;

const uint8_t REG_X_LSB          = 0x02;
const uint8_t REG_X_MSB          = 0x03;
const uint8_t REG_Y_LSB          = 0x04;
const uint8_t REG_Y_MSB          = 0x05;
const uint8_t REG_Z_LSB          = 0x06;
const uint8_t REG_Z_MSB          = 0x07;

const uint8_t REG_INT_EN_0       = 0x16;
const uint8_t REG_INT_STATUS_3   = 0x0C;

const uint8_t REG_PMU_RANGE      = 0x0F;
const uint8_t REG_BW_SELECT      = 0x10;
const uint8_t REG_PMU_LPW        = 0x11;
const uint8_t REG_PMU_LOW_POWER  = 0x12;

enum class PMU_LPW : uint8_t {
  NORMAL        = 0b00000000,
  DEEP_SUSPEND  = 0b00100000,
  LOW_POWER     = 0b01000000,
  SUSPEND       = 0b10000000,
};

enum class PMU_SDUR : uint8_t {
  MS_0_5  = 0b00000000, MS_25   = 0b00001011,
  MS_1    = 0b00000110, MS_50   = 0b00001100,
  MS_2    = 0b00000111, MS_100  = 0b00001101,
  MS_4    = 0b00001000, MS_500  = 0b00001110,
  MS_6    = 0b00001001, S_1     = 0b00001111,
};

enum class PMU_LOW_POWER : uint8_t {
  LPM_1  = 0b00000000,
  LPM_2 =  0b01000000,
};

enum class PMU_BW : uint8_t {
  HZ_7_81 = 0b1000, HZ_15_63 = 0b1001, HZ_31_25 = 0b1010,
  HZ_62_5 = 0b1011, HZ_125   = 0b1100, HZ_250   = 0b1101,
  HZ_500  = 0b1110, HZ_1000  = 0b1111,
};

enum class PMU_RANGE : uint8_t {
  G_2  = 0b0011, G_4 = 0b0101, G_8 = 0b1000, G_16 = 0b1100,
};
// clang-format on

bool BMA253Component::read_int16_le(uint8_t start, int16_t *values, size_t len) {
  uint8_t data[len * 2];
  if (!this->read_bytes(start, data, len * 2)) {
    return false;
  }

  for (size_t i = 0; i < len; i++) {
    values[i] = static_cast<int16_t>((data[i * 2 + 1] << 8) | (data[i * 2]));
  }
  return true;
}

void BMA253Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up BMA253...");

  uint8_t chip_id{};
  if (!this->read_byte(REG_CHIPID, &chip_id) || (chip_id != CHIP_ID)) {
    ESP_LOGE(TAG, "BMA253 has wrong chip ID!");
    this->mark_failed();
    return;
  }

  if (!this->write_byte(REG_PMU_LPW, static_cast<uint8_t>(PMU_LPW::NORMAL)) ||
      !this->write_byte(REG_BW_SELECT, static_cast<uint8_t>(PMU_BW::HZ_7_81)) ||
      !this->write_byte(REG_PMU_RANGE, static_cast<uint8_t>(PMU_RANGE::G_2)) ||
      !this->write_byte(REG_INT_EN_0, 0b01000000)) {
    ESP_LOGE(TAG, "Error during BMA253 configuration setup");
    this->mark_failed();
    return;
  }
}

void BMA253Component::update() {
  int16_t data[3] = {};
  if (!this->read_int16_le(REG_X_LSB, data, 3)) {
    this->status_set_warning();
    return;
  }

  float accel_x = (float) data[0] / (float) INT16_MAX * 2 * GRAVITY_EARTH;
  float accel_y = (float) data[1] / (float) INT16_MAX * 2 * GRAVITY_EARTH;
  float accel_z = (float) data[2] / (float) INT16_MAX * 2 * GRAVITY_EARTH;

  ESP_LOGV(TAG, "Accel={x=0x%x, y=0x%x, z=0x%x}", data[0], data[1], data[2]);
  ESP_LOGV(TAG, "Accel={x=%.3f m/s², y=%.3f m/s², z=%.3f m/s²}", accel_x, accel_y, accel_z);

  if (this->accel_x_sensor_)
    this->accel_x_sensor_->publish_state(accel_x);
  if (this->accel_y_sensor_)
    this->accel_y_sensor_->publish_state(accel_y);
  if (this->accel_z_sensor_)
    this->accel_z_sensor_->publish_state(accel_z);

  uint8_t int_status_3{};
  if (!this->read_byte(REG_INT_STATUS_3, &int_status_3)) {
    ESP_LOGD(TAG, "int_status_3 error");
    this->status_set_warning();
    return;
  }

  // 0b'0ZXY0'0000
  // Z  0 up | 1 down
  // XY 00 portrait up    | 01 portrait down
  //    10 landscape left | 11 landscape right
  uint8_t z_orient = (int_status_3 >> 6) & 0x01;
  uint8_t xy_orient = (int_status_3 >> 4) & 0x03;
  ESP_LOGV(TAG, "z_orient: %d", z_orient);
  ESP_LOGV(TAG, "xy_orient: %d", xy_orient);

  if (this->orientation_sensor_)
    this->orientation_sensor_->publish_state(xy_orient);

  this->status_clear_warning();
}

void BMA253Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BMA253:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with BMA253 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Acceleration X", this->accel_x_sensor_);
  LOG_SENSOR("  ", "Acceleration Y", this->accel_y_sensor_);
  LOG_SENSOR("  ", "Acceleration Z", this->accel_z_sensor_);
  LOG_SENSOR("  ", "Orientation", this->orientation_sensor_);
}

float BMA253Component::get_setup_priority() const { return esphome::setup_priority::DATA; };

}  // namespace bma253
}  // namespace esphome
