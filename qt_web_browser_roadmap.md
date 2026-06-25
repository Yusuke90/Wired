# 

# Qt C++ Web Browser — Build Roadmap

## How to use this

Work through the modules in order. Each one ends with a concrete milestone —
don't move to the next module until that milestone actually works. Commit to
git after every milestone; it gives you clean checkpoints if something breaks
later and a built-in changelog if this is for academic submission.

## Tech stack at a glance

|Required technology|Where it actually shows up|
|-|-|
|C++|The whole application, Modules 1–9|
|Qt Creator|IDE and UI layout tool, used continuously|
|Networking APIs|Module 6 (downloads, `QNetworkAccessManager`)|
|RESTful APIs|Module 6 (API tester panel, JSON parsing)|
|HTML \& CSS Parsing|Module 3 — handled internally by QtWebEngine (Blink)|
|JavaScript Engines|Module 3 — handled internally by QtWebEngine (V8)|
|WebGL|Module 3 — Chromium's GPU backend inside QtWebEngine|

## Module 1 — Environment \& Project Setup

**Goal:** a buildable, empty app before writing any browser logic.

* Install Qt Creator + Qt 6.5+ via the Qt Online Installer. Specifically
check the **Qt WebEngine** component in the installer — it's a separate
download from base Qt and easy to miss. On Windows, WebEngine only builds
with the **MSVC 2022 64-bit** kit, never MinGW (a permanent Chromium
limitation, not version-specific) — install "Build Tools for Visual
Studio 2022" (Desktop development with C++ workload only) rather than
the full IDE to keep this lightweight.
* Create a new Qt Widgets Application (CMake or qmake) targeting C++17.
* Set up git and a README describing the project and stack.
* Confirm the empty project builds and runs on your target OS.

**Milestone:** a blank window opens when you run the app.

## Module 2 — Core UI Shell

**Goal:** build the browser "chrome" — everything that isn't the web page itself.

* Lay out a `QMainWindow` with a toolbar: Back, Forward, Reload, Home, and an
address bar (`QLineEdit`) taking most of the toolbar width.
* Add a menu bar (File, Edit, View, Bookmarks, History, Help). Stub the
actions for now — most get wired up in later modules.
* Add a `QStatusBar` and a `QProgressBar` for load status.
* Build this with Qt Creator's Designer (`.ui` files) rather than
hand-positioning widgets.

**Milestone:** every UI element is visible and clickable, even before it
does anything.

## Module 3 — Rendering Engine Integration

**Goal:** actually load and display web pages.

* Add the WebEngine module to your build (CMake:
`find\\\\\\\_package(Qt6 COMPONENTS WebEngineWidgets)`; qmake:
`QT += webenginewidgets`).
* Embed a `QWebEngineView` as the central widget.
* Wire the address bar's `returnPressed` to `QWebEngineView::load`.
* Connect Back/Forward/Reload toolbar buttons to the matching
`QWebEngineView` slots.
* Bind `loadProgress` to your progress bar and `titleChanged` to the
window title.

**Milestone:** you can type a URL, hit enter, and see a real rendered page —
including JS-driven sites and a WebGL demo — with no extra parsing/engine
code, because Blink and V8 inside QtWebEngine do that work.

## Module 4 — Tabs \& Multi-Window Browsing

**Goal:** support browsing the way people actually do it.

* Replace the single `QWebEngineView` with a `QTabWidget`, one
`QWebEngineView` per tab.
* Implement new tab (button + Ctrl+T) and close tab (× + Ctrl+W); update
each tab's label/icon from `titleChanged`/`iconChanged`.
* Support "open in new window" via additional `QMainWindow` instances
sharing the same logic.
* Make the toolbar and address bar reflect whichever tab is currently active.

**Milestone:** several tabs and windows open at once, each navigating
independently.

## Module 5 — Popups, Permissions, Security \& Ad Blocking

**Goal:** make the browser safe by default, not just functional.

* Override `QWebEnginePage::createWindow` to intercept popup/new-window
requests. Default to blocking, with a small "popup blocked — open
anyway?" bar instead of silently allowing or silently dropping it.
* Handle `featurePermissionRequested` for location, camera, notifications,
etc., and prompt the user.
* Handle `certificateError` instead of ignoring HTTPS problems.
* Add a private/incognito mode using a separate off-the-record
`QWebEngineProfile`.
* **Ad blocker:** subclass `QWebEngineUrlRequestInterceptor`, register it
via `QWebEngineProfile::setUrlRequestInterceptor`, and reject requests
whose host matches a blocklist. Start with a small static list of known
ad/tracker domains; once that works, parse a subset of EasyList-format
rules (domain blocking and `||domain^` patterns are enough for a v1 —
skip cosmetic/element-hiding rules, that's a CSS-injection problem, not
a networking one). Add a toolbar toggle to enable/disable it per-tab via
the profile.

**Milestone:** test pages that try to auto-spawn popups get blocked;
permission prompts appear instead of silent grants; a page loaded with the
blocker on visibly has fewer/no ad requests than with it off (check the
Module 6 network status or a browser dev-tools equivalent to confirm
blocked requests).

## Module 6 — Networking, Downloads \& REST Tooling

**Goal:** the networking/REST work that's actually yours to write.

* Hook `QWebEngineProfile::downloadRequested` to a download manager panel:
progress, pause/cancel, "open containing folder."
* Add a `QNetworkAccessManager`-based utility — e.g. a small "API tester"
tab that sends GET/POST requests and pretty-prints JSON responses with
`QJsonDocument`. This is where "RESTful APIs" as a listed requirement
actually lives in your code.
* Surface basic network status (online/offline, SSL errors) in the status
bar.

**Milestone:** files download with visible progress; you can fire a REST
request from inside the app and see a formatted response.

## Module 7 — History, Bookmarks \& Search

**Goal:** make the browser persistently useful across sessions.

* Store visited URLs/timestamps and bookmarks locally — `QSqlDatabase`
(SQLite) for something real, or `QSettings`/JSON if you want it lighter.
* Build a searchable History dialog and a Bookmarks menu/bar.
* Add address-bar autocomplete sourced from history/bookmarks.
* Detect plain search terms vs. URLs in the address bar and route searches
to a configurable default search engine.

**Milestone:** closing and reopening the app preserves history/bookmarks;
typing a non-URL term searches the web.

## Module 8 — Performance \& Polish

**Goal:** back up "fast page loading" and "efficient pop-up blocker" with
actual settings, not just defaults.

* Deliberately configure `QWebEngineProfile`'s HTTP cache type and size.
* Suspend or unload background tabs that have been idle a while, to save
memory.
* Add zoom controls, find-in-page (`QWebEnginePage::findText`), and a
full-screen handler for video/WebGL content (`fullScreenRequested`).
* Add a dark/light theme toggle via Qt stylesheets.

**Milestone:** you can point to specific numbers (cache size, idle-tab
suspension threshold, ad blocker request counts) instead of "it feels fast."

## Module 9 — Testing, Packaging \& Submission

**Goal:** ship something that runs outside your dev environment.

* Smoke-test on every target OS — QtWebEngine has real platform quirks
(Linux sandbox flags especially), so "builds on my machine" isn't enough.
* Package with `windeployqt`/`macdeployqt`/`linuxdeployqt` (or
`linuxdeploy` + AppImage on Linux).
* Write a short README/user guide and grab screenshots or a demo recording.
* If this is for academic submission, map each module's milestone back to
the original technology list so it's obvious where each requirement was
satisfied.

**Milestone:** a packaged build someone else can install and run without
your dev setup.

\---

## Suggested pace

A part-time pace of roughly one module per week puts the full build at
about 9 weeks. Working on it full-time, modules 1–4 (the core shell) can
often be compressed into a week or two, with modules 5–9 taking another two
to three weeks depending on how deep you go on persistence, ad-block rule
parsing, and packaging.

