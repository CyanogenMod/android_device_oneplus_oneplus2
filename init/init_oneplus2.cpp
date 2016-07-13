/*
   Copyright (c) 2015, The CyanogenMod Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

void init_variant_properties() {
    char device[PROP_VALUE_MAX];
    char rf_version[PROP_VALUE_MAX];
    int rc;

    rc = property_get("ro.cm.device", device);
    if (!rc || strncmp(device, "oneplus2", 8))
        return;

    property_get("ro.boot.rf_v1", rf_version);

    if (strstr(rf_version, "14")) {
        /* Chinese */
        property_set("ro.product.model", "ONE A2001");
        property_set("ro.rf_version", "TDD_FDD_Ch_All");
        property_set("telephony.lteOnCdmaDevice", "1");
        property_set("ro.telephony.default_network", "20,20");
    } else if (strstr(rf_version, "24")) {
        /* Asia/Europe */
        property_set("ro.product.model", "ONE A2003");
        property_set("ro.rf_version", "TDD_FDD_Eu");
        property_set("ro.telephony.default_network", "9,9");
    } else if (strstr(rf_version, "34")) {
        /* America */
        property_set("ro.product.model", "ONE A2005");
        property_set("ro.rf_version", "TDD_FDD_Am");
        property_set("telephony.lteOnCdmaDevice", "1");
        property_set("ro.telephony.default_network", "9,9");
    }
}

void vendor_load_properties() {
    init_variant_properties();
}
