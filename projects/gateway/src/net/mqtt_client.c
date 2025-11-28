#define _POSIX_C_SOURCE 200809L
#include "mqtt_client.h"
#include <mosquitto.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../log/log.h"
#include "../common/config.h"

static struct mosquitto *mosq = NULL;
static pthread_mutex_t mqtt_lock = PTHREAD_MUTEX_INITIALIZER;
static int mqtt_initialized = 0;

static void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) LOGI("MQTT connected");
    else LOGW("MQTT connect failed rc=%d", rc);
}
static void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
    LOGI("MQTT disconnected rc=%d", rc);
}
static void on_publish(struct mosquitto *mosq, void *obj, int mid) {
    LOGD("MQTT publish mid=%d", mid);
}

int mqtt_init(const char *broker_host, int broker_port, const char *client_id) {
    if (!broker_host) return -1;
    pthread_mutex_lock(&mqtt_lock);
    if (mqtt_initialized) { pthread_mutex_unlock(&mqtt_lock); return 0; }
    mosquitto_lib_init();
    mosq = mosquitto_new(client_id ? client_id : "gateway-client", true, NULL);
    if (!mosq) {
        LOGE("mosquitto_new failed");
        pthread_mutex_unlock(&mqtt_lock);
        return -1;
    }
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_publish_callback_set(mosq, on_publish);

    if (mosquitto_connect(mosq, broker_host, broker_port, 60) != MOSQ_ERR_SUCCESS) {
        LOGW("mosquitto_connect failed, will try loop_start to reconnect");
        /* we'll still start loop so library can attempt reconnects */
    }
    if (mosquitto_loop_start(mosq) != MOSQ_ERR_SUCCESS) {
        LOGE("mosquitto_loop_start failed");
        mosquitto_destroy(mosq); mosq = NULL; pthread_mutex_unlock(&mqtt_lock); return -1;
    }
    mqtt_initialized = 1;
    pthread_mutex_unlock(&mqtt_lock);
    return 0;
}

void mqtt_deinit(void) {
    pthread_mutex_lock(&mqtt_lock);
    if (!mqtt_initialized) { pthread_mutex_unlock(&mqtt_lock); return; }
    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    mosq = NULL;
    mqtt_initialized = 0;
    pthread_mutex_unlock(&mqtt_lock);
}

int mqtt_publish(const char *topic, const char *payload) {
    if (!topic || !payload) return -1;
    pthread_mutex_lock(&mqtt_lock);
    if (!mqtt_initialized) { pthread_mutex_unlock(&mqtt_lock); return -1; }
    int mid = 0;
    int qos = 0;
    int ret = mosquitto_publish(mosq, &mid, topic, (int)strlen(payload), payload, qos, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        LOGW("mosquitto_publish failed: %d", ret);
        pthread_mutex_unlock(&mqtt_lock);
        return -1;
    }
    LOGD("mqtt_publish queued mid=%d topic=%s", mid, topic);
    pthread_mutex_unlock(&mqtt_lock);
    return 0;
}

int mqtt_publish_telemetry(double voltage, double current, double energy) {
    char payload[256];
    int n = snprintf(payload, sizeof(payload), "{\"ts\":%lu,\"voltage\":%.3f,\"current\":%.3f,\"energy\":%.3f}",
                     (unsigned long)time(NULL), voltage, current, energy);
    if (n <= 0) return -1;
    return mqtt_publish("gateway/telemetry", payload);
}
