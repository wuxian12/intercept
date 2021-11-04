/* performance extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_performance.h"
#include "main/SAPI.h"
#include "include/performance_util.h"

static int le_deliverer;
static FILE *fp;

ZEND_DECLARE_MODULE_GLOBALS(performance)

static void (*ori_execute_ex)(zend_execute_data *execute_data);
static void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);
ZEND_API void pid_execute_ex(zend_execute_data *execute_data);
ZEND_API void pid_execute_internal(zend_execute_data *execute_data, zval *return_value);
ZEND_API void pid_execute_core(int internal, zend_execute_data *execute_data, zval *return_value);
static char *get_function_name(zend_execute_data *execute_data);
static char *build_deliverer_cli_argv();
static void frame_build(p_frame_t *frame, zend_execute_data *ex);
static void frame_destroy(p_frame_t *frame);

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void test1() */
PHP_FUNCTION(test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "performance");
}
/* }}} */

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


static void create_dir()  /* {{{ */
{
    int stat;
    stat = access("/tmp/performance/", F_OK);
    if(stat == -1){
        mkdir("/tmp/performance/", 0755);
    }
    stat = access("/tmp/performance/config/", F_OK);
    if(stat == -1){
        mkdir("/tmp/performance/config/", 0755);
    }
    stat = access("/tmp/performance/log/", F_OK);
    if(stat == -1){
        mkdir("/tmp/performance/log/", 0755);
    }
    
    
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(performance)
{

    ori_execute_ex = zend_execute_ex;
    zend_execute_ex = pid_execute_ex;
    ori_execute_internal = zend_execute_internal;
    zend_execute_internal = pid_execute_internal;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(performance)
{
	zend_execute_ex = ori_execute_ex;
    zend_execute_internal = ori_execute_internal;
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(performance)
{
#if defined(ZTS) && defined(COMPILE_DL_PERFORMANCE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	create_dir();

   
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long ts = tv.tv_sec * 1000000 + tv.tv_usec;

    char logfile[128] = {0};
    le_deliverer++;
    sprintf(logfile, "/tmp/performance/log/%d-%ld.log", le_deliverer, ts);

    fp = fopen(logfile, "a+");
    if(fp == NULL){
        printf("file cannot open \n");
    }
    chmod(logfile, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

    if (strcmp(sapi_module.name, "cli") == 0) {

        char *cli_argv_string = build_deliverer_cli_argv();
        fprintf(fp, "---\n%d-%ld  %s\n", le_deliverer, ts, cli_argv_string);
        efree(cli_argv_string);

    } else {
        fprintf(fp, "---\n%d-%ld %s %s %s %s\n", getpid(), ts, sapi_module.name, SG(request_info).request_method,
                SG(request_info).request_uri, SG(request_info).query_string);
    }

    return SUCCESS;
	
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(performance)
{
	fprintf(fp, "---end---\n");
    fclose(fp);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(performance)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "performance support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ deliverer_functions[]
 *
 */
const zend_function_entry per_functions[] = {
	PHP_FE_END
};
/* }}} */

/* {{{ performance_module_entry */
zend_module_entry performance_module_entry = {
	STANDARD_MODULE_HEADER,
	"performance",					/* Extension name */
	per_functions,					/* zend_function_entry */
	PHP_MINIT(performance),							/* PHP_MINIT - Module initialization */
	PHP_MSHUTDOWN(performance),							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(performance),			/* PHP_RINIT - Request initialization */
	PHP_RSHUTDOWN(performance),							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(performance),			/* PHP_MINFO - Module info */
	PHP_PERFORMANCE_VERSION,		/* Version */
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

    int hit = 0;
    p_frame_t frame;
    zval retval;
    char *method_or_function_name = NULL;
	if(execute_data->func->common.function_name != NULL){
		method_or_function_name = get_function_name(execute_data);
		hit = p_intercept_hit(method_or_function_name);
        if(hit == 1){
            frame_build(&frame, execute_data);
            smart_string_appends(&(frame.function), method_or_function_name);
            frame.entry_time = mo_time_usec();

            if (!internal && execute_data->return_value == NULL) {
                ZVAL_UNDEF(&retval);
                execute_data->return_value = &retval;
            }

        }
	}
    smart_str res = {0};
    if (internal) {
        if (ori_execute_internal) {
            ori_execute_internal(execute_data, return_value);
        } else {
            execute_internal(execute_data, return_value);
        }
    } else {
        ori_execute_ex(execute_data);
    }
    frame.ori_ret = NULL;
    char *tmp1;
    if (return_value) { /* internal */
            if(hit == 1){
                php_var_export_ex(return_value, 2, &res);
                tmp1 = malloc(ZSTR_LEN(res.s)+1);
                memset(tmp1, 0, ZSTR_LEN(res.s)+1);
                memcpy(tmp1, ZSTR_VAL(res.s), ZSTR_LEN(res.s));
                frame.ori_ret = tmp1;  
            } 
    } else if (execute_data->return_value) { /* user function */
            if(hit == 1){
                php_var_export_ex(execute_data->return_value, 2, &res);
                tmp1 = malloc(ZSTR_LEN(res.s)+1);
                memset(tmp1, 0, ZSTR_LEN(res.s)+1);
                memcpy(tmp1, ZSTR_VAL(res.s), ZSTR_LEN(res.s));
                frame.ori_ret = tmp1;
            }
        
    }
    
    if(hit == 1){
        frame.exit_time = mo_time_usec();
        char *value = convert_args_to_string(&frame);
        size_t exec_time = frame.exit_time - frame.entry_time;
        float memory = zend_memory_usage(0) / (1024 * 1024);
        float peak_momory = zend_memory_peak_usage(0) / (1024 * 1024);
        fprintf(fp,"执行函数名:%s===函数的参数:%s====函数返回结果%s===函数开始时间:%ld===函数结束时间:%ld函数执行时间:%d微妙函数执行时内存:%0.2fM函数执行时峰值内存:%0.2fM\n", method_or_function_name,value,frame.ori_ret,frame.entry_time,frame.exit_time,exec_time,memory,peak_momory);
        //printf("执行函数名%s===函数的参数%s===函数开始时间%ld===函数结束时间%ld====函数返回结果%s\n", method_or_function_name,value,frame.entry_time,frame.exit_time,ZSTR_VAL(frame.ori_ret->s));
        //printf("执行函数的返回结果 %s ......\n", "test");
        
        efree(method_or_function_name);
        efree(value);
        free(tmp1);
        frame_destroy(&frame);
    }


}

static char *get_function_name(zend_execute_data *execute_data) /* {{{ */
{
    char *method_or_function_name = NULL;
    zend_function    *fbc                     = execute_data->func;
    zend_string      *function_name           = fbc->common.function_name;
    zend_class_entry *scope                   = fbc->common.scope;
    char             *class_name              = NULL;

    if (scope != NULL) {
        class_name = ZSTR_VAL(scope->name);
        int len    = strlen(class_name) + strlen("::") + ZSTR_LEN(function_name) + 1;
        method_or_function_name = (char *) emalloc(len);
        memset(method_or_function_name, 0, len);
        strcat(method_or_function_name, class_name);
        strcat(method_or_function_name, "::");
        strcat(method_or_function_name, ZSTR_VAL(function_name));
    } else {
        int len = ZSTR_LEN(function_name) + 1;
        method_or_function_name = (char *) emalloc(len);
        memset(method_or_function_name, 0, len);
        strcat(method_or_function_name, ZSTR_VAL(function_name));
    }


    return method_or_function_name;
}
/* }}} */

static char *build_deliverer_cli_argv()  /* {{{ */
{
    char *cli_argv_string;

    int len = 0;
    int i;

    if (SG(request_info).argc > 0) {

        for (i = 0; i < SG(request_info).argc; i++) {
            len += strlen(SG(request_info).argv[i]);
        }

        len += SG(request_info).argc;

        cli_argv_string = (char *) emalloc(len + SG(request_info).argc);

        memset(cli_argv_string, 0, len);

        for (i = 0; i < SG(request_info).argc; i++) {
            strcat(cli_argv_string, SG(request_info).argv[i]);
            strcat(cli_argv_string, " ");
        }
    }

    return cli_argv_string;
}
/* }}} */

/* {{{ Build molten frame */
static void frame_build(p_frame_t *frame, zend_execute_data *ex)
{
    zval **args = NULL;
    /* init */
    memset(frame, 0, sizeof(p_frame_t));
    frame->arg_count = ZEND_CALL_NUM_ARGS(ex);

    int i;
    if (frame->arg_count) {
        i = 0;
        zval *p = ZEND_CALL_ARG(ex, 1);
        if (ex->func->type == ZEND_USER_FUNCTION) {
            uint32_t first_extra_arg = ex->func->op_array.num_args;

            if (first_extra_arg && frame->arg_count > first_extra_arg) {
                p = ZEND_CALL_VAR_NUM(ex, ex->func->op_array.last_var + ex->func->op_array.T);
            }
        }
        frame->ori_args = p;
    }

}
/* }}} */

/* {{{ Destroy frame */
static void frame_destroy(p_frame_t *frame)
{
    smart_string_free(&frame->function);
}
/* }}} */

#ifdef COMPILE_DL_PERFORMANCE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(performance)
#endif
