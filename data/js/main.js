var urlBase = window.location.href.substring(7); // used when hosting the site on the ESP8266

var connection = new WebSocket("ws://"+urlBase+"stripcontroll");
alert("ws://"+urlBase+"stripcontroll");
function hexToRgb(hex) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}


// some element to monitor
var el = $(".jscolor");

// hook up the watcher
el.watch({
    // specify CSS styles or attribute names to monitor
	properties: "background-color",

    // callback function when a change is detected
    callback: function(data, i) {
		var propChanged = data.props[i];
        var newValue = data.vals[i];

		var el = this;
		var el$ = $(this);
		//
        // do what you need based on changes
        // or do your own checks
        var rgb = newValue.substring(4,newValue.length-1);
        rgb = rgb.split(',');



       connection.send('{"command":"solidcolor","data":['+rgb[0]+','+rgb[1]+','+rgb[2]+']}');
        console.log("Color:"+rgb[0]+","+rgb[1].substring(1)+","+rgb[2].substring(1));
	}
});

connection.onopen = function ()
{
  connection.send('{"command":"solidcolor","data":[0,0,255]}'); // Send the message 'Ping' to the server
};
