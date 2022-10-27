# <img src="https://user-images.githubusercontent.com/4105962/198399711-db9e4e56-1aae-4e60-88a9-4f96484c1681.png" width="27"> GDMosquitto


Mosquitto addon for Godot3.5!

<img width="1136" alt="Capture d’écran 2022-10-27 à 15 27 05" src="https://user-images.githubusercontent.com/4105962/198297381-d3eea888-d09f-4532-a38c-585850918de8.png">

In the picture:
 - The first line is the subscription parameters, subscribe to the SIN topic and expect JSON formatted data like [t,sin(t)] and plot it.
 - The second line is the pulisher parameters, publish JSON formatted data to the SIN topic [t,sin(t)].
 - The last line sends the text from the first edit line to the topic DATA (the second edit line subscribes to DATA and displays the sent text).

To compile it
 - Run git clone --recurse-submodules https://github.com/jferdelyi/GDMosquitto.git
 - Install the Godot dependencies (https://docs.godotengine.org/en/stable/development/compiling/index.html)
 - Run scons on the root folder.
 - Start Godot3.5 and open the project in the demo folder

Works on editor and exports for Windows, Linux and macOS. By default, the Windows version is compiled without pthreads and loop_start is not usable (mosquitto version 2.1 should solve this thread problem). You can recompile it to use loop_start or use this workaround:

```
func _ready() -> void:
	_mqtt_client.initialise(client_id, clean_session)
	_mqtt_client.broker_connect(broker_address, broker_port, broker_keep_alive)
	_loop_start_supported = not (_mqtt_client.loop_start() == GDMosquitto.RC.MOSQ_ERR_NOT_SUPPORTED)
	
	if not _loop_start_supported:
		_mqtt_client.threaded_set(true)
		_loop_thread = Thread.new()
		if _loop_thread.start(self, "_mqtt_client_loop") != OK:
			printerr("Error while the loop thread is created")

func _mqtt_client_loop() -> void:
	_mqtt_client.loop_forever(0)
```
However, the behavior on Windows is not exactly the same. If QoS 0 is used, there are some lost packets (it's ok with QoS 1).

About export on Android: 
 - Mosquitto build is OK
 - GDMosquitto build OK
 - APK/AAB build OK
 - Runtime KO (on emulator, the application is open but no GDNative call works)

Methods not implemented
 - opts_set (I have implemented set_protocol_version for the protocol version, but I don't know how to implement set_ssl_ctx, if someone needs it, I will ask)
 - message_retry_set (no effect now, will never be implemented)

Never tested methods (returns rc=0 but has never been tested further)
 - reinitialise
 - will_set
 - will_clear
 - username_pw_set
 - connect_async
 - connect_bind
 - connect_async_bind
 - reconnect_async
 - reconnect_delay_set
 - max_inflight_messages_set
 - mosquitto_int_option
 - loop_misc
 - loop_write
 - want_write
 - threaded_set
 - socks5_set
 - user_data_set
 - int tls_set
 - int tls_opts_set
 - int tls_insecure_set
 - int tls_psk_set

QoS behaviour is not really tested (QoS 0 and 1 durring tests)	
Connect with flags seems not returns good flags (always 0)
