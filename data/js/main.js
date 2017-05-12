var urlBase = window.location.href.substring(7); // used when hosting the site on the ESP8266

//WebSocket connection
var connection = new WebSocket("ws://"+urlBase+"stripcontroll");

//Color Picker
var picker = new thistle.Picker('rgb(129,34,203)');
var pickerContainer = document.getElementById("picker_container");
pickerContainer.appendChild(picker.el);

connection.onopen = function ()
{
	console.log("connection established!");

	picker.on('changed', function()
	{
	  document.body.style.backgroundColor = picker.getCSS();
	  var args = {
	  	'rgb' : picker.getRGB()
	  }

	  sendCommand("SET_COLOR", args);
	});
};

connection.onmessage = function(message)
{
	console.log(message.data);
}

function sendCommand(command, params)
{
	var o = {
		"command" : command,
		"data" : params
	};
	var json = JSON.stringify(o);
	console.log("sending: " + json);
	connection.send(json);
}

function ChangeModule(i)
{
	sendCommand("SET_MODULE", {
		"module" : i
	});
}
