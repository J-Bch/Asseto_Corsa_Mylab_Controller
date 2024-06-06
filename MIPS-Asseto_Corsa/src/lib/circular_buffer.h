/*
 * circular_buffer.h
 *
 *  Created on: Jun 2, 2024
 *      Author: joachim
 */

#ifndef LIB_CIRCULAR_BUFFER_H_
#define LIB_CIRCULAR_BUFFER_H_

#include <stdint.h>

// Huge ugly macro that dynamically will create a circular buffer with the type and size specified
// Name is for the prefixes of the functions, so that you can create multiple buffers with different names
#define DEFINE_CIRCULAR_BUFFER(type, name, size)         						\
    typedef struct {                                    						\
        uint32_t head;                                   						\
        uint32_t tail;                                   						\
        type buffer[size];                                    					\
    } name##_circular_buffer;                           						\
                                                       							\
    void name##_circular_buffer_init(name##_circular_buffer* buff) { 			\
        buff->head = 0;                                  						\
        buff->tail = 0;                                  						\
    }                                                   						\
																				\
    void name##_circular_buffer_put(name##_circular_buffer* buff, type value) { \
        buff->buffer[buff->head] = value;                						\
        buff->head = (buff->head + 1) % size;      						\
    }                                                   						\
                                                        						\
    type name##_circular_buffer_get(name##_circular_buffer* buff) { 			\
        type value = buff->buffer[buff->tail];           						\
        buff->tail = (buff->tail + 1) % size;      						\
        return value;                                    						\
    }                                                   						\
                                                        						\
    void name##_circular_buffer_clear(name##_circular_buffer* buff) { 			\
        buff->head = 0;                                  						\
        buff->tail = 0;                                  						\
    }                                                   						\

#endif /* LIB_CIRCULAR_BUFFER_H_ */
