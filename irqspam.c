#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>
#include <linux/time.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Online Labs - Cloud Team");
MODULE_DESCRIPTION("a simple module to spam soft IRQs");

#define SOFTIRQ_BATCH	10000

static atomic_t nb_softirqs;
static struct task_struct *spamming_thread;
static struct timespec started_at;

void irqspam_handler(unsigned long id)
{
  atomic_inc(&nb_softirqs);
}

DECLARE_TASKLET(irqspam, irqspam_handler, 0);

static int irqspam_spamming_thread(void * data)
{
  int i = 0;

  while (!kthread_should_stop()) {
    tasklet_schedule(&irqspam);
    ++i;
    if (i % SOFTIRQ_BATCH == 0) {
      schedule();
    }
  }
  return 0;
}

static int __init irqspam_init(void)
{
  printk(KERN_INFO "irqspam.init\n");
  spamming_thread = kthread_create(&irqspam_spamming_thread, NULL, "spamirq");
  if (IS_ERR(spamming_thread)) {
    printk(KERN_ALERT "kthread_create error\n");
    return 1;
  }
  kthread_bind(spamming_thread, 0);
  getnstimeofday(&started_at);
  wake_up_process(spamming_thread);
  return 0;
}

static void __exit irqspam_cleanup(void)
{
  struct timespec ended_at;
  unsigned long diff_s;
  unsigned long diff_ms;
  unsigned long throughput;
  int n;

  kthread_stop(spamming_thread);
  tasklet_kill(&irqspam);
  getnstimeofday(&ended_at);
  n = atomic_read(&nb_softirqs);
  diff_s = ended_at.tv_sec - started_at.tv_sec;
  diff_ms = diff_s * 1000 + (ended_at.tv_nsec - started_at.tv_nsec) / 1000000;
  throughput = (n * 1000) / diff_ms;
  printk(KERN_INFO "irqspam.had %d interrupts in ~%ld seconds (%ld/sec)\n", n, diff_s, throughput);
  printk(KERN_INFO "irqspam.cleanup\n");
}

module_init(irqspam_init);
module_exit(irqspam_cleanup);
