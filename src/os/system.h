
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 * @brief Hardware spinlock with red led notification
 * @note uses `HAL_Delay` directly to decouple from successful RTOS instantiation
 *
 */
void system_spinlock(void);

/**
 * @brief System operating system health indicator (HID)
 *
 */
void system_health_indicator(void);

/**
 * @brief Operating system startup entry
 *
 */
void system_boot(void);

#endif // __SYSTEM_H__
