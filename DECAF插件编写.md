#DECAF插件编写
##插件加载
***
DECAF插件的加载在DECAF_main.c内实现
分别是  
do_load_plugin_internal()  
do_load_plugin()  
qemu所提供的插件借口位于vl.c中  
DECAF插件的本质是一个一个动态链接库，它在加载的时候要通过以下的mon_cmd_t来完成  
 
	{
	.name       = "load_plugin",
	.args_type  = "filename:F",
	.params     = "filename",
	.help       = "Load a DECAF plugin",
	.mhandler.cmd_new = do_load_plugin,
	},
也就是说，加载插件的过程是由DECAF和QEMU共同实现的  
我们在使用插件的时候，只用输入相应的命令:load_plugin plugin_path  
其实也就是动态加载了一个库，但是加载的时候DECAF会默认执行初始化函数
init_plugin,因为load_plugin的时候实际上通过符号的方式绑定了初始化函数 
 	
 	plugin_interface_t *(*init_plugin)(void);
	plugin_handle = dlopen(plugin_path, RTLD_NOW);
	init_plugin = dlsym(plugin_handle,"init_plugin");
因此要在init_plugin中实现插件的初始化，这个函数要返回一个plugin_interface_t*指针，这个指针也就相当于给了DECAF_main.c中的
	
	plugin_interface_t *decaf_plugin = NULL;
	decaf_plugin = init_plugin();
此处就相当于执行了init_plugin()中的所有代码。在callbacktests.c这个例子中,init_plugin()调用了callbacktests_init()这个函数进行初始化。在这个函数中，插件完成了回调事件的注册，并且进行了某些参数的初始化。这里关注一下回调注册的方法:此处是注册了进程开始和结束的回调
	DECAF中回调可以分成两个种类，VMI回调和DECAF回调。
	VMI和DECAF分别自己的注册/注销函数。例如VMI:
	
	DECAF_Handle VMI_register_callback(
                VMI_callback_type_t cb_type,
                VMI_callback_func_t cb_func,
                int *cb_cond
                )
	{
	  if ((cb_type > VMI_LAST_CB) || (cb_type < 0)) {
	    return (DECAF_NULL_HANDLE);	
	  return (SimpleCallback_register(&VMI_callbacks[cb_type], (SimpleCallback_func_t)cb_func, cb_cond));
	}
		　
		processbegin_handle = VMI_register_callback(VMI_CREATEPROC_CB, &callbacktests_loadmainmodule_callback, NULL);
	 	 removeproc_handle = VMI_register_callback(VMI_REMOVEPROC_CB, &callbacktests_removeproc_callback, NULL);	
VMI回调注册的相关说明,也即将回调函数和对应的回调类型注册。

 	 DECAF_register_callback(VMI_callback_type_t cb_type,VMI_callback_func_t cb_func, int *cb_cond)
 	 
 	 在callbacktests中，策略在于利用VMI回调发现进程的创建，在创建的回调中实现DECAF回调的注册。
 	 
 	 
 	 