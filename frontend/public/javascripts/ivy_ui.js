"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
let timers = [];
let prevClientErrorTimer = null;
let prevServerErrorTimer = null;
let optimal_values = {};
let last_water_time = null;

$(document).ready(function() {
	// call all these once a second to get live readings
	setInterval(light, 1000);
	setInterval(moisture, 1000);
	setInterval(humidityAndTemperature, 1000);
	setInterval(calculateSensorScore, 1000);
	setInterval(displayGraphs, 1000);
	setInterval(checkWaterPump, 1000);

	// setInterval(detectError, 1000);

	// send command to get live light reading
	$('#light-reading').click(function(){
		sendCommand("light-reading");
	});
	// send command to get live moisture reading
	$('#moisture-reading').click(function(){
		sendCommand("moisture-reading");
	});
	// send command to get live humidity reading
	$('#humidity-reading').click(function(){
		sendCommand("humidity-reading");
	});
	// send command to get live temperature reading
	$('#temperature-reading').click(function(){
		sendCommand("temperature-reading");
	});
	// send command to manually water plant
	$('#pump-btn').click(function(){
		console.log('calling pump ************************')
		const button = document.getElementById('pump-btn')
		button.style.backgroundColor = '#2584C4'
		button.style.color = 'white'
		button.value = 'watering...'
		last_water_time = new Date()
		sendCommand("pump-on");
	});
	// send command to manually water plant
	$('#display-btn').click(function(){
		console.log('calling display ************************')
		
		sendCommand("display");
	});
	
	
	
	socket.on('commandReply', function(result) {
		
		let res = result.split(' ');
		if(res[0] == "Error" && $('#error-box').css('display') == 'none'){
			let errorMsg = res.slice(1).join(' ');
			document.getElementById("error-text").innerHTML = errorMsg;
			
			$('#error-box').show();
			let errorTimer = setTimeout(function() {
				$('#error-box').hide();
			}, 10000);

			return;
		}

		let tag = res[0];
		let val = res[1];



		// console.log(tag, val)
		if(tag === 'light-reading'){
			// use innerHTML
			val = get_lux(val)
			document.getElementById(tag).innerHTML = val + ' LUX';

		} else if(tag == 'moisture-reading'){
			document.getElementById(tag).innerHTML = val;
		} else if(tag == 'humidity-temperature-reading'){
			const humidityAndTemp = val.split('-');
			const hum = humidityAndTemp[0];
			const temp = humidityAndTemp[1];
			// console.log('%$%$%$%$%$', hum, temp);
			document.getElementById('humidity-reading').innerHTML = parseFloat(hum).toFixed(2) + '%';
			document.getElementById('temperature-reading').innerHTML = parseFloat(temp).toFixed(2) + 'C';
		}
	});

	// socket.on('fileContents', function(result) {
	// 	let SECONDS = result.contents.split(' ')[0];
	// 	document.getElementById("status").innerHTML = 'Device up for:\n' + new Date(SECONDS * 1000).toISOString().substr(11, 8) + '(H:M:S)';
	// });

	/* ....WHEN GETTING CSV VALUES....
	*	- items are 6 apart... need to pull i value and then navigate to 
	*	  each column (Plant Name(i), Temp, Humidity, Moisture, pH, Light)
	*/

	/**
	 * Make a dict of the plant type and its values
	 * {'snake_plant': {temp: '', humidity: '', ...}}
	 * so when we want to use a certain plant, we just look at what is selected in the dropdown and then select the corresponding optimal values.
	 */
	$.get( "plants.csv", function(CSVdata) {
		// Data holds items
		let data = $.csv.toArray(CSVdata);
		console.log(data)
		var select = document.getElementById("select-plant");

		for(var i = 6; i < data.length; i+=6) {
    		var opt = data[i];
    		var el = document.createElement("option");
			el.classList.add('opts');
    		el.textContent = opt;
    		el.value = opt;
    		select.appendChild(el);

			// append data to the optimal dict
			if(!(opt in optimal_values)){
				// if dict does not exist, init with dict as value
				optimal_values[opt] = {};
			}
			
			optimal_values[opt]['Temp'] = data[i+1];
			optimal_values[opt]['Humidity'] = data[i+2];
			optimal_values[opt]['Moisture'] = data[i+3];
			optimal_values[opt]['pH'] = data[i+4];
			optimal_values[opt]['Light'] = data[i+5];
		}
		
		// set the opt values when new plant is selected
		select.addEventListener('change', () => setPlantOptimalValues());

		// set the optimal values based on the selected option
		setPlantOptimalValues();
	});
});

function get_lux(val){
	let result = 0;
	if (val >= 1.79) {
		result = 60000;
	} else if (val >= 1.76) {
		result = 37500
	} else if(val >= 1.5){
		result = 17000
	} else if (val >= 1.47) {
		result = 8000
	} else if (val >= 1.22) {
		result = 250
	} else if (val >= 0.72) {
		result = 90
	} else if (val >= 0.64) {
		result = 45
	} else if (val >= 0.16) {
		result = 6.7
	} else if (val >= 0.07) {
		result = 2.2
	} else if (val >= 0.03) {
		result = 0.45
	} else if (val >= 0.00) {
		result = 0.05
	} else {
		result = 0.00
	}

	return result
}

function light(){
	console.log('calling light');
	sendCommand('light-reading');
}

function moisture(){
	console.log('calling moisture');
	sendCommand('moisture-reading');
}

function humidityAndTemperature(){
	console.log('calling humidity and temperature');
	sendCommand('humidity-temperature-reading');
}

function sendCommand(message) {
	socket.emit('ivy', message);
};

function checkWaterPump(){
	if(last_water_time === null){
		return
	}
	// wait for 10 seconds before turning off pump
	var t = new Date();
	t.setSeconds(t.getSeconds() - 5);
	if(last_water_time < t){
		sendCommand("pump-off")
		const button = document.getElementById('pump-btn')
		button.style.backgroundColor = 'white'
		button.style.color = '#494949'
		button.value = 'water your plant!'
		last_water_time = null;
	}
}


// CHECK IF IMAGE EXISTS
function checkIfImageExists(url) {
	const img = new Image();
	img.src = url;
	
	if (img.complete) {
	  return true;
	} else {
	  img.onload = () => {
		return true;
	  };
	  
	  img.onerror = () => {
		return false;
	  };
	}
  }

async function displayGraphs(){
	const light_graph = document.getElementById("light-graph");
	const humidity_graph = document.getElementById("humidity-graph");
	const temp_graph = document.getElementById("temp-graph");

	// console.log(light_graph.src)
	var d = new Date();
	var datestring = ("0" + d.getDate()).slice(-2) + "-" + ("0"+(d.getMonth()+1)).slice(-2) + "-" +
    d.getFullYear() + "-" + ("0" + d.getHours()).slice(-2) + ":" + ("0" + d.getMinutes()).slice(-2);

	const light_url = "http://localhost:8088/lightfig-" + datestring + ".png";
	if(checkIfImageExists(light_url)){
		light_graph.src = light_url;
	}
	const humidity_url = "http://localhost:8088/humidityfig-" + datestring + ".png"
	if(checkIfImageExists(humidity_url)){
		humidity_graph.src = humidity_url;
	}

	const temp_url = "http://localhost:8088/tempfig-" + datestring + ".png";
	if(checkIfImageExists(temp_url)){
		temp_graph.src = temp_url
	}


	
}

function setProgressBar(circle){
	let progressBar = document.getElementById(`${circle}-progress-circle`);
	let valueContainer = document.getElementById(`${circle}-progress`);

	// let progressValue = 0;
	let progressEndValue = parseInt(valueContainer.textContent);
	// console.log(progressValue, progressEndValue)

	// let speed = 50;

	// let progress = setInterval(() => {
	// 	if (progressValue >= progressEndValue) {
	// 		clearInterval(progress);
	// 	}
	// 	progressValue++;
	// 	valueContainer.textContent = `${progressValue}%`;
	// 	progressBar.style.background = `conic-gradient(
	// 		#4d5bf9 ${progressValue * 3.6}deg,
	// 		#cadcff ${progressValue * 3.6}deg
	// 	)`;
	// }, speed);

	valueContainer.textContent = `${progressEndValue}%`;
		progressBar.style.background = `conic-gradient(
			#4d5bf9 ${progressEndValue * 3.6}deg,
			#cadcff ${progressEndValue * 3.6}deg
		)`;
}


function setPlantOptimalValues(){
	// get selected plant name from dropdown
	var select = document.getElementById("select-plant");

	const plant = select.value;
	// console.log(optimal_values)
	// console.log(plant)

	// set the optimal values for the four sensors
	const light = document.getElementById("optimum-light-value");
	const moisture = document.getElementById("optimum-moisture-value");
	const temperature = document.getElementById("optimum-temperature-value");
	const humidity = document.getElementById("optimum-humidity-value");

	light.innerHTML = optimal_values[plant]['Light'];
	moisture.innerHTML = optimal_values[plant]['Moisture'];
	temperature.innerHTML = optimal_values[plant]['Temp'];
	humidity.innerHTML = optimal_values[plant]['Humidity'];

}

function calculateSensorScore(){
	// take the live reading + optimal reading and determine a score
	const live_light = (document.getElementById("light-reading").textContent).slice(0, -4);
	const live_moisture = (document.getElementById("moisture-reading").textContent);
	const live_temperature = (document.getElementById("temperature-reading").textContent).slice(0, -1);
	const live_humidity = (document.getElementById("humidity-reading").textContent).slice(0, -1);

	// get currently selected plant
	var select = document.getElementById("select-plant");
	const plant = select.value;

	const light = optimal_values[plant]['Light'];
	const moisture = optimal_values[plant]['Moisture'];
	const temp = optimal_values[plant]['Temp'];
	const humidity = optimal_values[plant]['Humidity'];

	// for the values that are ranges, we must get the high and low
	const light_range = light.split('-').map((str) => parseInt(str));
	const temp_range = temp.split('-').map((str) => parseInt(str));
	const humidity_range = humidity.split('-').map((str) => parseInt(str));

	// moisture is not a range so simulate range by having same int as high and low
	const moisture_range = [parseInt(moisture), parseInt(moisture)];

	const light_health = get_health(live_light, light_range);
	const moisture_health = get_health(live_moisture, moisture_range);
	const temp_health = get_health(live_temperature, temp_range);
	const humidity_health = get_health(live_humidity, humidity_range);

	document.getElementById("light-progress").innerHTML = light_health;
	document.getElementById("moisture-progress").innerHTML = moisture_health;
	document.getElementById("temp-progress").innerHTML = temp_health;
	document.getElementById("humidity-progress").innerHTML = humidity_health;

	const overall_health = Math.floor((light_health + moisture_health + temp_health + humidity_health) / 4)
	const overall_health_bar = document.getElementById("overall-health");
	overall_health_bar.innerHTML = overall_health + '%';
	overall_health_bar.style.width = overall_health + '%';

	setProgressBar("light");
	setProgressBar("moisture");
	setProgressBar("temp");
	setProgressBar("humidity");

	
	if(parseInt(live_moisture) < parseInt(optimal_values[plant]['Moisture'])){
		console.log('turning on water pump')
		sendCommand("pump-on");
	} else {
		if(last_water_time === null){
			//  turn off the pump
			sendCommand("pump-off");
		}
	}


	// sendCommand("display");


}

// TODO: not sure if this works.  
function voltageToLux(voltage){
	const Vout = (voltage * 0.0048828125);
  	
	const RLDR = (10000.0 * (5 - Vout))/Vout;    // Equation to calculate Resistance of LDR, [R-LDR =(R1 (Vin - Vout))/ Vout]
   	// R1 = 10,000 Ohms , Vin = 5.0 Vdc.

	// print("%^%^%^%^%^%^%", 500.0/RLDR);
	return 500.0 / RLDR;
}

function get_health(live_reading, optimum_range){
	// console.log(optimum_range, live_reading)
	// live reading is within optimal values
	if(live_reading >= optimum_range[0] && live_reading <= optimum_range[1]){
		return 100;
	} else if(live_reading < optimum_range[0]){
		// live reading is below optimal
		return Math.floor((live_reading / optimum_range[0]) * 100);
	} else {
		// live reading is above optimal
		return Math.floor((optimum_range[1] / live_reading) * 100);
	}
}

// check if the nodejs server disconnected
function detectError(){
	socket.on('disconnect', function(){
		document.getElementById("error-text").innerHTML = 'CLIENT ERROR: Looks like your node.js server is down.';
		$('#error-box').show();
		let clientErrorTimer = setTimeout(function() {
			$('#error-box').hide();
		}, 10000);

		// remove all other timeouts before adding new one.
		if(prevClientErrorTimer){
			clearInterval(prevClientErrorTimer);
		}

		// set prev so we can remove it next time there is a newer error.
		prevClientErrorTimer = clientErrorTimer;

	})
}
