/*
 * Copyright (C) 2015 The CyanogenMod Project
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

package com.cyanogenmod.settings.onepluscontrol.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class Constants {

    // Proc nodes
    public static final String KEY_SWAP_NODE = "/proc/s1302/key_rep";

    // Notification slider
    public static final String KEYCODE_SLIDER_TOP = "/sys/devices/soc.0/tri_state_key.82/keyCode_top";
    public static final String KEYCODE_SLIDER_MIDDLE = "/sys/devices/soc.0/tri_state_key.82/keyCode_middle";
    public static final String KEYCODE_SLIDER_BOTTOM = "/sys/devices/soc.0/tri_state_key.82/keyCode_bottom";

    public static void savePreferenceInt(Context context, String key, int value) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        preferences.edit().putInt(key, value).apply();
    }

    public static int getPreferenceInteger(Context context, String key, int defaultValue) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getInt(key, defaultValue);
    }
}
