#include <Arduino.h>

constexpr char DEVICE_ID[] = "shokudachi-01";
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;

constexpr int THIRSTY_ENTER = 30;
constexpr int THIRSTY_EXIT = 40;

enum class ShokuState{
  Happy,
  Thirsty,
  SensorErr
};

struct SoilReading {
  bool moisture_valid;
  int moisture_index;
  bool simulated;
};

ShokuState shoku_state = ShokuState::Happy;

uint32_t last_sample_ms = 0;
uint32_t sequence_num = 0;
size_t demo_pos = 0;

const char* state_name(ShokuState state){
  switch(state){
    case ShokuState::Happy:
      return "HAPPY";
    case ShokuState::Thirsty:
      return "THIRSTY";
    case ShokuState::SensorErr:
      return "SENSOR_ERROR";
  }

  return "SENSOR_ERROR";
}

const char* shoku_face(ShokuState state){
  switch(state){
    case ShokuState::Happy:
      return "๑(◕‿◕)๑";
    case ShokuState::Thirsty:
      return "(ᗒᗣᗕ)՞";
    case ShokuState::SensorErr:
      return "┐(´ー｀)┌";
  }
  return "┐(´ー｀)┌";
}

SoilReading read_sensor() {
  static const int demo_vals[] = {
    65, 50, 35, 25, 20, 32, 38, 45, 65
  };

  constexpr size_t count = sizeof(demo_vals[0]);

  SoilReading reading {
    true,
    demo_vals[demo_pos],
    true
  };

  demo_pos = (demo_pos + 1) % count;

  return reading;
}

void update_shoku_states(const SoilReading& reading){
  if(!reading.moisture_valid){
    shoku_state = ShokuState::SensorErr;
    return;
  }

  if(shoku_state == ShokuState::Thirsty){
    if(reading.moisture_index >= THIRSTY_EXIT){
      shoku_state = ShokuState::Happy;
    }
  }else{
    shoku_state = reading.moisture_index < THIRSTY_ENTER ? ShokuState::Thirsty : ShokuState::Happy;
  }
}

void print_reading(const SoilReading& reading){
  Serial.printf(
    "[Shokudachi] face=%s state=%s moisture_index=%d "
    "source=%s\n",
    shoku_face(shoku_state),
    state_name(shoku_state),
    reading.moisture_index,
    reading.simulated ? "simulated" : "sensor"
  );

  Serial.printf(
    "{\"device_id\":\"%s\","
    "\"sequence\":%lu,"
    "\"uptime_ms\":%lu,"
    "\"source\":\"%s\","
    "\"moisture_ok\":%s,"
    "\"moisture_index\":%d,"
    "\"state\":\"%s\"}\n",
    DEVICE_ID,
    static_cast<unsigned long>(sequence_num),
    static_cast<unsigned long>(millis()),
    reading.simulated ? "simulated" : "sensor",\
    reading.moisture_valid ? "true" : "false",
    reading.moisture_index,
    state_name(shoku_state)
  );
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Shokudachi booted");
  Serial.println("DEMO MODE: readings are simulated");

  last_sample_ms = millis();
}

void loop() {
  const uint32_t now = millis();

  if(static_cast<uint32_t>(now - last_sample_ms) >= SAMPLE_INTERVAL_MS){
    last_sample_ms = now;

    const SoilReading reading = read_sensor();

    update_shoku_states(reading);

    print_reading(reading);

    ++sequence_num;
  }
}