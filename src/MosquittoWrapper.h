#ifndef MOSQUITTO_WRAPPER_H
#define MOSQUITTO_WRAPPER_H

#include <mosquittopp.h>

#include "GDMosquitto.h"

/**
 * Documentation:
 * API https://mosquitto.org/api/files/mosquitto-h.html
 * MQTT https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html
 */
class MosquittoWrapper : public mosqpp::mosquittopp {
	protected:

		/**
		 * MQTT client node
		 */
		godot::GDMosquitto& m_mqtt_client;

	public:
	
		/**
		 * Constructor
		 * @param p_mqtt_client MQTT client node
		 * @param p_id String to use as the client id.  If NULL, a random client id will be generated.  If id is NULL, clean_session must be true
		 * @param p_clean_session set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them.
		 */
		MosquittoWrapper(godot::GDMosquitto& p_mqtt_client, const char* p_id, bool p_clean_session);

		/**
		 * Override on_connect
		 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 */
		void on_connect(int p_rc) override;

		/**
		 * Override on_connect_with_flags (not really tested)
		 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 * @param p_flags the connect flags (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901038)
		 */
		void on_connect_with_flags(int p_rc, int p_flags) override;

		/**
		 * Override on_disconnect
		 * @param p_rc the reason code, if something wrong happen. 0 = OK (see https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901031)
		 */
		void on_disconnect(int p_rc) override;

		/**
		 * Override on_publish
		 * @param p_mid the message id of the subscribe message
		 */
		void on_publish(int p_mid) override;

		/**
		 * Override on_message
		 * @param p_message the received message
		 */
		void on_message(const struct mosquitto_message* p_message) override;
		
		/**
		 * Override on_subscribe
		 * @param p_mid the message id of the subscribe message
		 * @param p_qos_count the number of granted subscriptions (size of granted_qos)
		 * @param p_granted_qos an array of integers indicating the granted QoS for each of the subscriptions
		 */
		void on_subscribe(int p_mid, int p_qos_count, const int* p_granted_qos) override;

		/**
		 * Override on_unsubscribe
		 * @param p_mid the message id of the subscribe message
		 */
		void on_unsubscribe(int p_mid) override;

		/**
		 * Override on_log
		 * @param p_level the log message level from the values: MOSQ_LOG_INFO MOSQ_LOG_NOTICE MOSQ_LOG_WARNING MOSQ_LOG_ERR MOSQ_LOG_DEBUG
		 * @param p_str the message string
		 */
		void on_log(int p_level, const char* p_str) override;

		/**
		 * Override on_error
		 */
		void on_error() override;

		/**
		 * Callback pw_callback
		 * @param p_buf the callback must write the password into “p_buf”
		 * @param p_size the size (bytes long) of the buffer
		 * @param p_rwflag ????
		 * @return size of the password
		 */
		int call_callback(char* p_buf, int p_size, int p_rwflag);
};

#endif //MOSQUITTO_WRAPPER_H
