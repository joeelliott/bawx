
Boxee applications fully support the use and integration of Python. Currently, Boxee is running Python 2.4. Code can be integrated into the UI by placing python scripts into the root of your applications directory. Additionally python code can execute from within the Window XML Files themselves by way of Control Events and `CDATA` tags.

### MC Module ###
Boxee has built the 'mc' module, available to all applications, for integration with your applications and most of its controls. With built-in methods to modify button and label text, make controls visible/hidden, modify list containers and their content and much more.

### Usage Examples ###
#### Button OnClick Event ####
```
<control type="button" id="7002">
   <width>115</width>
   <height>60</height>
   <label>Search</label>
   <onclick lang="python">
<![CDATA[
mc.ShowDialogNotification('You clicked the search button!')
]]>
   </onclick>
</control>
```

#### Window OnLoad Event ####
```
<window type="window" id="14000">
   <defaultcontrol always="true">1000</defaultcontrol>
   <allowoverlay>no</allowoverlay>
   <onload lang="python">
<![CDATA[
config = mc.GetApp().GetLocalConfig()
path = config.GetValue("feed"):
mc.GetActiveWindow().GetList(120).SetContentURL(path)
]]>
   </onload>
   <controls>
      <control type="list" id="120">
         ...
      </control>
   </controls>
</window>
```

#### Importing from Root ####
Python in applications root directory:<br />
```
<tt>myapp/myscript.py</tt>
<source lang="python" line>import mc
def login():
   uname = mc.ShowDialogKeyboard("Enter Username", "")
   pword = mc.ShowDialogKeyboard("Enter Password", "", True)
   if uname and pword:
      http = mc.Http()
      params = "user=%s&pass=%s" % (uname, pword)
      http.Post("http://mysite.com/login/", params)
      responseCookie = str(http.GetHttpHeader("Set-cookie"))
      if 'PHPSESSID=' in responseCookie:
         return True
   return False
```

Importing the script from within your application:
```
<control type="button" id="7002">
   <width>115</width>
   <height>60</height>
   <label>LOGIN</label>
   <onclick lang="python">
<![CDATA[
import myscript
result = myscript.login()
if result:
   mc.ShowDialogNotification("You are now authenticated!")
]]>
   </onclick>
</control>
```

#### Launching with Python ####
It is possible to launch your applications with a python script instead of a window. First order of business is to modify your `descriptor.xml` and set the `startWindow` tag to the script you'd like loaded when your application is launched.
```
<app>
  ...
  <type>skin</type>
  <platform>all</platform>
  <minversion>0.9.21</minversion>
  <startWindow>myscript</startWindow>
  <test-app>true</test-app>
</app>
```

Make sure that myscript.py is located in the root if your app directory.<br />
```
<tt>myapp/myscript.py</tt>
<source lang="python" line>import mc
# do stuff
# launch your main application window
mc.ActivateWindow(14001)
```

### Things to Remember ###
  * If you import a script from root in your window's `<onload>` event, you can freely utilize it throughout that window in other controls, without the need to call the import statement.

  * If you application launch window is instead a python script, that script is then accessible throughout your entire application, all windows and all controls, without the need of calling the import statement.