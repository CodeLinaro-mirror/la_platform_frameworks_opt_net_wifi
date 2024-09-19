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

/*
 * Changes from Qualcomm Innovation Center, Inc. are provided under the following license:
 *
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "wifi_system/supplicant_manager.h"
#include <rpc/util/log_common.h>
#include <stdlib.h>

#if 0
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <cutils/properties.h>

// This ugliness is necessary to access internal implementation details
// of the property subsystem.
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#endif

namespace android {
namespace wifi_system {
namespace {

#if 0
const char kSupplicantInitProperty[] = "init.svc.wpa_supplicant";
#endif

const char kSupplicantServiceName[] = "wpa_supplicant";

enum Supplicant_State {
	STOPPED,
	RUNNING
};

static Supplicant_State state = STOPPED;

}  // namespace


bool SupplicantManager::StartSupplicant() {
    int ret;

    if (state == RUNNING)
        return true;

    ret = system("wpa_supplicant -B");
    if (!ret) {
        ALOGD("Wpa_supplicant started successfully");
        state = RUNNING;
        return true;
    }

    return false;
}

bool SupplicantManager::StopSupplicant() {
    int ret;

    if (state == STOPPED)
        return true;

    ret = system("killall wpa_supplicant 1>/dev/null 2>/dev/null");
    if (!ret) {
        ALOGD("Wpa_supplicant stopped successfully");
        state = STOPPED;
        return true;
    }

    return false;
}

bool SupplicantManager::IsSupplicantRunning() {
    return state == RUNNING;
}

#if 0
bool SupplicantManager::StartSupplicant() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  int count = 200; /* wait at most 20 seconds for completion */
  const prop_info* pi;
  unsigned serial = 0;

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
          return false;
        }
      }
    }
    usleep(100000);
  }
  return false;
}

bool SupplicantManager::StopSupplicant() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  int count = 200; /* wait at most 20 seconds for completion */

  /* Check whether supplicant already stopped */
  if (property_get(kSupplicantInitProperty, supp_status, NULL) &&
      strcmp(supp_status, "stopped") == 0) {
    return true;
  }

  property_set("ctl.stop", kSupplicantServiceName);
  sched_yield();

  while (count-- > 0) {
    if (property_get(kSupplicantInitProperty, supp_status, NULL)) {
      if (strcmp(supp_status, "stopped") == 0) return true;
    }
    usleep(100000);
  }
  LOG(ERROR) << "Failed to stop supplicant";
  return false;
}

bool SupplicantManager::IsSupplicantRunning() {
  char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
  if (property_get(kSupplicantInitProperty, supp_status, NULL)) {
    return strcmp(supp_status, "running") == 0;
  }
  return false;  // Failed to read service status from init.
}
#endif

}  // namespace wifi_system
}  // namespace android
