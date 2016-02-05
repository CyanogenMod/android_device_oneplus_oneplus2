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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.cyanogenmod.settings.onepluscontrol.utils.Constants;
import com.cyanogenmod.settings.onepluscontrol.utils.FileUtils;

public class Startup extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
            int keyCode_swap_button = Constants.getPreferenceInteger(context, "keycode_swap_button", 0);

            FileUtils.writeLine(Constants.KEY_SWAP_NODE, String.valueOf(keyCode_swap_button));

            int keyCode_slider_top = Constants.getPreferenceInteger(context, "keycode_slider_top", 1);
            int keyCode_slider_middle = Constants.getPreferenceInteger(context, "keycode_slider_middle", 2);
            int keyCode_slider_bottom = Constants.getPreferenceInteger(context, "keycode_slider_bottom", 3);

            FileUtils.writeLine(Constants.KEYCODE_SLIDER_TOP, String.valueOf(keyCode_slider_top + 600));
            FileUtils.writeLine(Constants.KEYCODE_SLIDER_MIDDLE, String.valueOf(keyCode_slider_middle + 600));
            FileUtils.writeLine(Constants.KEYCODE_SLIDER_BOTTOM, String.valueOf(keyCode_slider_bottom + 600));
        }
    }

}
