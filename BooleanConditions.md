Below is a list of each conditional available to you when skinning your applications. Much thanks go out to the xbmc wiki where, as boxee is based of xbmc, much of this list came from.

|Player.HasMedia|True if the player has an audio or video file.|
|:--------------|:---------------------------------------------|
|Player.HasAudio|True if the player has an audio file.         |
|Player.HasDuration|True if Media isn't a true stream             |
|Player.HasVideo|True if the player has a video file.          |
|Player.Playing |True if the player is currently playing (ie not ffwding, rewinding or paused.)|

|Player.Paused|True if the player is paused.|
|:------------|:----------------------------|
|Player.Forwarding|True if the player is fast forwarding.|
|Player.Forwarding2x|True if the player is fast forwarding at 2x.|
|Player.Forwarding4x|True if the player is fast forwarding at 4x.|
|Player.Forwarding8x|True if the player is fast forwarding at 8x.|
|Player.Forwarding16x|True if the player is fast forwarding at 16x.|
|Player.Forwarding32x|True if the player is fast forwarding at 32x.|
|Player.Rewinding|True if the player is rewinding.|
|Player.Rewinding2x|True if the player is rewinding at 2x.|
|Player.Rewinding4x|True if the player is rewinding at 4x.|
|Player.Rewinding8x|True if the player is rewinding at 8x.|
|Player.Rewinding16x|True if the player is rewinding at 16x.|
|Player.Rewinding32x|True if the player is rewinding at 32x.|
|Player.CanRecord|True if the player can record the current internet stream.|
|Player.Recording|True if the player is recording the current internet stream.|
|Player.Caching|True if the player is current re-caching data (internet based video playback).|
|Player.SeekBar|True for the first 2.5 seconds after a video seek.|
|Player.DisplayAfterSeek|True after any seek.         |
|Player.Seeking|True if a seek is in progress|
|Player.Muted |True if the volume is muted. |
|Playlist.IsRandom|True if the player is in random mode.|
|Playlist.IsRepeat|True if the player is in repeat all mode.|
|Playlist.IsRepeatOne|True if the player is in repeat one mode.|
|AudioScrobbler.Enabled|True if songs played are submitted to audioscrobbler.|
|MusicPlayer.HasNext|True if the music player has a next song queued inthe Playlist.|
|MusicPlayer.HasPrevious|True if the music player has a a Previous Song in the Playlist.|
|MusicPlayer.Offset(number).Exists|True if the music players playlist has a song queued in position (number).|
|VideoPlayer.UsingOverlays|True if the video player is using the hardware overlays render method. Useful, as with hardware overlays you have no alpha blending to the video image, so shadows etc. need redoing, or disabling.|
|VideoPlayer.IsFullscreen|True if the video player is in fullscreen mode.|
|VideoPlayer.HasMenu|True if the video player has a menu (ie is playing a DVD)|
|VideoPlayer.HasInfo|True if the current playing video has information from the library or from a plugin (eg director/plot etc.)|
|VideoPlayer.Content(parameter)|True if the current Video you are playing is contained in corresponding Video Library sections. The following values are accepted : files, movies, episodes, musicvideos|
|Window.IsVisible(window)|True if the window is visible (includes fade out time on dialogs)|
|Window.IsActive(window)|True if the window with id or title "window" is active (excludes fade out time on dialogs) See here for a list of windows|
|Window.Next(window)|True if the window with id or title "window" is being moved to. See here for a list of windows. Only valid while windows are changing.|
|Window.Previous(window)|True if the window with id or title "window" is being moved from. See here for a list of windows. Only valid while windows are changing.|
|System.HasNetwork|True if the ethernet cable is plugged in.|
|System.HasMediadvd|True if there is a CD or DVD in the DVD-ROM drive.|
|System.IdleTime(time)|True if Boxee has had no input for "time" amount of seconds.|
|system.time(startTime,endTime)|True if the current system time is >= startTime and < endTime. endTime is optional. Time must be specified in the format HH:mm, using a 24 hour clock.|
|system.date(startDate,endDate)|True if the current system date is >= startDate and < endDate. endDate is optional. Date must be specified in the format MM-DD.|
|system.platform.xbox|True if Boxee is running on a xbox.|
|system.platform.linux|True if Boxee is running on a linux/unix/osx based computer.|
|system.platform.windows|True if Boxee is running on a windows based computer.|
|Control.IsVisible(id)|True if the control with id "id" is visible.|
|Control.HasFocus(id)|True if the currently focused control has id "id".|
|ControlGroup(group).HasFocus(id)|True if the control group with id "group" has control id "id" as it's focused item. If "id" is not present, or is 0, then it will return true if the currently focused control is in the control group with id "group". Note that if the control group with id "group" does not have focus, then this will still return true if the last focused item in the group had control id "id".|
|ButtonScroller.HasFocus(id)|True if the buttonscroller is focused, and the current button has id "id".|
|App.HasSetting(setting)|Returns the state of the skin-specified setting "setting". Set via `App.SetBool(mysetting, true)`. You can toggle a setting from a button by using `<onclick>App.ToggleSetting(setting)</onclick>`.|
|App.String(string)|Returns whether the skin string (set via App.SetString or via python api ''LocalConfig()'') is non-empty.|
|ListItem.IsFolder|Returns whether the current ListItem is a folder|
|ListItem.IsPlaying|Returns whether the current `ListItem.*` info labels and images are currently Playing media|
|Container.HasThumb|True if the current container you are in has a thumb assigned to it|
|Container(id).OnNext|True if the container with id (or current container if id is omitted) is moving to the next item. Allows views to be custom-designed (such as 3D coverviews etc.)|
|Container(id).OnPrevious|True if the container with id (or current container if id is omitted) is moving to the previous item. Allows views to be custom-designed (such as 3D coverviews etc.)|
|Container(id).HasFocus(item\_number)|True if the container with id (or current container if id is omitted) has static content and is focused on the item with id item\_number.|
|Container(id).HasFiles|True if the container contains files (or current container if id is omitted).|
|Container(id).HasFolders|True if the container contains folders (or current container if id is omitted).|
|Container(id).HasNext|True if the container (id) has a next page.|
|Container(id).HasPrevious|True if the container (id) has a previous page.|
|Container(id).IsStacked|True if the container is currently in stacked mode (or current container if id is omitted).|
|Container(id).Row(row)|True if the container with id (or current container if id is omitted) is focused on the row given.|
|Container(id).Column(col)|True if the container with id (or current container if id is omitted) is focused on the column given.|
|Container(id).Position(pos)|True if the container with id (or current container if id is omitted) is focused on the position given.|
|Container(id).Scrolling|True if the user is currently scrolling through the container with id (or current container if id is omitted).|