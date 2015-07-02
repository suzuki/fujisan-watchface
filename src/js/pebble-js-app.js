/*global Pebble */
var xhrRequest = function(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var now = new Date();
  var today = now.getFullYear() + '-' + (now.getMonth() + 1) + '-' + now.getDate();

  var url = 'http://api.sunrise-sunset.org/json?lat='
          + pos.coords.latitude
          + '&lng='
          + pos.coords.longitude
          + '&date='
          + today
          + '&formatted=0';

  xhrRequest(url, 'GET', function(responseText) {
    var json = JSON.parse(responseText);

    var sunriseTime = 0;
    var sunsetTime = 0;

    if (json.status == 'OK') {
      var sunriseDate = new Date(json.results.sunrise);
      var sunsetDate = new Date(json.results.sunset);
      sunriseTime = sunriseDate.getTime() / 1000;
      sunsetTime = sunsetDate.getTime() / 1000;
    }

    var dictionary = {
      'KEY_SUNRISE_TIME': sunriseTime,
      'KEY_SUNSET_TIME': sunsetTime
    };

    Pebble.sendAppMessage(dictionary,
      function(e) {
        //console.log('Sunrise/Sunset info sent to Pebble successfully!');
      },
      function(e) {
        //console.log('Error sending weather info to Pebble!');
      }
    );
  });
}

function locationError(err) {
  //console.log('Error requesting location!');
}

function getSunriseSunset() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', function(e) {
  getSunriseSunset();
});

Pebble.addEventListener('appmessage', function(e) {
  getSunriseSunset();
});
