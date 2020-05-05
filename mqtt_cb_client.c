#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mosquitto.h"
#include <jansson.h>

/*
{"action": "right", "value": "10"}
sau
{"action": "right", "value": 10}
 */

#define ACTION_TAG "action"
#define VALUE_TAG "value"
#define QUIT_TAG "quit"

static struct mosquitto *mosq = NULL;

int on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
	json_t *root;
	json_t *val;
	const char *key;
	json_error_t jerr;
	char *str = (char *)msg->payload;
	char *action = NULL;
	char *value = NULL;
	int num_val = 0;

	if (str == NULL)
		return -1;

#ifdef DEBUG
	printf("%s %s (%d)\n", msg->topic, (const char *)msg->payload, msg->payloadlen);
#endif

	root = json_loads(msg->payload, 0, &jerr);
	if (NULL == root) {
        printf("%s:%d - error: on line %d: %s\n", __FUNCTION__, __LINE__, jerr.line, jerr.text);
        return 0;
    }

    json_object_foreach(root, key, val) {
		if (strncmp(key, QUIT_TAG, strlen(QUIT_TAG)) == 0) {
			printf("%s:%d -> quit\n", __FUNCTION__, __LINE__);
			mosquitto_disconnect(mosq);
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(0);
		}
		if (strncmp(key, ACTION_TAG, strlen(ACTION_TAG)) == 0) {
			if (json_is_string(val)) {
				action = strdup(json_string_value(val));
			}
		} else if (strncmp(key, VALUE_TAG, strlen(VALUE_TAG)) == 0) {
			if (json_is_string(val)) {
				value = strdup(json_string_value(val));
				if (value) {
					num_val = atoi(value);
				}
			} else if (json_is_integer(val)) {
				num_val = json_integer_value(val);
			}
		}
	}

	printf("%s:%d - action = %s num_val = %d\n", __FILE__, __LINE__, action, num_val);

	if (action)
		free(action);
	if (value)
		free(value);



	return 0;
}


int main(int argc, char *argv[])
{
	int rc;

	mosquitto_lib_init();
	
	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	char *ipaddr = "127.0.0.1";
	
	rc = mosquitto_subscribe_callback(
			on_message, NULL,
			"/bb", 0,
			ipaddr, 1883,
			NULL, 60, true,
			NULL, NULL,
			NULL, NULL);

	if (rc) {
		printf("Error: %s\n", mosquitto_strerror(rc));
	}

	mosquitto_loop_forever(mosq, -1, 1);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}

