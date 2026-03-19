#include <Arduino.h>
#include <HTTPClient.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <math.h>
#include <string.h>

#include "dashboard_config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "CHANGE_ME_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CHANGE_ME_WIFI_PASSWORD"
#endif

#ifndef HA_BASE_URL
#define HA_BASE_URL "http://homeassistant.local:8123"
#endif

#ifndef HA_TOKEN
#define HA_TOKEN "CHANGE_ME_LONG_LIVED_ACCESS_TOKEN"
#endif

#ifndef HA_ALLOW_INSECURE_TLS
#define HA_ALLOW_INSECURE_TLS 1
#endif

#ifndef HA_ROOT_CA_PEM
#define HA_ROOT_CA_PEM ""
#endif

namespace {

enum class Page : uint8_t {
  kOverview = 0,
  kDetails = 1,
  kTotals = 2,
  kGrid = 3,
};

struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;

  bool contains(int16_t px, int16_t py) const {
    return px >= x && px < (x + w) && py >= y && py < (y + h);
  }
};

struct Reading {
  const char *entity_id;
  float value = 0.0f;
  bool available = false;

  Reading() = default;
  explicit Reading(const char *id) : entity_id(id) {}
};

struct DashboardState {
  Reading solar_power{Config::kSolarPowerEntity};
  Reading house_power{Config::kHousePowerEntity};
  Reading solar_day_energy{Config::kSolarDayEnergyEntity};
  Reading house_day_energy{Config::kHouseDayEnergyEntity};
  Reading grid_import_power{Config::kGridImportPowerEntity};
  Reading grid_export_power{Config::kGridExportPowerEntity};
  Reading grid_import_day_energy{Config::kGridImportDayEnergyEntity};
  Reading grid_export_day_energy{Config::kGridExportDayEnergyEntity};
};

DashboardState g_state;
Page g_active_page = Page::kOverview;
uint32_t g_last_poll_ms = 0;
uint32_t g_last_history_ms = 0;
uint32_t g_last_success_ms = 0;
uint32_t g_last_touch_ms = 0;
String g_last_error;
bool g_needs_redraw = true;
bool g_display_sleeping = false;
bool g_ignore_touch_until_release = false;
float g_solar_history[Config::kHistoryPoints] = {0.0f};
float g_house_history[Config::kHistoryPoints] = {0.0f};
size_t g_history_count = 0;

constexpr uint8_t kActiveBrightness = 128;
constexpr uint8_t kSleepBrightness = 1;
constexpr uint32_t kDisplayTimeoutMs = 60000;

constexpr Rect kTabs[] = {
    {0, 214, 80, 26},
    {80, 214, 80, 26},
    {160, 214, 80, 26},
    {240, 214, 80, 26},
};

uint16_t rgb(uint8_t red, uint8_t green, uint8_t blue) {
  return M5.Display.color565(red, green, blue);
}

uint16_t color_bg() { return rgb(14, 20, 27); }
uint16_t color_panel() { return rgb(23, 32, 42); }
uint16_t color_grid() { return rgb(80, 96, 111); }
uint16_t color_text() { return rgb(243, 246, 248); }
uint16_t color_solar() { return rgb(247, 181, 0); }
uint16_t color_house() { return rgb(83, 197, 255); }
uint16_t color_sum() { return rgb(154, 205, 50); }
uint16_t color_warn() { return rgb(255, 122, 89); }
uint16_t color_import() { return rgb(255, 94, 77); }
uint16_t color_export() { return rgb(72, 217, 168); }

void set_text(uint8_t size, uint16_t fg, uint16_t bg = 0) {
  M5.Display.setTextSize(size);
  M5.Display.setTextColor(fg, bg == 0 ? color_bg() : bg);
}

void draw_label_value(int16_t x, int16_t y, const char *label, const Reading &reading,
                      const char *unit, uint16_t accent, uint8_t value_size = 2) {
  set_text(1, accent);
  M5.Display.setCursor(x, y);
  M5.Display.print(label);

  set_text(value_size, color_text());
  M5.Display.setCursor(x, y + 18);
  if (reading.available) {
    M5.Display.printf(value_size > 1 ? "%.1f %s" : "%.2f %s", reading.value, unit);
  } else {
    M5.Display.print("n/v");
  }
}

void draw_split_value(int16_t x, int16_t y, const char *label, float value, const char *unit,
                      const char *suffix, uint16_t accent, uint8_t value_size = 1) {
  set_text(1, accent);
  M5.Display.setCursor(x, y);
  M5.Display.print(label);

  set_text(value_size, color_text());
  M5.Display.setCursor(x, y + 16);
  if (value_size > 1) {
    M5.Display.printf("%.0f %s", value, unit);
  } else {
    M5.Display.printf("%.2f %s", value, unit);
  }

  if (suffix != nullptr && suffix[0] != '\0') {
    set_text(1, color_text());
    M5.Display.setCursor(x, y + 30);
    M5.Display.print(suffix);
  }
}

void draw_stack_metric(int16_t x, int16_t y, const char *label, float value, const char *unit,
                       uint16_t accent, uint8_t value_size = 2) {
  set_text(1, accent);
  M5.Display.setCursor(x, y);
  M5.Display.print(label);

  set_text(value_size, color_text());
  M5.Display.setCursor(x, y + 18);
  if (value_size > 1) {
    M5.Display.printf("%.0f %s", value, unit);
  } else {
    M5.Display.printf("%.2f %s", value, unit);
  }
}

void draw_card(const Rect &rect, uint16_t accent) {
  M5.Display.fillRoundRect(rect.x, rect.y, rect.w, rect.h, 10, color_panel());
  M5.Display.drawRoundRect(rect.x, rect.y, rect.w, rect.h, 10, accent);
}

void draw_gauge(int16_t cx, int16_t cy, int16_t radius, const char *title, const Reading &reading,
                uint16_t accent) {
  Rect card{static_cast<int16_t>(cx - radius - 8), static_cast<int16_t>(cy - radius - 18),
            static_cast<int16_t>(radius * 2 + 16), static_cast<int16_t>(radius * 2 + 36)};
  draw_card(card, accent);

  set_text(1, accent);
  M5.Display.setCursor(cx - radius, cy - radius - 12);
  M5.Display.print(title);

  M5.Display.drawCircle(cx, cy, radius, color_grid());
  M5.Display.drawCircle(cx, cy, radius - 1, color_grid());
  M5.Display.fillCircle(cx, cy, 4, accent);

  for (int i = 0; i <= 5; ++i) {
    float tick_ratio = static_cast<float>(i) / 5.0f;
    float tick_deg = 150.0f + tick_ratio * 240.0f;
    float tick_rad = tick_deg * 0.0174532925f;
    int16_t x1 = cx + static_cast<int16_t>((radius - 6) * cosf(tick_rad));
    int16_t y1 = cy + static_cast<int16_t>((radius - 6) * sinf(tick_rad));
    int16_t x2 = cx + static_cast<int16_t>(radius * cosf(tick_rad));
    int16_t y2 = cy + static_cast<int16_t>(radius * sinf(tick_rad));
    M5.Display.drawLine(x1, y1, x2, y2, color_grid());
  }

  float value = reading.available ? reading.value : 0.0f;
  if (value < 0.0f) {
    value = 0.0f;
  }
  if (value > Config::kPowerScaleMaxW) {
    value = Config::kPowerScaleMaxW;
  }

  float ratio = value / Config::kPowerScaleMaxW;
  float angle_deg = 150.0f + ratio * 240.0f;
  float angle_rad = angle_deg * 0.0174532925f;
  int16_t nx = cx + static_cast<int16_t>((radius - 10) * cosf(angle_rad));
  int16_t ny = cy + static_cast<int16_t>((radius - 10) * sinf(angle_rad));
  M5.Display.drawLine(cx, cy, nx, ny, accent);

  set_text(2, color_text());
  M5.Display.setCursor(cx - 38, cy + radius + 6);
  if (reading.available) {
    M5.Display.printf("%.0f W", reading.value);
  } else {
    M5.Display.print("n/v");
  }
}

void draw_history_row(int16_t x, int16_t y, const char *label, uint16_t accent, const float *values) {
  constexpr int16_t kBarWidth = 10;
  constexpr int16_t kBarGap = 4;
  constexpr int16_t kBarHeight = 28;
  constexpr int16_t kLabelWidth = 28;
  constexpr int16_t kBarOffset = 20;

  set_text(1, accent);
  M5.Display.setCursor(x, y + 10);
  M5.Display.print(label);

  for (size_t i = 0; i < Config::kHistoryPoints; ++i) {
    float value = values[i];
    if (value < 0.0f) {
      value = 0.0f;
    }
    if (value > Config::kPowerScaleMaxW) {
      value = Config::kPowerScaleMaxW;
    }
    int16_t fill_height = static_cast<int16_t>((value / Config::kPowerScaleMaxW) * kBarHeight);
    int16_t bar_x = x + kLabelWidth + kBarOffset + static_cast<int16_t>(i) * (kBarWidth + kBarGap);
    int16_t bar_y = y + (kBarHeight - fill_height);

    M5.Display.drawRect(bar_x, y, kBarWidth, kBarHeight, color_grid());
    if (fill_height > 0) {
      int16_t inner_height = fill_height > 2 ? fill_height - 2 : 1;
      M5.Display.fillRect(bar_x + 1, bar_y + 1, kBarWidth - 2, inner_height, accent);
    }
  }
}

void draw_progress(int16_t x, int16_t y, int16_t w, int16_t h, float value, uint16_t accent,
                   const char *label) {
  if (value < 0.0f) {
    value = 0.0f;
  }
  if (value > Config::kPowerScaleMaxW) {
    value = Config::kPowerScaleMaxW;
  }
  int16_t fill = static_cast<int16_t>((value / Config::kPowerScaleMaxW) * w);

  set_text(1, accent);
  M5.Display.setCursor(x, y - 16);
  M5.Display.print(label);
  M5.Display.drawRect(x, y, w, h, color_grid());
  if (fill > 0) {
    int16_t inner_width = fill > 2 ? fill - 2 : 1;
    M5.Display.fillRect(x + 1, y + 1, inner_width, h - 2, accent);
  }
}

void draw_tab(size_t index, const char *label, uint16_t accent) {
  const bool active = static_cast<size_t>(g_active_page) == index;
  const Rect &rect = kTabs[index];
  const uint16_t fill = active ? accent : color_panel();
  const uint16_t border = active ? accent : color_grid();
  const uint16_t text = active ? color_bg() : color_text();

  M5.Display.fillRect(rect.x, rect.y, rect.w, rect.h, fill);
  M5.Display.drawRect(rect.x, rect.y, rect.w, rect.h, border);
  set_text(1, text, fill);
  M5.Display.setCursor(rect.x + 12, rect.y + 8);
  M5.Display.print(label);
}

void draw_status() {
  const int battery_level = M5.Power.getBatteryLevel();

  if (g_active_page == Page::kOverview && battery_level >= 0) {
    set_text(1, color_text());
    const String battery_text = String("Akku ") + battery_level + "%";
    const int16_t battery_x = 318 - static_cast<int16_t>(battery_text.length()) * 6;
    M5.Display.setCursor(battery_x, 8);
    M5.Display.print(battery_text);
  }

  if (WiFi.status() == WL_CONNECTED && WiFi.RSSI() <= -70) {
    set_text(1, color_text());
    M5.Display.setCursor(196, 24);
    M5.Display.printf("WiFi %d dBm", WiFi.RSSI());
  }

  set_text(1, color_text());
  M5.Display.setCursor(196, 40);
  if (g_last_success_ms > 0) {
    const uint32_t age = (millis() - g_last_success_ms) / 1000;
    M5.Display.printf("Sync %lus", static_cast<unsigned long>(age));
  } else {
    M5.Display.print("Sync offen");
  }
}

void draw_overview_page() {
  draw_gauge(82, 78, 42, "Solar", g_state.solar_power, color_solar());
  draw_gauge(238, 78, 42, "Verbrauch", g_state.house_power, color_house());
  draw_history_row(12, 158, "Solar", color_solar(), g_solar_history);
  draw_history_row(12, 196, "Haus", color_house(), g_house_history);
}

void draw_details_page() {
  const Rect left_top{12, 12, 142, 64};
  const Rect right_top{166, 12, 142, 64};
  const Rect left_mid{12, 88, 142, 54};
  const Rect right_mid{166, 88, 142, 108};
  const Rect status{12, 154, 142, 52};
  draw_card(left_top, color_solar());
  draw_card(right_top, color_house());
  draw_card(left_mid, color_sum());
  draw_card(right_mid, color_warn());
  draw_card(status, color_grid());

  draw_label_value(20, 20, "Solar aktuell", g_state.solar_power, "W", color_solar(), 2);
  draw_label_value(174, 20, "Verbrauch", g_state.house_power, "W", color_house(), 2);

  Reading net_balance{};
  if (g_state.solar_power.available && g_state.house_power.available) {
    net_balance.available = true;
    net_balance.value = g_state.solar_power.value - g_state.house_power.value;
  }

  set_text(1, color_sum());
  M5.Display.setCursor(20, 96);
  M5.Display.print("Bilanz jetzt");
  if (net_balance.available) {
    const float balance_value = fabsf(net_balance.value);
    set_text(2, color_text());
    M5.Display.setCursor(20, 114);
    if (net_balance.value >= 0.0f) {
      M5.Display.printf("+%.0f W", balance_value);
      set_text(1, color_text());
      M5.Display.setCursor(20, 130);
      M5.Display.print("Export");
    } else {
      M5.Display.printf("%.0f W", balance_value);
      set_text(1, color_text());
      M5.Display.setCursor(20, 130);
      M5.Display.print("Bezug");
    }
  } else {
    set_text(2, color_text());
    M5.Display.setCursor(20, 114);
    M5.Display.print("n/v");
  }

  float solar_peak = 0.0f;
  float house_peak = 0.0f;
  for (size_t i = 0; i < Config::kHistoryPoints; ++i) {
    if (g_solar_history[i] > solar_peak) {
      solar_peak = g_solar_history[i];
    }
    if (g_house_history[i] > house_peak) {
      house_peak = g_house_history[i];
    }
  }

  set_text(1, color_warn());
  M5.Display.setCursor(174, 96);
  M5.Display.print("Session Peaks");
  draw_stack_metric(174, 114, "PV max", solar_peak, "W", color_text(), 2);
  draw_stack_metric(174, 150, "Last", house_peak, "W", color_text(), 2);

  set_text(1, color_grid());
  M5.Display.setCursor(20, 162);
  M5.Display.print("REST Status");
  set_text(2, color_text());
  M5.Display.setCursor(20, 178);
  if (g_last_error.isEmpty()) {
    M5.Display.print("OK");
  } else {
    M5.Display.print("!");
  }
}

void draw_totals_page() {
  const Rect left_top{12, 12, 142, 82};
  const Rect right_top{166, 12, 142, 82};
  const Rect middle{12, 106, 296, 48};
  const Rect left_bottom{12, 166, 142, 40};
  const Rect right_bottom{166, 166, 142, 40};
  draw_card(left_top, color_solar());
  draw_card(right_top, color_house());
  draw_card(middle, color_grid());
  draw_card(left_bottom, color_solar());
  draw_card(right_bottom, color_house());

  draw_label_value(20, 22, "Solar Summe", g_state.solar_day_energy, "kWh", color_solar(), 2);
  draw_label_value(174, 22, "Haus Summe", g_state.house_day_energy, "kWh", color_house(), 2);

  Reading day_balance{};
  if (g_state.solar_day_energy.available && g_state.house_day_energy.available) {
    day_balance.available = true;
    day_balance.value = g_state.solar_day_energy.value - g_state.house_day_energy.value;
  }

  set_text(1, color_text());
  M5.Display.setCursor(20, 116);
  M5.Display.print("Tagesbilanz");
  set_text(2, color_text());
  M5.Display.setCursor(20, 132);
  if (day_balance.available) {
    M5.Display.printf("%.2f kWh", day_balance.value);
  } else {
    M5.Display.print("n/v");
  }

  float solar_peak = 0.0f;
  float house_peak = 0.0f;
  for (size_t i = 0; i < Config::kHistoryPoints; ++i) {
    if (g_solar_history[i] > solar_peak) {
      solar_peak = g_solar_history[i];
    }
    if (g_house_history[i] > house_peak) {
      house_peak = g_house_history[i];
    }
  }

  set_text(1, color_solar());
  M5.Display.setCursor(20, 176);
  M5.Display.print("Session PV Peak");
  set_text(2, color_text());
  M5.Display.setCursor(20, 190);
  M5.Display.printf("%.0f W", solar_peak);

  set_text(1, color_house());
  M5.Display.setCursor(174, 176);
  M5.Display.print("Session Last");
  set_text(2, color_text());
  M5.Display.setCursor(174, 190);
  M5.Display.printf("%.0f W", house_peak);
}

void draw_grid_page() {
  const Rect left_top{12, 12, 142, 70};
  const Rect right_top{166, 12, 142, 70};
  const Rect middle{12, 94, 296, 48};
  const Rect left_bottom{12, 154, 142, 52};
  const Rect right_bottom{166, 154, 142, 52};
  draw_card(left_top, color_import());
  draw_card(right_top, color_export());
  draw_card(middle, color_grid());
  draw_card(left_bottom, color_import());
  draw_card(right_bottom, color_export());

  draw_label_value(20, 20, "Netzbezug", g_state.grid_import_power, "W", color_import(), 2);
  draw_label_value(174, 20, "Einspeisung", g_state.grid_export_power, "W", color_export(), 2);

  draw_progress(20, 114, 130, 14,
                g_state.grid_import_power.available ? g_state.grid_import_power.value : 0.0f,
                color_import(), "Import Last");
  draw_progress(174, 114, 130, 14,
                g_state.grid_export_power.available ? g_state.grid_export_power.value : 0.0f,
                color_export(), "Export Last");

  if (g_state.grid_import_day_energy.available) {
    draw_split_value(20, 164, "Bezug heute", g_state.grid_import_day_energy.value, "kWh", "",
                     color_import(), 1);
  } else {
    set_text(1, color_import());
    M5.Display.setCursor(20, 164);
    M5.Display.print("Bezug heute");
    set_text(1, color_text());
    M5.Display.setCursor(20, 180);
    M5.Display.print("n/v");
  }

  if (g_state.grid_export_day_energy.available) {
    draw_split_value(174, 164, "Einspeisung heute", g_state.grid_export_day_energy.value, "kWh", "",
                     color_export(), 1);
  } else {
    set_text(1, color_export());
    M5.Display.setCursor(174, 164);
    M5.Display.print("Einspeisung heute");
    set_text(1, color_text());
    M5.Display.setCursor(174, 180);
    M5.Display.print("n/v");
  }
}

void draw_dashboard() {
  if (g_display_sleeping) {
    return;
  }

  M5.Display.fillScreen(color_bg());
  draw_status();

  switch (g_active_page) {
    case Page::kOverview:
      draw_overview_page();
      break;
    case Page::kDetails:
      draw_details_page();
      break;
    case Page::kTotals:
      draw_totals_page();
      break;
    case Page::kGrid:
      draw_grid_page();
      break;
  }

  draw_tab(0, "Ueber", color_solar());
  draw_tab(1, "Detail", color_house());
  draw_tab(2, "Summen", color_sum());
  draw_tab(3, "Netz", color_import());
}

bool is_valid_state_text(const char *value) {
  if (value == nullptr || value[0] == '\0') {
    return false;
  }
  return strcmp(value, "unknown") != 0 && strcmp(value, "unavailable") != 0;
}

bool fetch_entity_state(const char *entity_id, float &result) {
  if (entity_id == nullptr || entity_id[0] == '\0' || WiFi.status() != WL_CONNECTED) {
    if (entity_id != nullptr && entity_id[0] != '\0' && WiFi.status() != WL_CONNECTED) {
      Serial.printf("[REST] skip %s, WiFi offline\n", entity_id);
    }
    return false;
  }

  const String url = String(HA_BASE_URL) + "/api/states/" + entity_id;
  HTTPClient http;
  int status_code = -1;
  const bool use_https = String(HA_BASE_URL).startsWith("https://");
  WiFiClient client;
  WiFiClientSecure secure_client;

  if (use_https) {
    if (HA_ALLOW_INSECURE_TLS) {
      secure_client.setInsecure();
    } else if (strlen(HA_ROOT_CA_PEM) > 0) {
      secure_client.setCACert(HA_ROOT_CA_PEM);
    } else {
      g_last_error = "TLS ohne CA";
      Serial.printf("[REST] %s failed: %s\n", entity_id, g_last_error.c_str());
      return false;
    }

    if (!http.begin(secure_client, url)) {
      g_last_error = "HTTP begin fehl";
      Serial.printf("[REST] %s failed: %s\n", entity_id, g_last_error.c_str());
      return false;
    }
  } else {
    if (!http.begin(client, url)) {
      g_last_error = "HTTP begin fehl";
      Serial.printf("[REST] %s failed: %s\n", entity_id, g_last_error.c_str());
      return false;
    }
  }

  http.setConnectTimeout(1500);
  http.setTimeout(1500);
  http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
  http.addHeader("Content-Type", "application/json");
  status_code = http.GET();
  if (status_code != 200) {
    g_last_error = "REST " + String(status_code);
    Serial.printf("[REST] %s failed: %s\n", entity_id, g_last_error.c_str());
    http.end();
    return false;
  }

  const String body = http.getString();
  http.end();

  const String state_token = "\"state\":\"";
  const int state_start = body.indexOf(state_token);
  if (state_start < 0) {
    g_last_error = "State fehlt";
    Serial.printf("[REST] %s failed: %s (token)\n", entity_id, g_last_error.c_str());
    return false;
  }

  const int value_start = state_start + state_token.length();
  const int value_end = body.indexOf('"', value_start);
  if (value_end < 0) {
    g_last_error = "State fehlt";
    Serial.printf("[REST] %s failed: %s (end)\n", entity_id, g_last_error.c_str());
    return false;
  }

  const String state = body.substring(value_start, value_end);
  if (!is_valid_state_text(state.c_str())) {
    g_last_error = "State fehlt";
    Serial.printf("[REST] %s failed: %s (%s)\n", entity_id, g_last_error.c_str(), state.c_str());
    return false;
  }

  result = strtof(state.c_str(), nullptr);
  return true;
}

bool update_reading(Reading &reading) {
  float value = 0.0f;
  if (!fetch_entity_state(reading.entity_id, value)) {
    reading.available = false;
    return false;
  }
  reading.value = value;
  reading.available = true;
  return true;
}

void sample_history() {
  if (!g_state.solar_power.available || !g_state.house_power.available) {
    return;
  }

  if (g_history_count < Config::kHistoryPoints) {
    g_solar_history[g_history_count] = g_state.solar_power.value;
    g_house_history[g_history_count] = g_state.house_power.value;
    ++g_history_count;
    return;
  }

  for (size_t i = 1; i < Config::kHistoryPoints; ++i) {
    g_solar_history[i - 1] = g_solar_history[i];
    g_house_history[i - 1] = g_house_history[i];
  }
  g_solar_history[Config::kHistoryPoints - 1] = g_state.solar_power.value;
  g_house_history[Config::kHistoryPoints - 1] = g_state.house_power.value;
}

void refresh_data() {
  Serial.println("[REST] refresh start");
  bool any_success = false;
  any_success |= update_reading(g_state.solar_power);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.house_power);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.solar_day_energy);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.house_day_energy);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.grid_import_power);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.grid_export_power);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.grid_import_day_energy);
  M5.update();
  delay(100);
  any_success |= update_reading(g_state.grid_export_day_energy);

  if (any_success) {
    g_last_success_ms = millis();
    g_last_error = "";
    Serial.printf("[REST] refresh ok solar=%.2fW house=%.2fW import=%.2fW export=%.2fW\n",
                  g_state.solar_power.available ? g_state.solar_power.value : -1.0f,
                  g_state.house_power.available ? g_state.house_power.value : -1.0f,
                  g_state.grid_import_power.available ? g_state.grid_import_power.value : -1.0f,
                  g_state.grid_export_power.available ? g_state.grid_export_power.value : -1.0f);
  } else {
    Serial.printf("[REST] refresh failed: %s\n", g_last_error.c_str());
  }
  g_needs_redraw = true;
}

void ensure_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.printf("[WiFi] connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - started) < 15000) {
    M5.update();
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] connected, ip=%s rssi=%d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    Serial.printf("[WiFi] connect failed, status=%d\n", WiFi.status());
  }

  g_needs_redraw = true;
}

void sleep_display() {
  if (g_display_sleeping) {
    return;
  }

  Serial.println("[UI] display sleep");
  M5.Display.setBrightness(kSleepBrightness);
  g_display_sleeping = true;
}

void wake_display() {
  if (!g_display_sleeping) {
    return;
  }

  Serial.println("[UI] display wake");
  M5.Display.setBrightness(kActiveBrightness);
  g_display_sleeping = false;
  g_needs_redraw = true;
}

void update_display_timeout() {
  if (!g_display_sleeping && (millis() - g_last_touch_ms) >= kDisplayTimeoutMs) {
    sleep_display();
  }
}

void handle_touch() {
  auto detail = M5.Touch.getDetail(0);

  if (g_ignore_touch_until_release) {
    if (!detail.isPressed()) {
      g_ignore_touch_until_release = false;
    }
    return;
  }

  const bool touch_active = detail.wasPressed() || detail.isPressed() || detail.wasReleased();
  if (!touch_active) {
    return;
  }

  g_last_touch_ms = millis();

  if (g_display_sleeping) {
    wake_display();
    g_ignore_touch_until_release = true;
    return;
  }

  if (!detail.wasPressed()) {
    return;
  }

  for (size_t i = 0; i < (sizeof(kTabs) / sizeof(kTabs[0])); ++i) {
    if (kTabs[i].contains(detail.x, detail.y)) {
      g_active_page = static_cast<Page>(i);
      g_needs_redraw = true;
      return;
    }
  }
}

}  // namespace

void setup() {
  auto config = M5.config();
  config.clear_display = true;
  config.serial_baudrate = 115200;
  M5.begin(config);
  Serial.println("[BOOT] Core2 REST dashboard starting");

  M5.Display.setRotation(1);
  M5.Display.setBrightness(kActiveBrightness);
  M5.Display.fillScreen(color_bg());
  g_last_touch_ms = millis();

  ensure_wifi();
  refresh_data();
  sample_history();
  g_last_history_ms = millis();
  draw_dashboard();
}

void loop() {
  M5.update();
  handle_touch();
  update_display_timeout();
  ensure_wifi();

  const uint32_t now = millis();
  if ((now - g_last_poll_ms) >= Config::kPollIntervalMs) {
    g_last_poll_ms = now;
    refresh_data();
  }

  if ((now - g_last_history_ms) >= Config::kHistorySampleIntervalMs) {
    g_last_history_ms = now;
    sample_history();
    g_needs_redraw = true;
  }

  if (g_needs_redraw) {
    draw_dashboard();
    g_needs_redraw = false;
  }

  delay(20);
}
