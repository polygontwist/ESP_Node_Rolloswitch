//---------------------------------WWW------------------------------
//16x16 16farben
//ESP8266-03


//HTML
const String indexHTM="<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"\t<meta content=\"width=device-width,initial-scale=0.65,maximum-scale=2,user-scalable=yes\" name=\"viewport\" />\r\n"
"\t<meta charset=\"utf-8\"/>\r\n"
"\t<link rel=\"shortcut icon\" href=\"favicon.ico\">\r\n"
"\t<link rel=\"STYLESHEET\" type=\"text/css\" href=\"style.css\">\r\n"
"\t<script type=\"text/javascript\" src=\"system.js\"></script>\r\n"
"</head>\r\n"
"<body>\r\n"
"<h1>$h1gtag</h1>\r\n"
"<p class=\"buttons\">\r\n"
"<a href=\"#\" data-433='{\"action\":\"UP\"}'>UP</a>"
"<a href=\"#\" data-433='{\"action\":\"STOP\"}'>STOP</a>"
"<a href=\"#\" data-433='{\"action\":\"DOWN\"}'>DOWN</a>"
"</p>\r\n"
"<p class=\"buttons\">"
"<a href=\"#\" data-433='{\"action\":\"UP\",\"dauer\":29.5}'>UP 29.5s</a>"
"<a href=\"#\" data-433='{\"action\":\"DOWN\",\"dauer\":2}'>DOWN 2s</a>"
"</p>\r\n"
"<div id=\"timersetting\"></div>\r\n"
"<div id=\"sysinfo\">$sysinfo</div>\r\n"
"<div id=\"filelist\">\r\n"
"$filelist"
"</div>\r\n"
"<form class=\"upload\" method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
"<input type=\"file\" name=\"upload\" required>\r\n"
"<input type=\"submit\" value=\"Upload\" class=\"button\">\n</form>\r\n"
"</body>\r\n"
"</html>\r\n"
;


