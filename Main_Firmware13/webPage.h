

//HTML setup web site
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<form action="/index" method=get name="Index">INDEX
<br><input type='radio' name='index' value='1'> SETUP WIFI <br/>
<br><input type='radio' name='index' value='2'> SETUP DEVICE INFORMATION<br/>
<br><input type='radio' name='index' value='3'> SEND MESSAGE<br/>
<br><input type='submit' value='Submit'><br/>
</form>
</html>
)rawliteral";

//HTML setting WIFI
const char settingWIFI[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<form action="/submit_settingwifi" method=get name="SetupWifi">Enter SSID and Password
<br> SSID <input type='text' name='SSID'><br/>
<br> PASSWORD <input type='text' name='password'><br/>
<br> MESH SSID <input type='text' name='meshSSID'><br/>
<br> MESH PASSWORD <input type='text' name='meshpassword'><br/>
<br><input type='radio' name='customfieldid' value='1'> root <br/>
<br><input type='radio' name='customfieldid' value='2'> normal<br/>
<br><input type='submit' value='Submit'><br/>
</form>
</html>
)rawliteral";

const char settingDEVICEINFO[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<form action="/submit_deviceinfo" method=get name="SetupDeviceInformation">Enter Device Information
<br> COMPANY NAME <input type='text' name='companyName'><br/>
<br> SITE NAME <input type='text' name='siteName'><br/>
<br> GROUP <input type='text' name='group'><br/>
<br> DEVICE NAME <input type='text' name='deviceName'><br/>
<br><input type='submit' value='Submit'><br/>
</form>
</html>
)rawliteral";

//HTML set up ok
const char setting_ok[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<h1>
Form submitted. ESP restart now.
</h1>
</html>
)rawliteral";

//HTML send message page
const char send_message[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<form action="/sendMessage" method=get name="SendMessage">Enter Message here
<br><input type='text' name='Message'><br/>
<br><input type='submit' value='Submit'><br/>
</html>
)rawliteral";
