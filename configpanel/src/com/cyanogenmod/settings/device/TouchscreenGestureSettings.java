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

import java.io.File;

import com.cyanogenmod.settings.device.utils.Constants;
import com.cyanogenmod.settings.device.utils.FileUtils;
import com.cyanogenmod.settings.device.utils.NodePreferenceActivity;

import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.SwitchPreference;

public class TouchscreenGestureSettings extends NodePreferenceActivity {

    private static final String NAV_SWITCH_NODE = "/proc/nav_switch";

    private ListPreference mSliderTop;
    private ListPreference mSliderMiddle;
    private ListPreference mSliderBottom;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.touchscreen_panel);

        SwitchPreference mKeySwap = (SwitchPreference) findPreference(Constants.KEY_SWAP_KEY);
        String nav_switch = FileUtils.readOneLine(NAV_SWITCH_NODE);
        if (nav_switch != null && nav_switch.contains(":0"))
            mKeySwap.setEnabled(false);

        mSliderTop = (ListPreference) findPreference("keycode_slider_top");
        mSliderMiddle = (ListPreference) findPreference("keycode_slider_middle");
        mSliderBottom = (ListPreference) findPreference("keycode_slider_bottom");

        if (!new File(Constants.KEYCODE_SLIDER_TOP).exists()
                || !new File(Constants.KEYCODE_SLIDER_MIDDLE).exists()
                || !new File(Constants.KEYCODE_SLIDER_BOTTOM).exists()) {
            mSliderTop.setEnabled(false);
            mSliderMiddle.setEnabled(false);
            mSliderBottom.setEnabled(false);
            return;
        }

        setSummary(mSliderTop, Constants.KEYCODE_SLIDER_TOP);
        setSummary(mSliderMiddle, Constants.KEYCODE_SLIDER_MIDDLE);
        setSummary(mSliderBottom, Constants.KEYCODE_SLIDER_BOTTOM);
    }

    private void setSummary(ListPreference preference, String file) {
        String[] notiactions = getResources().getStringArray(R.array.notification_slider_action_entries);

        String keyCode = FileUtils.readOneLine(file);
        if (keyCode == null) return;
        int value = Integer.parseInt(keyCode);
        preference.setSummary(notiactions[value - 600]);
        preference.setValueIndex(value - 600);
        preference.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference instanceof SwitchPreference)
            return super.onPreferenceChange(preference, newValue);
        String file = null;
        int value = ((ListPreference) preference).findIndexOfValue((String) newValue);
        if (preference == mSliderTop) file = Constants.KEYCODE_SLIDER_TOP;
        else if (preference == mSliderMiddle) file = Constants.KEYCODE_SLIDER_MIDDLE;
        else if (preference == mSliderBottom) file = Constants.KEYCODE_SLIDER_BOTTOM;
        if (file == null) return false;

        FileUtils.writeLine(file, String.valueOf(value + 600));
        String[] notiactions = getResources().getStringArray(R.array.notification_slider_action_entries);
        preference.setSummary(notiactions[value]);

        Constants.savePreferenceInt(this, preference.getKey(), value);
        return true;
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
