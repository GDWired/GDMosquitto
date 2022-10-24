##
# Plugin utils 
##

extends Node

# Integer values returned from many libmosquitto functions
enum RC {
	MOSQ_ERR_NOTINIT = -1000
	MOSQ_ERR_AUTH_CONTINUE = -4,
	MOSQ_ERR_NO_SUBSCRIBERS = -3,
	MOSQ_ERR_SUB_EXISTS = -2,
	MOSQ_ERR_CONN_PENDING = -1,
	MOSQ_ERR_SUCCESS = 0,
	MOSQ_ERR_NOMEM = 1,
	MOSQ_ERR_PROTOCOL = 2,
	MOSQ_ERR_INVAL = 3,
	MOSQ_ERR_NO_CONN = 4,
	MOSQ_ERR_CONN_REFUSED = 5,
	MOSQ_ERR_NOT_FOUND = 6,
	MOSQ_ERR_CONN_LOST = 7,
	MOSQ_ERR_TLS = 8,
	MOSQ_ERR_PAYLOAD_SIZE = 9,
	MOSQ_ERR_NOT_SUPPORTED = 10,
	MOSQ_ERR_AUTH = 11,
	MOSQ_ERR_ACL_DENIED = 12,
	MOSQ_ERR_UNKNOWN = 13,
	MOSQ_ERR_ERRNO = 14,
	MOSQ_ERR_EAI = 15,
	MOSQ_ERR_PROXY = 16,
	MOSQ_ERR_PLUGIN_DEFER = 17,
	MOSQ_ERR_MALFORMED_UTF8 = 18,
	MOSQ_ERR_KEEPALIVE = 19,
	MOSQ_ERR_LOOKUP = 20,
	MOSQ_ERR_MALFORMED_PACKET = 21,
	MOSQ_ERR_DUPLICATE_PROPERTY = 22,
	MOSQ_ERR_TLS_HANDSHAKE = 23,
	MOSQ_ERR_QOS_NOT_SUPPORTED = 24,
	MOSQ_ERR_OVERSIZE_PACKET = 25,
	MOSQ_ERR_OCSP = 26,
	MOSQ_ERR_TIMEOUT = 27,
	MOSQ_ERR_RETAIN_NOT_SUPPORTED = 28,
	MOSQ_ERR_TOPIC_ALIAS_INVALID = 29,
	MOSQ_ERR_ADMINISTRATIVE_ACTION = 30,
	MOSQ_ERR_ALREADY_EXISTS = 31
}


# MQTT Protocol
enum Protocol {
	MQTT_PROTOCOL_V31 = 3,
	MQTT_PROTOCOL_V311 = 4,
	MQTT_PROTOCOL_V5 = 5
}


# Client options
# MOSQ_OPT_TCP_NODELAY - Set to 1 to disable Nagle's algorithm on client
# MOSQ_OPT_PROTOCOL_VERSION - Value must be set to either MQTT_PROTOCOL_V31,
# MOSQ_OPT_RECEIVE_MAXIMUM - Value can be set between 1 and 65535 inclusive,
# MOSQ_OPT_SEND_MAXIMUM - Value can be set between 1 and 65535 inclusive,
# MOSQ_OPT_SSL_CTX_WITH_DEFAULTS - If value is set to a non zero value,
# MOSQ_OPT_TLS_OCSP_REQUIRED - Set whether OCSP checking on TLS
# MOSQ_OPT_TLS_USE_OS_CERTS - Set to 1 to instruct the client to load and
enum Options {
	MOSQ_OPT_PROTOCOL_VERSION = 1,
	#MOSQ_OPT_SSL_CTX = 2,
	MOSQ_OPT_SSL_CTX_WITH_DEFAULTS = 3,
	MOSQ_OPT_RECEIVE_MAXIMUM = 4,
	MOSQ_OPT_SEND_MAXIMUM = 5,
	#MOSQ_OPT_TLS_KEYFORM = 6,
	#MOSQ_OPT_TLS_ENGINE = 7,
	#MOSQ_OPT_TLS_ENGINE_KPASS_SHA1 = 8,
	MOSQ_OPT_TLS_OCSP_REQUIRED = 9,
	#MOSQ_OPT_TLS_ALPN = 10,
	MOSQ_OPT_TCP_NODELAY = 11,
	#MOSQ_OPT_BIND_ADDRESS = 12,
	MOSQ_OPT_TLS_USE_OS_CERTS = 13
}


enum MosquittoMessageIdAction {
	MESSAGE_ID_ACTION_PUBLISH = 0, 
	MESSAGE_ID_ACTION_SUBSCRIBE = 1, 
	MESSAGE_ID_ACTION_UNSUBSCRIBE = 2
}

