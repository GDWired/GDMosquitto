#ifndef GDMOSQUITTO
#define GDMOSQUITTO

#include <map>
#include <Godot.hpp>
#include <Reference.hpp>
#include <mosquitto.h>
#include <mosquittopp.h>

#define DEFAULT_BUFFER_SIZE 50
#define MOSQ_ERR_NOTINIT -1000

using namespace godot;

class MosquittoWrapper;

namespace godot {
	enum MosquittoMessageIdAction {
		MESSAGE_ID_ACTION_PUBLISH = 0, 
		MESSAGE_ID_ACTION_SUBSCRIBE = 1, 
		MESSAGE_ID_ACTION_UNSUBSCRIBE = 2
	};

	/**
	 * Not implemented methods
	 * 	- void message_retry_set(unsigned int message_retry); -> no effect now, will never be implemented
	 *	- void user_data_set(void *userdata);
	 *	- int opts_set(enum mosq_opt_t option, void *value); -> but I provide mosquitto_int_option (easy to implement but MOSQ_OPT_SSL_CTX is impossible to set)
	 * Everythings relative to TLS are not implemented
	 *	- int tls_set(const char *cafile, const char *capath=NULL, const char *certfile=NULL, const char *keyfile=NULL, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata)=NULL);
	 *	- int tls_opts_set(int cert_reqs, const char *tls_version=NULL, const char *ciphers=NULL);
	 *	- int tls_insecure_set(bool value);
	 *	- int tls_psk_set(const char *psk, const char *identity, const char *ciphers=NULL);
	 * 
	 * I have tested only a simple exemple
	 * 	- init
	 * 	- connect
	 * 	- socket (not sure if this method is usefull in Godot)
	 * 	- loop_start
	 * 	- loop
	 * 	- loop_read
	 *  - loop_forever
	 * 	- subscribe (some topics)
	 * 	- publish (some topics regulary)
	 * 	- reconnect
	 * 	- unsubscribe
	 * 	- disconnect
	 * 	- loop_stop
	 * 
	 * Never tested methods
	 * 	- reinitialise (return rc=0 seems to works, but not sure)
	 * 	- will_set (return rc=0 but never tested further)
	 * 	- will_clear (return rc=0 but never tested further)
	 * 	- username_pw_set (return rc=0 but never tested further)
	 * 	- connect_async
	 * 	- connect_bind
	 * 	- connect_async_bind
	 * 	- reconnect_async
	 * 	- reconnect_delay_set
	 * 	- max_inflight_messages_set
	 * 	- mosquitto_int_option
	 * 	- loop_misc
	 * 	- loop_write
	 * 	- loop_forever
	 * 	- want_write
	 * 	- threaded_set
	 * 	- socks5_set
	 * 
	 * QoS behaviour is not really tested (only QoS 0 and 1 are used in my tests)	
	 * Connect with flags seems to returns not a good flags (always 0)
	 */
	class GDMosquitto : public Reference {
		GODOT_CLASS(GDMosquitto, Reference)

		// Private attributs
		private:
			/**
			 * The mosquitto wrapper
			 */
			MosquittoWrapper* m_wrapper;

			/**
			 * Buffer (use on_message)
			 */
			char* m_buf;

			/**
			 * Bufer size by default = 10
			 */
			size_t m_buf_size;

			/**
			 * Callback name (pw_callback)
			 */
			String m_pw_callback;

			/**
			 * Userdata
			 */
			Variant m_userdata;

		// Private methods
		private:

			//###############################################################
			//	Internals
			//###############################################################

			/**
			 * Realloc the buffer
			 * @param p_payload_size the size of the payload to be stored
			 */
			void realloc_buffer(const size_t p_payload_size);

			//###############################################################
			//	Wrapped methods
			//###############################################################

			/**
			 * Create a new mosquitto client instance
			 * @param p_id string to use as the client id. If NULL, a random client id will be generated. If id is NULL, clean_session must be true
			 * @param p_clean_session set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 	 */
			int initialise(const String p_id, const bool p_clean_session);

			/**
			 * This function allows an existing mosquitto client to be reused
			 * Call on a mosquitto instance to close any open network connections, 
			 * free memory and reinitialise the client with the new parameters
			 * @param p_id string to use as the client id. If NULL, a random client id will be generated. If id is NULL, clean_session must be true
			 * @param p_clean_session set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 	 */
			int reinitialise(const String p_id, const bool p_clean_session);

			/**
			 * Return the socket handle for a mosquitto instance 
			 * Useful if you want to include a mosquitto client in your own select() calls
			 * (Not sure if it usefull in this addon)
			 * @return socket
		 	 */
			int socket();

			/**
			 * Configure will information for a mosquitto instance, with attached properties
			 * By default, clients do not have a will
			 * This must be called before calling mosquitto_connect
			 * @param p_topic the topic on which to publish the will
			 * @param p_payload the data to send, can be empty
			 * @param p_qos integer value 0, 1 or 2 indicating the Quality of Service to be used
			 * @param p_retain set to true to make the will a retained message
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 	 */
			int will_set(const String p_topic, const String p_payload, const int p_qos, const bool p_retain);
			
			/**
			 * Remove a previously configured will. This must be called before calling mosquitto_connect
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 	 */
			int will_clear();

			/**
			 * Remove a previously configured will. This must be called before calling mosquitto_connect
			 * @param p_username the username to send as a string, or NULL to disable authentication
			 * @param p_password the password to send as a string. Set to NULL when username is valid in order to send just a username
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 	 */
			int username_pw_set(const String p_username, const String p_password);

			/**
 			 * Connect to an MQTT broker
			 * @param p_host the hostname or ip address of the broker to connect to
			 * @param p_port the network port to connect to. Usually 1883
			 * @param p_keepalive the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int connect(const String p_host, const int p_port, const int p_keepalive);

			/**
 			 * Connect to an MQTT broker. This is a non-blocking call. 
			 * If you use:
			 * <mosquitto_connect_async> your client must use the threaded interface
			 * <mosquitto_loop_start>. If you need to use <mosquitto_loop>, you must use
			 * <mosquitto_connect> to connect the client.
			 * @param p_host the hostname or ip address of the broker to connect to
			 * @param p_port the network port to connect to. Usually 1883
			 * @param p_keepalive the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int connect_async(const String p_host, const int p_port, const int p_keepalive);

			/**
 			 * Connect to an MQTT broker
			 * This extends the functionality of:
			 * <mosquitto_connect> by adding the bind_address parameter. 
			 * Use this function if you need to restrict network communication over a particular interface.
			 * @param p_host the hostname or ip address of the broker to connect to
			 * @param p_port the network port to connect to. Usually 1883
			 * @param p_keepalive the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time
			 * @param p_bind_address the hostname or ip address of the local network interface to bind to. If you do not want to bind to a specific interface, set this to NULL
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int connect_bind(const String p_host, const int p_port, const int p_keepalive, const String p_bind_address);

			/**
 			 * Connect to an MQTT broker. This is a non-blocking call. 
			 * This extends the functionality of:
			 * <mosquitto_connect> by adding the bind_address parameter. 
			 * Use this function if you need to restrict network communication over a particular interface.
			 * @param p_host the hostname or ip address of the broker to connect to
			 * @param p_port the network port to connect to. Usually 1883
			 * @param p_keepalive the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time
			 * @param p_bind_address the hostname or ip address of the local network interface to bind to. If you do not want to bind to a specific interface, set this to NULL
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int connect_async_bind(const String p_host, const int p_port, const int p_keepalive, const String p_bind_address);

			/**
			 * Reconnect to a broker
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int reconnect();

			/**
			 * Reconnect to a broker. Non blocking version
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int reconnect_async();

			/**
			 * Disconnect from the broker
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int disconnect();

			/**
			 * Publish a message on a given topic (Message ID emit message_id_updated)
			 * @param p_topic the topic on which to publish the will
			 * @param p_payload the data to send, can be empty
			 * @param p_qos integer value 0, 1 or 2 indicating the Quality of Service to be used
			 * @param p_retain set to true to make the will a retained message
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int publish(const String p_topic, const String p_payload, const int p_qos, const bool p_retain);

			/**
			 * Subscribe to a topic (Message ID emit message_id_updated)
			 * @param p_sub the subscription pattern
			 * @param p_qos integer value 0, 1 or 2 indicating the Quality of Service to be used
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int subscribe(const String p_sub, const int p_qos);

			/**
			 * Unsubscribe from a topic (Message ID emit message_id_updated)
			 * @param p_sub the unsubscription pattern
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int unsubscribe(const String p_sub);

			/**
			 * Control the behaviour of the client when it has unexpectedly disconnected in mosquitto_loop_forever or after mosquitto_loop_start.  
			 * The default behaviour if this function is not used is to repeatedly attempt to reconnect with a delay of 1 second until the connection succeeds.
			 * Use reconnect_delay parameter to change the delay between successive reconnection attempts.  
			 * You may also enable exponential backoff of the time between reconnections by setting reconnect_exponential_backoff to true and set an upper bound on the delay with reconnect_delay_max.
			 * @param p_reconnect_delay the number of seconds to wait between reconnects
			 * @param p_reconnect_delay_max the maximum number of seconds to wait between reconnects
			 * @param p_reconnect_exponential_backoff use exponential backoff between reconnect attempts. Set to true to enable exponential backoff
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int reconnect_delay_set(const unsigned int p_reconnect_delay, const unsigned int p_reconnect_delay_max, const bool p_reconnect_exponential_backoff);
			
			/**
			 * Set the number of QoS 1 and 2 messages that can be “in flight” at one time  
			 * An in flight message is part way through its delivery flow
			 * Attempts to send further messages with mosquitto_publish will result in the messages being queued until the number of in flight messages reduces
			 * A higher number here results in greater message throughput, but if set higher than the maximum in flight messages on the broker may lead to delays in the messages being acknowledged
			 * Set to 0 for no maximum
			 * @param p_max_inflight_messages the maximum number of inflight messages. Defaults to 20
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int max_inflight_messages_set(const unsigned int p_max_inflight_messages);
			
			/**
			 * When mosquitto_new is called, the pointer given as the “obj” parameter will be passed to the callbacks as user data.
			 * The mosquitto_user_data_set function allows this obj parameter to be updated at any time.
			 * This function will not modify the memory pointed to by the current user data pointer.
			 * If it is dynamically allocated memory you must free it yourself.
			 * @param p_userdata data that will be passed as an argument to any callbacks that are specified.
			 */
			void user_data_set(Variant p_userdata);

			/**
			 * Configure the client for certificate based SSL/TLS support.  Must be called before mosquitto_connect.
			 * Cannot be used in conjunction with mosquitto_tls_psk_set.
			 * Define the Certificate Authority certificates to be trusted (ie. the server certificate must be signed with one of these certificates) using cafile.
			 * If the server you are connecting to requires clients to provide a certificate, define certfile and keyfile with your client certificate and private key.  
			 * If your private key is encrypted, provide a password callback function or you will have to enter the password at the command line.
			 * @param p_cafile path to a file containing the PEM encoded trusted CA certificate files.  Either cafile or capath must not be NULL.
			 * @param p_capath path to a directory containing the PEM encoded trusted CA certificate files.  See mosquitto.conf for more details on configuring this directory.  Either cafile or capath must not be NULL.
			 * @param p_certfile path to a file containing the PEM encoded certificate file for this client.  If NULL, keyfile must also be NULL and no client certificate will be used.
			 * @param p_keyfile path to a file containing the PEM encoded private key for this client.  If NULL, certfile must also be NULL and no client certificate will be used.
			 * @param p_pw_callback if keyfile is encrypted, set pw_callback to allow your client to pass the correct password for decryption.  If set to NULL, the password must be entered on the command line.  Your callback must write the password into “buf”, which is “size” bytes long.  The return value must be the length of the password. “userdata” will be set to the calling mosquitto instance.  The mosquitto userdata member variable can be retrieved using mosquitto_userdata.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int tls_set(const String p_cafile, const String p_capath, const String p_certfile, const String p_keyfile, const String p_pw_callback);
			
			/**
			 * Set advanced SSL/TLS options.  Must be called before mosquitto_connect.
			 * @param p_cert_reqs an integer defining the verification requirements the client will impose on the server. This can be one of: SSL_VERIFY_NONE (0) or SSL_VERIFY_PEER (1)
			 * @param p_tls_version the version of the SSL/TLS protocol to use as a string. If NULL, the default value is used. The default value and the available values depend on the version of openssl that the library was compiled against.  For openssl >= 1.0.1, the available options are tlsv1.2, tlsv1.1 and tlsv1, with tlv1.2 as the default. For openssl < 1.0.1, only tlsv1 is available.
			 * @param p_ciphers a string describing the ciphers available for use. See the “openssl ciphers” tool for more information. If NULL, the default ciphers will be used.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int tls_opts_set(const int p_cert_reqs, const String p_tls_version, const String p_ciphers);
			
			/**
			 * Configure verification of the server hostname in the server certificate.  
			 * If value is set to true, it is impossible to guarantee that the host you are connecting to is not impersonating your server.  
			 * This can be useful in initial server testing, but makes it possible for a malicious third party to impersonate your server through DNS spoofing, for example.  
			 * Do not use this function in a real system.  Setting value to true makes the connection encryption pointless.  
			 * Must be called before mosquitto_connect.
			 * @param p_value if set to false, the default, certificate hostname checking is performed.  If set to true, no hostname checking is performed and the connection is insecure
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int tls_insecure_set(const bool p_value);
			
			/**
			 * Configure the client for pre-shared-key based TLS support.  
			 * Must be called before mosquitto_connect.
			 * Cannot be used in conjunction with mosquitto_tls_set.
			 * @param p_psk the pre-shared-key in hex format with no leading “0x”
			 * @param p_identity the identity of this client.  May be used as the username depending on the server settings
			 * @param p_ciphers a string describing the PSK ciphers available for use.  See the “openssl ciphers” tool for more information.  If NULL, the default ciphers will be used
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int tls_psk_set(const String p_psk, const String p_identity, const String p_ciphers);
			
			/**
			 * Set protocol version
			 * @param p_value the option specific value MQTT_PROTOCOL_V31 = 3,MQTT_PROTOCOL_V311 = 4,MQTT_PROTOCOL_V5 = 5
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int set_protocol_version(int p_value);

			/**
			 * Set ssl ctx
			 * @param p_value SSLv23 = 0, TLS = 1
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int set_ssl_ctx(int p_value);
			
			/**
			 * The main network loop for the client. This must be called frequently
			 * to keep communications between the client and broker working. This is
			 * carried out by <mosquitto_loop_forever> and <mosquitto_loop_start>, which
			 * are the recommended ways of handling the network loop. You may also use this
			 * function if you wish. It must not be called inside a callback.
			 * @param p_timeout maximum number of milliseconds to wait for network activity before timing out. Set to 0 for instant return. Set negative to use the default of 1000ms
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop(const int p_timeout);

			/**
			 * Carry out miscellaneous operations required as part of the network loop.
			 * This should only be used if you are not using mosquitto_loop() and are
			 * monitoring the client network socket for activity yourself.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_misc();

			/**
			 * Carry out network read operations.
			 * This should only be used if you are not using mosquitto_loop() and are
			 * monitoring the client network socket for activity yourself.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_read();

			/**
			 * Carry out network write operations
			 * This should only be used if you are not using mosquitto_loop() and are
			 * monitoring the client network socket for activity yourself.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_write();
			
			/**
			 * This function call loop() for you in an infinite blocking loop. It is useful
 			 * for the case where you only want to run the MQTT client loop in your program
			 * It handles reconnecting in case server connection is lost. If you call
 			 * mosquitto_disconnect() in a callback it will return.
			 * @param p_timeout maximum number of milliseconds to wait for network activity before timing out. Set to 0 for instant return. Set negative to use the default of 1000ms
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_forever(const int p_timeout);

			/**
			 * This is part of the threaded client interface. Call this once to start a new
			 * thread to process network traffic. This provides an alternative to
			 * repeatedly calling <mosquitto_loop> yourself.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_start();

			/**
			 * This is part of the threaded client interface. Call this once to stop the
			 * network thread previously created with <mosquitto_loop_start>. This call
			 * will block until the network thread finishes. For the network thread to end,
			 * you must have previously called <mosquitto_disconnect> or have set the force parameter to true
			 * @param p_force set to true to force thread cancellation. If false, <mosquitto_disconnect> must have already been called.
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int loop_stop(const bool p_force);

			/**
			 * Returns true if there is data ready to be written on the socket
			 * @return true if there is data ready to be written on the socket
			 */
			int want_write();

			/**
			 * Used to tell the library that your application is using threads, but not
			 * using <mosquitto_loop_start>. The library operates slightly differently when
			 * not in threaded mode in order to simplify its operation. If you are managing
			 * your own threads and do not use this function you will experience crashes due to race conditions
			 * @param p_threaded rue if your application is using threads, false otherwise
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int threaded_set(const bool p_threaded);

			/**
			 * Configure the client to use a SOCKS5 proxy when connecting. Must be called
			 * before connecting. "None" and "username/password" authentication is supported
			 * @param p_host the SOCKS5 proxy host to connect to
			 * @param p_port the SOCKS5 proxy port to use
			 * @param p_username if not NULL, use this username when authenticating with the proxy
			 * @param p_password if not NULL and username is not NULL, use this password when authenticating with the proxy
			 * @return the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			int socks5_set(const String p_host, const int p_port, const String p_username, const String p_password);

		// Public methods
		public:

			//###############################################################
			//	Constructor
			//###############################################################

			/**
			 * Constructor
			 */
			GDMosquitto();

			/**
			 * Destructor
			 */
			~GDMosquitto();

			//###############################################################
			//	Godot methods
			//###############################################################

			/**
			 * Bind methods, signals etc.
			 */
			static void _register_methods();

			/**
			 * Init the class
			 */
			void _init();

			//###############################################################
			//	Callbacks
			//###############################################################

			/**
			 * Password callback
			 * @param p_buf the callback must write the password into “p_buf”...
			 * @param p_size ...which is “p_size” bytes long
			 * @param p_rwflag ????
			 * @param userdata will be set to the calling mosquitto instance
			 * @return must be the length of the password
			 */
			static int pw_callback(char* p_buf, int p_size, int p_rwflag, void* p_userdata);

			/**
			 * Callback pw_callback
			 * @param p_buf the callback must write the password into “p_buf”
			 * @param p_size the size (bytes long) of the buffer
			 * @param p_rwflag ????
			 * @return size of the password
			 */
			int call_callback(char* p_buf, int p_size, int p_rwflag);

			//###############################################################
			//	Emit handlers
			//###############################################################

			/**
			 * Emit on_connect
			 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			void emit_on_connect(int p_rc);

			/**
			 * Emit on_connect_with_flags
			 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 * @param p_username_flag presence or absence of username
			 * @param p_password_flag presence or absence of password
			 * @param p_will_retain if true, the last will is a retained message (ignored if p_will_flag is false)
			 * @param p_will_qos the QoS of the last will (ignored if p_will_flag is false)
			 * @param p_will_flag_flag presence or absence of the last will
			 * @param p_clean_session if true, this is a clean session
			 */
			void emit_on_connect_with_flags(int p_rc, bool p_username_flag, bool p_password_flag, bool p_will_retain, int p_will_qos, bool p_will_flag, bool p_clean_session);

			/**
			 * Emit on_disconnect
			 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
			 */
			void emit_on_disconnect(int p_rc);

			/**
			 * Emit on_publish
			 * @param p_mid the message id of the subscribe message
			 */
			void emit_on_publish(int p_mid);

			/**
			 * Emit on_message
			 * @param p_message the received message
			 */
			void emit_on_message(const struct mosquitto_message* p_message);
			
			/**
			 * Emit on_subscribe
			 * @param p_mid the message id of the subscribe message
			 * @param p_qos_count the number of granted subscriptions (size of granted_qos)
			 * @param p_granted_qos an array of integers indicating the granted QoS for each of the subscriptions
			 */
			void emit_on_subscribe(int p_mid, int p_qos_count, const int* p_granted_qos);

			/**
			 * Emit on_unsubscribe
			 * @param p_mid the message id of the subscribe message
			 */
			void emit_on_unsubscribe(int p_mid);

			/**
			 * Emit on_log
			 * @param p_level the log message level from the values: MOSQ_LOG_INFO MOSQ_LOG_NOTICE MOSQ_LOG_WARNING MOSQ_LOG_ERR MOSQ_LOG_DEBUG
			 * @param p_str the message string
			 */
			void emit_on_log(int p_level, const char* p_str);

			/**
			 * Emit on_error
			 */
			void emit_on_error();
	};
}

#endif // GDMOSQUITTO
