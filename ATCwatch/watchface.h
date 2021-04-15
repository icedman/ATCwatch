#pragma once

typedef struct {
    uint8_t type;
    const char *description;

    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint8_t align;
    uint8_t sprite;

} watchface_gfx_t;

typedef struct {
    uint32_t offset;
    uint16_t size;
} watchface_gfx_spr_t;

static watchface_gfx_spr_t watchface_spr[64];
static watchface_gfx_t watchface_gfx[] = {
  { 0x10, "MonthName" },
  { 0x11, "Month" },
  { 0x12, "YearLeft" },
  { 0x30, "Day" },
  { 0x60, "DayOfWeek" },
  { 0x61, "DayOfWeekCZ" },
  { 0x40, "HoursDigit1" },
  { 0x41, "HoursDigit2" },
  { 0x43, "MinutesDigit1" },
  { 0x44, "MinutesDigit2" },
  { 0x45, "PMIcon" },
  { 0x46, "AMIcon" },
  { 0x70, "StepsProgress" },
  { 0x71, "StepsIcon" },
  { 0x72, "StepsLeft", 0 },
  { 0x73, "StepsCentered", 1 },
  { 0x74, "StepsRight", 2 },
  { 0x76, "StepsGoal" },
  { 0x80, "HeartRateProgress" },
  { 0x81, "HeartRateIcon" },
  { 0x82, "HeartRateLeft", 0 },
  { 0x83, "HeartRateCentered", 1 },
  { 0x84, "HeartRateRight", 2 },
  { 0x90, "CaloriesProgress" },
  { 0x91, "CaloriesIcon" },
  { 0x92, "CaloriesLeft", 0 },
  { 0x93, "CaloriesCentered", 1 },
  { 0x94, "CaloriesRight", 2 },
  { 0xa0, "DistanceProgress" },
  { 0xa1, "DistanceIcon" },
  { 0xa2, "DistanceLeft", 0 },
  { 0xa3, "DistanceCentered", 1 },
  { 0xa4, "DistanceRight", 2 },
  { 0xb1, "SleepIcon" },
  { 0xb2, "SleepHours" },
  { 0xb4, "SleepMinutes" },
  { 0xd1, "BatteryIcon" },
  { 0xd2, "BatteryPercentageLeft", 0 },
  { 0xd3, "BatteryPercentageCentered", 1 },
  { 0xd4, "BatteryPercentageRight", 2 },
  { 0xc0, "ConnectionStatusIcon" },
  { 0xf0, "Icon" },
  { 0x00, "" }
};

void watchface_init(void);

void watchface_clear();
void watchface_draw(void);