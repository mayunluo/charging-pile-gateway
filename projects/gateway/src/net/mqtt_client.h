#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>

int mqtt_init(const char *broker_host, int broker_port, const char *client_id);
void mqtt_deinit(void);

/* publish json payload to topic. topic must be valid string. payload is null-terminated. */
int mqtt_publish(const char *topic, const char *payload);

/* helper: publish telemetry in JSON form (voltage,current,energy) */
int mqtt_publish_telemetry(double voltage, double current, double energy);

#endif
