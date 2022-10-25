# GDMosquitto
Mosquitto addon for Godot3.5!

<img width="1136" alt="Capture d’écran 2022-10-25 à 21 17 36" src="https://user-images.githubusercontent.com/4105962/197862495-2d376e52-c780-4551-be5a-c9a95795d2d0.png">

In the picture:
 - The first line is the subscription parameters, subscribe to the SIN topic and expect JSON formatted data like [t,sin(t)] and plot it.
 - The second line is the pulisher parameters, publish JSON formatted data to the SIN topic [t,sin(t)].
 - The last line sends the text from the first edit line to the topic DATA (the second edit line subscribes to DATA and displays the sent text).

Works on Linux, macOS and Windows. On Windows, pthreads can be used but the behavior is not clean, without pthread, there are some lost packages... mosquitto version 2.1 should solve this thread problem.

To compile it
 - Run git clone --recurse-submodules https://github.com/jferdelyi/GDMosquitto.git
 - Install the Godot dependencies (https://docs.godotengine.org/en/stable/development/compiling/index.html)
 - Run scons on the root folder.
 - Start Godot3.5 and open the project in the demo folder

No exports are tested.

Methods not implemented (not easy or because I'm not sure how to test them)
 - message_retry_set (no effect now, will never be implemented)
 - user_data_set
 - opts_set (mosquitto_int_option is provided for integer options only, easy to implement but MOSQ_OPT_SSL_CTX is impossible to set)
 - int tls_set
 - int tls_opts_set
 - int tls_insecure_set
 - int tls_psk_set

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

QoS behaviour is not really tested (only QoS 0 durring tests)	
Connect with flags seems not returns good flags (always 0)
