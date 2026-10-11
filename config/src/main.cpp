#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

constexpr char DEVICE_ID[] = "shokudachi-01";
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;

constexpr int THIRSTY_ENTER = 30;
constexpr int THIRSTY_EXIT = 40;

// Sensor Analog pin
constexpr int MOISTURE_PIN = 34;

constexpr int MOISTURE_SAMPLE_COUNT = 16;

// LCD Display screen pins
constexpr int TFT_SCL = 18;
constexpr int TFT_SDA = 23;
constexpr int TFT_CS = 22;
constexpr int TFT_DC = 21;
constexpr int TFT_RST = 27;

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

constexpr uint32_t TFT_SPI_HZ = 4000000;

constexpr int DRY_RAW = 4095;
constexpr int WET_RAW = 1500;

enum class ShokuState{
  Happy,
  Thirsty,
  SensorErr
};

struct SoilReading {
  bool moisture_valid;
  int moisture_raw;
  int moisture_index;
  bool simulated;
};

ShokuState shoku_state = ShokuState::Happy;

uint32_t last_sample_ms = 0;
uint32_t sequence_num = 0;

bool display_has_state = false;
ShokuState displayed_state = ShokuState::SensorErr;

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
  uint32_t total = 0;

  for(int i = 0; i < MOISTURE_SAMPLE_COUNT; ++i){
    total += analogRead(MOISTURE_PIN);
    delay(2);
  }

  const int raw = total / MOISTURE_SAMPLE_COUNT;

  SoilReading reading {
    false,
    raw,
    0,
    false
  };

  if(DRY_RAW == WET_RAW){
    return reading;
  }

  const float index = 100.0f * (raw - DRY_RAW) / (WET_RAW - DRY_RAW);

  reading.moisture_index = constrain(static_cast<int>(roundf(index)), 0, 100);

  reading.moisture_valid = true;

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

// LCD Display Helpers

void draw_centered_text(const char* text, int16_t y, uint8_t size, uint16_t color){
  tft.setTextSize(size);
  tft.setTextColor(color);

  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  tft.getTextBounds(text, 0, y, &x1, &y1, &width, &height);

  const int16_t x = (static_cast<int16_t>(tft.width()) - static_cast<int16_t>(width)) / 2 - x1;

  tft.setCursor(x, y);
  tft.print(text);
}

void draw_happy_face() {
    const uint16_t color = GC9A01A_GREEN;

    tft.fillRect(65, 65, 24, 32, color);
    tft.fillRect(151, 65, 24, 32, color);

    tft.fillRect(75, 120, 12, 18, color);
    tft.fillRect(153, 120, 12, 18, color);
    tft.fillRect(87, 138, 66, 12, color);
}

void draw_thirsty_face() {
    const uint16_t color = GC9A01A_YELLOW;

    tft.fillRect(65, 65, 24, 24, color);
    tft.fillRect(151, 65, 24, 24, color);

    // Tears.
    tft.fillRect(73, 94, 8, 18, GC9A01A_CYAN);
    tft.fillRect(159, 94, 8, 18, GC9A01A_CYAN);

    // Upside-down smile.
    tft.fillRect(87, 125, 66, 12, color);
    tft.fillRect(75, 137, 12, 18, color);
    tft.fillRect(153, 137, 12, 18, color);
}

void draw_error_face() {
    const uint16_t color = GC9A01A_RED;

    // X-shaped eyes.
    for (int offset = -1; offset <= 1; ++offset) {
        tft.drawLine(
            65, 65 + offset, 89, 89 + offset, color
        );
        tft.drawLine(
            65, 89 + offset, 89, 65 + offset, color
        );

        tft.drawLine(
            151, 65 + offset, 175, 89 + offset, color
        );
        tft.drawLine(
            151, 89 + offset, 175, 65 + offset, color
        );
    }

    tft.fillRect(90, 130, 60, 10, color);
}

void draw_state_face() {
    tft.fillScreen(GC9A01A_BLACK);

    draw_centered_text(
        "Shokudachi", 25, 2, GC9A01A_WHITE
    );

    switch (shoku_state) {
        case ShokuState::Happy:
            draw_happy_face();
            break;

        case ShokuState::Thirsty:
            draw_thirsty_face();
            break;

        case ShokuState::SensorErr:
            draw_error_face();
            break;
    }
}

void update_display(const SoilReading& reading) {
    if (!display_has_state || displayed_state != shoku_state) {
        draw_state_face();

        displayed_state = shoku_state;
        display_has_state = true;
    }

    // Clear only the status/value area between samples.
    tft.fillRect(25, 163, 190, 57, GC9A01A_BLACK);

    if (!reading.moisture_valid) {
        draw_centered_text(
            "Check sensor", 171, 2, GC9A01A_RED
        );
        draw_centered_text(
            "Check calibration", 201, 1, GC9A01A_WHITE
        );
        return;
    }

    char moisture_text[32];

    snprintf(
        moisture_text,
        sizeof(moisture_text),
        "Moisture: %d",
        reading.moisture_index
    );

    draw_centered_text(
        moisture_text, 171, 2, GC9A01A_WHITE
    );

    draw_centered_text(
        shoku_state == ShokuState::Thirsty
            ? "Need water"
            : "Feeling good",
        201,
        1,
        shoku_state == ShokuState::Thirsty
            ? GC9A01A_YELLOW
            : GC9A01A_GREEN
    );
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

  analogReadResolution(12);
  analogSetPinAttenuation(MOISTURE_PIN, ADC_11db);

  SPI.begin(TFT_SCL, -1, TFT_SDA, TFT_CS);
  tft.begin(TFT_SPI_HZ);
  tft.setRotation(0);
  tft.setTextWrap(false);

  tft.fillScreen(GC9A01A_BLACK);

  draw_centered_text("Shokudachi", 90, 2, GC9A01A_GREEN);

  draw_centered_text("Reading soil...", 125, 1, GC9A01A_WHITE);

  Serial.println();
  Serial.println("Shokudachi booted");
  Serial.println("LIVE MODE: moisture sensor enabled");
  Serial.printf("Moisture GPIO: %d\n", MOISTURE_PIN);
  Serial.printf("Calibration references: dry=%d wet=%d\n", DRY_RAW, WET_RAW);

  last_sample_ms = millis();
}

void loop() {
  const uint32_t now = millis();

  if(static_cast<uint32_t>(now - last_sample_ms) >= SAMPLE_INTERVAL_MS){
    last_sample_ms = now;

    const SoilReading reading = read_sensor();

    update_shoku_states(reading);
    update_display(reading);
    print_reading(reading);


    ++sequence_num;
  }
}


/*
void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    delay(500);

    Serial.println("Shokudachi moisture test");
}

void loop() {
    uint32_t total = 0;

    for (int i = 0; i < 16; ++i) {
        total += analogRead(MOISTURE_PIN);
        delay(2);
    }

    const int raw = total / 16;

    Serial.printf("Moisture raw: %d\n", raw);
    delay(1000);
}





Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

void drawHappyFace() {
    tft.fillScreen(GC9A01A_BLACK);

    // Pixel-style eyes.
    tft.fillRect(65, 75, 24, 32, GC9A01A_GREEN);
    tft.fillRect(151, 75, 24, 32, GC9A01A_GREEN);

    // Pixel-style smile.
    tft.fillRect(75, 130, 12, 18, GC9A01A_GREEN);
    tft.fillRect(153, 130, 12, 18, GC9A01A_GREEN);
    tft.fillRect(87, 148, 66, 12, GC9A01A_GREEN);

    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(2);
    tft.setCursor(60, 185);
    tft.print("Shokudachi");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    SPI.begin(TFT_SCL, -1, TFT_SDA, TFT_CS);

    tft.begin();
    tft.setRotation(0);

    // Brief color tests.
    tft.fillScreen(GC9A01A_RED);
    delay(400);

    tft.fillScreen(GC9A01A_GREEN);
    delay(400);

    tft.fillScreen(GC9A01A_BLUE);
    delay(400);

    drawHappyFace();

    Serial.println("Display initialized; happy face drawn.");
}

void loop() {
}
*/