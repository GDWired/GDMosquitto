#include "GDMosquitto.h"

#include <cerrno>
#include <string.h>  // strlen
#include <Godot.hpp>

#include "MosquittoWrapper.h"

using namespace godot;

GDMosquitto::GDMosquitto() : m_wrapper(nullptr), m_buf_size(DEFAULT_BUFFER_SIZE), m_buf(new char[DEFAULT_BUFFER_SIZE]), m_pw_callback(), m_userdata() {
	// Can return MOSQ_ERR_UNKNOWN on Windows if sockets couldn't be initialized
	if (mosqpp::lib_init()) {
		ERR_PRINT("Mosquitto lib init error: sockets couldn't be initialized");
	}
}

GDMosquitto::~GDMosquitto() {
	if (m_wrapper) {
		m_wrapper->disconnect();
		m_wrapper->loop_stop();
		delete m_wrapper;
	}
	delete[] m_buf;
	mosqpp::lib_cleanup(); // Always MOSQ_ERR_SUCCESS
}

//###############################################################
//	Godot methods
//###############################################################

void GDMosquitto::_register_methods() {
	// Methods
	register_method("initialise", &GDMosquitto::initialise);
	register_method("reinitialise", &GDMosquitto::reinitialise);
	register_method("socket", &GDMosquitto::socket);
	register_method("will_set", &GDMosquitto::will_set);
	register_method("will_clear", &GDMosquitto::will_clear);
	register_method("username_pw_set", &GDMosquitto::username_pw_set);
	register_method("broker_connect", &GDMosquitto::connect);
	register_method("broker_connect_async", &GDMosquitto::connect_async);
	register_method("broker_connect_bind", &GDMosquitto::connect_bind);
	register_method("broker_connect_async_bind", &GDMosquitto::connect_async_bind);
	register_method("broker_reconnect", &GDMosquitto::reconnect);
	register_method("broker_reconnect_async", &GDMosquitto::reconnect_async);
	register_method("broker_disconnect", &GDMosquitto::disconnect);
	register_method("publish", &GDMosquitto::publish);
	register_method("subscribe", &GDMosquitto::subscribe);
	register_method("unsubscribe", &GDMosquitto::unsubscribe);
	register_method("reconnect_delay_set", &GDMosquitto::reconnect_delay_set);
	register_method("max_inflight_messages_set", &GDMosquitto::max_inflight_messages_set);
	//register_method("message_retry_set", &GDMosquitto::message_retry_set); // No effect now, will never be implemented 
	register_method("user_data_set", &GDMosquitto::user_data_set); 
	register_method("tls_set", &GDMosquitto::tls_set);
	register_method("tls_opts_set", &GDMosquitto::tls_opts_set);
	register_method("tls_insecure_set", &GDMosquitto::tls_insecure_set);
	register_method("tls_psk_set", &GDMosquitto::tls_psk_set);
	register_method("set_protocol_version", &GDMosquitto::set_protocol_version);
	register_method("set_ssl_ctx", &GDMosquitto::set_ssl_ctx);
	
	register_method("loop", &GDMosquitto::loop);
	register_method("loop_misc", &GDMosquitto::loop_misc);
	register_method("loop_read", &GDMosquitto::loop_read);
	register_method("loop_write", &GDMosquitto::loop_write);
	register_method("loop_forever", &GDMosquitto::loop_forever);
	register_method("loop_start", &GDMosquitto::loop_start);
	register_method("loop_stop", &GDMosquitto::loop_stop);

	register_method("want_write", &GDMosquitto::want_write);
	register_method("threaded_set", &GDMosquitto::threaded_set);
	register_method("socks5_set", &GDMosquitto::socks5_set);

	// Signals
	register_signal<GDMosquitto>("connected", 
		"reason_code", GODOT_VARIANT_TYPE_INT
	);
	register_signal<GDMosquitto>("connected_with_flags", 
		"reason_code", GODOT_VARIANT_TYPE_INT, 
		"username_flag", GODOT_VARIANT_TYPE_BOOL,
		"password_flag", GODOT_VARIANT_TYPE_BOOL,
		"will_retain", GODOT_VARIANT_TYPE_BOOL,
		"will_qos", GODOT_VARIANT_TYPE_INT,
		"will_flag", GODOT_VARIANT_TYPE_BOOL,
		"clean_session", GODOT_VARIANT_TYPE_BOOL
	);
	register_signal<GDMosquitto>("disconnected", 
		"reason_code", GODOT_VARIANT_TYPE_INT
	);
	register_signal<GDMosquitto>("published", 
		"message_id", GODOT_VARIANT_TYPE_INT
	);
	register_signal<GDMosquitto>("received", 
		"message_id", GODOT_VARIANT_TYPE_INT, 
		"topic", GODOT_VARIANT_TYPE_STRING,
		"payload", GODOT_VARIANT_TYPE_STRING
	);
	register_signal<GDMosquitto>("subscribed", 
		"message_id", GODOT_VARIANT_TYPE_INT, 
		"qos_count", GODOT_VARIANT_TYPE_INT, 
		"granted_qos", GODOT_VARIANT_TYPE_INT
	);
	register_signal<GDMosquitto>("unsubscribed", 
		"message_id", GODOT_VARIANT_TYPE_INT
	);
	register_signal<GDMosquitto>("log", 
		"level", GODOT_VARIANT_TYPE_INT,
		"message", GODOT_VARIANT_TYPE_STRING
	);
	register_signal<GDMosquitto>("error");
	register_signal<GDMosquitto>("message_id_updated", 
		"message_id", GODOT_VARIANT_TYPE_INT,
		"topic", GODOT_VARIANT_TYPE_STRING,
		"action", GODOT_VARIANT_TYPE_INT // Publish, Subscribe, Unsubscribe
	);
}

void GDMosquitto::_init() {
}

//###############################################################
//	Internals
//###############################################################

void GDMosquitto::realloc_buffer(const size_t p_payload_size) {
	// Realloc the buffer to payload size + 10
	delete[] m_buf;
	m_buf_size = p_payload_size + 10;
	m_buf = new char[m_buf_size];
}

const char* GDMosquitto::alloc_char_array(const String p_data) {
	// Workaround because on Windows utf8 seems corrupted
	if (p_data != "") {
		return p_data.alloc_c_string();
	}
	return nullptr;
}

void GDMosquitto::free_char_array(const char* p_data) {
	if (p_data != nullptr) {
		godot::api->godot_free(const_cast<void*>(static_cast<const void*>(p_data)));
	}
}

//###############################################################
//	Wrapped methods
//###############################################################

int GDMosquitto::initialise(const String p_id, const bool p_clean_session) {
	m_wrapper = new MosquittoWrapper(*this, p_id.utf8().get_data(), p_clean_session);
	
	if (errno == ENOMEM) {
		return MOSQ_ERR_NOMEM;
	} else if (errno == EINVAL) {
		return MOSQ_ERR_INVAL;
	}
	return MOSQ_ERR_SUCCESS;
}

int GDMosquitto::reinitialise(const String p_id, const bool p_clean_session) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->reinitialise(p_id.utf8().get_data(), p_clean_session);
}

int GDMosquitto::socket() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->socket();
}

int GDMosquitto::will_set(const String p_topic, const String p_payload, const int p_qos, const bool p_retain) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_payload = alloc_char_array(p_payload);
	size_t l_payload_size = 0;
	if (l_payload != nullptr) {
		l_payload_size = strlen(l_payload) + 1;
	}
	int l_rc = m_wrapper->will_set(p_topic.utf8().get_data(), l_payload_size, l_payload, p_qos, p_retain);
	free_char_array(l_payload);
	return l_rc;
}

int GDMosquitto::will_clear() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->will_clear();
}

int GDMosquitto::username_pw_set(const String p_username, const String p_password){
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_password = alloc_char_array(p_password);
	int l_rc = m_wrapper->username_pw_set(p_username.utf8().get_data(), l_password);
	free_char_array(l_password);
	return l_rc;
}

int GDMosquitto::connect(const String p_host, const int p_port, const int p_keepalive) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->connect(p_host.utf8().get_data(), p_port, p_keepalive);
}

int GDMosquitto::connect_async(const String p_host, const int p_port, const int p_keepalive) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->connect_async(p_host.utf8().get_data(), p_port, p_keepalive);
}

int GDMosquitto::connect_bind(const String p_host, const int p_port, const int p_keepalive, const String p_bind_address) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->connect(p_host.utf8().get_data(), p_port, p_keepalive, p_bind_address.utf8().get_data());
}

int GDMosquitto::connect_async_bind(const String p_host, const int p_port, const int p_keepalive, const String p_bind_address) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->connect_async(p_host.utf8().get_data(), p_port, p_keepalive, p_bind_address.utf8().get_data());
}

int GDMosquitto::reconnect() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->reconnect();
}

int GDMosquitto::reconnect_async() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->reconnect_async();
}

int GDMosquitto::disconnect() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->disconnect();
}	

int GDMosquitto::publish(const String p_topic, const String p_payload, const int p_qos, const bool p_retain) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	int l_id;
	const char* l_payload = alloc_char_array(p_payload);
	size_t l_payload_size = 0;
	if (l_payload != nullptr) {
		l_payload_size = strlen(l_payload) + 1;
	}
	int l_rc = m_wrapper->publish(&l_id, p_topic.utf8().get_data(),	l_payload_size, l_payload, p_qos, p_retain);
	free_char_array(l_payload);
	if (!l_rc) {
		emit_signal("message_id_updated", l_id, p_topic, MESSAGE_ID_ACTION_PUBLISH);
	}
	return l_rc;
}	

int GDMosquitto::subscribe(const String p_sub, const int p_qos) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	int l_id;
	int l_rc = m_wrapper->subscribe(&l_id, p_sub.utf8().get_data(), p_qos);
	if (!l_rc) {
		emit_signal("message_id_updated", l_id, p_sub, MESSAGE_ID_ACTION_SUBSCRIBE);
	}
	return l_rc;
}	

int GDMosquitto::unsubscribe(const String p_sub) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	int l_id;
	int l_rc = m_wrapper->unsubscribe(&l_id, p_sub.utf8().get_data());
	if (!l_rc) {
		emit_signal("message_id_updated", l_id, p_sub, MESSAGE_ID_ACTION_UNSUBSCRIBE);
	}
	return l_rc;
}

int GDMosquitto::reconnect_delay_set(const unsigned int p_reconnect_delay, const unsigned int p_reconnect_delay_max, const bool p_reconnect_exponential_backoff)  {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	m_wrapper->reconnect_delay_set(p_reconnect_delay, p_reconnect_delay_max, p_reconnect_exponential_backoff);
	return MOSQ_ERR_SUCCESS;
}

int GDMosquitto::max_inflight_messages_set(const unsigned int p_max_inflight_messages) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->max_inflight_messages_set(p_max_inflight_messages);
}

void GDMosquitto::user_data_set(Variant p_userdata) {
	m_userdata = p_userdata;
	return m_wrapper->user_data_set(&m_userdata);
}

int GDMosquitto::tls_set(const String p_cafile, const String p_capath, const String p_certfile, const String p_keyfile, const String p_pw_callback) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_capath = alloc_char_array(p_capath);
	const char* l_certfile = alloc_char_array(p_certfile);
	const char* l_keyfile = alloc_char_array(p_keyfile);
	m_pw_callback = "";
	int l_rc = 0;
	if (m_pw_callback != "") {
		m_pw_callback = p_pw_callback;
		l_rc = m_wrapper->tls_set(p_cafile.utf8().get_data(), l_capath, l_certfile, l_keyfile, pw_callback);
	} else {		
		l_rc = m_wrapper->tls_set(p_cafile.utf8().get_data(), l_capath, l_certfile, l_keyfile, NULL);
	}
	free_char_array(l_capath);
	free_char_array(l_certfile);
	free_char_array(l_keyfile);
	return l_rc;
}

int GDMosquitto::tls_opts_set(const int p_cert_reqs, const String p_tls_version, const String p_ciphers) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_tls_version = alloc_char_array(p_tls_version);
	const char* l_ciphers = alloc_char_array(p_ciphers);
	int l_rc = m_wrapper->tls_opts_set(p_cert_reqs, l_tls_version, l_ciphers);
	free_char_array(l_ciphers);
	free_char_array(l_tls_version);
	return l_rc;
}

int GDMosquitto::tls_insecure_set(const bool p_value) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->tls_insecure_set(p_value);
}

int GDMosquitto::tls_psk_set(const String p_psk, const String p_identity, const String p_ciphers) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_ciphers = alloc_char_array(p_ciphers);
	int l_rc = m_wrapper->tls_psk_set(p_psk.utf8().get_data(), p_identity.utf8().get_data(), l_ciphers);
	free_char_array(l_ciphers);
	return l_rc;
}
			
int GDMosquitto::set_protocol_version(int p_value) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->opts_set(MOSQ_OPT_PROTOCOL_VERSION, static_cast<void*>(&p_value));
}

int GDMosquitto::set_ssl_ctx(int p_value) {
	return MOSQ_ERR_NOT_SUPPORTED;
}

int GDMosquitto::loop(const int p_timeout) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->loop(p_timeout);
}

int GDMosquitto::loop_misc() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->loop_misc();
}

int GDMosquitto::loop_read() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->disconnect();
}

int GDMosquitto::loop_write() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->disconnect();
}

int GDMosquitto::loop_forever(const int p_timeout) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->loop_forever(p_timeout);
}

int GDMosquitto::loop_start() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->loop_start();
}

int GDMosquitto::loop_stop(const bool p_force) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->loop_stop(p_force);
}

int GDMosquitto::want_write() {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->want_write();
}

int GDMosquitto::threaded_set(const bool p_threaded) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	return m_wrapper->threaded_set(p_threaded);
}

int GDMosquitto::socks5_set(const String p_host, const int p_port, const String p_username, const String p_password) {
	if (!m_wrapper) {
		return MOSQ_ERR_NOTINIT;
	}
	const char* l_username = alloc_char_array(p_username);
	const char* l_password = alloc_char_array(p_password);
	int l_rc = m_wrapper->socks5_set(p_host.utf8().get_data(), p_port, l_username, l_password);
	free_char_array(l_username);
	free_char_array(l_password);
	return l_rc;
}

//###############################################################
//	Callback
//###############################################################

int GDMosquitto::pw_callback(char* p_buf, int p_size, int p_rwflag, void* p_userdata) {
	MosquittoWrapper* l_mosquitto_instance = reinterpret_cast<MosquittoWrapper*>(p_userdata);
	return l_mosquitto_instance->call_callback(p_buf, p_size, p_rwflag);
}

int GDMosquitto::call_callback(char* p_buf, int p_size, int p_rwflag) {
	Array l_arguments;
	l_arguments.append(p_size);
	l_arguments.append(p_rwflag);
	String l_password = call(m_pw_callback, l_arguments);
	
	const char* l_char_password = l_password.utf8().get_data();
	strncpy(p_buf, l_char_password, p_size);
	p_buf[p_size - 1] = '\0';
	return strlen(p_buf);
}

//###############################################################
//	Signals
//###############################################################

void GDMosquitto::emit_on_connect(int p_rc) {
	emit_signal("connected", p_rc);
}

void GDMosquitto::emit_on_connect_with_flags(int p_rc, bool p_username_flag, bool p_password_flag, bool p_will_retain, int p_will_qos, bool p_will_flag, bool p_clean_session) {
	emit_signal("connected_with_flags", p_rc, p_username_flag, p_password_flag, p_will_retain, p_will_qos, p_will_flag, p_clean_session);
}

void GDMosquitto::emit_on_disconnect(int p_rc) {
	emit_signal("disconnected", p_rc);
}

void GDMosquitto::emit_on_publish(int p_mid) {
	emit_signal("published", p_mid);
}

void GDMosquitto::emit_on_message(const struct mosquitto_message* p_message) {
	String l_payload = "";
	
	// If the message is not empty
	if (p_message->payloadlen != 0) {
		const size_t l_payload_size = static_cast<size_t>(p_message->payloadlen) + 1;

		// Check if the buffer can contains the payload
		if (l_payload_size > m_buf_size) {
			realloc_buffer(l_payload_size);
		}

		// Clean and copy data to the buffer
		memset(m_buf, 0, l_payload_size * sizeof(char));
		memcpy(m_buf, p_message->payload, l_payload_size * sizeof(char));
		l_payload = String(m_buf);
	}
	
	// Parse topic
	auto l_topic = String(p_message->topic);

	// Emit
	emit_signal("received", p_message->mid, l_topic, l_payload);

}

void GDMosquitto::emit_on_subscribe(int p_mid, int p_qos_count, const int* p_granted_qos) {
	emit_signal("subscribed", p_mid, p_qos_count, p_granted_qos);
}

void GDMosquitto::emit_on_unsubscribe(int p_mid) {
	emit_signal("unsubscribed", p_mid);
}

void GDMosquitto::emit_on_log(int p_level, const char* p_str) {
	auto l_message = String(p_str);
	emit_signal("log", p_level, l_message);
}

void GDMosquitto::emit_on_error() {
	emit_signal("error");
}
