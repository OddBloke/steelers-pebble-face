var sendSuccess = function(e) {
  console.log("sendSuccess, e: " + JSON.stringify(e));
};

var sendFailure = function(e) {
  console.log("sendFailure, e: " + JSON.stringify(e));
};

function jsonToQueryString(json) {
  return '?' + 
    Object.keys(json).map(function(key) {
      return encodeURIComponent(key) + '=' +
        encodeURIComponent(json[key]);
  }).join('&');
}

Pebble.addEventListener("showConfiguration", function(e) {
  var currentSettings = {
    "AppKeyAnimations": localStorage.getItem('AppKeyAnimations') || 1,
  };
  var url = 'http://oddbloke.github.io/steelers-pebble-face/' + jsonToQueryString(currentSettings);
  console.log('URL: ' + url);
  Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("response: " + e.response);
  var configData = JSON.parse(decodeURIComponent(e.response));
  var msg = {
    'AppKeyAnimations': configData.AppKeyAnimations
  };
  localStorage.setItem('AppKeyAnimations', configData.AppKeyAnimations);
  console.log("msg: " + JSON.stringify(msg));
  Pebble.sendAppMessage(msg, sendSuccess, sendFailure);
});