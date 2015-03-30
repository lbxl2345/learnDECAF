//
//  datatracer.c
//  datatracer
//
//  Created by 刘本熙 on 15/3/15.
//
//

#include <stdio.h>
#include <time.h>

#include "DECAF_types.h"
#include "DECAF_main.h"
#include "DECAF_target.h"
#include "hookapi.h"
#include "DECAF_callback.h"
#include "utils/Output.h"
#include "function_map.h"
#include "vmi_callback.h"
#include "vmi_c_wrapper.h"
#include "vmi.h"

#define STACK_SIZE 10000
char modname_t[512];
char funcname_t[512];
static plugin_interface_t datatracer_interface;
static DECAF_Handle create_process_handle = DECAF_NULL_HANDLE;
static DECAF_Handle remove_process_handle = DECAF_NULL_HANDLE;
static DECAF_Handle write_taint_handle = DECAF_NULL_HANDLE;
static DECAF_Handle read_taint_handle = DECAF_NULL_HANDLE;
static DECAF_Handle block_end_handle = DECAF_NULL_HANDLE;
static DECAF_Handle key_stroke_handle = DECAF_NULL_HANDLE;
static time_t ks_time;
static uint32_t source_pid;
static uint32_t source_cr3;
static uint32_t pc_stack[STACK_SIZE];
static uint32_t fun_stack[STACK_SIZE];
static uint32_t cr3_stack[STACK_SIZE];
static uint32_t ptr_stack = 0;
static char source_name[512];
FILE *tracelog = NULL;
void do_create_proc(VMI_Callback_Params *param)
{
    char procname[64];
    uint32_t pid;
    if(param == NULL){
        return;
    }
    //通过param获取当前的进程pid以及进程名
    VMI_find_process_by_cr3_c(param->cp.cr3, procname, 64, &pid);
    if (pid == (uint32_t)(-1)) {
        return;
    }
    if (strcmp(source_name, procname) == 0) {
        source_pid = pid;
        source_cr3 = param->cp.cr3;
    }
    DECAF_printf("Process name:%s\t pid:%d\t cr3:%u was created\n", procname, pid, param->cp.cr3);
}
void handle_call(DECAF_Callback_Params *param)
{
    if (ptr_stack == STACK_SIZE) {
        memcpy(pc_stack,&pc_stack[STACK_SIZE/10],STACK_SIZE-STACK_SIZE/10);
        memcpy(cr3_stack, &cr3_stack[STACK_SIZE/10], STACK_SIZE-STACK_SIZE/10);
        memcpy(fun_stack, &fun_stack, STACK_SIZE-STACK_SIZE/10);
        return;
    }
    CPUState *env = param->be.env;
    //调用call时，下一条pc即所调用代码块的入口
    target_ulong next_pc = param->be.next_pc;
    target_ulong cr3 = DECAF_getPGD(env);
    //在当前进程空间中获取这个call指令所调用函数的函数名和模块名
    if (funcmap_get_name_c(next_pc, cr3, modname_t, funcname_t)) {
    //ESP：栈指针寄存器，其内存放着一个指针，该指针永远指向帧栈的栈顶，这里也就是指向call调用完成后所执行的下一条指令
        DECAF_read_mem(env, env->regs[R_ESP], 4, &pc_stack[ptr_stack]);
        fun_stack[ptr_stack] = next_pc;
        cr3_stack[ptr_stack] = cr3;
        ptr_stack++;
    }
}
void handle_ret(DECAF_Callback_Params *param)
{
    if (!ptr_stack) {
        return;
    }
    if (param->be.next_pc == pc_stack[ptr_stack]) {
        ptr_stack--;
    }
}
void do_remove_proc(VMI_Callback_Params *param)
{
    process proc;
}
void do_write_taint_mem(DECAF_Callback_Params *param)
{
    uint32_t eip = DECAF_getPC(cpu_single_env);
    uint32_t cr3 = DECAF_getPGD(cpu_single_env);
    tmodinfo_t mod_info_t;
    time_t wt_t;
    vmi_
    if (ptr_stack) {
        if (cr3 == cr3_stack[ptr_stack-1]) {
            funcmap_get_name_c(pc_stack[pc_stack-1], cr3, modname_t, funcname_t);
        }
        else{
            memset(modname_t, 0, 512);
            memset(funcname_t, 0, 512);
        }
    }
    else{
        memset(modname_t, 0, 512);
        memset(funcname_t, 0, 512);
    }
}
void do_read_taint_mem(DECAF_Callback_Params *param)
{
    uint32_t eip = DECAF_getPC(cpu_single_env);
    uint32_t cr3 = DECAF_getPGD(cpu_single_env);
    tmodinfo_t mod_info_t;
    time_t wr_t;
    if (ptr_stack) {
        if (cr3 == cr3_stack[ptr_stack-1]) {
            funcmap_get_name_c(pc_stack[pc_stack-1], cr3, modname_t, funcname_t);
        }
        else{
            memset(modname_t, 0, 512);
            memset(funcname_t, 0, 512);
        }
    }
    else{
        memset(modname_t, 0, 512);
        memset(funcname_t, 0, 512);
    }
}
void do_block_end(DECAF_Callback_Params *param)
{
    uint8_t ins_buf[2];
    int call_flag = 0, ret_flag = 0;
    int b;
    DECAF_read_mem(param->be.env, param->be.cur_pc, 2 * sizeof(char),ins_buf);
    switch (ins_buf[0]) {
        case 0x9a:
        case 0x38:
            call_flag = 1;
            break;
        case 0xff:
            b = (ins_buf[1]>>3) & 7;
            if (b == 2 || b == 3)
                call_flag = 1;
            break;
        case 0xc2:
        case 0xc3:
        case 0xca:
        case 0xcb:
            ret_flag = 1;
            break;
        default:
            break;
    }
    if (call_flag) {
        
    }
}

void do_analysis(Monitor *mon, const QDict *qdict)
{
    
}
void tracing_send_key(DECAF_Callback_Params *params)
{
    int keycode = params->ks.keycode;
    uint32_t *taint_mark = params->ks.taint_mark;
    *taint_mark = taint_key_enabled;
    taint_key_enabled = 0;
    printf("taint keystroke %d \n", keycode);
}
void do_taint_sendkey(Monitor *mon, const QDict *qdict)
{
    if (qdict_haskey(qdict, "key")) {
        if (!key_stroke_handle) {
            key_stroke_handle = DECAF_register_callback(DECAF_KEYSTROKE_CB, tracing_send_key, NULL);
        }
        do_send_key(qdict_get_str(qdict, "key"));
    }
    else
        ;
}
void do_enable_data_trace(Monitor *mon, const QDict *qdict)
{
    DECAF_printf("******enable data trace******\n");
    //delete here
    if ((qdict != NULL)&&(qdict_haskey(qdict,"procname"))) {
        strncpy(source_name, qdict_get_str(qdict,"procname"), 512);
    }
    else
    {
        DECAF_printf("A program name was not specified.\n");
        return;
    }
    const char *tracefile_t = qdict_get_str(qdict, "tracefile");
    tracelog = fopen(tracefile_t, "w");
    if (!tracelog) {
        DECAF_printf("the %s can not be open or created.\n", tracefile_t);
	return;
    }
    //注册进程开始和结束的回调
    if (create_process_handle != DECAF_NULL_HANDLE) {
        create_process_handle = VMI_register_callback(VMI_CREATEPROC_CB, do_create_proc, NULL);
    }
    if (remove_process_handle != DECAF_NULL_HANDLE) {
        remove_process_handle = VMI_register_callback(VMI_REMOVEPROC_CB, do_remove_proc, NULL);
    }
    //注册污点内存读写的回调
    if(read_taint_handle != DECAF_NULL_HANDLE){
        read_taint_handle = DECAF_register_callback(DECAF_READ_TAINTMEM_CB, do_write_taint_mem, NULL);
    }
    if(write_taint_handle != DECAF_NULL_HANDLE){
        write_taint_handle = DECAF_register_callback(DECAF_WRITE_TAINTMEM_CB, do_read_taint_mem, NULL);
    }
    //注册基本块结束的回调
    if(block_end_handle != DECAF_NULL_HANDLE){
        block_end_handle = DECAF_registerOptimizedBlockEndCallback(do_block_end, NULL, INV_ADDR, INV_ADDR);
    }
}
void do_disable_data_trace(Monitor *mon, const QDict *qdict)
{
    
}
static void do_select_source_process(Monitor *mon, const QDict *qdict)
{
    
}
static void datatracer_init(void)
{
    DECAF_printf("******datatracer initializing******\n");
    //输入源程序相关信息初始化
    source_name[0] = '\0';
    source_cr3 = 0;
    source_pid = (uint32_t)(-1);
}
static void datatracer_cleanup(void)
{
    //注销所有的回调函数
    if (create_process_handle != DECAF_NULL_HANDLE) {
        VMI_unregister_callback(VMI_CREATEPROC_CB, create_process_handle);
        create_process_handle = DECAF_NULL_HANDLE;
    }
    if(remove_process_handle != DECAF_NULL_HANDLE){
        VMI_unregister_callback(VMI_REMOVEMODULE_CB, remove_process_handle);
        remove_process_handle = DECAF_NULL_HANDLE;
    }
    if (read_taint_handle != DECAF_NULL_HANDLE) {
        DECAF_unregister_callback(DECAF_READ_TAINTMEM_CB,read_taint_handle);
        read_taint_handle = DECAF_NULL_HANDLE;
    }
    if (write_taint_handle != DECAF_NULL_HANDLE) {
        DECAF_unregister_callback(DECAF_WRITE_TAINTMEM_CB, write_taint_handle);
        write_taint_handle = DECAF_NULL_HANDLE;
    }
    if (block_end_handle != DECAF_NULL_HANDLE) {
        DECAF_unregisterOptimizedBlockEndCallback(block_end_handle);
        block_end_handle = DECAF_NULL_HANDLE;
    }
    DECAF_printf("datatracer terminate\n");
}

static mon_cmd_t datatracer_term_cmds[] = {
#include "plugin_cmds.h"
		{ NULL, NULL, }, };

plugin_interface_t* init_plugin(void)
{
    datatracer_interface.mon_cmds = datatracer_term_cmds;
    datatracer_interface.plugin_cleanup = &datatracer_cleanup;
    //初始化插件
    datatracer_init();
    return &(datatracer_interface);
}

