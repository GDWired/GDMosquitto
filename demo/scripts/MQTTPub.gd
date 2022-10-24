extends Control


export var client_id: String = "MQTTPub" 
export var clean_session: bool = true 

export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

var _loop: bool = true
var _time: float = 0.0
var _x: float = 0.0
var _publish_frequency: float = 10

onready var _mqtt_client : Node = $MQTTClient
onready var _publish_button : Button = $Publish
onready var _publish_frequency_spin : SpinBox = $PublishFrequency


func _ready() -> void:
	# Init the client
	_mqtt_client.initialise(client_id, clean_session)
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)
	_mqtt_client.loop_start();
	
	# Init values from UI
	_loop = _publish_button.pressed
	_publish_frequency = _publish_frequency_spin.value


func _notification(what: int) -> void:
	# Clean everything when program is closed
	if what == NOTIFICATION_WM_QUIT_REQUEST:
		_mqtt_client.broker_disconnect()
		_mqtt_client.loop_stop(false)


func _process(delta: float) -> void:
	# Publish loop
	_time += delta
	_x += delta
	if _loop and _time >= (1.0 / _publish_frequency):
		_mqtt_client.publish("SIN", "[" + str(_x) + "," + str(sin(_x)) + "]", 0, false)
		_time = 0.0


func _on_Publish_toggled(button_pressed: bool) -> void:
	_loop = button_pressed


func _on_PublishFrequency_value_changed(value: float) -> void:
	_publish_frequency = value

