/* update readings quickly, update other controls not as often */ 
loopsWithoutJSON = 0;
currentScreen = 'M';
currentScreenType = 'M';
var dial_el; 
var dial;
var lastDial = 0;
var dialX = 0;
var settingToChange = null;
var settingToChangeName = null;
var buttonMul = 1;
var fetchTime = 1000; // 500 - 1000 mS in prod
var disconLoops = 15000/fetchTime;

sessionID = -1; // Set random at runtime. Sent with every request. 
//var dial_val = 0;

setInterval(
  () => {
	loopsWithoutJSON++;
	ems = document.getElementById("input_errorID"); // errorText
 
	//console.log("sType: no fetch", currentScreenType);
	/*
	if(currentScreenType == 'L') // update data is useless here
	{
		  //console.log("Log: no fetch");
		  return 0;
	}
	*/
	//console.log("Fetching");
	if(loopsWithoutJSON > disconLoops) // (~15 secs) increase value for shorter Intervals
	{
		  console.log("No FETCH JSON " + loopsWithoutJSON);		 
		  ems.value = "Disconnected";
	}
	var sid = document.getElementById("sessionIDM"); // any one will do
	//console.log("Current screen ", currentScreen);
    fetch("/readings?screen="+currentScreen+"&sessionID="+sid.value)
    .then(function (response) {
      return response.json();
    })
    .then(function (myJson) {      
	  loopsWithoutJSON = 0;
	  ems.value = "";
	  //console.log("/readings");
	  // floats - 1 or 2 decimals
      for(i of ["volts1R","volts2R","volts1S","freqS", "scaleS"]) 
      {
		  var digits = 1;
        // get reading or setting i
           //objStr = i.toString;
		  //console.log(i);
		   //console.log(objstr);
          if(i in myJson)
          {
			  if (["volts1R","volts2R","volts1S"].indexOf(i) >= 0)
				digits = 2;
              ss = myJson[i]; // value to set           
              //console.log(i, ss);
              bs = document.getElementById(i+"ID");
			  if(bs != null)
			  {
              //fs = document.getElementById("lvbar"+(i).toString());
				bs.value = parseFloat(ss).toFixed(digits);              
				//bs.setShowSoftInputOnFocus(false);
			  }
          } 
	  }
	  // integer values
	  for(i of ["potS", "potPcS"])
		  if(i in myJson)
		  {          
			  bs = document.getElementById(i+"ID");
			  if(bs != null)
			  {
				ss = myJson[i]; 
				bs.value = parseInt(ss).toFixed(0);  
				//console.log(i, bs.value);
			  }				
		  } 	  
	  
	  // buttons
	  for(i of ["R1", "R2", "R3", "D1", "D2", "dig1R", "dig2R"]) 
		  if(i in myJson)
		  {
			// SET checked as well
			var bs = document.getElementById(i);
			var valx = myJson[i].toFixed(0);
			//console.log("Button", i, valx);
			if(bs != null)
				bs.checked = (valx > 0.5) ? true : false; // 1 or 0. Not sure if arithmetic is float here
			setButtonColor(i);		// this may cause a loop, by triggering clickDiv
		  }
	  // Sine scale range button set
	  for(i of ["SSR"]) 
		  if(i in myJson)
		  {
			var valx = myJson[i].toFixed(0);
			for (j of [1,8]) // [1,2,4,8]
			{
				var cn = "SSR"+String(j);
				
				var bs = document.getElementById(cn);	
				bs.checked = (j == valx) ? true : false; 
				//console.log("SSX", cn, valx, bs.checked);
			}
			setSSButtonColor(valx);	
		  }
      // screens
	  if("screen" in myJson)
	  {             
		 ss = myJson["screen"]; // value to set
		 currentScreen = ss;
// console.log("Setting currentScreen to",ss); 
		 dispC = "none";
		 dispA = "center";
		 bs = document.getElementById("screenName");
		 sb = document.getElementById("submit"+ss);
		 if(ss == "M") 
		 {
			bs.value = "Main";	
			currentScreenType = 'M';
		 }
		 if (ss == "S")
	     {
		    bs.value = "Cal"; 
		    currentScreenType = 'S';					  
	     }
		 
		 // if (ss == "L") // nothing to do.
		 
		 sb.style.backgroundColor = "#cce2ff";	
		 sb.style.borderColor = "#cce2ff";
		 //sb.style.color = "white";	
		 bs.style.textAlign = dispA;	// realign ScreenName text	 
	  }	  
	  if("device" in myJson)
	  {
		  ss = myJson["device"]; // value to set
		  sb = document.getElementById("devName");
		  sb.value = ss;
		  //console.log("Device", ss);
	  }
    })
    .catch(function (error) {
      console.log("Error: " + error);
  });
  },
  fetchTime // 500-1000 mS is about right
);

// screen load
function setScreen(element) {
	// a random number to differentiate from other sessions from the same IP - may get changed on each page load?
	if (sessionStorage.getItem('sessionID') == null)
	{
		
		ssval = parseInt(Math.random() * 1000000);
		sessionStorage.setItem('sessionID', ssval); 
	}
	sessionID = sessionStorage.getItem('sessionID');
	//console.log("SessionID", sessionID);
	var scr = document.getElementById("screenID");
	for(i of ['M','S','L']) 
	{ 
		ss = document.getElementById("sessionID"+i)	
		if(ss)		
			ss.value = sessionID;
	 }
	currentScreen = scr.value;		
	if (document.body.id == "bodyO")
	{
		 currentScreenType = 'S';	//a settings screen
	
		//console.log("starting knob");
		dial_el = document.getElementById("jog_dial");
		var dial_options = {wheelSize:'111px', knobSize:'30px', minDegree:null, maxDegree:null, degreeStartAt: 0};//debug: true, 
		dial = JogDial(dial_el, dial_options).on('mousemove', function(evt){ 
			var delta = 0;

			var dialVar = document.getElementById("dialVal");
			var dTest = document.getElementById("testVal");
			var ddV = document.getElementById("deltaVal");
			var mBut = document.getElementsByName("mulBut");
			var errLine = document.getElementById("input_errorID");
			var decimals = 2;
			if(["potS", "potPcS"].includes(settingToChangeName)) // integers
				decimals = 0;
			for(i = 0; i < mBut.length; i++)
				if(mBut[i].checked)
				{
					//console.log("mBut found", i);
					delta = +mBut[i].value;
					ddV.value = delta;
				}
			//console.log("DM", delta);
			var dial_val = evt.target.rotation;
			dialX = 0;
			if( dial_val - lastDial > 5)
			{	
				dialX = +delta;			
				lastDial = dial_val;				
			}
			else if(dial_val - lastDial < -5)
			{
				dialX = -delta;	
				lastDial = dial_val;			
			}
			dialVar.value = dialX; 
			if(dialX == 0)
				return;
			
			if(settingToChange != null) // don't allow overruns
			{ 
				dTest.value = +settingToChange.value + dialX;
				if(+settingToChange.value + dialX > +settingToChange.max)
				{
					settingToChange.value = (+settingToChange.max).toFixed(decimals);
					setChange(settingToChange);
					errLine.value = "Max value";				
				}
				else
					if(+settingToChange.value + dialX < +settingToChange.min)
					{				
						settingToChange.value = (+settingToChange.min).toFixed(decimals);
						errLine.value = "Min value";
						setChange(settingToChange);
					}
					else {
						settingToChange.value = (+settingToChange.value + dialX).toFixed(decimals);
						errLine.value = " ";
						setChange(settingToChange);
					}
				// ganged controls 0..255 / 0.100
				if("potS" == settingToChangeName) // integers
				{
					bs = document.getElementById("potPcSID");
					bs.value  = (settingToChange.value * 100 / 256).toFixed(0);
				}
				if("potPcS" == settingToChangeName) // integers
				{
					bs = document.getElementById("potSID");
					bs.value  = (settingToChange.value * 256 / 100).toFixed(0);
				}
			}
		});
	}
	else // bodyL
	{
		 sb = document.getElementById("submitL"); 
		 //sb.style.color = "white";	
		 currentScreenType = 'L';	// log screen/  will be updated on first GET?	 
	}
};

// Channel buttons 
// this may cause a loop, by triggering clickDiv
function setButtonColor(con)
{
	var isRange = false;
	div = "D" +con;
	spa = "S"+con;	// the span
	divEl = document.getElementById(div) 		// Div
	conEl = document.getElementById(con);		// control
   // labEl = document.getElementById(lab);		// label
	spaEl = document.getElementById(spa);		// span
	var oncolor = "blue";
	var found = false;
	//console.log("SBC", con);
	for(i of ["R1", "R2",  "D1", "D2"]) 
	  if(i == con)
	  {
		oncolor = "red";		// this may cause a loop, by triggering clickDiv
		found = true;
	  }
	for(i of ["dig1R", "dig2R"]) 
	  if(i == con)
	  {
		oncolor = "OrangeRed";		// this may cause a loop, by triggering clickDiv
		found = true;
	  }
	for(i of ["AR1", "AR2"]) // sine scale buttons here
	{	  
	  if(i == con)
	  {
		isRange = true;
		oncolor =  "blue";
		found = true;
	  }
	}
	  
	if (!found)
		return;
	
	if (conEl.checked)
	{
	//console.log("CHK");
	  divEl.style.background = oncolor; //oncolor;
	  spaEl.style.background = oncolor;
	  spaEl.style.color = "white"
	  if(isRange)
		spaEl.innerHTML = "H";
	}
	else
	{
		//	console.log("UN-CHK");
	  divEl.style.background = "#ccc";
	  spaEl.style.background = "#ccc";
	  spaEl.style.color = "black";
	  if(isRange)
		spaEl.innerHTML = "L";
	}
	//console.log("setButtonColor ID =", con, div, spa, conEl.checked, oncolor);
};

// for the SSR button set - make them in to radio buttons
// update to ESP done in clickDiv
function setSSButtonColor(num)
{
	var isRange = false;

	var oncolor = "blue";
	var found = false;
	//console.log("SSBC", num);
	for(i of [1,8]) //[1,2,4,8
	{
	  div = "DAR" +i;
	  spa = "SSR"+i;	// the span
	  divEl = document.getElementById(div) 		// Div
		//conEl = document.getElementById(con);		// control
        // labEl = document.getElementById(lab);		// label
	  spaEl = document.getElementById(spa);		// span
	  if(i == num)
	  {
		  //console.log("CHK");
		  divEl.style.background = "gold"; //oncolor;
		  spaEl.style.background = "gold";
		  spaEl.style.color = "white"
	  }
	  else
	  {
		  //	console.log("UN-CHK");
		  divEl.style.background = "#ccc";
		  spaEl.style.background = "#ccc";
		  spaEl.style.color = "black";
	  }	
	}
	//console.log("setButtonColor ID =", con, div, spa, conEl.checked, oncolor);
};

// clicked a button
function clickDiv(element) {
	var xhr = new XMLHttpRequest();
	//con = (element.id).substring(1); // actual control name
	con = element.id; //(element.id).substring(1,4);
//	lab = "L"+con;	// the label
	div = "D" +con;
	spa = "S"+con;	// the span
	//nam = "N" + con;
	//console.log(con, spa, div);
	divEl = document.getElementById(div); 		// Div
	conEl = document.getElementById(con);		// control
   // labEl = document.getElementById(lab);		// label
	spaEl = document.getElementById(spa);		// span
	//console.log("ClickDiv ID =", con, conEl.checked);
	
	// state changes before we get here
	var ssrbuts = ["SSR1", "SSR2","SSR4","SSR8"];
	if(ssrbuts.indexOf(con) !== -1)
	{		
		var sv = (element.id).substring(3,4);
		//console.log("ClickDiv SSR", sv);
		setSSButtonColor(sv);
	}
	else
	{
		var ncon = (element.id).substring(1,2);
		//console.log("ClickDiv", con, ncon);
		setButtonColor(con);
		var sv = (conEl.checked) ? +1 : +0 ;
	}
	// send 'uncheck' update
	
	xhr.open("GET", "/update?cmd=3&value_1="+con+"&value_2="+sv+"&screen="+currentScreen, true); 
	xhr.send();		
};

// clicked the calibration save button
// values have already been transmitted
function calSave(element) {	
	var xhr = new XMLHttpRequest();
	xhr.open("GET", "/update?cmd=3&value_1=calSave&value_2=1&screen="+currentScreen, true); 
	xhr.send();		
	for (el of ["calD1SID", "calA1SID", "calA2SID"] )
	{
		elx = document.getElementById(el);
		elx.value = (0).toFixed(2);
	}
};

function selSet(element)
{
	clearSettingBackgrounds();
	el = document.getElementById(element.id) 		
	el.style.backgroundColor = "white";
	//el.style.border = "1px";
	el.blur();
};
function clearSettingBackgrounds()
{
	// ugly but effective!
	var setList;
	if (currentScreen == "M" )
		setlist = ["potSID", "potPcSID", "volts1SID", "freqSID"] ; 
	else
		setlist = ["calD1SID", "calA1SID", "calA2SID"] ; 
	for(i of setlist)
	{
		//console.log("CSB", i);
		elx = document.getElementById(i) 	
		elx.style.backgroundColor = "transparent";
		//elx.style.border = "none";
	}
};

  // Update the current  value (each time you turn the knob)
function setChange(element) {
    var x3hr = new XMLHttpRequest();
    var strx = "/update?cmd=4&value_1="+element.id+"&value_2="+element.value+"&screen="+currentScreen;
    x3hr.open("GET", strx, true); x3hr.send();
   // console.log("set Change",strx);
};


 var RegExp = new RegExp(/^-?\d*\.?\d*$/); // pos, neg floats or integers only

function selSetting(elem)
{
	settingToChange = elem; // this one gets changed by knob
	settingToChangeName = elem.name;
	//console.log("STC:", settingToChangeName);
}

function valid(elem) {
	var val = elem.value;	
	var errLine = document.getElementById("input_errorID");
	if (RegExp.test(val)) {
		errLine.value = " ";
		// should probably include tests here for max/min being present
		if (+val > +elem.max) 
			errLine.value = "Max = " + elem.max;
		if (+val < +elem.min) 
			errLine.value = "Min = " + elem.min;
	} else {
		//elem.value = val;
		errLine.value = "Illegal char.";
	}
}

// no longer used
/*
function openFile(event)
{
    var file = event.target.files[0];
	textTarget = document.getElementById("inJSON");
	if (!file)
	{
		textTarget.value = "No file selected to load.";	
		return;	
	}
    var reader = new FileReader();
    reader.onload = function(e){
		var contents = e.target.result;			 
		textTarget.value = contents;
		// execute the changes!
    };
	reader.readAsText(file);
};
*/
