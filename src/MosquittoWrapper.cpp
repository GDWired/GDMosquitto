#include "MosquittoWrapper.h"

#include "GDMosquitto.h"

MosquittoWrapper::MosquittoWrapper(godot::GDMosquitto& p_mqtt_client, const char* p_id, bool p_clean_session) : mosquittopp(p_id, p_clean_session), m_mqtt_client(p_mqtt_client) {
}

void MosquittoWrapper::on_connect(int p_rc) /*override*/ {
	m_mqtt_client.emit_on_connect(p_rc);
}

void MosquittoWrapper::on_connect_with_flags(int p_rc, int p_flags) /*override*/ {
	bool p_username_flag = (p_flags >> 0) & 0x1;
	bool p_password_flag = (p_flags >> 1) & 0x1;
	bool p_will_retain = (p_flags >> 2) & 0x1;
	int p_will_qos = (p_flags >> 3) & 0x3; // Two bits
	bool p_will_flag = (p_flags >> 5) & 0x1;
	bool p_clean_session = (p_flags >> 6) & 0x1;
	
	m_mqtt_client.emit_on_connect_with_flags(p_rc, p_username_flag, p_password_flag, p_will_retain, p_will_qos, p_will_flag, p_clean_session);
}

void MosquittoWrapper::on_disconnect(int p_rc) /*override*/ {
	m_mqtt_client.emit_on_disconnect(p_rc);
}

void MosquittoWrapper::on_publish(int p_mid) /*override*/ {
	m_mqtt_client.emit_on_publish(p_mid);
}

void MosquittoWrapper::on_message(const struct mosquitto_message* p_message) /*override*/ {
	m_mqtt_client.emit_on_message(p_message);
}

void MosquittoWrapper::on_subscribe(int p_mid, int p_qos_count, const int* p_granted_qos) /*override*/ {
	m_mqtt_client.emit_on_subscribe(p_mid, p_qos_count, p_granted_qos);
}

void MosquittoWrapper::on_unsubscribe(int p_mid) /*override*/ {
	m_mqtt_client.emit_on_unsubscribe(p_mid);
}

void MosquittoWrapper::on_log(int p_level, const char* p_str) /*override*/ {
	m_mqtt_client.emit_on_log(p_level, p_str);
}

void MosquittoWrapper::on_error() /*override*/ {
	m_mqtt_client.emit_on_error();
}

int MosquittoWrapper::call_callback(char* p_buf, int p_size, int p_rwflag) {
	Array l_arguments;
	l_arguments.append(p_size);
	l_arguments.append(p_rwflag);
	String l_password = m_mqtt_client.call("s_pw_callback", l_arguments);
	
	const char* l_char_password = l_password.utf8().get_data();
	strncpy(p_buf, l_char_password, p_size);
    p_buf[p_size - 1] = '\0';
	return strlen(p_buf);
}
