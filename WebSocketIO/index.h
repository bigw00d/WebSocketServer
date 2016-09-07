static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>WiFi制御</title>
<script>
var W=400;
var ws;
function $(id){return document.getElementById(id);}
function setup(){
  ws=new WebSocket('ws://'+window.location.hostname+':81/');
  ws.onopen  = function(e){ console.log('websock open'); };
  ws.onclose = function(e){ console.log('websock close'); };
  ws.onerror = function(e){ console.log(e); };
  ws.onmessage=function(e){
    console.log(e);
    if(e.data=='ledon')       $('LED').style.color='red';
    else if(e.data=='ledoff') $('LED').style.color='black';
    else printBars(e.data);
  };
}
function printBars(w){  $("BAR").style.width=w*4+"px";  $("_BAR").style.width=(W-w*4)+"px";}
</script>
</head>
<body onload=setup()>
<div align=right>2016.6.13</div>
<h1>WiFi制御</h1>
<div id="LED" style="font-size:100px">●</div>
<table border=0 cellspacing=0 cellpadding=0><tr>
  <td id="BAR"  style='background-color:lightgreen;width:0px;height:60px'></td>
  <td id="_BAR" style='background-color:gray;width:400px;height:60px'></td>
</tr></table>
<input id="SLIDER" type="range" value=0 min=0 max=100 step=5 onchange="ws.send(this.value)"><br>
<button onclick="ws.send('ledon')">On</button> <button onclick="ws.send('ledoff')">Off</button>
<hr><div align=right>koyama@hirosaki-u.ac.jp</div>
</body>
</html>
)rawliteral";
