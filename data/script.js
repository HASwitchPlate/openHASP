function aref(e) { setTimeout(function() { ref("") }, 1e3 * e) }

function upd(e) {var o = (new Date).getTime(); return document.getElementById("bmp").src = "?a=" + e + "&q=" + o, !1 }

async function ref(e) {var o = (new Date).getTime(); const response = await fetch("/screenshot?d="+ o + "&a=" + e); if (response.ok) {upd(e)} else {aref(2)} }

function license(tft) { 
  var lic = '<h3>openHASP</h3>Copyright&copy; 2019-2022 Francis Van Roie</br>MIT License</p><p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p><p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p><p>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>';
  document.getElementById("lic").innerHTML = lic ;

  var libs = '<hr><p>Based on the previous work of the following open source developers:</p>';
  libs += '<h3>HASwitchPlate</h3><p>Copyright&copy; 2019 Allen Derusha allen @derusha.org</b></br>MIT License</p><h3>LVGL</h3><p>Copyright&copy; 2021 LVGL Kft</br>MIT License</p>';
  if (tft == 1) libs += '<h3>LovyanGFX</h3><p>Copyright&copy; 2020 lovyan03 (https://github.com/lovyan03) All rights reserved.</br>FreeBSD License</p>';
  libs += '<h3>TFT_eSPI</h3><p>Copyright&copy; 2020 Bodmer (https://github.com/Bodmer) All rights reserved.</br>FreeBSD License</p>';
  libs += '<h3>Adafruit_GFX</h3><p>Copyright&copy; 2012 Adafruit Industries. All rights reserved</br>BSD License</p>';
  libs += '<h3>ArduinoJson</h3><p>Copyright&copy; 2014-2021 Benoit BLANCHON</br>MIT License</p>';
  libs += '<h3>PubSubClient</h3><p>Copyright&copy; 2008-2015 Nicholas O&apos;Leary</br>MIT License</p>';
  libs += '<h3>ArduinoLog</h3><p>Copyright&copy; 2017,2018 Thijs Elenbaas, MrRobot62, rahuldeo2047, NOX73, dhylands, Josha blemasle, mfalkvidd</br>MIT License</p>';
  libs += '<h3>QR Code generator</h3><p>Copyright&copy; Project Nayuki</br>MIT License</p>';
  libs += '<h3>SimpleFTPServer</h3><p>Copyright&copy; 2017 Renzo Mischianti www.mischianti.org All right reserved.</br>MIT License</p>';
  libs += '<h3>AceButton</h3><p>Copyright&copy; 2018 Brian T. Park</br>MIT License</p>';
  document.getElementById("pkg").innerHTML = libs;
}
function about() {license(1);} 

function handleSubmit(event) {
  event.preventDefault();
  const data = new FormData(event.target);
  const value = Object.fromEntries(data.entries());
  console.log({ value });
}

function info() {
  data = JSON.parse(this.response);
  var table = "<table>";
  for (let header in data) { table += `<tr><td colspan=2></td></tr><tr><th colspan=2>${header}</th></tr>`; for (let key in data[header]) { table += `<tr><td>${key}: </td><td>${data[header][key]}</td></tr>`; } }
  table += "</table>";
  document.getElementById("info").innerHTML = table;
}

function loader(method, uri, func) {
  window.addEventListener("load", function() {
    var rq = new XMLHttpRequest();
    rq.addEventListener("load", func);
    rq.open(method, uri);
    rq.send();
  })
}

function fill() {
  data = JSON.parse(this.response);
  form = document.forms.item(0);
  populate(form, data);
}

function filler(method, uri) {
  window.addEventListener("load", function() {
    var rq = new XMLHttpRequest();
    rq.addEventListener("load", fill);
    rq.open(method, uri);
    rq.send();
  })
}

function filler2(method, uri) {
  var rq = new XMLHttpRequest();
  rq.addEventListener("load", fill);
  rq.open(method, uri);
  rq.send();
}

/** https://github.com/dannyvankooten/populate.js
 * Populate form fields from a JSON object.
 *
 * @param form object The form element containing your input fields.
 * @param data array JSON data to populate the fields with.
 * @param basename string Optional basename which is added to `name` attributes
 */
function populate(form, data, basename) {
  for (var key in data) {
    if (!data.hasOwnProperty(key)) {
      continue;
    }

    var name = key;
    var value = data[key];

    if ('undefined' === typeof value) {
      value = '';
    }

    if (null === value) {
      value = '';
    }

    // handle array name attributes
    if (typeof(basename) !== "undefined") {
      name = basename + "[" + key + "]";
    }

    if (value.constructor === Array) {
      name += '[]';
    } else if (typeof value == "object") {
      populate(form, value, name);
      continue;
    }

    // only proceed if element is set
    var element = form.elements.namedItem(name);
    if (!element) {
      continue;
    }

    var type = element.type || element[0].type;

    switch (type) {
      default: element.value = value;
      break;

      case 'radio':
        var values = value.constructor === Array ? value : [value];
        for (var j = 0; j < element.length; j++) {
          element[j].checked = values.indexOf(Number(element[j].value)) > -1;
        }
        break;

      case 'checkbox':
        var values = value.constructor === Array ? value : [value];
        element.checked = values.indexOf(Number(element.value)) > -1;
        break;

      case 'select-multiple':
        var values = value.constructor === Array ? value : [value];
        for (var k = 0; k < element.options.length; k++) {
          element.options[k].selected = (values.indexOf(element.options[k].value) > -1);
        }
        break;

      case 'select':
          case 'select-one':
          element.value = value.toString() || value;
        break;

      case 'date':
          element.value = new Date(value).toISOString().split('T')[0];
        break;
    }

  }
};