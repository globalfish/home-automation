
var c1 = document.getElementById("floorPlan"); // static image
var ctx1 = c1.getContext("2d");

ctx1.scale(1.5,1.5);
// load icon images
var imgLight = new Image();
imgLight.src = "lightbulb.png";
var imgMotionOn = new Image();
imgMotionOn.src = "motionOn.png";
var imgMotionOff = new Image();
imgMotionOff.src = "motionOff.png";
var imgTemp = new Image();
imgTemp.src = "temperature.png";
var imgOpen = new Image();
imgOpen.src = "doorOpen.png";
var imgClosed = new Image();
imgClosed.src = "doorClosed.png";

// websocket callbacks
window.onload = function() {

    var d = document.getElementById("floorPlan");
    var ctx = d.getContext("2d");
    
    // draw out the floor outlines
    drawFloor1(10,10, "Basement Level");
    drawFloor2(200,220, "Family Level");
    drawFloor3(430,10, "Bedroom Level");
    drawLegend(ctx, 10, 300);
  
    // one socket per update for each sensor value
    // sockets named by Sensor Id (s01...) and measurement (...t01, m01)
    var s01 = new WebSocket("ws://192.168.86.36:1880/sensorData");
    s01.onopen = function(e) {
	ctx.fillStyle = "brown";
	//ctx.fillText("Waiting for data...",20, 450);
	ctx.className = "open";
    }

    s01.onerror = function(e) {
	console.log("WebSocket error: " + e);
    }

    s01.onmessage = function(e) {

	var sensorData = JSON.parse(e.data);
	ctx.font = "italic 16pt Calibri";
	ctx.fillStyle = "black";
	var x1, y1;
	x1 = 50;
	y1 = 220; //default
	switch (sensorData.SensorId) {
	case "1":
	    switch (sensorData.Measurement) {
	    case "light":
		x1 = 300;
		y1 = 145;
		break;
	    case "motion":
		x1 = 310;
		y1 = 170;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 5500, 3000);
		else
		    drawMotionOff(ctx, 5500, 3000);
		break;
	    case "light2":
		x1 = 205;
		y1 = 50;
		break;
	    case "motion2":
		x1 = 200;
		y1 = 80;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 3500, 1100);
		else
		    drawMotionOff(ctx, 3500, 1100);
		break;
	    case "temperature":
		x1 = 300;
		y1 = 120;
		break;
	    }
	    break;
	case "2":
	    switch (sensorData.Measurement) {
	    case "light":
		x1 = 415;
		y1 = 370;
		break;
	    case "motion":
		x1 = 425;
		y1 = 395;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 7800, 7500);
		else
		    drawMotionOff(ctx, 7800, 7500);
		break;
	    case "light2":
		break; // not implemented
	    case "motion2":
		break;
	    case "temperature":
		x1 = 415;
		y1 = 345;
		break;
	    case "door":
	     	if( sensorData.Value == "1") 
	     	    drawDoorOpen(ctx, 7000,8700);
	     	
	     	else 
	     	    drawDoorClosed(ctx, 7000, 8700);
	    	break;
	    }
	    break;
	case "3":
	    switch (sensorData.Measurement) {
	    case "light":
		x1 = 550;
		y1 = 95;
		break;
	    case "motion":
		x1 = 560;
		y1 = 120;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 10500, 2000);
		else
		    drawMotionOff(ctx, 10500, 2000);
		break;
	    case "light2":
		x1 = 700;
		y1 = 95;
		break;
	    case "motion2":
		x1 = 710;
		y1 = 130;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 13500, 2000);
		else
		    drawMotionOff(ctx, 13500, 2000);
		break;
	    case "temperature":
		x1 = 620;
		y1 = 120;
		break;
	    }
	    break;
	case "4":
	    switch (sensorData.Measurement) {
	    case "light":
		x1 = 550;
		y1 = 370;
		break;
	    case "motion":
		x1 = 560;
		y1 = 395;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 10500, 7500);
		else
		    drawMotionOff(ctx, 10500, 7500);

		break;
	    case "temperature":
		x1 = 550;
		y1 = 345;
		break;
	    case "light2": // do nothing since not implemented
	    case "motion2": 
		break;
	    }
	    break;
	case "5":
	    switch (sensorData.Measurement) {
	    case "light":
		break;
	    case "motion":
		x1 = 560;
		y1 = 310;
		if( sensorData.Value == "1")
		    drawMotionOn(ctx, 10500, 5800);
		else
		    drawMotionOff(ctx, 10500, 5800);

		break;
	    case "light2":
		x1 = 550;
		y1 = 285;
		break;
	    case "motion2":
		break;
	    case "temperature1":
		x1 = 620;
		y1 = 260;
		break;
	    case "temperature2":
		x1 = 550;
		y1 = 260;
		break;
	    case "door1":
		if( sensorData.Value == "1") 
		    drawDoorOpen(ctx, 9200,6450);
		else 
		    drawDoorClosed(ctx, 9200, 6450);
		break;
	    case "door2":
	    	if( sensorData.Value == "1") 
	    	    drawDoorOpen(ctx, 9200,4450);
	    	else 
	    	    drawDoorClosed(ctx, 9200, 4450);
	    	break;
	    }
    	    break;
	}
	ctx.clearRect(x1,y1-15,20,20);
	if( sensorData.Measurement.substring(0,4) != "door" &&
	    sensorData.Measurement.substring(0,6) != "motion") {
	    ctx.fillText( sensorData.Value, x1, y1);
	}
    }
}

function drawFloor1(x, y, caption) {

    var width = 350;
    var depth = 175;
    ctx1.beginPath();
    ctx1.moveTo(x,y);
    ctx1.lineTo(x+width,y+0);
    ctx1.lineTo(x+width,y+depth);;
    ctx1.lineTo(x+0,y+depth);
    ctx1.lineTo(x+0,y+0);
    ctx1.strokeStyle = "LightBlue";
    ctx1.stroke();

    ctx1.scale(0.05,0.05);
    ctx1.drawImage(imgLight, 3500, 600);
    ctx1.drawImage(imgTemp, 5500, 2000);
    ctx1.drawImage(imgLight, 5500, 2500);
    ctx1.scale(20,20);
    
    drawMotionOff(ctx1, 3500, 1100);
    drawMotionOff(ctx1, 5500, 3000);
    drawDoorClosed(ctx1,1000, 250);
    
    ctx1.font = "24pt Calibri";
    ctx1.fillStyle = "LightGray";
    ctx1.fillText(caption, x+20,y+(depth*0.7));
    
}

function drawFloor2(x, y, caption) {

    var width = 400;
    var depth = 200;
    ctx1.beginPath();
    ctx1.moveTo(x,y);
    ctx1.lineTo(x+width,y+0);
    ctx1.lineTo(x+width,y+depth);
    ctx1.lineTo(x+width/2,y+depth);
    ctx1.lineTo(x+width/2,y+depth+depth/4);
    ctx1.lineTo(x+width/4,y+depth+depth/4);
    ctx1.lineTo(x+width/4,y+depth);
    ctx1.lineTo(x+0,y+depth);
    ctx1.lineTo(x+0,y+0);
    ctx1.moveTo(x+width,y+depth/2);
    ctx1.lineTo(x+width/2+width/8,y+depth/2);
    ctx1.lineTo(x+width/2+width/8,y+depth);
    ctx1.strokeStyle = "LightBlue";
    ctx1.stroke();
    
    ctx1.scale(0.05,0.05);
    ctx1.drawImage(imgTemp, 10500, 4800);
    ctx1.drawImage(imgLight, 10500, 5300);
    ctx1.drawImage(imgTemp, 7800, 6500);
    ctx1.drawImage(imgLight, 7800, 7000);
    ctx1.drawImage(imgTemp, 10500, 6500);
    ctx1.drawImage(imgLight, 10500, 7000);
    ctx1.drawImage(imgTemp, 12000, 4800);
    ctx1.scale(20,20);

    drawMotionOff(ctx1, 10500, 5800);
    drawMotionOff(ctx1, 7800, 7500);
    drawMotionOff(ctx1, 10500, 7500);

    drawDoorClosed(ctx1, 9200, 4450);
    drawDoorClosed(ctx1, 9200, 6450);
    drawDoorClosed(ctx1, 7000, 8700);
    
    ctx1.font = "24pt Calibri";
    ctx1.fillStyle = "LightGray";
    ctx1.fillText(caption, x+20,y+depth*0.3);
    ctx1.font = "16pt Calibri";
    ctx1.fillText("Garage", x+253, y+depth*0.95);
}

function drawFloor3(x, y, caption) {

    var width = 350;
    var depth = 175;
    ctx1.beginPath();
    ctx1.moveTo(x,y);
    ctx1.lineTo(x+width,y+0);
    ctx1.lineTo(x+width,y+depth-50);
    ctx1.lineTo(x+width-200,y+depth-50);
    ctx1.lineTo(x+width-200,y+depth);
    ctx1.lineTo(x+0,y+depth);
    ctx1.lineTo(x+0,y+0);
    ctx1.strokeStyle = "LightBlue";
    ctx1.stroke();

    ctx1.scale(0.05,0.05);
    ctx1.drawImage(imgLight, 13500, 1500);
    ctx1.drawImage(imgTemp, 12000, 2000);
    ctx1.drawImage(imgLight, 10500, 1500);
    ctx1.scale(20,20);

    drawMotionOff(ctx1, 13500, 2000);
    drawMotionOff(ctx1, 10500, 2000);

    ctx1.font = "24pt Calibri";
    ctx1.fillStyle = "LightGray";
    ctx1.fillText(caption, x+20,y+depth*0.3);
}

function drawLegend(ctx, x, y) {

    ctx.font = "10pt Calibri";
    ctx.fillStyle = "DimGray";
    ctx.fillText("Legend", x, y);
    ctx.moveTo(x, y+10);
    ctx.lineTo(x+150, y+10);
    ctx.lineTo(x+150, y+180);
    ctx.lineTo(x, y+180);
    ctx.lineTo(x, y+10);
    ctx.strokeStyle = "LightBlue";
    ctx.stroke();

    ctx.scale(0.04,0.04); 
    ctx.drawImage(imgTemp, x+300, (y+20)*25);
    ctx.drawImage(imgLight, x+300, (y+45)*25);
    ctx.drawImage(imgOpen, x+300, (y+70)*25);
    ctx.drawImage(imgClosed, x+300, (y+95)*25);
    ctx.drawImage(imgMotionOff, x+300, (y+120)*25);
    ctx.drawImage(imgMotionOn, x+300, (y+145)*25);
    ctx.scale(25,25);

    ctx.fillText("Temperature (�F)", x+30,y+35);
    ctx.fillText("Light level (0-20)", x+30,y+60);
    ctx.fillText("Door Open", x+30,y+85);
    ctx.fillText("Door Closed", x+30,y+110);
    ctx.fillText("No Motion", x+30,y+135);
    ctx.fillText("Motion Sensed", x+30,y+160);
}

function drawDoorOpen(ctx, x, y) {

    ctx.scale(0.05,0.05);
    ctx.clearRect(x,y,512,512);
    ctx.drawImage(imgOpen, x, y);
    ctx.scale(20,20);
}

function drawDoorClosed(ctx, x, y) {

    ctx.scale(0.05,0.05);
    ctx.clearRect(x,y,512,512);
    ctx.drawImage(imgClosed, x, y);
    ctx.scale(20,20);
}

function drawMotionOn(ctx, x, y) {

    ctx.scale(0.05,0.05);
    ctx.drawImage(imgMotionOn, x, y);
    ctx.scale(20,20);
}

function drawMotionOff(ctx, x, y) {

    ctx.scale(0.05,0.05);
    ctx.drawImage(imgMotionOff, x, y);
    ctx.scale(20,20);
}

function drawTemperature(ctx, x, y) {
    ctx.scale(0.05, 0.05);
    ctx.drawImage(imgTemp, x, y);
    ctx.scale( 20,20);
}

function drawLight(ctx, x, y) {
    ctx.scale(0.05, 0.05);
    ctx.drawImage(imgLight, x, y);
    ctx.scale( 20,20);
}
