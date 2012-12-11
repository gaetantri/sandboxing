var sys = require('sys')
var exec = require('child_process').exec;
var execSync = require("exec-sync");
var fs = require('fs');

function puts(error, stdout, stderr) { sys.puts(stdout) }

var vzdispo = [ 
	{nom : "vzsand0", vzid : "220", state:0},
	{nom : "vzsand1", vzid : "221", state:0},
	{nom : "vzsand2", vzid : "222", state:0},
	{nom : "vzsand3", vzid : "223", state:0},
	{nom : "vzsand4", vzid : "224", state:0}
		];

function SelectVZ() {
	vzselected = null;
	for(var i = 0 ; i < vzdispo.length && vzselected == null ; i++ ) {
		if ( vzdispo[i].state == 0) {
			vzdispo[i].state = 1;
			vzselected = i;
		}
	}
	console.log("VZ Selected : "+vzselected);
	return vzselected;
}

function cleanExistingMess() {
	for(var i = 0 ; i < vzdispo.length ; i++ ) {
		execSync("rm /home/"+vzdispo[i].nom+"/* -rf ");
	}
}

function protectBash(chaine) {
 var n=chaine.replace("\"","\\\""); 
 n = n.replace("'","\\'");
 n = n.replace("`","");
 return n;
}

// Cleaning existing files
cleanExistingMess();
// generate executionID
var executionID = 0;

var filesToMove = ["test.cpp" ];
var toExecute = "cd /root/"+executionID+" \n g++ test.cpp \n ./a.out \n";
toExecute = protectBash(toExecute);

// Selecting sandbox
vzsel = SelectVZ();
if(vzsel != null) {
	vzFolder = "/home/"+vzdispo[vzsel].nom+"/"+executionID ;
	// putting file in sandbox
	fs.mkdirSync(vzFolder);
	console.log("mkdir "+vzFolder);
	var fd = fs.openSync(vzFolder + "/run.sh", "w");
	console.log(vzFolder + "/run.sh");
	console.log("fd:"+fd);
	var n = fs.writeSync(fd,toExecute,0,toExecute.length,0);
	fs.closeSync(fd);
	execSync("chmod +x "+vzFolder+"/run.sh");
	console.log("echo \""+toExecute+"\" > \"" + vzFolder + "/run.sh\"");
	for(var i=0;i<filesToMove.length;i++){
		execSync("cp \""+protectBash(filesToMove[i])+"\" "+vzFolder+"/"); 	
		console.log("cp \""+protectBash(filesToMove[i])+"\" "+vzFolder+"/"); 	
	}
	// executing local script
	var net = require('net');
	var client = net.connect({port: 33333, host:"10.10.2.1"},
    		function() { //'connect' listener
	  console.log('connected');
	  client.write(vzdispo[vzsel].vzid+" "+executionID+"\n");
	});
	client.on('data', function(data) {
	  console.log(data.toString());
	});
	 client.on('end', function() {
  	 console.log('client disconnected');
	});

	// Cleaning mess
	vzdispo[vzsel].state=0;
}
