// Function to get current readings on the webpage when it loads for the first time
// function getReadings() {
//   var xhr = new XMLHttpRequest();
//   xhr.onreadystatechange = function () {
//     if (this.readyState == 4 && this.status == 200) {
//       var myObj = JSON.parse(this.responseText);
//       console.log(myObj);
//       var temp = myObj.temperature;
//       var hum = myObj.humidity;
//       gaugeTemp.value = temp;
//       gaugeHum.value = hum;
//     }
//   };
//   xhr.open("GET", "/readings", true);
//   xhr.send();
// }
function led(state) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", `/led?state=${state}`, true);
  xhr.send();
}

function engine(state) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", `/engine?state=${state}`, true);
  xhr.send();
}

document.querySelector('.btn-on').addEventListener('click', () => led('on'));
document.querySelector('.btn-off').addEventListener('click', () => led('off'));

document.querySelector('.engine-clockwise-on').addEventListener('click', () => engine('on'));
document.querySelector('.engine-clockwise-off').addEventListener('click', () => engine('off'));

// document.querySelector('.engine-anticlockwise-on').addEventListener('click', engineAnticlockwiseOn);
// document.querySelector('.engine-anticlockwise-off').addEventListener('click', engineAnticlockwiseOff);

if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function (e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('ping', function (e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Ping", e);
    }
  }, false);

  source.addEventListener('error', function (e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('message', function (e) {
    console.log("message", e.data);
  }, false);

  source.addEventListener('new_readings', function (e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    gaugeTemp.value = myObj.temperature;
    gaugeHum.value = myObj.humidity;
  }, false);
}