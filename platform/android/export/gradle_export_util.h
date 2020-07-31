/*************************************************************************/
/*  gradle_export_util.h                                                 */
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

#ifndef GODOT_GRADLE_EXPORT_UTIL_H
#define GODOT_GRADLE_EXPORT_UTIL_H

#include "core/io/zip_io.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "editor/editor_export.h"

enum AssetPackDeliveryMode {
	INSTALL_TIME,
	FAST_FOLLOW,
	ON_DEMAND,
};

typedef struct _AssetPackInfo {
	String file_path;
	AssetPackDeliveryMode delivery_mode;
	String name;
} AssetPackInfo;

const String godot_project_name_xml_string = R"(<?xml version="1.0" encoding="utf-8"?>
<!--WARNING: THIS FILE WILL BE OVERWRITTEN AT BUILD TIME-->
<resources>
	<string name="godot_project_name_string">%s</string>
</resources>
)";

// Utility method used to create a directory.
Error create_directory(const String &p_dir) {
	if (!DirAccess::exists(p_dir)) {
		DirAccess *filesystem_da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
		ERR_FAIL_COND_V_MSG(!filesystem_da, ERR_CANT_CREATE, "Cannot create directory '" + p_dir + "'.");
		Error err = filesystem_da->make_dir_recursive(p_dir);
		ERR_FAIL_COND_V_MSG(err, ERR_CANT_CREATE, "Cannot create directory '" + p_dir + "'.");
		memdelete(filesystem_da);
	}
	return OK;
}

// Implementation of EditorExportSaveSharedObject.
// This method will only be called as an input to export_project_files.
// This method lets the .so files for all ABIs to be copied
// into the gradle project from the .AAR file
Error ignore_so_file(void *p_userdata, const SharedObject &p_so) {
	return OK;
}

// Writes p_data into a file at p_path, creating directories if necessary.
// Note: this will overwrite the file at p_path if it already exists.
Error store_file_at_path(const String &p_path, const Vector<uint8_t> &p_data) {
	String dir = p_path.get_base_dir();
	Error err = create_directory(dir);
	if (err != OK) {
		return err;
	}
	FileAccess *fa = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(!fa, ERR_CANT_CREATE, "Cannot create file '" + p_path + "'.");
	fa->store_buffer(p_data.ptr(), p_data.size());
	memdelete(fa);
	return OK;
}

// Writes string p_data into a file at p_path, creating directories if necessary.
// Note: this will overwrite the file at p_path if it already exists.
Error store_string_at_path(const String &p_path, const String &p_data) {
	String dir = p_path.get_base_dir();
	Error err = create_directory(dir);
	if (err != OK) {
		return err;
	}
	FileAccess *fa = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(!fa, ERR_CANT_CREATE, "Cannot create file '" + p_path + "'.");
	fa->store_string(p_data);
	memdelete(fa);
	return OK;
}

// Implementation of EditorExportSaveFunction.
// This method will only be called as an input to export_project_files.
// It is used by the export_project_files method to save all the asset files into the gradle project.
// It's functionality mirrors that of the method save_apk_file.
// This method will be called ONLY when custom build is enabled.
Error rename_and_store_file_in_gradle_project(void *p_userdata, const String &p_path, const Vector<uint8_t> &p_data, int p_file, int p_total) {
	String dst_path = p_path.replace_first("res://", "res://android/build/assets/");
	Error err = store_file_at_path(dst_path, p_data);
	return err;
}

// Creates strings.xml files inside the gradle project for different locales.
Error _create_project_name_strings_files(const Ref<EditorExportPreset> &p_preset, const String &project_name) {
	// Stores the string into the default values directory.
	String processed_default_xml_string = vformat(godot_project_name_xml_string, project_name.xml_escape(true));
	store_string_at_path("res://android/build/res/values/godot_project_name_string.xml", processed_default_xml_string);

	// Searches the Gradle project res/ directory to find all supported locales
	DirAccessRef da = DirAccess::open("res://android/build/res");
	if (!da) {
		return ERR_CANT_OPEN;
	}
	da->list_dir_begin();
	while (true) {
		String file = da->get_next();
		if (file == "") {
			break;
		}
		if (!file.begins_with("values-")) {
			// NOTE: This assumes all directories that start with "values-" are for localization.
			continue;
		}
		String locale = file.replace("values-", "").replace("-r", "_");
		String property_name = "application/config/name_" + locale;
		String locale_directory = "res://android/build/res/" + file + "/godot_project_name_string.xml";
		if (ProjectSettings::get_singleton()->has_setting(property_name)) {
			String locale_project_name = ProjectSettings::get_singleton()->get(property_name);
			String processed_xml_string = vformat(godot_project_name_xml_string, locale_project_name.xml_escape(true));
			store_string_at_path(locale_directory, processed_xml_string);
		} else {
			// TODO: Once the legacy build system is deprecated we don't need to have xml files for this else branch
			store_string_at_path(locale_directory, processed_default_xml_string);
		}
	}
	da->list_dir_end();
	return OK;
}

String bool_to_string(bool v) {
	return v ? "true" : "false";
}

String _get_gles_tag() {
	bool min_gles3 = ProjectSettings::get_singleton()->get("rendering/quality/driver/driver_name") == "GLES3" &&
					 !ProjectSettings::get_singleton()->get("rendering/quality/driver/fallback_to_gles2");
	return min_gles3 ? "    <uses-feature android:glEsVersion=\"0x00030000\" android:required=\"true\" />\n" : "";
}

String _get_screen_sizes_tag(const Ref<EditorExportPreset> &p_preset) {
	String manifest_screen_sizes = "    <supports-screens \n        tools:node=\"replace\"";
	String sizes[] = { "small", "normal", "large", "xlarge" };
	size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
	for (size_t i = 0; i < num_sizes; i++) {
		String feature_name = vformat("screen/support_%s", sizes[i]);
		String feature_support = bool_to_string(p_preset->get(feature_name));
		String xml_entry = vformat("\n        android:%sScreens=\"%s\"", sizes[i], feature_support);
		manifest_screen_sizes += xml_entry;
	}
	manifest_screen_sizes += " />\n";
	return manifest_screen_sizes;
}

String _get_xr_features_tag(const Ref<EditorExportPreset> &p_preset) {
	String manifest_xr_features;
	bool uses_xr = (int)(p_preset->get("xr_features/xr_mode")) == 1;
	if (uses_xr) {
		int dof_index = p_preset->get("xr_features/degrees_of_freedom"); // 0: none, 1: 3dof and 6dof, 2: 6dof
		if (dof_index == 1) {
			manifest_xr_features += "    <uses-feature tools:node=\"replace\" android:name=\"android.hardware.vr.headtracking\" android:required=\"false\" android:version=\"1\" />\n";
		} else if (dof_index == 2) {
			manifest_xr_features += "    <uses-feature tools:node=\"replace\" android:name=\"android.hardware.vr.headtracking\" android:required=\"true\" android:version=\"1\" />\n";
		}
		int hand_tracking_index = p_preset->get("xr_features/hand_tracking"); // 0: none, 1: optional, 2: required
		if (hand_tracking_index == 1) {
			manifest_xr_features += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"false\" />\n";
		} else if (hand_tracking_index == 2) {
			manifest_xr_features += "    <uses-feature tools:node=\"replace\" android:name=\"oculus.software.handtracking\" android:required=\"true\" />\n";
		}
	}
	return manifest_xr_features;
}

String _get_instrumentation_tag(const Ref<EditorExportPreset> &p_preset) {
	String package_name = p_preset->get("package/unique_name");
	String manifest_instrumentation_text = vformat(
			"    <instrumentation\n"
			"        tools:node=\"replace\"\n"
			"        android:name=\".GodotInstrumentation\"\n"
			"        android:icon=\"@mipmap/icon\"\n"
			"        android:label=\"@string/godot_project_name_string\"\n"
			"        android:targetPackage=\"%s\" />\n",
			package_name);
	return manifest_instrumentation_text;
}

String _get_plugins_tag(const String &plugins_names) {
	if (!plugins_names.empty()) {
		return vformat("    <meta-data tools:node=\"replace\" android:name=\"plugins\" android:value=\"%s\" />\n", plugins_names);
	} else {
		return "    <meta-data tools:node=\"remove\" android:name=\"plugins\" />\n";
	}
}

String _get_activity_tag(const Ref<EditorExportPreset> &p_preset) {
	bool uses_xr = (int)(p_preset->get("xr_features/xr_mode")) == 1;
	String orientation = (int)(p_preset->get("screen/orientation")) == 1 ? "portrait" : "landscape";
	String manifest_activity_text = vformat(
			"        <activity android:name=\"com.godot.game.GodotApp\" "
			"tools:replace=\"android:screenOrientation\" "
			"android:screenOrientation=\"%s\">\n",
			orientation);
	if (uses_xr) {
		String focus_awareness = bool_to_string(p_preset->get("xr_features/focus_awareness"));
		manifest_activity_text += vformat("            <meta-data tools:node=\"replace\" android:name=\"com.oculus.vr.focusaware\" android:value=\"%s\" />\n", focus_awareness);
	} else {
		manifest_activity_text += "            <meta-data tools:node=\"remove\" android:name=\"com.oculus.vr.focusaware\" />\n";
	}
	manifest_activity_text += "        </activity>\n";
	return manifest_activity_text;
}

String _get_application_tag(const Ref<EditorExportPreset> &p_preset, const String &plugins_names) {
	bool uses_xr = (int)(p_preset->get("xr_features/xr_mode")) == 1;
	String manifest_application_text =
			"    <application android:label=\"@string/godot_project_name_string\"\n"
			"        android:allowBackup=\"false\" tools:ignore=\"GoogleAppIndexingWarning\"\n"
			"        android:icon=\"@mipmap/icon\">)\n\n"
			"        <meta-data tools:node=\"remove\" android:name=\"xr_mode_metadata_name\" />\n";

	manifest_application_text += _get_plugins_tag(plugins_names);
	if (uses_xr) {
		manifest_application_text += "        <meta-data tools:node=\"replace\" android:name=\"com.samsung.android.vr.application.mode\" android:value=\"vr_only\" />\n";
	}
	manifest_application_text += _get_activity_tag(p_preset);
	manifest_application_text += "    </application>\n";
	return manifest_application_text;
}

/*
 * This method writes to the build.gradle and settings.gradle files in the root Gradle project.
 */
Error _update_root_project_with_asset_pack_info(Vector<AssetPackInfo> &p_asset_pack_info) {
	//TODO: completely overwrite the asset pack info in root directory settings.gradle and build.gradle file
	String settings_gradle_string = "//ASSET_PACK_INFO_START !!!DO NOT EDIT THIS LINE!!!\ninclude ':app'\n";
	String build_gradle_string = "//ASSET_PACK_INFO_START !!!DO NOT EDIT THIS LINE!!!\nassetPacks = [";
	for (int i = 0; i < p_asset_pack_info.size(); i++) {
		String name = p_asset_pack_info.get(i).name;
		settings_gradle_string += vformat("include ':%s'\n", name);
		build_gradle_string += vformat("\":%s\"", name);
		if (i < p_asset_pack_info.size() - 1) {
			build_gradle_string += ", ";
		}
	}
	build_gradle_string += "]\n";

	print_line(build_gradle_string); //TODO: delete unnecessary debugging print statements
	print_line(settings_gradle_string); //TODO: delete unnecessary debugging print statements

	Error err;
	String build_gradle_file_string = FileAccess::get_file_as_string("res://android/build/build.gradle", &err);
	String settings_gradle_file_string = FileAccess::get_file_as_string("res://android/build/settings.gradle", &err);
	String begin_tag = "//ASSET_PACK_INFO_START !!!DO NOT EDIT THIS LINE!!!";
	String end_tag = "//ASSET_PACK_INFO_END !!!DO NOT EDIT THIS LINE!!!";
	int b_start = build_gradle_file_string.find(begin_tag);
	int b_end = build_gradle_file_string.find(end_tag);
	int b_len = build_gradle_file_string.length();
	int s_start = settings_gradle_file_string.find(begin_tag);
	int s_end = settings_gradle_file_string.find(end_tag);
	int s_len = settings_gradle_file_string.length();
	String b_write = build_gradle_file_string.substr(0, b_start) +
					 build_gradle_string + build_gradle_file_string.substr(b_end, b_len - b_end);
	String s_write = settings_gradle_file_string.substr(0, s_start) +
					 settings_gradle_string + settings_gradle_file_string.substr(s_end, s_len - s_end);

	store_string_at_path("res://android/build/build.gradle", b_write);
	store_string_at_path("res://android/build/settings.gradle", s_write);

	return OK;
}

/*
 * This method recursively searches the res:// directory for any .pck files that may not have been
 * included in the Asset Pack Gradle module.
 */
Error _handle_pack_files(const String &path, const AssetPackInfo &packInfo) {
	print_line("path: res://" + path); //TODO: delete unnecessary debugging print statements
	DirAccess *da = DirAccess::open("res://" + path);
	if (!da) {
		return ERR_CANT_OPEN;
	}
	da->list_dir_begin();
	while (true) {
		String file = da->get_next();
		print_line(file); //TODO: delete unnecessary debugging print statements
		if (file == "") {
			break;
		}
		if (file == "." || file == "..") {
			continue;
		}
		if (file.ends_with(".pck") || file.ends_with(".zip")) {
			String dest_base_path = path.replace_first(packInfo.file_path, "");
			String dest = "res://android/build/" + packInfo.name + "/src/main/assets/" + dest_base_path + file;
			da->make_dir_recursive(dest.get_base_dir());
			print_line("destination: " + dest); //TODO: delete unnecessary debugging print statements
			da->copy(path + file, dest);
		} else if (da->dir_exists(file)) {
			Error err = _handle_pack_files(path + file + "/", packInfo);
			if (err != OK) {
				da->list_dir_end();
				return err;
			}
		}
	}
	da->list_dir_end();
	return OK;
}

/*
 * This method copies the compiled assets from the `res://android/build/assets/` directory to the
 * appropriate folder in the Asset Pack module
 */
Error _copy_asset_directory_files(const AssetPackInfo &packInfo) {
	DirAccess *da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
	String asset_path_from = "res://android/build/assets/" + packInfo.file_path;
	String asset_path_to = "res://android/build/" + packInfo.name + "/src/main/assets/";
	print_line(asset_path_to); //TODO: delete unnecessary debugging print statements
	print_line(asset_path_from); //TODO: delete unnecessary debugging print statements
	da->make_dir_recursive(asset_path_to);
	//	Error err = da->copy(asset_path_from, asset_path_to);
	Error err = da->rename(asset_path_from, asset_path_to);
	if (err != OK) {
		print_line("could not rename"); //TODO: delete unnecessary debugging print statements
		//		return err;
	}
	print_line("dealing with pack files now"); //TODO: delete unnecessary debugging print statements
	err = _handle_pack_files(packInfo.file_path, packInfo);

	return err;
}

/*
 * This method creates the Gradle Asset Pack module by writing the module's build.gradle file and
 * then calling `_copy_asset_directory_files()` which copies the assets from the appropriate
 * directories into the gradle module.
 */
Error _create_asset_pack_folder(const AssetPackInfo &packInfo) {
	String packName = packInfo.name;
	String deliveryType = "install-time"; //default
	if (packInfo.delivery_mode == INSTALL_TIME) {
		deliveryType = "install-time";
	}
	if (packInfo.delivery_mode == FAST_FOLLOW) {
		deliveryType = "fast-follow";
	}
	if (packInfo.delivery_mode == ON_DEMAND) {
		deliveryType = "on-demand";
	}
	String build_gradle_string = vformat(
			"apply plugin: 'com.android.asset-pack'\n\n"
			"assetPack {\n"
			"    packName = \"%s\"\n"
			"    dynamicDelivery {\n"
			"        deliveryType = \"%s\"\n"
			"    }\n"
			"}\n",
			packName, deliveryType);
	String build_gradle_path = vformat("res://android/build/%s/build.gradle", packName);
	store_string_at_path(build_gradle_path, build_gradle_string);
	Error err = _copy_asset_directory_files(packInfo);
	return err;
}

Error _read_asset_pack_config_file(const String config_filepath, Vector<AssetPackInfo> &r_asset_pack_info) {
	FileAccess *f = FileAccess::open(config_filepath, FileAccess::READ);
	if (f == NULL) {
		return ERR_DOES_NOT_EXIST;
	}
	while (!f->eof_reached()) {
		/*
		 * This while loop reads the file in chunks of three lines at a time, and it assumes that
		 * the file is formatted correctly. As you can see, there is no error checking here.
		 * The format of the asset pack config file must be very strictly enforced, or else this
		 * method will break.
		 *
		 * The first line is the path, the second line is the delivery mode, and the third line is
		 * the name of the asset pack.
		 */
		AssetPackInfo temp_info;

		String path = f->get_line().strip_edges();
		int mode = f->get_line().strip_edges().get(0) - 48; //48 = Ascii 0
		String asset_pack_name = f->get_line().strip_edges();

		temp_info.file_path = path;
		temp_info.delivery_mode = (AssetPackDeliveryMode)mode;
		temp_info.name = asset_pack_name;

		r_asset_pack_info.push_back(temp_info);
	}
	return OK;
}

Error _handle_asset_packs(const String config_filepath) {
	//TODO: check for errors
	Vector<AssetPackInfo> asset_pack_info;
	/*
	 * _read_asset_pack_config_file populates the asset_pack_info vector by parsing the contents of
	 * the asset pack config file.
	 */
	_read_asset_pack_config_file(config_filepath, asset_pack_info);
	for (int i = 0; i < asset_pack_info.size(); i++) {
		/*
		 * _create_asset_pack_folder creates the gradle module for each asset pack
		 */
		_create_asset_pack_folder(asset_pack_info.get(i));
	}
	/*
	 * _update_root_project_with_asset_pack_info updates the root gradle project's buid.gradle and
	 * settings.gradle files
	 */
	_update_root_project_with_asset_pack_info(asset_pack_info);
	return OK;
}

/*
 * This method deletes everything that is not part of the base Gradle project
 */
Error delete_asset_folders() {
	print_line("deleting asset folders"); //TODO: delete unnecessary debugging print statements
	DirAccessRef da = DirAccess::open("res://android/build");
	if (!da) {
		return ERR_CANT_OPEN;
	}
	da->list_dir_begin();

	String keep[] = { ".", "..", "res", "AndroidManifest.xml", "config.gradle",
		"libs", "gradle", "gradlew", ".gdignore", "build.gradle",
		"gradle.properties", "gradlew.bat", "settings.gradle", "src" };

	//TODO: Use a better data structure, generate this list dynamically by parsing the asset pack config file.
	while (true) {
		String file = da->get_next();
		if (file == "") {
			break;
		}
		bool need_to_delete = true;
		for (int i = 0; i < 14; i++) { //TODO: use a better data structure for faster lookup
			if (file == keep[i]) {
				need_to_delete = false;
			}
		}
		if (need_to_delete) {
			print_line("need to delete"); //TODO: delete unnecessary debugging print statements
			if (file.find(".") == -1) {
				DirAccessRef da = DirAccess::open("res://android/build/" + file);
				da->erase_contents_recursive();
			}
			da->remove(file);
		}
		print_line(file); //TODO: delete unnecessary debugging print statements
	}
	da->list_dir_end();
	return OK;
}
#endif //GODOT_GRADLE_EXPORT_UTIL_H
