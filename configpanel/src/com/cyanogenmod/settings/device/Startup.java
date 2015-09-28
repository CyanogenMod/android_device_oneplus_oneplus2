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

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.hardware.input.InputManager;
import android.os.SystemClock;
import android.view.InputDevice;
import android.view.InputEvent;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;

import java.io.File;

import com.cyanogenmod.settings.device.utils.Constants;
import com.cyanogenmod.settings.device.utils.FileUtils;

public class Startup extends BroadcastReceiver {
    @Override
    public void onReceive(final Context context, final Intent intent) {
        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            // Disable touchscreen gesture settings if needed
            if (!hasTouchscreenGestures()) {
                disableComponent(context, TouchscreenGestureSettings.class.getName());
            } else {
                enableComponent(context, TouchscreenGestureSettings.class.getName());
                // Restore nodes to saved preference values
                for (String pref : Constants.sNodePreferenceMap.keySet()) {
                    boolean defaultValue = Constants.sNodeDefaultMap.get(pref).booleanValue();
                    boolean value = Constants.isPreferenceEnabled(context, pref, defaultValue);
                    String node = Constants.sNodePreferenceMap.get(pref);
                    FileUtils.writeLine(node, value ? "1" : "0");
                }

                int keyCode_slider_top = Constants.getPreferenceInteger(context, "keycode_slider_top", 0);
                int keyCode_slider_middle = Constants.getPreferenceInteger(context, "keycode_slider_middle", 1);
                int keyCode_slider_bottom = Constants.getPreferenceInteger(context, "keycode_slider_bottom", 2);

                FileUtils.writeLine(Constants.KEYCODE_SLIDER_TOP, String.valueOf(keyCode_slider_top + 600));
                FileUtils.writeLine(Constants.KEYCODE_SLIDER_MIDDLE, String.valueOf(keyCode_slider_middle + 600));
                FileUtils.writeLine(Constants.KEYCODE_SLIDER_BOTTOM, String.valueOf(keyCode_slider_bottom + 600));
            }

        } else if (intent.getAction().equals("cyanogenmod.intent.action.GESTURE_CAMERA")) {
            long now = SystemClock.uptimeMillis();
            sendInputEvent(new KeyEvent(now, now, KeyEvent.ACTION_DOWN,
                    KeyEvent.KEYCODE_CAMERA, 0, 0,
                    KeyCharacterMap.VIRTUAL_KEYBOARD, 0, 0, InputDevice.SOURCE_KEYBOARD));
            sendInputEvent(new KeyEvent(now, now, KeyEvent.ACTION_UP,KeyEvent.KEYCODE_CAMERA,
                    0, 0, KeyCharacterMap.VIRTUAL_KEYBOARD, 0, 0, InputDevice.SOURCE_KEYBOARD));
        }
    }

    private void sendInputEvent(InputEvent event) {
        InputManager inputManager = InputManager.getInstance();
        inputManager.injectInputEvent(event,
                InputManager.INJECT_INPUT_EVENT_MODE_WAIT_FOR_FINISH);
    }

    private boolean hasTouchscreenGestures() {
        return new File(Constants.TOUCHSCREEN_CAMERA_NODE).exists() &&
            new File(Constants.TOUCHSCREEN_MUSIC_NODE).exists() &&
            new File(Constants.TOUCHSCREEN_FLASHLIGHT_NODE).exists();
    }

    private void disableComponent(Context context, String component) {
        ComponentName name = new ComponentName(context, component);
        PackageManager pm = context.getPackageManager();
        pm.setComponentEnabledSetting(name,
                PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);
    }

    private void enableComponent(Context context, String component) {
        ComponentName name = new ComponentName(context, component);
        PackageManager pm = context.getPackageManager();
        if (pm.getComponentEnabledSetting(name)
                == PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
            pm.setComponentEnabledSetting(name,
                    PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
        }
    }
}
