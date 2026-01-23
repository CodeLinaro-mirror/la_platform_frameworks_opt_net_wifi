/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wifi_system/hostapd_manager.h"

#include <android-base/logging.h>
#include <cutils/properties.h>
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
#include "wifi_fst.h"
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)

namespace android {
namespace wifi_system {
const char kHostapdServiceName[] = "hostapd";
// QTI_BEGIN: 2018-12-18: WIGIG: Revert "libwifi_system: use "vendor" prefix for vendor services"
const char kHostapdFSTServiceName[] = "hostapd_fst";
// QTI_END: 2018-12-18: WIGIG: Revert "libwifi_system: use "vendor" prefix for vendor services"
// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
const char kHostapdFullServiceName[] = "init.svc.hostapd";
// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-12-18: WIGIG: Revert "libwifi_system: use "vendor" prefix for vendor services"
const char kHostapdFSTFullServiceName[] = "init.svc.hostapd_fst";
// QTI_END: 2018-12-18: WIGIG: Revert "libwifi_system: use "vendor" prefix for vendor services"

bool HostapdManager::StartHostapd() {
// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.

  // Check if hostapd already started
  char hostapd_status[PROPERTY_VALUE_MAX];
// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  property_get(is_fst_softap_enabled() ? kHostapdFSTFullServiceName :
    kHostapdFullServiceName, hostapd_status, "");
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.

  if (strcmp(hostapd_status, "running") == 0) {
    LOG(DEBUG) << "SoftAP already started. Skip another start";
    return true;
  }

// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  if (wifi_start_fstman(1)) {
    return false;
  }

  if (property_set("ctl.start",
                   is_fst_softap_enabled() ?
                     kHostapdFSTServiceName : kHostapdServiceName) != 0) {
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    LOG(ERROR) << "Failed to start SoftAP";
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    wifi_stop_fstman(1);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    return false;
  }

  LOG(DEBUG) << "SoftAP started successfully";
  return true;
}

bool HostapdManager::StopHostapd() {
  LOG(DEBUG) << "Stopping the SoftAP service...";

// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
  // Check if hostapd already stopped
  char hostapd_status[PROPERTY_VALUE_MAX];
// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  property_get(is_fst_softap_enabled() ? kHostapdFSTFullServiceName :
    kHostapdFullServiceName, hostapd_status, "");
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.

  if (!strlen(hostapd_status) || strcmp(hostapd_status, "stopped") == 0) {
    LOG(DEBUG) << "SoftAP already stopped. Skip another stop";
// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    wifi_stop_fstman(1);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
// QTI_BEGIN: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
    return true;
  }

// QTI_END: 2018-05-25: WLAN: libwifi_system: Skip duplicate hostapd service start/stop.
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  if (property_set("ctl.stop",
                   is_fst_softap_enabled() ?
                     kHostapdFSTServiceName : kHostapdServiceName) < 0) {
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    LOG(ERROR) << "Failed to stop hostapd service!";
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    wifi_stop_fstman(1);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    return false;
  }

// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  wifi_stop_fstman(1);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  LOG(DEBUG) << "SoftAP stopped successfully";
  return true;
}
}  // namespace wifi_system
}  // namespace android
