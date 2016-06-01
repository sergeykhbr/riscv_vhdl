#include <nanokernel.h>
#include <nano_private.h>
#include <misc/__assert.h>
#include <board.h>

IsrEntryType isr_table[CONFIG_NUM_IRQS];
IsrEntryType isr_demux_table[CONFIG_NUM_IRQS];


#ifdef _WIN32
void _IsrExit(void) {
    if ((_nanokernel.current->flags & TASK) == TASK) {
        if (_nanokernel.fiber) {
            _nanokernel.current->intlock = _arch_irq_lock_state();
            _nanokernel.current = _nanokernel.fiber;
            _nanokernel.fiber = _nanokernel.fiber->link;

            irq_unlock(_nanokernel.current->intlock);
            LIBH_swap_preemptive((uint64_t)_nanokernel.current);
        }
    }
}
#else
extern void _IsrExit(void);
#endif

void _IsrWrapper(void *arg) {
    uint32_t isr_idx = READ32(&__IRQCTRL->irq_cause_idx);
    ((NANO_EOI_GET_FUNC)isr_demux_table[isr_idx].handler)(arg);
    _IsrExit();
}

unsigned int _arch_irq_lock(void) {
    unsigned int ret = READ32(&__IRQCTRL->irq_lock);
    WRITE32(&__IRQCTRL->irq_lock, 1);
    return ret;
}

void _arch_irq_unlock(unsigned int key) {
    WRITE32(&__IRQCTRL->irq_lock, 0);
}

unsigned int _arch_irq_lock_state() {
    return READ32(&__IRQCTRL->irq_lock);
}

/**
 *
 * @brief Enable an interrupt line
 *
 * Clear possible pending interrupts on the line, and enable the interrupt
 * line. After this call, the CPU will receive interrupts for the specified
 * <irq>.
 *
 * @return N/A
 */
void _arch_irq_enable(unsigned int irq)
{
	/* before enabling interrupts, ensure that interrupt is cleared */
    uint32_t bit = READ32(&__IRQCTRL->irq_mask);
    bit &= ~(1u << irq);
    WRITE32(&__IRQCTRL->irq_clear, 1u << irq);
    WRITE32(&__IRQCTRL->irq_mask, bit);
}

/**
 *
 * @brief Disable an interrupt line
 *
 * Disable an interrupt line. After this call, the CPU will stop receiving
 * interrupts for the specified <irq>.
 *
 * @return N/A
 */
void _arch_irq_disable(unsigned int irq)
{
    uint32_t bit = READ32(&__IRQCTRL->irq_mask);
    bit |= (1u << irq);
    WRITE32(&__IRQCTRL->irq_mask, bit);
}

/*
 * @internal
 *
 * @brief Replace an interrupt handler by another
 *
 * An interrupt's ISR can be replaced at runtime.
 *
 * @return N/A
 */

void _irq_handler_set(
	unsigned int irq,
	void (*new)(void *arg),
	void *arg
)
{
	int key = irq_lock();

	__ASSERT(irq < CONFIG_NUM_IRQS, "IRQ number too high");
	isr_table[irq].handler = (uint64_t)_IsrWrapper;
	isr_table[irq].arg = (uint64_t)arg;
    isr_demux_table[irq].handler = (uint64_t)new;

	irq_unlock(key);
}

/*
 * @brief Connect an ISR to an interrupt line
 *
 * @a isr is connected to interrupt line @a irq, a number greater than or equal
 * 16. No prior ISR can have been connected on @a irq interrupt line since the
 * system booted.
 *
 * This routine will hang if another ISR was connected for interrupt line @a irq
 * and ASSERT_ON is enabled; if ASSERT_ON is disabled, it will fail silently.
 *
 * @return the interrupt line number
 */
int _arch_irq_connect_dynamic(unsigned int irq, unsigned int priority,
			     void (*routine)(void *arg), void *parameter,
			     uint32_t flags)
{
	ARG_UNUSED(flags);
	_irq_handler_set(irq, routine, parameter);
	return irq;
}
