#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Config {

constexpr uint32_t kPollIntervalMs = 15000;
constexpr uint32_t kHistorySampleIntervalMs = 300000;
constexpr size_t kHistoryPoints = 12;
constexpr float kPowerScaleMaxW = 5000.0f;

constexpr const char *kSolarPowerEntity = "sensor.core2_solar_live";
constexpr const char *kHousePowerEntity = "sensor.core2_house_live";
constexpr const char *kSolarDayEnergyEntity = "sensor.core2_solar_day_energy_kwh";
constexpr const char *kHouseDayEnergyEntity = "sensor.core2_house_day_energy_kwh";
constexpr const char *kGridImportPowerEntity = "sensor.core2_grid_import_power";
constexpr const char *kGridExportPowerEntity = "sensor.core2_grid_export_power";
constexpr const char *kGridImportDayEnergyEntity = "sensor.core2_grid_import_day_energy_kwh";
constexpr const char *kGridExportDayEnergyEntity = "sensor.core2_grid_export_day_energy_kwh";

}  // namespace Config
