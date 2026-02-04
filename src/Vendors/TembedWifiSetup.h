#pragma once

#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include <Arduino.h>
#include <Inputs/InputKeys.h>
#include <Interfaces/IDeviceView.h>

#define NVS_SSID_KEY "ssid"
#define NVS_PASS_KEY "pass"

#define DARK_GREY 0x4208

bool setupTembedWifi(IDeviceView& view);

#endif