/* performance extension for PHP */

#ifndef PHP_PERFORMANCE_H
# define PHP_PERFORMANCE_H

extern zend_module_entry performance_module_entry;
# define phpext_performance_ptr &performance_module_entry

# define PHP_PERFORMANCE_VERSION "0.1.0"

#include "main/SAPI.h"
#include "php7_wrapper.h"
#include "include/performance_util.h"


  	
ZEND_BEGIN_MODULE_GLOBALS(performance)
	HashTable                     intercept;
ZEND_END_MODULE_GLOBALS(performance)


#define PERFORMANCE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(performance, v)

# if defined(ZTS) && defined(COMPILE_DL_PERFORMANCE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif



#endif	/* PHP_PERFORMANCE_H */
