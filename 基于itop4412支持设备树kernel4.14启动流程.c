/*
������itop4412 ֧���豸��Linux4.14�汾Դ��������������̡���uboot������Linux-Ubootanalys�У�

�ں�ӳ�񱻼��ص��ڴ沢��ÿ���Ȩ֮���ں��������̿�ʼ��ͨ�����ں�ӳ����ѹ����ʽ�洢��������һ������ִ�е��ںˡ���ˣ��ں˽׶ε���Ҫ�������Խ�ѹ�ں�ӳ��
 
�ں˱�������vmliunx��ͨ����������ѹ�����õ�zImage��С�ںˣ�С��512KB����bzImage�����ںˣ�����512KB���������ǵ�ͷ��Ƕ�н�ѹ������

ͨ��linux/arch/arm/boot/compressedĿ¼�µ�MakefileѰ�ҵ�vmlinux�ļ������ӽű���vmlinux.lds�������в���ϵͳ������ں���
*/
$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) $(obj)/piggy.$(suffix_y).o \  
        $(addprefix $(obj)/, $(OBJS)) $(lib1funcs) $(ashldi3) \  
        $(bswapsdi2) FORCE  
    @$(check_for_multiple_zreladdr)  
    $(call if_changed,ld)  
    @$(check_for_bad_syms) 
    
//vmlinux.lds(linux/arch/arm/kernel/vmlinux.lds)���ӽű���ͷ����
OUTPUT_ARCH(arm)
ENTRY(stext)
jiffies = jiffies_64;
SECTIONS
{
 /DISCARD/ : {
  *(.ARM.exidx.exit.text)
  *(.ARM.extab.exit.text)

//�õ��ں���ں���Ϊ stext��linux/arch/arm/kernel/head.S��
//�ں����
ENTRY(stext)  
    ��  
    ��  
    ��  
    bl  __lookup_processor_type @ r5=procinfo r9=cpuid                             //�������Ƿ�֧��  
    movs    r10, r5             @ invalid processor (r5=0)?  
 THUMB( it  eq )        @ force fixup-able long branch encoding  
    beq __error_p           @ yes, error 'p'                           //��֧�����ӡ������Ϣ  
  
          ��  
    ��  
    ��  
    bl  __create_page_tables                                                       //����ҳ��  
  
    /* 
     * The following calls CPU specific code in a position independent 
     * manner.  See arch/arm/mm/proc-*.S for details.  r10 = base of 
     * xxx_proc_info structure selected by __lookup_processor_type 
     * above.  On return, the CPU will be ready for the MMU to be 
     * turned on, and r0 will hold the CPU control register value. 
     */  
    ldr r13, =__mmap_switched       @ address to jump to after                 //����MMUʹ�ܺ���ת��ַ  
                        @ mmu has been enabled  
    adr lr, BSYM(1f)            @ return (PIC) address  
    mov r8, r4              @ set TTBR1 to swapper_pg_dir  
 ARM(   add pc, r10, #PROCINFO_INITFUNC )  
 THUMB( add r12, r10, #PROCINFO_INITFUNC    )  
 THUMB( mov pc, r12             )  
1:  b   __enable_mmu                                                                           //ʹ��MMU����ת��__mmap_switched  

//���ұ�ǩ__mmap_switched����λ�ã�/linux/arch/arm/kernel/head-common.S
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
    //�����豸��Ϣ���豸�������������洢��ַ
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
//�ں˳�ʼ���׶�
/*��start_kernel������ʼ���ں˽���C���Բ��֣�����ں˵Ĵ󲿷ֳ�ʼ��������
��������λ�ã�/linux/init/Main.c
start_kernel�漰������ʼ��������ֻ������Ҫ�ĳ�ʼ��������
*/
asmlinkage void __init start_kernel(void)  
{  
    ����                                                                              //�����ж�  
    smp_setup_processor_id();                                                         //smp��أ���������CPU��  
    ����  
    local_irq_disable();                                                                   //�رյ�ǰCPU�ж�  
    early_boot_irqs_disabled = true;  
/* 
 * Interrupts are still disabled. Do necessary setups, then 
 * enable them 
 */  
    boot_cpu_init();  
    page_address_init();                                                            //��ʼ��ҳ��ַ  
    pr_notice("%s", linux_banner);                                                   //��ʾ�ں˰汾��Ϣ  
    setup_arch(&command_line);  
		/*���ұ�ǩ��/arch/arm/kernel
			->
			->unflatten_device_tree
			һ�������豸������device node����
			��u-boot�����ں˵�ʱ�򣬻Ὣ�豸���������ڴ��е�������ʼ��ַ������ڼ�
			����r2�У����ݸ�Linux�ںˣ�Ȼ��Linux�ں��ں���unflatten_device_tree��
			������豸�����񣬲�����ɨ�赽����Ϣ������device node���ɵ�����ȫ��
			����of_rootָ������ĸ��ڵ㣬�豸����ÿ���ڵ㶼����һ��struct device_node��֮��Ӧ��
		*/
    mm_init_owner(&init_mm, &init_task);  
    mm_init_cpumask(&init_mm);  
    setup_command_line(command_line);  
    setup_nr_cpu_ids();  
    setup_per_cpu_areas();  
    smp_prepare_boot_cpu(); /* arch-specific boot-cpu hooks */  
  
    build_all_zonelists(NULL, NULL);  
    page_alloc_init();                                                                            //ҳ�ڴ������ʼ��  
  
    pr_notice("Kernel command line: %s\n", boot_command_line);                                     //��ӡ�ں����������в���  
    parse_early_param();  
    parse_args("Booting kernel", static_command_line, __start___param,  
           __stop___param - __start___param,  
           -1, -1, &unknown_bootoption);  
  
    ����  
    /* 
     * Set up the scheduler prior starting any interrupts (such as the 
     * timer interrupt). Full topology setup happens at smp_init() 
     * time - but meanwhile we still have a functioning scheduler. 
     */  
    sched_init();                                                                                    //���̵�������ʼ��  
    /* 
     * Disable preemption - early bootup scheduling is extremely 
     * fragile until we cpu_idle() for the first time. 
     */  
    preempt_disable();                                                                                    //��ֹ�ں���ռ  
    if (WARN(!irqs_disabled(), "Interrupts were enabled *very* early, fixing it\n"))  
        local_irq_disable();                                                                      //���ر�CPU�ж�  
      
      
          /*������ʼ������ ����֪��*/  
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
    local_irq_enable();                                                                            //�����жϿ���ʹ����  
  
    kmem_cache_init_late();  
  
    /* 
     * HACK ALERT! This is early. We're enabling the console before 
     * we've done PCI setups etc, and console_init() must be aware of 
     * this. But we do want output early, in case something goes wrong. 
     */  
    console_init();                                                                            //��ʼ������̨������ʹ��printk��  
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
    fork_init(totalram_pages);                                                             //��ʼ��fork  
    proc_caches_init();  
    buffer_init();  
    key_init();  
    security_init();  
    dbg_late_init();  
    vfs_caches_init(totalram_pages);                                                      //�����ļ�ϵͳ��ʼ��  
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
//����������rest_init()����
/*����Ҫʹ��������kernel_init���̣������к�����ʼ��*/  
static noinline void __init_refok rest_init(void)  
{  
    int pid;  
  
    rcu_scheduler_starting();  
    /* 
     * We need to spawn init first so that it obtains pid 1, however 
     * the init task will end up wanting to create kthreads, which, if 
     * we schedule it before we create kthreadd, will OOPS. 
     */  
      
    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);                             //����kernel_init����  
      
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
    //cpu_idle������ϵͳ����ʱ�������͵�����ʹ�úͼ����ȵĲ����Ŀ�ת�������������˲��ٷ��أ����๤����kernel_init���̴�����  
    cpu_startup_entry(CPUHP_ONLINE);  
}  
/*kernel_init����������豸��������ĳ�ʼ����������init_post���������û�����
�����鼮���ܵ��ں��������̻��ھ����2.6�汾��kernel_init�����������init_post
����ר�Ÿ���_init���̵��������ְ汾�Ѿ������ϵ���һ��
*/
static int __ref kernel_init(void *unused)  
{  
    int ret;  
  
    kernel_init_freeable();                 //�ú��������smp����  ������ʼ�� �����ڴ��ʼ���ȹ���  
    /*	-> do_basic_setup
    			->do_initcalls
    			��������device node����������ע��platform_device
    			��do_initcalls�����У�kernel������ִ�и���initcall����������������У������ 
    			��exynos_dt_machine_init��
    			static void __init exynos_dt_machine_init(void)
					{
					    ......
					 
					    of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
					}
					 ��of_platform_populate�л����of_platform_bus_create ---> of_platform_device_create_pdata��
					 ���platform_device�Ĵ�����ע�ᡣ��ôLinuxϵͳ����ô֪����Щdevice nodeҪע��Ϊplatform_device��
					 ��Щ������i2c_client����Щ������spi_device����֪������û��ע�⵽����of_platform_populate��ʱ�����������һ������
					 of_default_bus_match_table�����Ķ������£�
					 const struct of_device_id of_default_bus_match_table[] = {
			    { .compatible = "simple-bus", },
			    { .compatible = "simple-mfd", },
					#ifdef CONFIG_ARM_AMBA
			    { .compatible = "arm,amba-bus", },
					#endif /* CONFIG_ARM_AMBA */
			    {} /* Empty terminated list */
			    };
			    /*
			    �������˼�����ĳ��device node��compatible���Ե�ֵ������of_default_bus_match_table�е�����һ��Ԫ�ص�compatible��
			    ֵmatch�����Ƕ���compatible���Ե�ֵ��arm,primecell�Ľڵ���Щ���⣬���ǵ�������ģ�����ô���device node��child 
			    device node��device_node��child��Ա����ָ��������device node���ӽڵ㣬Ҳ��һ�������Ծɻᱻע��Ϊplatform_device��
			    
					 of_platform_populate��
				
				   1: int of_platform_populate(struct device_node *root,
				   2:             const struct of_device_id *matches,
				   3:             const struct of_dev_auxdata *lookup,
				   4:             struct device *parent)
				   5: {
				   6:     struct device_node *child;
				   7:     int rc = 0;
				   8:  
				   9:     root = root ? of_node_get(root) : of_find_node_by_path("/");  // �ҵ�root device node
				  10:     if (!root)
				  11:         return -EINVAL;
				  12:  
				  13:     for_each_child_of_node(root, child) { // ����root device node��child device node
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
							
							of_platform_bus_create ��
				
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
				  14:     if (strict && (!of_get_property(bus, "compatible", NULL))) { // �������԰�chosen��aliases��memory��û��compatible���ԵĽڵ��ų�����
				  15:         pr_debug("%s() - skipping %s, no compatible prop\n",
				  16:              __func__, bus->full_name);
				  17:         return 0;
				  18:     }
				  19:  
				  20:     auxdata = of_dev_lookup(lookup, bus);  // tiny4412��lookup���ݵ���NULL
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
				  35:     dev = of_platform_device_create_pdata(bus, bus_id, platform_data, parent); // ����device node���� platform_device��ע��
				  36:     if (!dev || !of_match_node(matches, bus)) // �ж��ǲ�����Ҫ�����������device node�µ�child device node
				  37:         return 0;
				  38:  
				  39:     for_each_child_of_node(bus, child) { // �������device node�µ�child device node����child device nodeҲע��Ϊplatform_device
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
    free_initmem();                         //��ʼ��β��������ڴ���������  
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
                                                          *Ѱ��init����������һ�Ž���_init <span style="color:#FF0000;">(��һ���û��ռ����)</span>*/  
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
���ˣ��ں˳�ʼ���Ѿ��ӽ�β�������еĳ�ʼ���������Ѿ����ã����free_initmem�������������ڴ��__init_begin��__init_end֮������ݡ�
���ں˱����������г�ʼ�����ں��������Լ��ĵ�һ���û��ռ�Ӧ�ó���_init�����ǵ��õĵ�һ��ʹ�ñ�׼C�����ĳ�������̱��ʱ��Ϊ1.
_init���������������Ľ��̣���ʹϵͳ����������õ�״̬��
*/
/*
������Ҫ�Ǹ���һ���ܽᣬ�ο��ܶ��ţ�����ϣ��ش˱�ʾ��л��
���Ĳο����ϣ�
https://blog.csdn.net/cc243494926/article/details/62247071
https://www.cnblogs.com/pengdonglin137/p/5248114.html
https://blog.csdn.net/richard_liujh/article/details/46758073
https://www.aliyun.com/jiaocheng/164283.html
https://blog.csdn.net/huang20083200056/article/details/77816713
*/




















