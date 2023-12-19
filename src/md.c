#include <linux/delay.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#define PROC_FS_NAME "processAnalyzer"
#define LOG_PREFIX "[PROCESS ANALYZER]"
#define TMP_STRLEN 1024
#define LOGS_COUNT 5
#define DELAY_MS 10 * 1000
#define LOG_SIZE 524288

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Inspirate789");

static struct proc_dir_entry *procFile;
static struct task_struct *kthread;
static char log[LOG_SIZE] = { 0 };

static int checkOverflow(char *fString, char *sString, int maxSize)
{
    int sumLen = strlen(fString) + strlen(sString);

    if (sumLen >= maxSize)
    {
        printk(KERN_ERR "%s not enough space in log (%d needed but %d available)\n", LOG_PREFIX, sumLen, maxSize);
        return -ENOMEM;
    }

    return 0;
}

static int printTasks(void *arg)
{
    struct task_struct *task;
    size_t currentPrint = 1;
    char currentString[TMP_STRLEN];

    while (currentPrint <= LOGS_COUNT)
    {
        task = &init_task;
        memset(currentString, 0, TMP_STRLEN);
        snprintf(currentString, TMP_STRLEN, "~~~~~~~~~~~~~~~~~~~~~~~~~~%lu TIME~~~~~~~~~~~~~~~~~~~~~~~~~~\n", currentPrint);
        checkOverflow(currentString, log, LOG_SIZE);
        strcat(log, currentString);

        for_each_process(task)
        {
            if (task->pid == 51122)
            {
                memset(currentString, 0, TMP_STRLEN);
                snprintf(currentString, TMP_STRLEN,
                        "pid: %-5d, comm: %15s\nprio: %3d, static_prio: %3d, normal_prio (with "
                        "scheduler policy): %3d, rt_priority: %3d\n"
                        "run_delay: %10lld, task_is_realtime: %d\nutime: %10lld (ticks), stime: %15lld (ticks)\n"
                        "sched_statistics:\nwait_max: %10llu, wait_count: %10llu, wait_sum: %10llu\n"
                        "iowait_count: %10llu, iowait_sum: %10llu\n"
                        "sleep_max: %10llu, sum_sleep_runtime: %10llu\n"
                        "block_max: %10llu, sum_block_runtime: %10llu, exec_max: %10llu\n\n",
                        task->pid, task->comm, task->prio, task->static_prio, task->normal_prio, task->rt_priority,
                        task->sched_info.run_delay, task_is_realtime(task), task->utime, task->stime,
                        task->stats.wait_max, task->stats.wait_count, task->stats.wait_sum,
                        task->stats.iowait_count, task->stats.iowait_sum,
                        task->stats.sleep_max, task->stats.sum_sleep_runtime,
                        task->stats.block_max, task->stats.sum_block_runtime, task->stats.exec_max
                );
                checkOverflow(currentString, log, LOG_SIZE);
                strcat(log, currentString);
            }
        }

        currentPrint++;
        mdelay(DELAY_MS);
    }

    return 0;
}

static int analyzerOpen(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "%s open called\n", LOG_PREFIX);
    try_module_get(THIS_MODULE);
    return 0;
}

static ssize_t analyzerRead(struct file *filep, char __user *buf, size_t count, loff_t *offp)
{
    ssize_t logLen = strlen(log);
    printk(KERN_INFO "%s read called\n", LOG_PREFIX);

    if (copy_to_user(buf, log, logLen))
    {
        printk(KERN_ERR "%s copy_to_user error\n", LOG_PREFIX);

        return -EFAULT;
    }

    memset(log, 0, LOG_SIZE);

    return logLen;
}

static ssize_t analyzerWrite(struct file *file, const char __user *buf, size_t len, loff_t *offp)
{
    printk(KERN_INFO "%s write called\n", LOG_PREFIX);
    return 0;
}

static int analyzerRelease(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "%s release called\n", LOG_PREFIX);
    module_put(THIS_MODULE);
    return 0;
}

static struct proc_ops fops = {
    proc_read:    analyzerRead,
    proc_write:   analyzerWrite,
    proc_open:    analyzerOpen,
    proc_release: analyzerRelease
};

static int __init md_init(void)
{
    if (!(procFile = proc_create(PROC_FS_NAME, 0666, NULL, &fops)))
    {
        printk(KERN_ERR "%s proc_create error\n", LOG_PREFIX);
        return -EFAULT;
    }

    kthread = kthread_run(printTasks, NULL, "taskPrintThread");
    if (IS_ERR(kthread))
    {
        printk(KERN_ERR "%s kthread_run error\n", LOG_PREFIX);
        return -EFAULT;
    }

    printk(KERN_INFO "%s module loaded\n", LOG_PREFIX);

    return 0;
}

static void __exit md_exit(void)
{
    remove_proc_entry(PROC_FS_NAME, NULL);
    printk(KERN_INFO "%s module unloaded\n", LOG_PREFIX);
}

module_init(md_init);
module_exit(md_exit);
