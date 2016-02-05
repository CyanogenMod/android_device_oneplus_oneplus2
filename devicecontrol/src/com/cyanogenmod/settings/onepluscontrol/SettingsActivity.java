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

package com.cyanogenmod.settings.onepluscontrol;

import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.SwitchPreference;
import android.provider.Settings;
import android.view.MenuItem;

import com.cyanogenmod.settings.onepluscontrol.utils.Constants;
import com.cyanogenmod.settings.onepluscontrol.utils.FileUtils;

import org.cyanogenmod.internal.util.ScreenType;

import java.io.File;

public class SettingsActivity extends PreferenceActivity implements Preference.OnPreferenceChangeListener {

    private static final String NAV_SWITCH_NODE = "/proc/nav_switch";
    private static final String KEY_HAPTIC_FEEDBACK = "notification_slider_haptic_feedback";

    private SwitchPreference mSwapButton;
    private ListPreference mSliderTop;
    private ListPreference mSliderMiddle;
    private ListPreference mSliderBottom;
    private SwitchPreference mHapticFeedback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        addPreferencesFromResource(R.xml.settings_dashboard);

        mSwapButton = (SwitchPreference) findPreference("keycode_swap_button");
        mSliderTop = (ListPreference) findPreference("keycode_slider_top");
        mSliderMiddle = (ListPreference) findPreference("keycode_slider_middle");
        mSliderBottom = (ListPreference) findPreference("keycode_slider_bottom");
        mHapticFeedback = (SwitchPreference) findPreference("keycode_haptic_feedback");

        mSwapButton.setOnPreferenceChangeListener(this);
        mHapticFeedback.setOnPreferenceChangeListener(this);
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
        if (preference == mSwapButton) {
            FileUtils.writeLine(Constants.KEY_SWAP_NODE, (Boolean) newValue ? "1" : "0");
            Constants.savePreferenceInt(this, preference.getKey(), (Boolean) newValue ? 1 : 0);
            return true;
        } else if (preference == mHapticFeedback) {
            Settings.System.putInt(getContentResolver(), KEY_HAPTIC_FEEDBACK, (Boolean) newValue ? 1 : 0);
            return true;
        } else {
            String file;
            int value = ((ListPreference) preference).findIndexOfValue((String) newValue);
            if (preference == mSliderTop) file = Constants.KEYCODE_SLIDER_TOP;
            else if (preference == mSliderMiddle) file = Constants.KEYCODE_SLIDER_MIDDLE;
            else if (preference == mSliderBottom) file = Constants.KEYCODE_SLIDER_BOTTOM;
            else return false;

            FileUtils.writeLine(file, String.valueOf(value + 600));
            String[] notiactions = getResources().getStringArray(R.array.notification_slider_action_entries);
            preference.setSummary(notiactions[value]);

            Constants.savePreferenceInt(this, preference.getKey(), value);
            return true;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        // If running on a phone, remove padding around the listview
        if (!ScreenType.isTablet(this)) {
            getListView().setPadding(0, 0, 0, 0);
        }

        String nav_switch = FileUtils.readOneLine(NAV_SWITCH_NODE);
        if (nav_switch != null && nav_switch.contains(":0"))
            mSwapButton.setEnabled(false);
        String swap_button = FileUtils.readOneLine(Constants.KEY_SWAP_NODE);
        if (swap_button != null)
            mSwapButton.setChecked(swap_button.equals("1"));

        if (!new File(Constants.KEYCODE_SLIDER_TOP).exists()
                || !new File(Constants.KEYCODE_SLIDER_MIDDLE).exists()
                || !new File(Constants.KEYCODE_SLIDER_BOTTOM).exists()) {
            mSliderTop.setEnabled(false);
            mSliderMiddle.setEnabled(false);
            mSliderBottom.setEnabled(false);
        } else {
            setSummary(mSliderTop, Constants.KEYCODE_SLIDER_TOP);
            setSummary(mSliderMiddle, Constants.KEYCODE_SLIDER_MIDDLE);
            setSummary(mSliderBottom, Constants.KEYCODE_SLIDER_BOTTOM);
        }

        mHapticFeedback.setChecked(
                Settings.System.getInt(getContentResolver(), KEY_HAPTIC_FEEDBACK, 1) != 0);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            // Respond to the action bar's Up/Home button
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

}
