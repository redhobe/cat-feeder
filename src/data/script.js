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

// import * as mock from './mock/mock.js'

let config;
async function led(state) {
  await fetch(`/led?state=${state}`);
}

async function engine(state) {
  await fetch(`/engine?state=${state}`);
}

async function getConfig() {
  const response = await fetch(`http://192.168.1.97/config`);

  if (response.ok) {
    const json = await response.json();
    return json;
  } else {
    // alert("Ошибка при получении данных конфигурации: код ошибки - " + response.status);
  }
}


async function saveConfig() {
  console.log('click')
  const response = await fetch('http://192.168.1.97/config', {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(config),
  });

  if (!response.ok) {
    // alert("Ошибка при сохранении данных конфигурации: код ошибки - " + response.status);
  }
}


function getScheduleRowNode(time, portions, index) {
  const tr = document.createElement('tr');
  tr.dataset.index = index;
  tr.innerHTML = `
          <td>
            <feeder-input
              type="time"
              value="${time}"
              required
            />
          </td>
          <td>
            <feeder-input
              type="number"
              min="1"
              max="10"
              step="1"
              value="${portions}"
              required
              pattern="\d*"
            />
          </td>
          <td><button class="button--error button--round"></button></td>`;
  tr.querySelector('button').addEventListener('click', () => onScheduleRowDeleteClick(tr));
  tr.querySelector('feeder-input[type="time"]').addEventListener('change', onScheduleRowTimeChange);
  tr.querySelector('feeder-input[type="number"]').addEventListener('change', onScheduleRowPortionsChange);
  return tr;
}

const onScheduleRowDeleteClick = (element) => {
  const index = element.dataset.index;
  config.schedule.splice(index, 1);
  renderScheduleTable();
}

function onScheduleRowTimeChange(event) {
  setTimeout(() => {
    const index = event.target.parentElement.parentElement.parentElement.dataset.index;
    config.schedule[index].time = event.target.value;
  }, 0);
}

function onScheduleRowPortionsChange(event) {
  setTimeout(() => {
    const index = event.target.parentElement.parentElement.parentElement.dataset.index;
    config.schedule[index].portions = event.target.value;
  }, 0);
}


window.onload = async () => {
  config = await getConfig();
  renderScheduleTable();
  console.log(config)
};

// document.querySelector('.btn-on').addEventListener('click', () => led('on'));
// document.querySelector('.btn-off').addEventListener('click', () => led('off'));

// document.querySelector('.engine-clockwise-on').addEventListener('click', () => engine('on'));
// document.querySelector('.engine-clockwise-off').addEventListener('click', () => engine('off'));

// document.querySelector('.engine-anticlockwise-on').addEventListener('click', engineAnticlockwiseOn);
// document.querySelector('.engine-anticlockwise-off').addEventListener('click', engineAnticlockwiseOff);

document.querySelector('[data-save-schedule-button]').addEventListener('click', saveConfig);
document.querySelector('[data-add-schedule-button]').addEventListener('click', () => {
  const time = document.querySelector('#time').input.value;
  const portions = document.querySelector('#portions').input.value;

  if (time && portions) {
    config.schedule.push({ time, portions });
    renderScheduleTable();
  }
});

function renderScheduleTable() {
  const scheduleTableBody = document.querySelector('#schedule-body');
  scheduleTableBody.innerHTML = '';

  config.schedule.forEach(({ time, portions }, index) => {
    scheduleTableBody.appendChild(getScheduleRowNode(time, portions, index));
  });
}


if (!!window.EventSource) {
  var source = new EventSource('http://192.168.1.97/events');

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
    json = JSON.parse(e.data);
    console.log(myObj);
  }, false);
}

