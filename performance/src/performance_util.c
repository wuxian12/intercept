
#include "include/performance_util.h"

/* {{{ convert args to string */
char *convert_args_to_string(p_frame_t *frame)
{
    int i = 0;
    int arg_len = 0;
#define ARGS_MAX_LEN 128
#define ARGS_ELLIPSIS "..."
#define ARGS_ELLIPSIS_LEN (sizeof("...") - 1)
#define ARGS_REAL_LEN (ARGS_MAX_LEN - ARGS_ELLIPSIS_LEN - 1)

    char *string = emalloc(ARGS_MAX_LEN);
    int real_len = 0;
    int stop = 0;
    memset(string, 0, ARGS_MAX_LEN);
    smart_str res = {0};
    for (; i < frame->arg_count; i++) {
        memset(&res, 0, sizeof(smart_str));
        php_var_export_ex(((zval *)(frame->ori_args) + i), 2, &res);
        real_len = ZSTR_LEN(res.s) + 1;
        if ((arg_len + real_len) >= ARGS_REAL_LEN)  {
            real_len = ARGS_REAL_LEN - arg_len;
            string = strncat(string, ZSTR_VAL(res.s), real_len - 1);
            stop = 1;  
            break;
        } else {
            string = strncat(string, ZSTR_VAL(res.s), real_len - 1);
            string = strncat(string, ",", 1);
            arg_len += real_len;
        }
    }
    
    if (stop == 1) {
        string = strncat(string, ARGS_ELLIPSIS, ARGS_ELLIPSIS_LEN);
        string[ARGS_MAX_LEN - 1] = '\0';
    }

    return string;
}
/* }}} */

/* {{{ pt intercept hit function */
int p_intercept_hit(char *function_name)
{
    char *hit[19] = {
        //"Swoole\\Coroutine\\Socket::__construct"
        //"Swoole\\Coroutine\\Socket::connect",
        //"Swoole\\Coroutine\\Socket::send",
        "Swoole\\Coroutine\\Http\\Client::__construct",
        "Swoole\\Coroutine\\Http\\Client::get",
        "fwrite",
        "fread",
        "PDO@exec",
        "PDO@query",
        "PDO@commit",
        "PDO@prepare",
        "PDOStatement::execute",
        "file_get_contents",
        "file_put_contents",
        "Redis::connect",
        "Redis::get",
        "Redis::setEx",
        "Redis::set",
        "Redis::setNx",
        "Redis::delete",
        "Redis::hGet",
        "Redis::hDel"
    };
    int flag = 0;
    int i;
    for(i = 0; i< 19; i++){
        if(strcmp(hit[i],function_name) == 0){
            flag = 1;
            break;
        }
    }
    return flag;
    
}
/* }}} */

