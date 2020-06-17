/*************************************************************************/
/*  export.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef ANDROID_EXPORT_H
#define ANDROID_EXPORT_H

#include <core/ustring.h>

void register_android_exporter();

String ANDROID_MANIFEST_TEXT = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                               "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
                               "    xmlns:tools=\"http://schemas.android.com/tools\"\n"
                               "    package=\"PACKAGE_NAME_HERE\"\n"
                               "    android:versionCode=\"VERSION_CODE_HERE\"\n"
                               "    android:versionName=\"VERSION_NAME_HERE\"\n"
                               "    android:installLocation=\"auto\" >\n"
                               "\n"
                               "    <!-- Adding custom text to the manifest is fine, but do it outside the custom USER and APPLICATION BEGIN/END comments, -->\n"
                               "    <!-- as that gets rewritten. -->\n"
                               "\n"
                               "    <supports-screens\n"
                               "        android:smallScreens=\"SMALL_SCREENS_HERE\"\n"
                               "        android:normalScreens=\"NORMAL_SCREENS_HERE\"\n"
                               "        android:largeScreens=\"LARGE_SCREENS_HERE\"\n"
                               "        android:xlargeScreens=\"X_LARGE_SCREENS_HERE\" />\n"
                               "\n"
                               "    <!-- glEsVersion is modified by the exporter, changing this value here has no effect. -->\n"
                               "    <uses-feature\n"
                               "        android:glEsVersion=\"GLES_VERSION_HERE\"\n"
                               "        android:required=\"true\" />\n"
                               "\n"
                               "<!-- Custom user permissions XML added by add-ons. It's recommended to add them from the export preset, though. -->\n"
                               "<!--CHUNK_USER_PERMISSIONS_BEGIN-->\n"
                               "<!--CHUNK_USER_PERMISSIONS_END-->\n"
                               "\n"
                               "    <!-- Any tag in this line after android:icon will be erased when doing custom builds. -->\n"
                               "    <!-- If you want to add tags manually, do before it. -->\n"
                               "    <!-- WARNING: This should stay on a single line until the parsing code is improved. See GH-32414. -->\n"
                               "    <application android:label=\"@string/godot_project_name_string\" android:allowBackup=\"false\" tools:ignore=\"GoogleAppIndexingWarning\" android:icon=\"@mipmap/icon\" >\n"
                               "\n"
                               "        <!-- The following metadata values are replaced when Godot exports, modifying them here has no effect. -->\n"
                               "        <!-- Do these changes in the export preset. Adding new ones is fine. -->\n"
                               "\n"
                               "        <!-- XR mode metadata. This is modified by the exporter based on the selected xr mode. DO NOT CHANGE the values here. -->\n"
                               "        <meta-data\n" //
                               "            android:name=\"XR_MODE_METADATA_NAME\"\n"
                               "            android:value=\"XR_MODE_METADATA_VALUE\" />\n"
                               "\n"
                               "        <!-- Metadata populated at export time and used by Godot to figure out which plugins must be enabled. -->\n"
                               "        <meta-data\n"
                               "            android:name=\"PLUGINS_HERE\"\n"
                               "            android:value=\"PLUGINS_VALUES_HERE\"/>\n" //might need something for degrees of freedom here
                               "\n"
                               "        <activity\n"
                               "            android:name=\".GodotApp\"\n"
                               "            android:label=\"@string/godot_project_name_string\"\n"
                               "            android:theme=\"@android:style/Theme.Black.NoTitleBar.Fullscreen\"\n"
                               "            android:launchMode=\"singleTask\"\n"
                               "            android:screenOrientation=\"SCREEN_ORIENTATION_HERE\"\n"
                               "            android:configChanges=\"orientation|keyboardHidden|screenSize|smallestScreenSize|density|keyboard|navigation|screenLayout|uiMode\"\n"
                               "            android:resizeableActivity=\"false\"\n"
                               "            tools:ignore=\"UnusedAttribute\" >\n"
                               "\n"
                               "            <!-- Focus awareness metadata populated at export time if the user enables it in the 'Xr Features' section. -->\n"
                               "            <meta-data\n"
                               "                android:name=\"com.oculus.vr.focusaware\"\n" //possibly need to update this and the line below with the *_HERE tags
                               "                android:value=\"oculus_focus_aware_value\"/>\n" //might also need something for hand tracking
                               "\n"
                               "            <intent-filter>\n"
                               "                <action android:name=\"android.intent.action.MAIN\" />\n"
                               "                <category android:name=\"android.intent.category.LAUNCHER\" />\n"
                               "            </intent-filter>\n"
                               "        </activity>\n"
                               "\n"
                               "<!-- Custom application XML added by add-ons. -->\n"
                               "<!--CHUNK_APPLICATION_BEGIN-->\n"
                               "<!--CHUNK_APPLICATION_END-->\n"
                               "\n"
                               "    </application>\n"
                               "\n"
                               "</manifest>"

#endif // ANDROID_EXPORT_H
