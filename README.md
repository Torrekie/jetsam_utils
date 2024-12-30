# jetsam utils

**iOS and macOS share a program: jetsam.** The tl;dr is that jetsam monitors memory usage, asks for memory when there isn't much free, and kills other programs that don't give back as much as was requested. This tool--a continuation from conradev's jetsamctl--serves to override the strict limits that jetsam sets on iOS devices with a simple command. All you need is a device with jailbroken iOS 12 - iOS 14.

### Installation

```
$ make package
$ dpkg -i ./package/*
```

### Usage

Type `jetsamctl` in a terminal to learn more about how to use. Here's some examples of what you can do:

Limit the Phone app to 48 MB of memory: 
```
jetsamctl -l 48 MobilePhone
```

Set the priority of the Photos app to that of an iOS keyboard extension: 
```
jetsamctl -p 8 MobileSlideShow
```

### Priorities

Here is a table of all of the priorities and their numerical values (Before macOS 13/ iOS 16). A lower priority value obviously means that jetsam will bother it more.

| Priority | Definition | Value | Examples | Priority (Short) |
|:--|:--:|:--:|:--:|:--:|
| Idle (Head) | `JETSAM_PRIORITY_IDLE_HEAD` | -2 | | |
| Default | | -1 | | |
| Idle | `JETSAM_PRIORITY_IDLE` | 0 | | (Idle) |
| Idle (Deferred) / Aging Band 1 | `JETSAM_PRIORITY_IDLE_DEFERRED` | 1 | | (Deferred) |
| Background (Opportunistic) / Aging Band 2 | `JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC` | 2 | IOAccelMemoryInfoCollector | (Opportunistic) |
| Background / Elevated Inactive | `JETSAM_PRIORITY_BACKGROUND` | 3 | Non-system extensions or daemons | (BG) |
| Mail | `JETSAM_PRIORITY_MAIL` | 4 | `com.apple.*.sync` services | (Mail) |
| Phone | `JETSAM_PRIORITY_PHONE` | 5 | MobileBackup Daemon | (-) |
| | | 6 | | (Phone) |
| | | 7 | ContextStored, CoreDuetd | (-) |
| UI Support | `JETSAM_PRIORITY_UI_SUPPORT` | 8 | Shazam, Siri, etc. | (UISupport) |
| Foreground Support | `JETSAM_PRIORITY_FOREGROUND_SUPPORT` | 9 | Sidecar, Spotlight, etc. | (FGSupport) |
| Foreground | `JETSAM_PRIORITY_FOREGROUND` | 10 | Foregrounding Apps, Non-UI Extensions, Diagnostics, QuickLook | (FG) |
| | | 11 | ioupsd, notification_proxy, etc. | (-) |
| Audio and Accessory | `JETSAM_PRIORITY_AUDIO_AND_ACCESSORY` | 12 | Bluetooth Server, MobileAccessoryUpdater | (Audio) |
| Conductor | `JETSAM_PRIORITY_CONDUCTOR` | 13 | | (AppleTV) |
| | | 14 | obiteration, softwareupdated, etc. | (-) |
| Driver Apple | `JETSAM_PRIORITY_DRIVER_APPLE` | 15 | Siri App, chronod, etc. | (-) |
| Home | `JETSAM_PRIORITY_HOME` | 16 | CarPlay App, SpringBoard, etc. | (Home) |
| Executive | `JETSAM_PRIORITY_EXECUTIVE` | 17 | backboardd | (God) |
| Important / Default | `JETSAM_PRIORITY_IMPORTANT` | 18 | CacheDelete Daemon | (Important) |
| Critical / Telephony | `JETSAM_PRIORITY_CRITICAL` | 19 | CommCenter | (Critical) |
| | | 20 | notifyd | (-) |
| | | 21 | ReportCrash, WirelessStress, etc. | (-) |
| | | 99 | launchd | (launchd) |

### TODO

#### Jetsam Operations
- [ ] Get/Set/Reset process "managed" status
- [ ] Enable/Disable Jetsam "lenient" mode
- [ ] Set/Reset task's status as a privileged listener w.r.t memory notifications
- [ ] Get/Set/Reset process freezable state

#### Jetsamctl abilities
- [ ] Operate by process user/group/status/workingdir/parent/child
- [x] Operate by process name/PID
- [ ] Run as launchctl daemons (jetsamctld?)
- [ ] config files

#### Misc
- [ ] Manual page
- [ ] Localizations
- [ ] replace Theos with GNU Autotools or pure makefiles

## License

jetsam utils is available under the MIT license. See the LICENSE file for more info.
