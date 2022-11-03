extends Control
class_name MQTTClient


signal connected(reason_code)
signal connected_with_flags(reason_code, username_flag, password_flag, will_retain, will_qos, will_flag, clean_session)
signal disconnected(reason_code)
signal error_received
signal log_received(level, message)
signal message_id_updated(message_id, topic, action)
signal published(message_id)
signal received(message_id, topic, payload)
signal subscribed(message_id, qos_count, granted_qos)
signal unsubscribed(message_id)


export var client_id: String = "MQTTClient" 
export var clean_session: bool = true 
export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

var _rng = RandomNumberGenerator.new()
var _loop_start_supported: bool = true
var _loop_thread: Thread = null
var _connected: bool = false

onready var _mqtt_client: Reference = preload("res://addons/GDMosquitto/GDMosquitto.gdns").new()


func _ready():
	_mqtt_client.connect("connected", self, "_on_MQTTClient_connected")
	_mqtt_client.connect("connected_with_flags", self, "_on_MQTTClient_connected_with_flags")
	_mqtt_client.connect("disconnected", self, "_on_MQTTClient_disconnected")
	_mqtt_client.connect("error", self, "_on_MQTTClient_error")
	_mqtt_client.connect("log", self, "_on_MQTTClient_log")
	_mqtt_client.connect("message_id_updated", self, "_on_MQTTClient_message_id_updated")
	_mqtt_client.connect("published", self, "_on_MQTTClient_published")
	_mqtt_client.connect("received", self, "_on_MQTTClient_received")
	_mqtt_client.connect("subscribed", self, "_on_MQTTClient_subscribed")
	_mqtt_client.connect("unsubscribed", self, "_on_MQTTClient_unsubscribed")
	
	_rng.randomize()
	var random_time = _rng.randf_range(100, 1000)
	
	# Init the client
	_mqtt_client.initialise(client_id, clean_session)
	yield(get_tree().create_timer(random_time / 1000), "timeout")
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)
	_loop_start_supported = not (_mqtt_client.loop_start() == GDMosquitto.RC.MOSQ_ERR_NOT_SUPPORTED)
	
	# On Windows, mosquitto is not compiled with pthread...
	if not _loop_start_supported:
		_mqtt_client.threaded_set(true)
		_loop_thread = Thread.new()
		if _loop_thread.start(self, "_mqtt_client_loop") != OK:
			printerr("Error while the loop thread is created")


func _mqtt_client_loop():
	_mqtt_client.loop_forever(-1)


func _notification(what: int) -> void:
	# Clean everything when program is closed
	if what == NOTIFICATION_WM_QUIT_REQUEST:
		_mqtt_client.broker_disconnect()
		if _loop_start_supported:
			_mqtt_client.loop_stop(false)
		else:
			_loop_thread.wait_to_finish()


func subscribe(topic: String, qos: int = 0) -> int:
	return _mqtt_client.subscribe(topic, qos)


func unsubscribe(topic: String) -> int:
	return _mqtt_client.unsubscribe(topic)


func publish(topic: String, payload: String, qos: int = 0, retain: bool = false) -> int:
	return _mqtt_client.publish(topic, payload, qos, retain)


func _on_MQTTClient_unsubscribed(message_id: int) -> void:
	 emit_signal("unsubscribed", message_id)


func _on_MQTTClient_subscribed(message_id: int, qos_count: int, granted_qos: int) -> void:
	 emit_signal("subscribed", message_id, qos_count, granted_qos)


func _on_MQTTClient_received(message_id: int, topic: String, payload: String) -> void:
	emit_signal("received", message_id, topic, payload)


func _on_MQTTClient_published(message_id: int) -> void:
	 emit_signal("published", message_id)


func _on_MQTTClient_message_id_updated(message_id: int, topic: String, action: int) -> void:
	 emit_signal("published", message_id, topic, action)


func _on_MQTTClient_log(level: int, message: String) -> void:
	 emit_signal("log_received", level, message)


func _on_MQTTClient_error() -> void:
	 emit_signal("error_received")


func _on_MQTTClient_disconnected(reason_code: int) -> void:
	 emit_signal("disconnected", reason_code)


func _on_MQTTClient_connected_with_flags(reason_code: int, username_flag: bool, password_flag: bool, will_retain: bool, will_qos: int, will_flag: bool, clean_session: bool) -> void:
	 emit_signal("connected_with_flags", reason_code, username_flag, password_flag, will_retain, will_qos, will_flag, clean_session)


func _on_MQTTClient_connected(reason_code: int) -> void:
	 emit_signal("connected", reason_code)

