#ifndef M4_UTILS_DEFINE_DEFINE_H_
#define M4_UTILS_DEFINE_DEFINE_H_

typedef enum
{
    E_OK       = 0x00U,  /* Operation successful */
    E_ERROR    = 0x01U,  /* Operation failed */
    E_BUSY     = 0x02U,  /* Resource is busy */
    E_TIMEOUT  = 0x03U   /* Operation timed out */
} Std_ReturnType;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* M4_UTILS_DEFINE_DEFINE_H_ */
