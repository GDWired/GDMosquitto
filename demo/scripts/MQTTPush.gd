extends Control


export var client_id: String = "MQTTPush" 
export var clean_session: bool = true 

export var broker_address: String = "localhost"
export var broker_port: int = 1883
export var broker_keep_alive: int = 60

onready var _mqtt_client : Node = $MQTTClient
onready var _publish_button : Button = $Publish
onready var _text_to_send : LineEdit = $Text

func _ready() -> void:
	# Init the client
	_mqtt_client.initialise(client_id, clean_session)
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)


func _notification(what: int) -> void:
	# Clean everything when program is closed
	if what == NOTIFICATION_WM_QUIT_REQUEST:
		_mqtt_client.broker_disconnect()


func _on_Publish_pressed() -> void:
	_mqtt_client.publish("DATA", _text_to_send.text, 0, false)

