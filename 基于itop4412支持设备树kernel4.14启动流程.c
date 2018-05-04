/*
本文以itop4412 支持设备树Linux4.14版本源码分析其启动流程。（uboot分析在Linux-Ubootanalys中）

内核映像被加载到内存并获得控制权之后，内核启动流程开始。通常，内核映像以压缩形式存储，并不是一个可以执行的内核。因此，内核阶段的首要工作是自解压内核映像。
 
内核编译生成vmliunx后，通常会对其进行压缩，得到zImage（小内核，小于512KB）或bzImage（大内核，大于512KB）。在它们的头部嵌有解压缩程序。

通过linux/arch/arm/boot/compressed目录下的Makefile寻找到vmlinux文件的链接脚本（vmlinux.lds），从中查找系统启动入口函数
*/
$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) $(obj)/piggy.$(suffix_y).o \  
        $(addprefix $(obj)/, $(OBJS)) $(lib1funcs) $(ashldi3) \  
        $(bswapsdi2) FORCE  
    @$(check_for_multiple_zreladdr)  
    $(call if_changed,ld)  
    @$(check_for_bad_syms) 
    
//vmlinux.lds(linux/arch/arm/kernel/vmlinux.lds)链接脚本开头内容
OUTPUT_ARCH(arm)
ENTRY(stext)
jiffies = jiffies_64;
SECTIONS
{
 /DISCARD/ : {
  *(.ARM.exidx.exit.text)
  *(.ARM.extab.exit.text)

//得到内核入口函数为 stext（linux/arch/arm/kernel/head.S）
//内核入口
ENTRY(stext)  
    。  
    。  
    。  
    bl  __lookup_processor_type @ r5=procinfo r9=cpuid                             //处理器是否支持  
    movs    r10, r5             @ invalid processor (r5=0)?  
 THUMB( it  eq )        @ force fixup-able long branch encoding  
    beq __error_p           @ yes, error 'p'                           //不支持则打印错误信息  
  
          。  
    。  
    。  
    bl  __create_page_tables                                                       //创建页表  
  
    /* 
     * The following calls CPU specific code in a position independent 
     * manner.  See arch/arm/mm/proc-*.S for details.  r10 = base of 
     * xxx_proc_info structure selected by __lookup_processor_type 
     * above.  On return, the CPU will be ready for the MMU to be 
     * turned on, and r0 will hold the CPU control register value. 
     */  
    ldr r13, =__mmap_switched       @ address to jump to after                 //保存MMU使能后跳转地址  
                        @ mmu has been enabled  
    adr lr, BSYM(1f)            @ return (PIC) address  
    mov r8, r4              @ set TTBR1 to swapper_pg_dir  
 ARM(   add pc, r10, #PROCINFO_INITFUNC )  
 THUMB( add r12, r10, #PROCINFO_INITFUNC    )  
 THUMB( mov pc, r12             )  
1:  b   __enable_mmu                                                                           //使能MMU后跳转到__mmap_switched  

//查找标签__mmap_switched所在位置：/linux/arch/arm/kernel/head-common.S
__mmap_switched:
	
 /* 
     * The following fragment of code is executed with the MMU on in MMU mode, 
     * and uses absolute addresses; this is not position independent. 
     * 
     *  r0  = cp#15 control register 
     *  r1  = machine ID 
     *  r2  = atags/dtb pointer 
     *  r9  = processor ID 
     */  
    //保存设备信息、设备树及启动参数存储地址
	adr	r3, __mmap_switched_data

	ldmia	r3!, {r4, r5, r6, r7}
	cmp	r4, r5				@ Copy data segment if needed
1:	cmpne	r5, r6
	ldrne	fp, [r4], #4
	strne	fp, [r5], #4
	bne	1b

	mov	fp, #0				@ Clear BSS (and zero fp)
1:	cmp	r6, r7
	strcc	fp, [r6],#4
	bcc	1b

 ARM(	ldmia	r3, {r4, r5, r6, r7, sp})
 THUMB(	ldmia	r3, {r4, r5, r6, r7}	)
 THUMB(	ldr	sp, [r3, #16]		)
	str	r9, [r4]			@ Save processor ID
	str	r1, [r5]			@ Save machine type
	str	r2, [r6]			@ Save atags pointer
	cmp	r7, #0
	strne	r0, [r7]			@ Save control register values
	b	start_kernel
//内核初始化阶段
/*从start_kernel函数开始，内核进入C语言部分，完成内核的大部分初始化工作。
函数所在位置：/linux/init/Main.c
start_kernel涉及大量初始化工作，只例举重要的初始化工作。
*/
asmlinkage void __init start_kernel(void)  
{  
    ……                                                                              //类型判断  
    smp_setup_processor_id();                                                         //smp相关，返回启动CPU号  
    ……  
    local_irq_disable();                                                                   //关闭当前CPU中断  
    early_boot_irqs_disabled = true;  
/* 
 * Interrupts are still disabled. Do necessary setups, then 
 * enable them 
 */  
    boot_cpu_init();  
    page_address_init();                                                            //初始化页地址  
    pr_notice("%s", linux_banner);                                                   //显示内核版本信息  
    setup_arch(&command_line);  
		/*查找标签在/arch/arm/kernel
			->
			->unflatten_device_tree
			一、根据设备树创建device node链表
			在u-boot引导内核的时候，会将设备树在物理内存中的物理起始地址（存放在寄
			存器r2中）传递给Linux内核，然后Linux内核在函数unflatten_device_tree中
			会解析设备树镜像，并利用扫描到的信息创建由device node构成的链表，全局
			变量of_root指向链表的根节点，设备树的每个节点都会有一个struct device_node与之对应。
		*/
    mm_init_owner(&init_mm, &init_task);  
    mm_init_cpumask(&init_mm);  
    setup_command_line(command_line);  
    setup_nr_cpu_ids();  
    setup_per_cpu_areas();  
    smp_prepare_boot_cpu(); /* arch-specific boot-cpu hooks */  
  
    build_all_zonelists(NULL, NULL);  
    page_alloc_init();                                                                            //页内存申请初始化  
  
    pr_notice("Kernel command line: %s\n", boot_command_line);                                     //打印内核启动命令行参数  
    parse_early_param();  
    parse_args("Booting kernel", static_command_line, __start___param,  
           __stop___param - __start___param,  
           -1, -1, &unknown_bootoption);  
  
    ……  
    /* 
     * Set up the scheduler prior starting any interrupts (such as the 
     * timer interrupt). Full topology setup happens at smp_init() 
     * time - but meanwhile we still have a functioning scheduler. 
     */  
    sched_init();                                                                                    //进程调度器初始化  
    /* 
     * Disable preemption - early bootup scheduling is extremely 
     * fragile until we cpu_idle() for the first time. 
     */  
    preempt_disable();                                                                                    //禁止内核抢占  
    if (WARN(!irqs_disabled(), "Interrupts were enabled *very* early, fixing it\n"))  
        local_irq_disable();                                                                      //检查关闭CPU中断  
      
      
          /*大量初始化内容 见名知意*/  
    idr_init_cache();  
    rcu_init();  
    tick_nohz_init();  
    context_tracking_init();  
    radix_tree_init();  
    /* init some links before init_ISA_irqs() */  
    early_irq_init();  
    init_IRQ();  
    tick_init();  
    init_timers();  
    hrtimers_init();  
    softirq_init();  
    timekeeping_init();  
    time_init();  
    sched_clock_postinit();  
    perf_event_init();  
    profile_init();  
    call_function_init();  
    WARN(!irqs_disabled(), "Interrupts were enabled early\n");  
    early_boot_irqs_disabled = false;  
    local_irq_enable();                                                                            //本地中断可以使用了  
  
    kmem_cache_init_late();  
  
    /* 
     * HACK ALERT! This is early. We're enabling the console before 
     * we've done PCI setups etc, and console_init() must be aware of 
     * this. But we do want output early, in case something goes wrong. 
     */  
    console_init();                                                                            //初始化控制台，可以使用printk了  
    if (panic_later)  
        panic("Too many boot %s vars at `%s'", panic_later,  
              panic_param);  
  
    lockdep_info();  
  
    /* 
     * Need to run this when irqs are enabled, because it wants 
     * to self-test [hard/soft]-irqs on/off lock inversion bugs 
     * too: 
     */  
    locking_selftest();  
  
#ifdef CONFIG_BLK_DEV_INITRD  
    if (initrd_start && !initrd_below_start_ok &&  
        page_to_pfn(virt_to_page((void *)initrd_start)) < min_low_pfn) {  
        pr_crit("initrd overwritten (0x%08lx < 0x%08lx) - disabling it.\n",  
            page_to_pfn(virt_to_page((void *)initrd_start)),  
            min_low_pfn);  
        initrd_start = 0;  
    }  
#endif  
    page_cgroup_init();  
    debug_objects_mem_init();  
    kmemleak_init();  
    setup_per_cpu_pageset();  
    numa_policy_init();  
    if (late_time_init)  
        late_time_init();  
    sched_clock_init();  
    calibrate_delay();  
    pidmap_init();  
    anon_vma_init();  
    acpi_early_init();  
#ifdef CONFIG_X86  
    if (efi_enabled(EFI_RUNTIME_SERVICES))  
        efi_enter_virtual_mode();  
#endif  
#ifdef CONFIG_X86_ESPFIX64  
    /* Should be run before the first non-init thread is created */  
    init_espfix_bsp();  
#endif  
    thread_info_cache_init();  
    cred_init();  
    fork_init(totalram_pages);                                                             //初始化fork  
    proc_caches_init();  
    buffer_init();  
    key_init();  
    security_init();  
    dbg_late_init();  
    vfs_caches_init(totalram_pages);                                                      //虚拟文件系统初始化  
    signals_init();  
    /* rootfs populating might need page-writeback */  
    page_writeback_init();  
#ifdef CONFIG_PROC_FS  
    proc_root_init();  
#endif  
    cgroup_init();  
    cpuset_init();  
    taskstats_init_early();  
    delayacct_init();  
  
    check_bugs();  
  
    sfi_init_late();  
  
    if (efi_enabled(EFI_RUNTIME_SERVICES)) {  
        efi_late_init();  
        efi_free_boot_services();  
    }  
  
    ftrace_init();  
  
    /* Do the rest non-__init'ed, we're now alive */  
    rest_init();  
}  
//函数最后调用rest_init()函数
/*最重要使命：创建kernel_init进程，并进行后续初始化*/  
static noinline void __init_refok rest_init(void)  
{  
    int pid;  
  
    rcu_scheduler_starting();  
    /* 
     * We need to spawn init first so that it obtains pid 1, however 
     * the init task will end up wanting to create kthreads, which, if 
     * we schedule it before we create kthreadd, will OOPS. 
     */  
      
    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);                             //创建kernel_init进程  
      
    numa_default_policy();  
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);  
    rcu_read_lock();  
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);  
    rcu_read_unlock();  
    complete(&kthreadd_done);  
  
    /* 
     * The boot idle thread must execute schedule() 
     * at least once to get things moving: 
     */  
    init_idle_bootup_task(current);  
    schedule_preempt_disabled();  
    /* Call into cpu_idle with preempt disabled */  
    //cpu_idle就是在系统闲置时用来降低电力的使用和减少热的产生的空转函数，函数至此不再返回，其余工作从kernel_init进程处发起  
    cpu_startup_entry(CPUHP_ONLINE);  
}  
/*kernel_init函数将完成设备驱动程序的初始化，并调用init_post函数启动用户进程
部分书籍介绍的内核启动流程基于经典的2.6版本，kernel_init函数还会调用init_post
函数专门负责_init进程的启动，现版本已经被整合到了一起。
*/
static int __ref kernel_init(void *unused)  
{  
    int ret;  
  
    kernel_init_freeable();                 //该函数中完成smp开启  驱动初始化 共享内存初始化等工作  
    /*	-> do_basic_setup
    			->do_initcalls
    			二、遍历device node链表，创建并注册platform_device
    			在do_initcalls函数中，kernel会依次执行各个initcall函数，在这个过程中，会调用 
    			到exynos_dt_machine_init：
    			static void __init exynos_dt_machine_init(void)
					{
					    ......
					 
					    of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
					}
					 在of_platform_populate中会调用of_platform_bus_create ---> of_platform_device_create_pdata，
					 完成platform_device的创建和注册。那么Linux系统是怎么知道哪些device node要注册为platform_device，
					 哪些是用于i2c_client，哪些是用于spi_device？不知道你有没有注意到调用of_platform_populate的时候给它传递了一个参数
					 of_default_bus_match_table，它的定义如下：
					 const struct of_device_id of_default_bus_match_table[] = {
			    { .compatible = "simple-bus", },
			    { .compatible = "simple-mfd", },
					#ifdef CONFIG_ARM_AMBA
			    { .compatible = "arm,amba-bus", },
					#endif /* CONFIG_ARM_AMBA */
			    {} /* Empty terminated list */
			    };
			    /*
			    是这个意思：如果某个device node的compatible属性的值与数组of_default_bus_match_table中的任意一个元素的compatible的
			    值match（但是对于compatible属性的值是arm,primecell的节点有些特殊，它是单独处理的），那么这个device node的child 
			    device node（device_node的child成员变量指向的是这个device node的子节点，也是一个链表）仍旧会被注册为platform_device。
			    
					 of_platform_populate：
				
				   1: int of_platform_populate(struct device_node *root,
				   2:             const struct of_device_id *matches,
				   3:             const struct of_dev_auxdata *lookup,
				   4:             struct device *parent)
				   5: {
				   6:     struct device_node *child;
				   7:     int rc = 0;
				   8:  
				   9:     root = root ? of_node_get(root) : of_find_node_by_path("/");  // 找到root device node
				  10:     if (!root)
				  11:         return -EINVAL;
				  12:  
				  13:     for_each_child_of_node(root, child) { // 遍历root device node的child device node
				  14:         rc = of_platform_bus_create(child, matches, lookup, parent, true);
				  15:         if (rc) {
				  16:             of_node_put(child);
				  17:             break;
				  18:         }
				  19:     }
				  20:     of_node_set_flag(root, OF_POPULATED_BUS);
				  21:  
				  22:     of_node_put(root);
				  23:     return rc;
				  24: }
							
							of_platform_bus_create ：
				
				   1: static int of_platform_bus_create(struct device_node *bus,
				   2:                   const struct of_device_id *matches,
				   3:                   const struct of_dev_auxdata *lookup,
				   4:                   struct device *parent, bool strict)
				   5: {
				   6:     const struct of_dev_auxdata *auxdata;
				   7:     struct device_node *child;
				   8:     struct platform_device *dev;
				   9:     const char *bus_id = NULL;
				  10:     void *platform_data = NULL;
				  11:     int rc = 0;
				  12:  
				  13:     /* Make sure it has a compatible property *//*
				  14:     if (strict && (!of_get_property(bus, "compatible", NULL))) { // 这样可以把chosen、aliases、memory等没有compatible属性的节点排除在外
				  15:         pr_debug("%s() - skipping %s, no compatible prop\n",
				  16:              __func__, bus->full_name);
				  17:         return 0;
				  18:     }
				  19:  
				  20:     auxdata = of_dev_lookup(lookup, bus);  // tiny4412给lookup传递的是NULL
				  21:     if (auxdata) {
				  22:         bus_id = auxdata->name;
				  23:         platform_data = auxdata->platform_data;
				  24:     }
				  25:  
				  26:     if (of_device_is_compatible(bus, "arm,primecell")) {
				  27:         /*
				  28:          * Don't return an error here to keep compatibility with older
				  29:          * device tree files.
				  30:          *//*
				  31:         of_amba_device_create(bus, bus_id, platform_data, parent);
				  32:         return 0;
				  33:     }
				  34:  
				  35:     dev = of_platform_device_create_pdata(bus, bus_id, platform_data, parent); // 根据device node创建 platform_device并注册
				  36:     if (!dev || !of_match_node(matches, bus)) // 判断是不是需要继续遍历这个device node下的child device node
				  37:         return 0;
				  38:  
				  39:     for_each_child_of_node(bus, child) { // 遍历这个device node下的child device node，将child device node也注册为platform_device
				  40:         pr_debug("   create child: %s\n", child->full_name);
				  41:         rc = of_platform_bus_create(child, matches, lookup, &dev->dev, strict);
				  42:         if (rc) {
				  43:             of_node_put(child);
				  44:             break;
				  45:         }
				  46:     }
				  47:     of_node_set_flag(bus, OF_POPULATED_BUS);
				  48:     return rc;
				  49: }
    */
    /* need to finish all async __init code before freeing the memory */  
    async_synchronize_full();  
    free_initmem();                         //初始化尾声，清除内存无用数据  
    mark_rodata_ro();  
    system_state = SYSTEM_RUNNING;  
    numa_default_policy();  
  
    flush_delayed_fput();  
  
    if (ramdisk_execute_command) {  
        ret = run_init_process(ramdisk_execute_command);  
        if (!ret)  
            return 0;  
        pr_err("Failed to execute %s (error %d)\n",  
               ramdisk_execute_command, ret);  
    }  
  
    /* 
     * We try each of these until one succeeds. 
     * 
     * The Bourne shell can be used instead of init if we are 
     * trying to recover a really broken machine. 
                                                          *寻找init函数，创建一号进程_init <span style="color:#FF0000;">(第一个用户空间进程)</span>*/  
    if (execute_command) {  
        ret = run_init_process(execute_command);  
        if (!ret)  
            return 0;  
        pr_err("Failed to execute %s (error %d).  Attempting defaults...\n",  
            execute_command, ret);  
    }  
    if (!try_to_run_init_process("/sbin/init") ||  
        !try_to_run_init_process("/etc/init") ||  
        !try_to_run_init_process("/bin/init") ||  
        !try_to_run_init_process("/bin/sh"))  
        return 0;  
  
    panic("No working init found.  Try passing init= option to kernel. "  
          "See Linux Documentation/init.txt for guidance.");  
}  
/*
到此，内核初始化已经接近尾声，所有的初始化函数都已经调用，因此free_initmem函数可以舍弃内存的__init_begin至__init_end之间的数据。
当内核被引导并进行初始化后，内核启动了自己的第一个用户空间应用程序_init，这是调用的第一个使用标准C库编译的程序，其进程编号时钟为1.
_init负责出发其他必须的进程，以使系统进入整体可用的状态。
*/
/*
此文主要是个人一个总结，参考很多大牛的资料，特此表示感谢！
此文参考资料：
https://blog.csdn.net/cc243494926/article/details/62247071
https://www.cnblogs.com/pengdonglin137/p/5248114.html
https://blog.csdn.net/richard_liujh/article/details/46758073
https://www.aliyun.com/jiaocheng/164283.html
https://blog.csdn.net/huang20083200056/article/details/77816713
*/




















