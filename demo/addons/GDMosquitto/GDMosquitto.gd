##
# Plugin main
# If you have any questions/remarks
# 	- https://github.com/jferdelyi/GDMosquitto
#	- jferdelyi@gmail.com
##

tool
extends EditorPlugin

##
# About pw_callback
#
# func tls_set(p_cafile: String, p_capath: String, p_certfile: String, p_keyfile: String, p_pw_callback: String) -> int:
#
# The string variable p_pw_callback must be the name of a function with this signature:
# 	- func pw_callback(size: int, rwflag: int) -> String:
# Where:
# 	- size is the size of the password buffer (the password should fit on it)
#	- rwflag is ????????
#	- return the paswword
##

func _enter_tree():
	add_custom_type("MQTTClient", "Node", preload("GDMosquitto.gdns"), preload("icon.png"))
	add_autoload_singleton("GDMosquitto", "res://addons/GDMosquitto/GDMosquittoUtils.gd")

func _exit_tree():
	remove_custom_type("MQTTClient")
	remove_autoload_singleton("GDMosquitto")
