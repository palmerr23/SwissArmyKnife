/* LITERAL HTML */
#ifndef MYLITERAL_H
#define MYLITERAL_H
// main page code
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Swiss Army Knife</title>
  <meta name="viewport" content="width=500, initial-scale=1, user-scalable=yes, minimum-scale=0.4">
 <link rel="stylesheet" href="myKnife.css"> 
</head>
<body onload="setScreen()" id="bodyO">
	<div class="pg_container">
	  <div class="buttonTop">
		  <input type="text" readonly id="devName" class="headlineL" value ="X">
		  <input type="hidden" id="screenName" class="headlineS" value ="XXX" style="pointer-events: none; text-align: center;">
		  %SCREENBUTTONS% 
	  </div>
	  <div class="tm_container">   
	    %SETVALUES%
      </div> 
  </div>
  <script src="myKnife.js"></script>
  <script src="jogDial.min.js"></script>  
</body>
</html>
)rawliteral";


#endif
