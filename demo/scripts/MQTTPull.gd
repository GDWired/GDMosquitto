extends Control


export var client_id: String = "MQTTPull" 
export var clean_session: bool = true 

export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

var _loop_start_supported: bool = true
var _loop_thread: Thread = null

onready var _mqtt_client : Node = $MQTTClient
onready var _data : Node = $Data


func _ready() -> void:
	# Init the client
	_mqtt_client.initialise(client_id, clean_session)
	yield(get_tree().create_timer(0.5), "timeout")
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)
	_loop_start_supported = not (_mqtt_client.loop_start() == GDMosquitto.RC.MOSQ_ERR_NOT_SUPPORTED)
	
	# On Windows, mosquitto is not compiled with pthread...
	if not _loop_start_supported:
		_mqtt_client.threaded_set(true)
		_loop_thread = Thread.new()
		if _loop_thread.start(self, "_mqtt_client_loop") != OK:
			printerr("Error while the loop thread is created")
	
	# Init values from UI
	_mqtt_client.subscribe("DATA", 1)


func _mqtt_client_loop():
	_mqtt_client.loop_forever(0)


func _notification(what: int) -> void:
	# Clean everything when program is closed
	if what == NOTIFICATION_WM_QUIT_REQUEST:
		_mqtt_client.broker_disconnect()
		if _loop_start_supported:
			_mqtt_client.loop_stop(false)
		else:
			_loop_thread.wait_to_finish()


func _on_MQTTClient_received(_message_id: int, _topic: String, payload: String) -> void:
	# MQTT data received
	if payload == "":
		_data.text = "<empty>"
	else:
		_data.text = payload

