# DECAF
***
### 应关注的文件
***
+ callbacktests.c
+ DECAF_types.h
+ DECAF_main.h
+ DECAF_callback.h
+ DECAF_callback_common.h
+ DECAF_target.h
+ hookapi.h
+ monitor.h
+ qdict.h
+ qemu-common.h 
+ qobject.h
+ vmi_callback.h
+ vmi_c_wrapper.h
+ vmi_callback.h
+ utils/Output.h

### callbacktests.c
***
	//init plugin
	plugin_interface_t* init_plugin(void)
	{
		//init mon_cmds
		callbacktests_interface.mon_cmds = callbacktests_term_cmds;
		//init cleanup
		callbacktests_interface.plugin_cleanup = &callbacktests_cleanup;
		callbacktest_init();
		return (&callbacktests_interface);
	}

### DECAF_types.h
***
	typedef uintptr_t DECAF_Handle;//uintptr_t hold any pointer  
	#define DECAF_NULL_HANDLE ((uintptr_t)NULL)
### DECAF_main.h
***
1.interface struct
	
	typedef struct _plugin_interface{
	const mon_cmd_t *mom_cmds;//array of moniter commands
	const mon_cmd_t *info_cmds;//array of informational commands
	void (*plugin_cleanup)(void);
	void (*after_loadvm)(const char *param);
	union
	{
		uint32_t monitored_cr3;
		uint32_t monitored_pgd;
	};
	}plugin_interface_t;  
	
2.plugin load

	void do_load_plugin_internal(Monitor *mon, const char *plugin_path);
	int do_load_plugin(Monitor *mon, const QDict *qdict, QObject **ret_data);
	int do_unload_plugin(Monitor *mon, const QDict *qdict, QObject **ret_data);

3.DECAF init
	
	void DECAF_init(void)
	{
		DECAF_callback_init();
		tainting_init();
		DECAF_virtdev_init();
		register_savevm(NULL,"DECAF",0,1,DECAF_save,DECAF_load,NULL);
		DECAF_vm_compress_init();
		function_map_init();
		init_hookapi();
	}

	

### DECAF_callback.h
***

### DECAF_callback_common.h
***
	ocb_t:optimized callback type
	DECAF_callback_type_t:DECAF callback type
### DECAF_target.h
***

### hookapi.h
***

### monitor.h
***
	typedef struct mon_cmd_t
	{
		const char *name;
		const char *args_type;
		const char *params;
		const char *help;
		void (*user_print)(Monitor *mon, const QObject *data);
		union
		{
			void (*info)(Monitor *mon);
			void (*cmd)(Monitor *mon, const QDict *qdict);
			int  (*cmd_new)(Monitor *mon, const QDict *params, QObject **ret_data);
			int  (*cmd_async)(Monitor *mon, const QDict *params, MonitorCompletion *cb, void *opaque);
		}mhandler;
		bool qapi;
		int flags;
	}mon_cmd_t;
	
### qdict.h
***
	typedef struct QDictEntry
	{
		char *key;
		QObject *value;
		QLIST_ENTRY(QDictEntry) next;
	}QDictEntry;
	typedef struct QDict
	{
		QObject_HEAD;
		size_t size;
		QLIST_HEAD(,QDictEntry) table[QDICT_BUCKET_MAX];
	}QDict;
	
### qemu-common.h
***
	//null struct Monitor
	struct Monitor;
	typedef struct Monitor Monitor;
	
### qobject.h
***
	typedef enum qtype_code
	//qemu object model
	typedef struct QType
	{
		qtype_code code;
		void (*destroy)(struct QObject *);
	}QType;
	typedef struct QObject
	{
		const QType *type;
		size_t refcnt;
	}QObject;
### vmi_callback.h
***

