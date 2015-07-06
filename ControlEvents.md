The window and each control has event tags (like onclick, onfocus, onload etc.) which are triggered upon specific events. Each event supports the attribute `lang="python"` after which, python code may be inline and wrapped with CDATA tags. It's important to note that Boxee currently supports Python 2.4. Below is a list of available event tags for focus-able controls.


### List of Events ###
|onload|Specifies the action to perform when the parent window is loaded. It is important to note that this will fire any time the window is made visible, not just the initial load. Example, when you leave the window and the player is launched, then user navigates back to previous window, the onload will fire again.|Windows|
|:-----|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:------|
|onunload|Specifies the action to perform when the parent window is unloaded (closed, another window is launched, a dialog steals focus). It is important to note that this will fire any time the window unloaded.                                                                                                            |Windows|
|onclick|Specifies the action to perform when the parent control is pressed. You may have more than one `<onclick>` tag, and they'll be executed in sequence.                                                                                                                                                                 |Controls|
|onup  |Specifies the `<id>` of the control that should be moved to when the user moves up off the parent control. Can also point to a Group Control (remembers previous focus).                                                                                                                                             |Controls|
|ondown|Specifies the `<id>` of the control that should be moved to when the user moves down off the parent control. Can also point to a [[Control](Group.md)] (remembers previous focus).                                                                                                                                   |Controls|
|onleft|Specifies the `<id>` of the control that should be moved to when the user moves left off the parent control. Can also point to a [[Control](Group.md)] (remembers previous focus).                                                                                                                                   |Controls|
|onright|Specifies the `<id>` of the control that should be moved to when the user moves right off the parent control. Can also point to a [[Control](Group.md)] (remembers previous focus).                                                                                                                                  |Controls|
|onfocus|Specifies the action to perform when the parent control is focused. The action is performed after any focus animations have completed.                                                                                                                                                                               |Controls|
|onnext|Specifies the `<id>` of the control that should be moved to when the user tabs forward from the parent control. Can also point to a [[Control](Group.md)] (remembers previous focus).                                                                                                                                |Controls|
|onprev|Specifies the `<id>` of the control that should be moved to when the user tabs backward from the parent control. Can also point to a [[Control](Group.md)] (remembers previous focus).                                                                                                                               |Controls|
|ontextchange|Specifies the action to perform when text from a parent [[Control](Edit.md)] modifies. Important to note that this event fires AFTER the text is modified.                                                                                                                                                           |Controls|



### Examples ###
Launch window where id is 736 when parent control is pressed.
```
<onclick>ActivateWindow(736)</onclick>
```

Set focus to control where id = 736 when left is pressed on parent control.
```
<onleft>SetFocus(736)</onleft>
```

Set visible control where id = 736 when text is changed in parent control.
```
<ontextchange>SetVisible(736)</ontextchange>
```

Advanced. Get auto-complete results when text is changed in parent control with id 9001 and set those results to a [[Container](List.md)] with id 736.
```
<ontextchange lang="python"><![CDATA[
import urllib
edit = mc.GetActiveWindow().GetEdit(9001).GetText()
items = mc.GetDirectory('http://mysite.com/autocomplete?value=' + urllib.quote(edit))
mc.GetActiveWindow().GetList(736).SetItems(items)
]]></ontextchange>
```