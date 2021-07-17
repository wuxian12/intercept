/* pid extension for PHP */

#ifndef PHP_PID_H
# define PHP_PID_H

extern zend_module_entry pid_module_entry;
# define phpext_pid_ptr &pid_module_entry

# define PHP_PID_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_PID)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_PID_H */
