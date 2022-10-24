##
# Plugin main 
##

tool
extends EditorPlugin


func _enter_tree():
	add_custom_type("MQTTClient", "Node", preload("GDMosquitto.gdns"), preload("icon.png"))
	add_autoload_singleton("GDMosquitto", "res://addons/GDMosquitto/GDMosquittoUtils.gd")

func _exit_tree():
	remove_custom_type("MQTTClient")
	remove_autoload_singleton("GDMosquitto")
