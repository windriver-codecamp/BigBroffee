import subprocess
from json import dumps
from bottle import Bottle, run, request, static_file, response

app = Bottle()

@app.route('/getStatus')
def getStatus():
	#print ("[INFO] Getting device status ...")
	text_line = ""
	with open("/bd0a/motion_log.txt", "r") as file:
	#with open("motion_log.txt", "r") as file:
		first_line = file.readline()
		for last_line in file:
			line = last_line.strip('\n')
			if line != '':
				text_line = line
			pass
		line_arr = text_line.split("@")

		if line_arr[0] == '':
			rv = { "event": '', "timestamp": '' }
		else:
			rv = { "event": line_arr[0], "timestamp": line_arr[1] }

		#print(line_arr)
	response.content_type = 'application/json'
	return dumps(rv)

run(app, host='0.0.0.0', port=8081, quiet=True)
