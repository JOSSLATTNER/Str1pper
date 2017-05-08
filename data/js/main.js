var urlBase = window.location.href.substring(7); // used when hosting the site on the ESP8266

var connection = new WebSocket("ws://"+urlBase+"stripcontroll");

var picker = new thistle.Picker('rgb(129,34,203)');
document.body.appendChild(picker.el);
picker.on('changed', function() {
  document.body.style.backgroundColor = picker.getCSS();
  var rgb = picker.getRGB();
   connection.send('{"command":"solidcolor", "data":['+ rgb.r+','+rgb.g+','+rgb.b+' ]}');
});


connection.onopen = function ()
{

};
