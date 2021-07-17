/* pid extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <string.h>
#include <stdio.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_pid.h"
#include "pid_arginfo.h"
#include "Zend/zend_smart_str.h"

static void (*ori_execute_ex)(zend_execute_data *execute_data);
static void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);
ZEND_API void pid_execute_ex(zend_execute_data *execute_data);
ZEND_API void pid_execute_internal(zend_execute_data *execute_data, zval *return_value);
ZEND_API void pid_execute_core(int internal, zend_execute_data *execute_data, zval *return_value);
static inline zend_function *obtain_zend_function(zend_execute_data *ex);


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* PHP_INI */
PHP_INI_BEGIN()
    PHP_INI_ENTRY("pid.enable", "1", PHP_INI_ALL, NULL)
    PHP_INI_ENTRY("pid.log_path",  "/tmp/logs/",  PHP_INI_ALL, NULL)
    PHP_INI_ENTRY("pid.function",   "",   PHP_INI_ALL,  NULL)
PHP_INI_END()

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "pid");
}
/* }}} */

ZEND_BEGIN_ARG_INFO(args_test2, 0)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

/* {{{ string test2( [ string $var ] ) */
PHP_FUNCTION(test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(pid)
{
#if defined(ZTS) && defined(COMPILE_DL_PID)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(pid)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pid support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ PHP MINIT Function */
PHP_MINIT_FUNCTION(pid)
{
    REGISTER_INI_ENTRIES();
    ori_execute_ex = zend_execute_ex;
    zend_execute_ex = pid_execute_ex;
    ori_execute_internal = zend_execute_internal;
    zend_execute_internal = pid_execute_internal;

    return SUCCESS;
}
/* }}} */

/* {{{ PHP MSHUTDOWN Function */
PHP_MSHUTDOWN_FUNCTION(pid)
{
    UNREGISTER_INI_ENTRIES();
    zend_execute_ex = ori_execute_ex;
    zend_execute_internal = ori_execute_internal;

    return SUCCESS;
}
/* }}} */

const zend_function_entry pid_functions[] = {
	PHP_FE(test2,args_test2)
	PHP_FE_END
};


/* {{{ pid_module_entry */
zend_module_entry pid_module_entry = {
	STANDARD_MODULE_HEADER,
	"pid",					/* Extension name */
	pid_functions,					/* zend_function_entry */
	PHP_MINIT(pid),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(pid),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(pid),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(pid),			/* PHP_MINFO - Module info */
	PHP_PID_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */


ZEND_API void pid_execute_ex(zend_execute_data *execute_data)
{
    pid_execute_core(0, execute_data, NULL);
}

ZEND_API void pid_execute_internal(zend_execute_data *execute_data, zval *return_value)
{
    pid_execute_core(1, execute_data, return_value);
}

ZEND_API void pid_execute_core(int internal, zend_execute_data *execute_data, zval *return_value)
{
    char *logs_dir = {0};
    char *re_func = {0};
    char *file_log = "trace.log";
    //从php.ini文件读取拦截函数和日志目录
    logs_dir = INI_STR("pid.log_path");
    re_func = INI_STR("pid.function");
    char *name = (char *) malloc(strlen(logs_dir) + strlen(file_log));
    sprintf(name, "%s%s", logs_dir, file_log);
    FILE *fp;
    if((fp = fopen(name, "a+")) == NULL){
        printf("file cannot open \n");
    }

    zend_function *zf = obtain_zend_function(execute_data);
    int flag = 0;
    char str_1[512] = {0};
    char *token;
    const char s[2] = ",";
    if (zf->common.function_name != NULL){
    	sprintf(str_1, ZSTR_VAL(zf->common.function_name));
        /* 获取第一个子字符串 */
        token = strtok(re_func, s);
        /* 继续获取其他的子字符串 */
        while( token != NULL ) {
            printf( "%s\n", token );
            if(strcmp(str_1, token) == 0){
                flag = 1;
                fputs("\n执行函数的名字:", fp);
                fputs( str_1, fp);
                break;
            }else{
                flag = 0;
            }
            token = strtok(NULL, s);
        }
    	
    	
    }
    int arg_count = 0;
    arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    smart_str res = {0};
    smart_str buf = {0};
    if (arg_count) {
        zval *p = ZEND_CALL_ARG(execute_data, 1);
        if (execute_data->func->type == ZEND_USER_FUNCTION) {
            uint32_t first_extra_arg = execute_data->func->op_array.num_args;

            if (first_extra_arg && arg_count > first_extra_arg) {
                p = ZEND_CALL_VAR_NUM(execute_data, execute_data->func->op_array.last_var + execute_data->func->op_array.T);
            }
        }
        int i;
        for (i = 0; i < arg_count; i++) {
        	php_var_export_ex(p, 2, &buf);

        	p++;
        }
        
        if(flag == 1){
            fputs(";执行函数的参数:", fp);
            fputs( ZSTR_VAL(buf.s), fp);
        	//printf("执行函数的参数 %s ......\n", ZSTR_VAL(buf.s));
        }
        
    }
    if (internal) {
        if (ori_execute_internal) {
            ori_execute_internal(execute_data, return_value);
        } else {
            execute_internal(execute_data, return_value);
        }
    } else {
        ori_execute_ex(execute_data);
    }

    if (return_value) { /* internal */
    		php_var_export_ex(return_value, 2, &res);
        	if(flag == 1){
                fputs(";执行函数的返回结果:", fp);
                fputs( ZSTR_VAL(res.s), fp);
	        	//printf("内部函数返回的结果 %s  === \n", ZSTR_VAL(res.s));
	        } 
    } else if (execute_data->return_value) { /* user function */
    	php_var_export_ex(execute_data->return_value, 2, &res);
        	if(flag == 1){
                fputs(";执行函数的返回结果:", fp);
                fputs( ZSTR_VAL(res.s), fp);
	        	//printf("自定义函数返回结果 %s  ===\n", ZSTR_VAL(res.s));
	        }
        
    }
    fclose(fp);

}

/* {{{ Obtain zend function */
static inline zend_function *obtain_zend_function(zend_execute_data *ex)
{
    return ex->func;

}
/* }}} */


#ifdef COMPILE_DL_PID
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(pid)
#endif
