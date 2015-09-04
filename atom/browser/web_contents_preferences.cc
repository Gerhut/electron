// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/web_contents_preferences.h"

#include "atom/common/options_switches.h"
#include "base/command_line.h"
#include "content/public/common/web_preferences.h"

#if defined(OS_WIN)
#include "ui/gfx/switches.h"
#endif

namespace atom {

namespace {

// Pointer as WebContents's user data key.
const char* kWebPreferencesKey = "WebContentsPreferences";

// Array of available web runtime features.
const char* kWebRuntimeFeatures[] = {
  switches::kExperimentalFeatures,
  switches::kExperimentalCanvasFeatures,
  switches::kSubpixelFontScaling,
  switches::kOverlayScrollbars,
  switches::kOverlayFullscreenVideo,
  switches::kSharedWorker,
  switches::kPageVisibility,
};

}  // namespace

WebContentsPreferences::WebContentsPreferences(
    content::WebContents* web_contents,
    base::DictionaryValue&& web_preferences) {
  web_preferences_.Swap(&web_preferences);
  web_contents->SetUserData(kWebPreferencesKey, this);
}

WebContentsPreferences::~WebContentsPreferences() {
}

// static
WebContentsPreferences* WebContentsPreferences::From(
    content::WebContents* web_contents) {
  return static_cast<WebContentsPreferences*>(
      web_contents->GetUserData(kWebPreferencesKey));
}

// static
void WebContentsPreferences::AppendExtraCommandLineSwitches(
    content::WebContents* web_contents, base::CommandLine* command_line) {
  WebContentsPreferences* self = From(web_contents);
  CHECK(self);

  bool b;
#if defined(OS_WIN)
  // Check if DirectWrite is disabled.
  if (self->web_preferences_.GetBoolean(switches::kDirectWrite, &b) && !b)
    command_line->AppendSwitch(::switches::kDisableDirectWrite);
#endif

  // Check if plugins are enabled.
  if (self->web_preferences_.GetBoolean("plugins", &b) && b)
    command_line->AppendSwitch(switches::kEnablePlugins);

  // This set of options are not availabe in WebPreferences, so we have to pass
  // them via command line and enable them in renderer procss.
  for (size_t i = 0; i < arraysize(kWebRuntimeFeatures); ++i) {
    const char* feature = kWebRuntimeFeatures[i];
    if (self->web_preferences_.GetBoolean(feature, &b))
      command_line->AppendSwitchASCII(feature, b ? "true" : "false");
  }
}

void WebContentsPreferences::OverrideWebkitPrefs(
    content::WebContents* web_contents, content::WebPreferences* prefs) {
  WebContentsPreferences* self = From(web_contents);
  CHECK(self);

  bool b;
  if (self->web_preferences_.GetBoolean("javascript", &b))
    prefs->javascript_enabled = b;
  if (self->web_preferences_.GetBoolean("images", &b))
    prefs->images_enabled = b;
  if (self->web_preferences_.GetBoolean("java", &b))
    prefs->java_enabled = b;
  if (self->web_preferences_.GetBoolean("text-areas-are-resizable", &b))
    prefs->text_areas_are_resizable = b;
  if (self->web_preferences_.GetBoolean("webgl", &b))
    prefs->experimental_webgl_enabled = b;
  if (self->web_preferences_.GetBoolean("webaudio", &b))
    prefs->webaudio_enabled = b;
  if (self->web_preferences_.GetBoolean("web-security", &b)) {
    prefs->web_security_enabled = b;
    prefs->allow_displaying_insecure_content = !b;
    prefs->allow_running_insecure_content = !b;
  }
  if (self->web_preferences_.GetBoolean("allow-displaying-insecure-content",
                                        &b))
    prefs->allow_displaying_insecure_content = b;
  if (self->web_preferences_.GetBoolean("allow-running-insecure-content", &b))
    prefs->allow_running_insecure_content = b;
}

}  // namespace atom
