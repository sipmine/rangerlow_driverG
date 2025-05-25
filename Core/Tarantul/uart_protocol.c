//
// Created by glebt on 17.05.2025.
//

#include "uart_protocol.h"

#include <string.h>
#include <sys/types.h>

protocol create_data(const uint8_t core_id, uint8_t cmd, uint8_t data[])
{
    protocol protocol;
    protocol.core_id = core_id;
    protocol.cmd = cmd;
    memcpy(protocol.data, data, 4);
    return protocol;
}

void create_msg(const protocol data, uint8_t* tx_buffer)
{
    tx_buffer[0] = data.core_id;
    tx_buffer[1] = data.cmd;
    memcpy(tx_buffer+2, data.data, 4);

}

void parse_msg(protocol *data, const uint8_t* rx_buffer)
{
    data->core_id = rx_buffer[0];
    data->cmd = rx_buffer[1];
    memcpy(data->data, rx_buffer+2, 4);
}

void float_to_bytes(const float x, uint8_t* bytes)
{
    memcpy(bytes, &x, 4);

}

float bytes_to_float(const uint8_t* bytes)
{
    float x;
    memcpy(&x, bytes, 4);
    return x;
}

void int_32_to_bytes(const int32_t x, uint8_t* bytes)
{
   memcpy(bytes, &x, 4);
}

int32_t bytes_to_int_32(const uint8_t* bytes)
{
    int32_t x;
    memcpy(&x, bytes, 4);
    return x;
}
