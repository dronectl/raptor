
#ifndef __MAIN_H__
#define __MAIN_H__

/**
 * @brief Global error handling macro
 *
 * @param ecode error code (HAL)
 */
#define EHANDLE(status) error_handler(status, __FILE__, __LINE__)

void error_handler(int ecode, const char *file, int line);

#endif // __MAIN_H__