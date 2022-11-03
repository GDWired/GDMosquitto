extends MQTTClient


onready var _data : Node = $Data


func _on_MQTTClient_received(_message_id: int, _topic: String, payload: String) -> void:
	# MQTT data received
	if payload == "":
		_data.text = "<empty>"
	else:
		_data.text = payload


func _on_MQTTPull_connected(_reason_code: int) -> void:
	var _rc = subscribe("DATA", 1)
