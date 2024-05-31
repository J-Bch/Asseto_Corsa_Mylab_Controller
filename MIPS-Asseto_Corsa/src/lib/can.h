/*
 * can.h
 *
 *  Created on: Apr 12, 2024
 *      Author: joachim
 */

#ifndef CAN_H_
#define CAN_H_
#include <stdint.h>

void can_init();
void can_send(uint32_t id, uint8_t ext_id, uint8_t data_len, uint8_t data[data_len]);
uint32_t can_get_last_id();
uint8_t* can_get_last_data();
void can_get_message(uint32_t* recieved_id, uint8_t** recieved_data);
uint8_t can_get_message_to_read_number();




#endif /* CAN_H_ */
