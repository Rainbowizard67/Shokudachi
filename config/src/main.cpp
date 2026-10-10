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
  int moister_index;
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
    if(reading.moister_index >= THIRSTY_EXIT){
      shoku_state = ShokuState::Happy;
    }
  }else{
    shoku_state = reading.moister_index < THIRSTY_ENTER ? ShokuState::Thirsty : ShokuState::Happy;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED, HIGH);
  Serial.println("LED is on");
  delay(1000);
  digitalWrite(LED, LOW);
  Serial.println("LED is off");
  delay(1000);
}