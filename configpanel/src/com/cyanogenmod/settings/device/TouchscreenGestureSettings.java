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

import com.cyanogenmod.settings.device.utils.NodePreferenceActivity;

import org.cyanogenmod.internal.util.ScreenType;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.SwitchPreference;
import android.provider.Settings;

public class TouchscreenGestureSettings extends NodePreferenceActivity {

    private static final String KEY_HAPTIC_FEEDBACK = "touchscreen_gesture_haptic_feedback";

    private SwitchPreference mHapticFeedback;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.touchscreen_panel);

        mHapticFeedback = (SwitchPreference) findPreference(KEY_HAPTIC_FEEDBACK);
        mHapticFeedback.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference == mHapticFeedback) {
            final boolean value = (Boolean) newValue;
            Settings.System.putInt(getContentResolver(), KEY_HAPTIC_FEEDBACK, value ? 1 : 0);
            return true;
        }
        return super.onPreferenceChange(preference, newValue);
    }

    @Override
    protected void onResume() {
        super.onResume();

        // If running on a phone, remove padding around the listview
        if (!ScreenType.isTablet(this)) {
            getListView().setPadding(0, 0, 0, 0);
        }

        mHapticFeedback.setChecked(
                Settings.System.getInt(getContentResolver(), KEY_HAPTIC_FEEDBACK, 1) != 0);
    }
}
