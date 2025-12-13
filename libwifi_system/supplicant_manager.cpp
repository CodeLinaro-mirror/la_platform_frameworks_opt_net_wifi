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

#include "wifi_system/supplicant_manager.h"

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <sched.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <unistd.h>

// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
#include "wifi_fst.h"

// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
namespace android {
namespace wifi_system {
namespace {

const char kSupplicantInitProperty[] = "init.svc.wpa_supplicant";
const char kSupplicantServiceName[] = "wpa_supplicant";

}  // namespace

bool SupplicantManager::StartSupplicant() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  int count = 200; /* wait at most 20 seconds for completion */
  const prop_info* pi;
  unsigned serial = 0;

// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  if (wifi_start_fstman(0)) {
    return -1;
  }

// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  /* Check whether already running */
  if (property_get(kSupplicantInitProperty, supp_status, NULL) &&
      strcmp(supp_status, "running") == 0) {
    return true;
  }

  /*
   * Get a reference to the status property, so we can distinguish
   * the case where it goes stopped => running => stopped (i.e.,
   * it start up, but fails right away) from the case in which
   * it starts in the stopped state and never manages to start
   * running at all.
   */
  pi = __system_property_find(kSupplicantInitProperty);
  if (pi != NULL) {
    serial = __system_property_serial(pi);
  }

  property_set("ctl.start", kSupplicantServiceName);
  sched_yield();

  while (count-- > 0) {
    if (pi == NULL) {
      pi = __system_property_find(kSupplicantInitProperty);
    }
    if (pi != NULL) {
      /*
       * property serial updated means that init process is scheduled
       * after we sched_yield, further property status checking is based on this
       */
      if (__system_property_serial(pi) != serial) {
        __system_property_read(pi, NULL, supp_status);
        if (strcmp(supp_status, "running") == 0) {
          return true;
        } else if (strcmp(supp_status, "stopped") == 0) {
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
          wifi_stop_fstman(0);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
          return false;
        }
      }
    }
    usleep(100000);
  }
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  wifi_stop_fstman(0);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  return false;
}

bool SupplicantManager::StopSupplicant() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  int count = 200; /* wait at most 20 seconds for completion */

  /* Check whether supplicant already stopped */
  if (property_get(kSupplicantInitProperty, supp_status, NULL) &&
      strcmp(supp_status, "stopped") == 0) {
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    wifi_stop_fstman(0);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    return true;
  }

  property_set("ctl.stop", kSupplicantServiceName);
  sched_yield();

  while (count-- > 0) {
    if (property_get(kSupplicantInitProperty, supp_status, NULL)) {
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
      if (strcmp(supp_status, "stopped") == 0) {
        wifi_stop_fstman(0);
        return true;
      }
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
    }
    usleep(100000);
  }
  LOG(ERROR) << "Failed to stop supplicant";
// QTI_BEGIN: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  wifi_stop_fstman(0);
// QTI_END: 2018-06-08: WIGIG: frameworks/opt/net/wifi: add support for Fast Session Transfer (FST)
  return false;
}

bool SupplicantManager::IsSupplicantRunning() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  if (property_get(kSupplicantInitProperty, supp_status, NULL)) {
    return strcmp(supp_status, "running") == 0;
  }
  return false;  // Failed to read service status from init.
}

}  // namespace wifi_system
}  // namespace android
