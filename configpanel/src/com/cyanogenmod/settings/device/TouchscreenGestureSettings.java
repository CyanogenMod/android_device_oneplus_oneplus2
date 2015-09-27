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

package com.cyanogenmod.settings.device;

import com.android.internal.util.cm.ScreenType;

import com.cyanogenmod.settings.device.utils.Constants;
import com.cyanogenmod.settings.device.utils.FileUtils;
import com.cyanogenmod.settings.device.utils.NodePreferenceActivity;

import android.os.Bundle;
import android.preference.SwitchPreference;

public class TouchscreenGestureSettings extends NodePreferenceActivity {

    private static final String NAV_SWITCH_NODE = "/proc/nav_switch";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.touchscreen_panel);

        SwitchPreference mKeySwap = (SwitchPreference) findPreference(Constants.KEY_SWAP_KEY);
        String nav_switch = FileUtils.readOneLine(NAV_SWITCH_NODE);
        if (nav_switch != null && nav_switch.contains(":0"))
            mKeySwap.setEnabled(false);
    }

    @Override
    protected void onResume() {
        super.onResume();

        // If running on a phone, remove padding around the listview
        if (!ScreenType.isTablet(this)) {
            getListView().setPadding(0, 0, 0, 0);
        }
    }

}
