#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := test_ir

EXCLUDE_COMPONENTS := openssl aws_iot libsodium json jsmn console mdns esp-tls aes coap asio esp_http_client esp_https_ota http_server 


include $(IDF_PATH)/make/project.mk

