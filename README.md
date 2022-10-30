<p align="center"><img src="https://user-images.githubusercontent.com/4105962/198399711-db9e4e56-1aae-4e60-88a9-4f96484c1681.png" width="256"></p>

# <p align="center">GDMosquitto (Godot 3.5)</p>

A wrapping of libmosquitto (https://mosquitto.org) able to make MQTT clients in Godot.

<img width="1136" alt="Capture d’écran 2022-10-27 à 15 27 05" src="https://user-images.githubusercontent.com/4105962/198297381-d3eea888-d09f-4532-a38c-585850918de8.png">

In the picture:
 - The first line is the subscription parameters, subscribe to the SIN topic and expect JSON formatted data like [t,sin(t)] and plot it.
 - The second line is the pulisher parameters, publish JSON formatted data to the SIN topic [t,sin(t)].
 - The last line sends the text from the first edit line to the topic DATA (the second edit line subscribes to DATA and displays the sent text).

Works on editor and exports for Windows, Linux and macOS. By default, the Windows version is compiled without pthreads and loop_start is not usable (mosquitto version 2.1 should solve this thread problem). You can recompile it to use loop_start or use this workaround:
```gdscript
var _loop_thread: Thread = null
onready var _mqtt_client: Reference = preload("res://addons/GDMosquitto/GDMosquitto.gdnlib").new()

func _ready() -> void:
	_mqtt_client.initialise(client_id, clean_session)
	_mqtt_client.broker_connect(host, port, keep_alive)
	_loop_start_supported = not (_mqtt_client.loop_start() == GDMosquitto.RC.MOSQ_ERR_NOT_SUPPORTED)
	
	if not _loop_start_supported:
		_mqtt_client.threaded_set(true)
		_loop_thread = Thread.new()
		_loop_thread.start(self, "_mqtt_client_loop")

func _mqtt_client_loop() -> void:
	_mqtt_client.loop_forever(0)
	
func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_QUIT_REQUEST:
		_mqtt_client.broker_disconnect()
		if _loop_start_supported:
			_mqtt_client.loop_stop(false)
		else:
			_loop_thread.wait_to_finish()
```
However, the behavior on Windows is not exactly the same. If QoS 0 is used, there are some lost packets (it's ok with QoS 1).

## To use it
### Prebuild
Precompiled for Win64, macOS Universal (arm64 and x86_64) and Linux x86_64. Just copy/past the folder GDMosquitto from demo/addons into your project.

### From source
If you need specific build (Win32 or Linux arm64 for exemple)
 - Install Mosquitto (brew install mosquitto, apt install mosquitto ...)
 - [Windows] Add mosquitto and mosquitto/devel to the path
 - Run git clone --recurse-submodules https://github.com/jferdelyi/GDMosquitto.git
 - Install the Godot dependencies (https://docs.godotengine.org/en/stable/development/compiling/index.html)
 - Run scons on the root folder.
 - Start Godot3.5 and open the project in the demo folder (or copy/past the folder GDMosquitto in addons into your project)


## Android
About export on Android: 
 - Mosquitto build is OK
 - GDMosquitto build OK
 - APK/AAB build OK
 - Runtime KO (on emulator, the application is open but no GDNative call works)
 
Android Mosquitto build (on macOS):
```
mkdir build
cd build
cmake -DANDROID_NDK=<NDK_PATH> -DANDROID_ABI="arm64-v8a" -DANDROID_NDK_HOST_X64="YES"  -DANDROID_TOOLCHAIN_NAME="arm-linux-androideabi-4.9" -DCMAKE_TOOLCHAIN_FILE="<NDK_PATH>/build/cmake/android.toolchain.cmake" -DWITH_TLS=OFF -DWITH_THREADING=OFF -DANDROID_TOOLCHAIN=clang ..
cmake --build .
```

## Todo
Not implemented methods 
 - opts_set, I have implemented set_protocol_version for the protocol version, but I don't know how to implement set_ssl_ctx (do nothing for now), if someone needs it, I will ask
 - message_retry_set (no effect now, will never be implemented)

Never tested methods (returns rc=0 but has never been tested further)
 - reinitialise
 - will_set
 - will_clear
 - username_pw_set
 - connect_async
 - connect(bind)
 - connect_async(bind)
 - reconnect
 - reconnect_async
 - reconnect_delay_set
 - max_inflight_messages_set
 - user_data_set
 - tls_set
 - tls_opts_set
 - tls_insecure_set
 - tls_psk_set
 - set_protocol_version
 - loop
 - loop_misc
 - loop_write
 - loop_read
 - want_write
 - threaded_set
 - socks5_set

QoS behaviour is not really tested (QoS 0 and 1 durring tests) and connect with flags seems not returns good flags (always 0).
