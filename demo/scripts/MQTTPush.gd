extends Control


export var client_id: String = "MQTTPush" 
export var clean_session: bool = true 

export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

var _loop_start_supported: bool = true
var _loop_thread: Thread = null

onready var _mqtt_client : Node = $MQTTClient
onready var _publish_button : Button = $Publish
onready var _text_to_send : LineEdit = $Text


func _ready() -> void:
	# Init the client
	_mqtt_client.initialise(client_id, clean_session)
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)
	_loop_start_supported = not (_mqtt_client.loop_start() == GDMosquitto.RC.MOSQ_ERR_NOT_SUPPORTED)
	
	# On Windows, mosquitto is not compiled with pthread...
	if not _loop_start_supported:
		_mqtt_client.threaded_set(true)
		_loop_thread = Thread.new()
		if _loop_thread.start(self, "_mqtt_client_loop") != OK:
			printerr("Error while the loop thread is created")


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


func _on_Publish_pressed() -> void:
	_mqtt_client.publish("DATA", _text_to_send.text, 1, false)

