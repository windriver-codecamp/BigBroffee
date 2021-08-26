import subprocess
from json import dumps
from bottle import Bottle, run, request, static_file, response

app = Bottle()

def start_application():
	filename = "/bd0a/t1.vxe"
	try:
		print("[INFO] Starting the application ...")
		subprocess.call(filename, shell=False)
	except FileNotFoundError:
		print ("No such file or directory: " + filename)

@app.route('/getStatus')
def getStatus():
	#print ("[INFO] Getting device status ...")
	text_line = ""
	#with open("/bd0a/motion_log.txt", "r") as file:
	with open("motion_log.txt", "r") as file:
		first_line = file.readline()
		for last_line in file:
			line = last_line.strip('\n')
			if line != '':
				text_line = line
			pass
		line_arr = text_line.split("@")
		print(line_arr)
	response.content_type = 'application/json'
	#return text_line
	rv = { "event": line_arr[0], "timestamp": line_arr[1] }
	return dumps(rv)


@app.route('/startApplication')
def startApplication():
	start_application()
	rv = { "event": "motion detection application started"}
	response.content_type = 'application/json'
	return dumps(rv)

run(app, host='0.0.0.0', port=8080)