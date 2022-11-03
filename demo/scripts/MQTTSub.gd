extends Control


export var client_id: String = "MQTTSub" 
export var clean_session: bool = true 

export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

var _time: float = 0.0
var _data: Array = []
var _refresh_time: float = 1
var _loop_start_supported: bool = true
var _loop_thread: Thread = null

onready var _mqtt_client : Node = $MQTTClient
onready var _chart : Node = $GDCharts # Not a part of this plugin (https://github.com/binogure-studio/chart-gd)
onready var _subscribe_button : Node = $Buttons/Subscribe
onready var _refresh_time_spin : SpinBox = $Buttons/RefreshTime
  

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
	
	# Init chart
	_chart.initialize(_chart.LABELS_TO_SHOW.NO_LABEL, {sinus_x = Color(1.0, 0.18, 0.18)})
	_chart.set_max_values(100)
	
	# Init values from UI
	if _subscribe_button.pressed:
		_mqtt_client.subscribe("SIN", 0)
	_refresh_time = _refresh_time_spin.value


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


func _process(delta: float) -> void:
	# Refresh loop
	_time += delta
	if _time >= _refresh_time:
		for d in _data:
			_chart.create_new_point({label = d[0], values = { sinus_x = d[1] }})
		_data.clear()
		_time = 0.0


func _on_MQTTClient_received(_message_id: int, _topic: String, payload: String) -> void:
	# MQTT data received
	var result = JSON.parse(payload).result
	_data.append(result)


func _on_Subscribe_toggled(button_pressed: bool) -> void:
	if button_pressed:
		_mqtt_client.subscribe("SIN", 1)
	else:
		_mqtt_client.unsubscribe("SIN")


func _on_RefreshFrequency_value_changed(value: float) -> void:
	_refresh_time = value

