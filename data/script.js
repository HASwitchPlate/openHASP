function aref(e) {
  setTimeout(function () {
    ref("");
  }, 1e3 * e);
}
function upd(e) {
  var t = new Date().getTime();
  return (document.getElementById("bmp").src = "?a=" + e + "&q=" + t), !1;
}
async function ref(e) {
  var t = new Date().getTime();
  (await fetch("/screenshot?d=" + t + "&a=" + e)).ok ? upd(e) : aref(2);
}
function about() {
  document.getElementById("lic").innerHTML =
    '<h3>openHASP</h3>Copyright&copy; 2019-2022 Francis Van Roie</br>MIT License</p><p>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p><p>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.</p><p>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>';
  document.getElementById("pkg").innerHTML =
    "<hr><p>Based on the previous work of the following open source developers:</p><h3>HASwitchPlate</h3><p>Copyright&copy; 2019 Allen Derusha allen @derusha.org</b></br>MIT License</p><h3>LVGL</h3><p>Copyright&copy; 2021 LVGL Kft</br>MIT License</p><h3>LovyanGFX</h3><p>Copyright&copy; 2020 lovyan03 (https://github.com/lovyan03) All rights reserved.</br>FreeBSD License</p><h3>TFT_eSPI</h3><p>Copyright&copy; 2020 Bodmer (https://github.com/Bodmer) All rights reserved.</br>FreeBSD License</p><h3>Adafruit_GFX</h3><p>Copyright&copy; 2012 Adafruit Industries. All rights reserved</br>BSD License</p><h3>ArduinoJson</h3><p>Copyright&copy; 2014-2021 Benoit BLANCHON</br>MIT License</p><h3>PubSubClient</h3><p>Copyright&copy; 2008-2015 Nicholas O&apos;Leary</br>MIT License</p><h3>ArduinoLog</h3><p>Copyright&copy; 2017,2018 Thijs Elenbaas, MrRobot62, rahuldeo2047, NOX73, dhylands, Josha blemasle, mfalkvidd</br>MIT License</p><h3>QR Code generator</h3><p>Copyright&copy; Project Nayuki</br>MIT License</p><h3>SimpleFTPServer</h3><p>Copyright&copy; 2017 Renzo Mischianti www.mischianti.org All right reserved.</br>MIT License</p><h3>AceButton</h3><p>Copyright&copy; 2018 Brian T. Park</br>MIT License</p>";
}
function handleSubmit(e) {
  e.preventDefault();
  const t = new FormData(e.target),
    o = Object.fromEntries(t.entries());
  console.log({ value: o });
}
function info() {
  data = JSON.parse(this.response);
  var e = "<table>";
  for (let t in data) {
    e += `<tr><td colspan=2></td></tr><tr><th colspan=2>${t}</th></tr>`;
    for (let o in data[t])
      e += `<tr><td>${o}: </td><td>${data[t][o]}</td></tr>`;
  }
  (e += "</table>"), (document.getElementById("info").innerHTML = e);
}
function loader(e, t, o) {
  window.addEventListener("load", function () {
    var n = new XMLHttpRequest();
    n.addEventListener("load", o), n.open(e, t), n.send();
  });
}
function fill() {
  data = JSON.parse(this.response);
  for (const form of document.forms) populate(form, data);
}
function filler(e, t) {
  window.addEventListener("load", function () {
    var o = new XMLHttpRequest();
    o.addEventListener("load", fill), o.open(e, t), o.send();
  });
}
function filler2(e, t) {
  var o = new XMLHttpRequest();
  o.addEventListener("load", fill), o.open(e, t), o.send();
}
function populate(e, t, o) {
  for (var n in t)
    if (t.hasOwnProperty(n)) {
      var r = n,
        i = t[n];
      if (
        (void 0 === i && (i = ""),
        null === i && (i = ""),
        void 0 !== o && (r = o + "[" + n + "]"),
        i.constructor === Array)
      )
        r += "[]";
      else if ("object" == typeof i) {
        populate(e, i, r);
        continue;
      }
      var a = e.elements.namedItem(r);
      if (a)
        switch (a.type || a[0].type) {
          default:
            a.value = i;
            break;
          case "radio":
            for (
              var s = i.constructor === Array ? i : [i], c = 0;
              c < a.length;
              c++
            )
              a[c].checked = s.indexOf(Number(a[c].value)) > -1;
            break;
          case "checkbox":
            (s = i.constructor === Array ? i : [i]),
              (a.checked = s.indexOf(Number(a.value)) > -1);
            break;
          case "select-multiple":
            s = i.constructor === Array ? i : [i];
            for (var p = 0; p < a.options.length; p++)
              a.options[p].selected = s.indexOf(a.options[p].value) > -1;
            break;
          case "select":
          case "select-one":
            a.value = i.toString() || i;
            break;
          case "date":
            a.value = new Date(i).toISOString().split("T")[0];
        }
    }
}
