#Device tree for the OnePlus 2

###Syncing repo
Add this to your localmanifest
```xml
<manifest>
  <project path="device/oneplus/oneplus2" name="regalstreak/android_device_oneplus_oneplus2" remote="github" revision="cm-13.0" />
  <project path="kernel/oneplus/msm8994" name="regalstreak/android_kernel_oneplus_msm8994" remote="github" revision="cm-13.0" />
  <project path="vendor/oneplus" name="regalstreak/android_vendor_oneplus" remote="github" revision="cm-13.0" />
```

###Building
```bash
. build/envsetup.sh
lunch cm_oneplus2-(eng/userdebug/user)
make #recipe
```
